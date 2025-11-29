#include "task_host.hpp"

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>
namespace task_host
{

const char * ssid = "RobotCmd_AP";  // 与 README 中一致
const char * password = "12345678";
const uint16_t listenPort = 3333;

WiFiUDP Udp;

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
                Serial.printf("From %s:%u -> %s\n", remoteIp.toString().c_str(), remotePort, buf);
            }
        }
        
        // yield to other tasks
        vTaskDelay(1 / portTICK_PERIOD_MS);
    }
}
}  // namespace task_host