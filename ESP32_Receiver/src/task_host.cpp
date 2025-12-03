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
{  // 测试数据
    sbus::SBUS.unpack.ch0 = 0x00;
    sbus::SBUS.unpack.ch1 = 0x01;
    sbus::SBUS.unpack.ch2 = 0x02;
    sbus::SBUS.unpack.ch3 = 0x03;
    sbus::SBUS.unpack.ch4 = 0x04;
    sbus::SBUS.unpack.ch5 = 0x05;
    sbus::SBUS.unpack.ch6 = 0x06;
    sbus::SBUS.unpack.ch7 = 0x07;
    sbus::SBUS.unpack.ch8 = 0x08;
    sbus::SBUS.unpack.ch9 = 0x09;
    sbus::SBUS.unpack.ch10 = 0x0A;
    sbus::SBUS.unpack.ch11 = 0x0B;
    sbus::SBUS.unpack.ch12 = 0x0C;
    sbus::SBUS.unpack.ch13 = 0x0D;
    sbus::SBUS.unpack.ch14 = 0x0E;
    sbus::SBUS.unpack.ch15 = 0x0F;
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