#include "task_imu.hpp"

#include <driver/twai.h>

#include "dm_imu.hpp"
#include "protocol_wifi.hpp"
#include "wifi_param.hpp"
#include "wifi_udp_connect.hpp"

// CAN 配置（按需修改引脚和波特率）
#define CAN_TX_PIN GPIO_NUM_5
#define CAN_RX_PIN GPIO_NUM_4

namespace task_imu
{

dm_imu::imu_t imu;
protocol_wifi::imu_u imu_trans_data;

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
        // 根据 ID 解析数据
        IMU_UpdateData(imu, rx_msg.data);
    }
}

static void UdpTransmit()
{
    imu_trans_data.decoded.type = PROTOCOL_WIFI_TYPE_IMU;
    imu_trans_data.decoded.can_id = imu.can_id;
    imu_trans_data.decoded.mst_id = imu.mst_id;
    imu_trans_data.decoded.ax = imu.accel[0];
    imu_trans_data.decoded.ay = imu.accel[1];
    imu_trans_data.decoded.az = imu.accel[2];
    imu_trans_data.decoded.dr = imu.gyro[0];
    imu_trans_data.decoded.dp = imu.gyro[1];
    imu_trans_data.decoded.dy = imu.gyro[2];
    imu_trans_data.decoded.r = imu.roll;
    imu_trans_data.decoded.p = imu.pitch;
    imu_trans_data.decoded.y = imu.yaw;

    wifi_udp_connect::UdpUpload(
        serverIp, serverPort, imu_trans_data.raw.data, sizeof(imu_trans_data.decoded));
}

static void Loop()
{
    // imu_request_accel(imu);
    // Receive();

    // imu_request_gyro(imu);
    // Receive();

    imu_request_euler(imu);
    Receive();

    UdpTransmit();
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