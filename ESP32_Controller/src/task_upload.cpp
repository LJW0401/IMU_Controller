#include "task_upload.hpp"

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>

#include "wifi_param.hpp"
#include "wifi_udp_connect.hpp"

namespace task_upload
{

WiFiUDP Udp;

static void Setup()
{
    // setup
    WiFi.begin(ssid, password);
    Serial.print("Connecting to AP");
    while (WiFi.status() != WL_CONNECTED) {
        delay(200);
        Serial.print(".");
    }
    Serial.println("\nConnected, IP: " + WiFi.localIP().toString());
}

static void Loop()
{
    if (WiFi.status() != WL_CONNECTED) {
        // 简单重连逻辑
        Serial.println("WiFi disconnected, reconnecting...");
        WiFi.reconnect();
        
        if (WiFi.status() == WL_CONNECTED) {
            Serial.println("Reconnected, IP: " + WiFi.localIP().toString());
        } else {
            Serial.println("Reconnection failed.");
        }

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void UploadTask(void * pvParameters)
{
    Setup();
    while (true) {
        Loop();
        // yield to other tasks
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
}  // namespace task_upload
