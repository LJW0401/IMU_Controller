#ifndef SBUS_HPP
#define SBUS_HPP

#include <Arduino.h>

namespace sbus
{
#define SBUS_RC_FRAME_LENGTH 25u

typedef union {
    struct __rc_ctrl
    {
        uint8_t start;
        uint16_t ch0 : 11;
        uint16_t ch1 : 11;
        uint16_t ch2 : 11;
        uint16_t ch3 : 11;
        uint16_t ch4 : 11;
        uint16_t ch5 : 11;
        uint16_t ch6 : 11;
        uint16_t ch7 : 11;
        uint16_t ch8 : 11;
        uint16_t ch9 : 11;
        uint16_t ch10 : 11;
        uint16_t ch11 : 11;
        uint16_t ch12 : 11;
        uint16_t ch13 : 11;
        uint16_t ch14 : 11;
        uint16_t ch15 : 11;
        uint8_t connect_flag;
        uint8_t end;
    } __attribute__((packed)) unpack;
    struct
    {
        uint8_t buf[SBUS_RC_FRAME_LENGTH];
    } __attribute__((packed)) pack;
} SBUS_ctrl_u;

void SetupSbus(int rx_pin, int tx_pin);
void SbusSendData();

extern SBUS_ctrl_u SBUS;

}  // namespace sbus
#endif  // SBUS_HPP