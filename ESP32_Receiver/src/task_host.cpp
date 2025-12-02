#include "task_host.hpp"

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>

#include "protocol_wifi.hpp"

protocol_wifi::imu_u imu_data;

namespace task_host
{

const char * ssid = "RobotCmd_AP";  // 与 README 中一致
const char * password = "12345678";
const uint16_t listenPort = 3333;

WiFiUDP Udp;

static void DecodeWifiData(char * buf, int len)
{
    switch (buf[0]) {
        case PROTOCOL_WIFI_TYPE_IMU: {
            if (len > sizeof(imu_data.raw.data)) {
                len = sizeof(imu_data.raw.data);
            }
            memcpy(&imu_data.raw.data[0], buf, len);
        } break;

        default:
            break;
    }
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
                // 处理收到的数据（尽量快速，避免阻塞）
                Serial.printf("From %s:%u -> \n", remoteIp.toString().c_str(), remotePort);

                // 逐字节打印16进制的接收到的数据
                // Serial.print("Data (hex): ");
                // for (int i = 0; i < len; i++) {
                //     Serial.printf("%02X ", static_cast<uint8_t>(buf[i]));
                // }
                // Serial.println();

                DecodeWifiData(buf, len);
                Serial.printf(
                    "IMU Data: ax=%.2f ay=%.2f az=%.2f dr=%.2f dp=%.2f dy=%.2f r=%.2f p=%.2f "
                    "y=%.2f\n",
                    imu_data.decoded.ax, imu_data.decoded.ay, imu_data.decoded.az,
                    imu_data.decoded.dr, imu_data.decoded.dp, imu_data.decoded.dy,
                    imu_data.decoded.r, imu_data.decoded.p, imu_data.decoded.y);
            }
        }

        // yield to other tasks
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}
}  // namespace task_host