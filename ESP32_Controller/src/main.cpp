#include <Arduino.h>

#include "task_imu.hpp"

using namespace task_imu;

// 定义任务句柄
TaskHandle_t ImuTaskHandle = NULL;

uint32_t count = 0;


void setup()
{
    Serial.begin(9600);
    delay(100);

    // 创建任务
    xTaskCreate(
        ImuTask,          // 任务函数
        "imu_task",        // 任务名称
        4096,              // 任务栈大小（字节）
        NULL,              // 任务参数
        1,                 // 任务优先级
        &ImuTaskHandle  // 任务句柄
    );

    Serial.println("初始化完成");
}

void loop() {}

// END =====================================================
