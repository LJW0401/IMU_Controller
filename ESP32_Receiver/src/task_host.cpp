#include "task_host.hpp"

#include <WiFi.h>
#include <WiFiUdp.h>

#include "protocol_wifi.hpp"
#include "sbus.hpp"

#define MAX_INTERVAL_CONNECTION 100  // ms

#define DEBUG_MODE 1
#define DEBUG_SHOW_TMP 1

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
protocol_wifi::trigger_u trigger_data;

uint32_t task_loop_count = 0;

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

#if DEBUG_MODE && DEBUG_SHOW_TMP
            // 打印接收到的imu r p y 数据
            Serial.printf(
                "IMU Data: can_id=%u, r=%.2f, p=%.2f, y=%.2f\n", imu_tmp.decoded.can_id,
                imu_tmp.decoded.r, imu_tmp.decoded.p, imu_tmp.decoded.y);
#endif

            if (imu_tmp.decoded.can_id = 0x01) {
                head_imu_data = imu_tmp;
                head_tracker.last_update_ms = millis();
            } else if (imu_tmp.decoded.can_id == 0x02) {
                pose_imu_data = imu_tmp;
                pose_tracker.last_update_ms = millis();
            }

        } break;

        case PROTOCOL_WIFI_TYPE_TRIGGER: {
            protocol_wifi::trigger_u trigger_tmp;
            if (len > sizeof(trigger_tmp.raw.data)) {
                len = sizeof(trigger_tmp.raw.data);
            }
            memcpy(&trigger_tmp.raw.data[0], buf, len);

            trigger_data = trigger_tmp;
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
    // SBUS通道设置如下
    // ch0,ch1,ch2 传输头追IMU的rpy数据，给云台进行跟踪。
    // ch3,ch4,ch5 传输姿态IMU的rpy数据，给底盘进行姿态控制【底盘姿态控制选项1】。
    // ch6,ch7,ch8 传输扳机数据，给底盘进行速度控制。
    // ch9         传输底盘和云台的指向偏差角度【底盘姿态控制选项2】。

    if (head_tracker.connected || pose_tracker.connected) {
        sbus::SBUS.unpack.connect_flag = 0x01;  // 标识该控制器为自定义头追控制器
    } else {
        sbus::SBUS.unpack.connect_flag = 0xFF;
    }

    // 头追IMU数据
    sbus::SBUS.unpack.ch0 = float_to_uint(head_imu_data.decoded.r, -180.0f, 180.0f, 11);
    sbus::SBUS.unpack.ch1 = float_to_uint(head_imu_data.decoded.p, -90.0f, 90.0f, 11);
    sbus::SBUS.unpack.ch2 = float_to_uint(head_imu_data.decoded.y, -180.0f, 180.0f, 11);

    // 姿态IMU数据
    sbus::SBUS.unpack.ch3 = float_to_uint(pose_imu_data.decoded.r, -180.0f, 180.0f, 11);
    sbus::SBUS.unpack.ch4 = float_to_uint(pose_imu_data.decoded.p, -90.0f, 90.0f, 11);
    sbus::SBUS.unpack.ch5 = float_to_uint(pose_imu_data.decoded.y, -180.0f, 180.0f, 11);

    // 扳机数据
    sbus::SBUS.unpack.ch6 = float_to_uint(trigger_data.decoded.kp_vx, -1.0f, 1.0f, 11);
    sbus::SBUS.unpack.ch7 = float_to_uint(trigger_data.decoded.kp_vy, -1.0f, 1.0f, 11);
    sbus::SBUS.unpack.ch8 = float_to_uint(trigger_data.decoded.kp_wz, -1.0f, 1.0f, 11);

    // 指向偏差角度
    float yaw_error = head_imu_data.decoded.y - pose_imu_data.decoded.y;
    // 归一化到 [-180, 180]
    if (yaw_error > 180.0f) {
        yaw_error -= 360.0f;
    } else if (yaw_error < -180.0f) {
        yaw_error += 360.0f;
    }
    sbus::SBUS.unpack.ch9 = float_to_uint(yaw_error, -180.0f, 180.0f, 11);
}

static void Loop()
{
    SolveWifiConnection();
    SolveStateControl();
    sbus::SbusSendData();

#if DEBUG_MODE
    if (task_loop_count % 500 == 0) {
        Serial.printf(
            "Head IMU Data: r=%.2f, p=%.2f, y=%.2f\n", head_imu_data.decoded.r,
            head_imu_data.decoded.p, head_imu_data.decoded.y);
        Serial.printf(
            "Pose IMU Data: r=%.2f, p=%.2f, y=%.2f\n", pose_imu_data.decoded.r,
            pose_imu_data.decoded.p, pose_imu_data.decoded.y);
        Serial.printf(
            "Trigger Data: kp_vx=%.2f, kp_vy=%.2f, kp_wz=%.2f\n", trigger_data.decoded.kp_vx,
            trigger_data.decoded.kp_vy, trigger_data.decoded.kp_wz);
    }

    // 打印连接到wifi的从机数量
    if (task_loop_count % 100 == 0) {
        Serial.printf("Connected devices: %u\n", WiFi.softAPgetStationNum());
    }
#endif
}

void HostTask(void * pvParameters)
{
    // setup WiFi
    Setup();

    // task loop
    while (true) {
        Loop();

        task_loop_count++;
        // yield to other tasks
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}
}  // namespace task_host