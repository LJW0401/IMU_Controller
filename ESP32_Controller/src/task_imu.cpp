#include "task_imu.hpp"

#include <Arduino.h>
#include <driver/twai.h>

#include "dm_imu.hpp"

// CAN 配置（按需修改引脚和波特率）
#define CAN_TX_PIN GPIO_NUM_5
#define CAN_RX_PIN GPIO_NUM_4

namespace task_imu
{

dm_imu::imu_t imu;

static void Setup()
{
    Serial.println("");
    Serial.println("ImuTask: starting TWAI init...");
    // 通用配置（TX, RX, 模式）
    twai_general_config_t g_config =
        TWAI_GENERAL_CONFIG_DEFAULT(CAN_TX_PIN, CAN_RX_PIN, TWAI_MODE_NORMAL);
    // 波特率配置：根据 SDK 提供的宏选择
    twai_timing_config_t t_config = TWAI_TIMING_CONFIG_1MBITS();
    twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

    if (twai_driver_install(&g_config, &t_config, &f_config) != ESP_OK) {
        Serial.println("TWAI driver install failed");
        vTaskDelay(pdMS_TO_TICKS(1000));
        while (1) vTaskDelay(pdMS_TO_TICKS(1000));
    }
    if (twai_start() != ESP_OK) {
        Serial.println("TWAI start failed");
        while (1) vTaskDelay(pdMS_TO_TICKS(1000));
    }
    Serial.println("TWAI init OK");

    imu_init(imu, 0x01, 0x51);
    Serial.println("IMU init OK");
}

static void Receive()
{
    // 接收（非阻塞，超时 10ms）
    twai_message_t rx_msg;
    if (twai_receive(&rx_msg, pdMS_TO_TICKS(10)) == ESP_OK) {
        // 处理接收到的消息
        // Serial.printf("CAN RX id=0x%03X len=%u data=", rx_msg.identifier, rx_msg.data_length_code);
        // for (uint8_t i = 0; i < rx_msg.data_length_code; ++i)
        //     Serial.printf("%02X ", rx_msg.data[i]);
        // Serial.println();
        
        // 根据 ID 解析数据
        IMU_UpdateData(imu, rx_msg.data);
        // 打印 IMU 数据：xyz的加速度，rpy的角速度，rpy的角度
        // Serial.printf("Accel [m/s^2] - X: %.3f, Y: %.3f, Z: %.3f\n", imu.accel[0], imu.accel[1], imu.accel[2]);
        // Serial.printf("Gyro  [deg/s]  - Roll: %.3f, Pitch: %.3f, Yaw: %.3f\n", imu.gyro[0], imu.gyro[1], imu.gyro[2]);
        Serial.printf("IMU Data - Pitch: %.2f, Roll: %.2f, Yaw: %.2f\n", imu.pitch, imu.roll, imu.yaw);
        Serial.println();
    }
}

static void Loop()
{
    // imu_request_accel(imu);
    // Receive();

    // imu_request_gyro(imu);
    // Receive();

    imu_request_euler(imu);
    Receive();
}

void ImuTask(void * pvParameters)
{
    Setup();
    while (true) {
        Loop();
        vTaskDelay(pdMS_TO_TICKS(7));
    }
}
}  // namespace task_imu