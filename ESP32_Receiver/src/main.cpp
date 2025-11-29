#include <Arduino.h>

#include "task_host.hpp"

using namespace task_host;

// 定义任务句柄
TaskHandle_t HostTaskHandle = NULL;

uint32_t count = 0;


void setup()
{
    Serial.begin(9600);
    delay(100);

    // 创建任务
    xTaskCreate(
        HostTask,          // 任务函数
        "host_task",        // 任务名称
        4096,              // 任务栈大小（字节）
        NULL,              // 任务参数
        1,                 // 任务优先级
        &HostTaskHandle  // 任务句柄
    );

    Serial.println("初始化完成");
}

void loop() {}

// END =====================================================
