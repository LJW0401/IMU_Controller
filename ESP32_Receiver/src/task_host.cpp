#include "task_host.hpp"

#include <WiFi.h>
#include <WiFiUdp.h>

#include "protocol_wifi.hpp"

#define MAX_INTERVAL_CONNECTION 100  // ms

protocol_wifi::imu_u head_imu_data;
protocol_wifi::imu_u pose_imu_data;

bool wifi_connected = false;

namespace task_host
{

const char * ssid = "RobotCmd_AP";  // 与 README 中一致
const char * password = "12345678";
const uint16_t listenPort = 3333;

WiFiUDP Udp;

connect_t head_tracker = {.connected = false, .last_update_ms = 0};
connect_t pose_tracker = {.connected = false, .last_update_ms = 0};

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

bool ConnectionCheck()
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

void HostTask(void * pvParameters)
{
    // setup WiFi
    WiFi.softAP(ssid, password);
    Serial.printf("AP started, IP=%s\n", WiFi.softAPIP().toString().c_str());

    Udp.begin(listenPort);
    Serial.printf("UDP listening on port %u\n", listenPort);

    // task loop
    while (true) {
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
            Serial.printf("Connection status changed: %s\n", wifi_connected ? "Connected" : "Disconnected");
        }
        
        // yield to other tasks
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}
}  // namespace task_host