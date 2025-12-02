
#include <Arduino.h>

namespace task_imu
{

union imu_trans_u {
    struct
    {
        uint8_t data[128];
    } raw;

    struct
    {
        uint8_t can_id;
        uint8_t mst_id;
        float ax;
        float ay;
        float az;

        float dr;
        float dp;
        float dy;

        float r;
        float p;
        float y;
    } decoded;
};

void ImuTask(void * pvParameters);
}  // namespace task_imu