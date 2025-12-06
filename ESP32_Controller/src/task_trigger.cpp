#include "task_trigger.hpp"

#include "protocol_wifi.hpp"
#include "wifi_param.hpp"
#include "wifi_udp_connect.hpp"

#define TRIGGER_PIN_VX GPIO_NUM_32

#define DEBUG_MODE 1

namespace task_trigger
{

protocol_wifi::trigger_u trigger_trans_data;

float kp_vx = 0.0f;
float kp_vy = 0.0f;
float kp_wz = 0.0f;

static void Setup()
{
    pinMode(TRIGGER_PIN_VX, INPUT_PULLDOWN);  // 内部下拉

    Serial.println("Trigger init OK");
}

static void UdpTransmit()
{
    trigger_trans_data.decoded.type = PROTOCOL_WIFI_TYPE_TRIGGER;
    trigger_trans_data.decoded.kp_vx = kp_vx;
    trigger_trans_data.decoded.kp_vy = kp_vy;
    trigger_trans_data.decoded.kp_wz = kp_wz;

    wifi_udp_connect::UdpUpload(
        serverIp, serverPort, trigger_trans_data.raw.data, sizeof(trigger_trans_data.decoded));
}

static void Loop()
{
    int level = digitalRead(TRIGGER_PIN_VX);
#if DEBUG_MODE
    Serial.printf("TRIGGER=%d\n", level);
#endif

    kp_vx = level ? 1.0f : 0.0f;
    kp_vy = 0.0f;
    kp_wz = 0.0f;

    UdpTransmit();
}

void TriggerTask(void * pvParameters)
{
    Setup();
    while (true) {
        Loop();
        vTaskDelay(pdMS_TO_TICKS(30));
    }
}
#undef DEBUG_MODE
}  // namespace task_trigger