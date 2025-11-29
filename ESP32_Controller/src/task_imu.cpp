#include "task_imu.hpp"

#include <Arduino.h>

namespace task_imu
{
void ImuTask(void * pvParameters)
{
    while (true) {
        vTaskDelay(2 / portTICK_PERIOD_MS);
    }
}
}  // namespace task_imu