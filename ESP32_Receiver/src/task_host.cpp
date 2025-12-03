#include "task_host.hpp"

#include <WiFi.h>
#include <WiFiUdp.h>

#include "protocol_wifi.hpp"
#include "sbus.hpp"

#define MAX_INTERVAL_CONNECTION 100  // ms

namespace task_host
{

const char * ssid = "RobotCmd_AP";  // 与 README 中一致
const char * password = "12345678";
const uint16_t listenPort = 3333;

WiFiUDP Udp;

connect_t head_tracker = {.connected = false, .last_update_ms = 0};
connect_t pose_tracker = {.connected = false, .last_update_ms = 0};
bool wifi_connected = false;

protocol_wifi::imu_u head_imu_data;
protocol_wifi::imu_u pose_imu_data;

/**
************************************************************************
* @brief:      	float_to_uint: 浮点数转换为无符号整数函数
* @param[in]:   x_float:	待转换的浮点数
* @param[in]:   x_min:		范围最小值
* @param[in]:   x_max:		范围最大值
* @param[in]:   bits: 		目标无符号整数的位数
* @retval:     	无符号整数结果
* @details:    	将给定的浮点数 x 在指定范围 [x_min, x_max] 内进行线性映射，映射结果为一个指定位数的无符号整数
************************************************************************
**/
int float_to_uint(float x_float, float x_min, float x_max, int bits)
{
    /* Converts a float to an unsigned int, given range and number of bits */
    float span = x_max - x_min;
    float offset = x_min;
    return (int)((x_float - offset) * ((float)((1 << bits) - 1)) / span);
}

//---------------------------------------------------------------------------
// Setup
//---------------------------------------------------------------------------

static void WifiSetup()
{
    WiFi.softAP(ssid, password);
    Serial.printf("AP started, IP=%s\n", WiFi.softAPIP().toString().c_str());

    Udp.begin(listenPort);
    Serial.printf("UDP listening on port %u\n", listenPort);
}

static void Setup()
{
    WifiSetup();
    sbus::SetupSbus(12, 13);
}

//---------------------------------------------------------------------------
// Loop
//---------------------------------------------------------------------------

static void DecodeWifiData(char * buf, int len)
{
    switch (buf[0]) {
        case PROTOCOL_WIFI_TYPE_IMU: {
            protocol_wifi::imu_u imu_tmp;
            if (len > sizeof(imu_tmp.raw.data)) {
                len = sizeof(imu_tmp.raw.data);
            }
            memcpy(&imu_tmp.raw.data[0], buf, len);

            if (imu_tmp.decoded.can_id = 0x01) {
                head_imu_data = imu_tmp;
                head_tracker.last_update_ms = millis();
            } else if (imu_tmp.decoded.can_id == 0x02) {
                pose_imu_data = imu_tmp;
                pose_tracker.last_update_ms = millis();
            }

        } break;

        default:
            break;
    }
}

static bool ConnectionCheck()
{
    ulong current_ms = millis();

    // head tracker
    if (current_ms - head_tracker.last_update_ms < MAX_INTERVAL_CONNECTION &&
        current_ms > MAX_INTERVAL_CONNECTION) {
        head_tracker.connected = true;
    } else {
        head_tracker.connected = false;
    }

    // pose tracker
    if (current_ms - pose_tracker.last_update_ms < MAX_INTERVAL_CONNECTION &&
        current_ms > MAX_INTERVAL_CONNECTION) {
        pose_tracker.connected = true;
    } else {
        pose_tracker.connected = false;
    }

    return head_tracker.connected || pose_tracker.connected;
}

static void SolveWifiConnection()
{
    int packetSize = Udp.parsePacket();
    if (packetSize) {
        IPAddress remoteIp = Udp.remoteIP();
        uint16_t remotePort = Udp.remotePort();
        char buf[512];
        int len = Udp.read(buf, sizeof(buf) - 1);
        if (len > 0) {
            buf[len] = 0;
            DecodeWifiData(buf, len);
        }
    }

    bool last_connected = wifi_connected;
    wifi_connected = ConnectionCheck();

    if (wifi_connected != last_connected) {
        Serial.printf(
            "Connection status changed: %s\n", wifi_connected ? "Connected" : "Disconnected");
    }
}

static void SolveStateControl()
{
    sbus::SBUS.unpack.ch15 = 0x01;  // 标识该控制器为自定义头追控制器
    // 头追IMU数据
    sbus::SBUS.unpack.ch0 = float_to_uint(head_imu_data.decoded.r, -180.0f, 180.0f, 11);
    sbus::SBUS.unpack.ch1 = float_to_uint(head_imu_data.decoded.p, -90.0f, 90.0f, 11);
    sbus::SBUS.unpack.ch2 = float_to_uint(head_imu_data.decoded.y, -180.0f, 180.0f, 11);
}

static void Loop()
{
    SolveWifiConnection();
    SolveStateControl();
    sbus::SbusSendData();
}

void HostTask(void * pvParameters)
{
    // setup WiFi
    Setup();

    // task loop
    while (true) {
        Loop();
        // yield to other tasks
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}
}  // namespace task_host