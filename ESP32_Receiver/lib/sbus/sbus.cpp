#include "sbus.hpp"

#include "driver/uart.h"
namespace sbus
{

SBUS_ctrl_u SBUS =
    {  // SBUS 遥控器数据
        .unpack = {
            .start = 0x0F,
            .end = 0x00,
        }};

HardwareSerial UartSerial(1);  // 使用 UART1

void SetupSbus(int rx_pin, int tx_pin)
{
    // 初始化 UART1
    // 波特率 100000，8 数据位，偶校验位，1 停止位，RX=GPIO12, TX=GPIO13
    UartSerial.begin(100000, SERIAL_8E1, rx_pin, tx_pin);
    // 设置 TXD 引脚为反相
    uart_set_line_inverse(UART_NUM_1, UART_SIGNAL_TXD_INV);

    Serial.printf("SBUS 已初始化，rx引脚%d，tx引脚%d\n", rx_pin, tx_pin);
}

void SbusSendData()
{
    UartSerial.write(SBUS.pack.buf, SBUS_RC_FRAME_LENGTH);  // 发送数据到 UART1
}

}  // namespace sbus