#include <Arduino.h>

#include "task_imu.hpp"
#include "task_upload.hpp"

#define MODE_HEAD_TRACKER 1
#define MODE_POSE_TRACKER 2

using namespace task_imu;
using namespace task_upload;

// 定义任务句柄
TaskHandle_t ImuTaskHandle = NULL;
TaskHandle_t UploadTaskHandle = NULL;

uint32_t count = 0;

// 设置工作模式
uint32_t tracker_mode = MODE_POSE_TRACKER;

void setup()
{
    Serial.begin(9600);
    delay(100);
    Serial.println();

    // 创建任务
    uint8_t can_id = 0x01;
    uint8_t mst_id = 0x51;
    if (tracker_mode == MODE_HEAD_TRACKER) {
        Serial.println("工作模式: HEAD TRACKER");
        can_id = 0x01;
        mst_id = 0x51;
    } else if (tracker_mode == MODE_POSE_TRACKER) {
        Serial.println("工作模式: POSE TRACKER");
        can_id = 0x02;
        mst_id = 0x52;
    }

    auto * imuParams = new task_imu::ImuParams{can_id, mst_id};
    xTaskCreate(
        ImuTask,        // 任务函数
        "ImuTask",      // 任务名称
        4096,           // 任务栈大小（字节）
        imuParams,      // 任务参数
        1,              // 任务优先级
        &ImuTaskHandle  // 任务句柄
    );

    xTaskCreate(
        UploadTask,        // 任务函数
        "UploadTask",      // 任务名称
        4096,              // 任务栈大小（字节）
        NULL,              // 任务参数
        1,                 // 任务优先级
        &UploadTaskHandle  // 任务句柄
    );

    Serial.println("初始化完成");
}

void loop() {}

// END =====================================================
