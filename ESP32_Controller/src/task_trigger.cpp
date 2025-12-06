#include "task_trigger.hpp"

#define TRIGGER_PIN_VX GPIO_NUM_32

#define DEBUG_MODE 1

namespace task_trigger
{

static void Setup()
{
    pinMode(TRIGGER_PIN_VX, INPUT_PULLDOWN);  // 内部下拉

    Serial.println("Trigger init OK");
}

static void Loop()
{
    int level = digitalRead(TRIGGER_PIN_VX);
#if DEBUG_MODE
    Serial.printf("TRIGGER=%d\n", level);
#endif
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