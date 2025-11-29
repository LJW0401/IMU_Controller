#include "task_host.hpp"

#include <Arduino.h>

namespace task_host
{
void HostTask(void * pvParameters)
{
    while (true) {
        vTaskDelay(1 / portTICK_PERIOD_MS);
    }
}
}  // namespace task_host