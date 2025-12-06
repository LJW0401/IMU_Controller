#ifndef PROTOCOL_WIFI_HPP
#define PROTOCOL_WIFI_HPP

#include <Arduino.h>

typedef enum {
    PROTOCOL_WIFI_TYPE_TEST = 0,
    PROTOCOL_WIFI_TYPE_IMU,
    PROTOCOL_WIFI_TYPE_TRIGGER,
    PROTOCOL_WIFI_TYPE_NONE
} protocol_wifi_type_e;

namespace protocol_wifi
{
union imu_u {
    struct
    {
        uint8_t data[64];
    } raw;

    struct
    {
        uint8_t type;

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

union trigger_u {
    struct
    {
        uint8_t data[16];
    } raw;

    struct
    {
        uint8_t type;

        float kp_vx;  // [-1.0, 1.0]
        float kp_vy;  // [-1.0, 1.0]
        float kp_wz;  // [-1.0, 1.0]
    } decoded;
};

}  // namespace protocol_wifi
#endif  // PROTOCOL_WIFI_HPP