#include "task_upload.hpp"

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>

#include "wifi_param.hpp"

namespace task_upload
{

WiFiUDP Udp;

// 模拟IMU数据（TODO：正式版要删掉）
float imu_x = 0.0;
float imu_y = 0.0;
float imu_z = 0.0;

void UploadTask(void * pvParameters)
{
    // setup
    WiFi.begin(ssid, password);
    Serial.print("Connecting to AP");
    while (WiFi.status() != WL_CONNECTED) {
        delay(200);
        Serial.print(".");
    }
    Serial.println("\nConnected, IP: " + WiFi.localIP().toString());

    // task loop
    while (true) {
        if (WiFi.status() != WL_CONNECTED) {
            // 简单重连逻辑
            // TODO: 可以改成更复杂的状态机
            WiFi.reconnect();
            delay(500);
            return;
        }

        // 模拟IMU数据变化（TODO：正式版要删掉）
        imu_x += 0.1;
        imu_y += 0.2;
        imu_z += 0.3;

        // 构造简单数据包并发送
        char payload[128];
        int len =
            snprintf(payload, sizeof(payload), "imu,x=%.2f,y=%.2f,z=%.2f\n", imu_x, imu_y, imu_z);
        Udp.beginPacket(serverIp, serverPort);
        Udp.write((uint8_t *)payload, len);
        Udp.endPacket();
        // 打印日志
        Serial.println("Uploaded: " + String(payload));

        // yield to other tasks
        vTaskDelay(pdMS_TO_TICKS(7));
    }
}
}  // namespace task_upload
