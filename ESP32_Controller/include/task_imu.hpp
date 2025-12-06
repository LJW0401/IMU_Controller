
#include <Arduino.h>

namespace task_imu
{
typedef struct{
    uint8_t can_id;
    uint8_t mst_id;
}ImuParams;

void ImuTask(void * pvParameters);
}  // namespace task_imu