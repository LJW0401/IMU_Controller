#include "task_state_control.hpp"

#include <Arduino.h>

#include "sbus.hpp"
#include "shared.hpp"

/**
************************************************************************
* @brief:      	float_to_uint: 浮点数转换为无符号整数函数
* @param[in]:   x_float:	待转换的浮点数
* @param[in]:   x_min:		范围最小值
* @param[in]:   x_max:		范围最大值
* @param[in]:   bits: 		目标无符号整数的位数
* @retval:     	无符号整数结果
* @details:    	将给定的浮点数 x 在指定范围 [x_min, x_max] 内进行线性映射，映射结果为一个指定位数的无符号整数
************************************************************************
**/
int float_to_uint(float x_float, float x_min, float x_max, int bits)
{
    /* Converts a float to an unsigned int, given range and number of bits */
    float span = x_max - x_min;
    float offset = x_min;
    return (int)((x_float - offset) * ((float)((1 << bits) - 1)) / span);
}

static void Setup() { sbus::SetupSbus(12, 13); }

static void Loop()
{
    head_imu_data.decoded.r;
    head_imu_data.decoded.p;
    head_imu_data.decoded.y;

    sbus::SBUS.unpack.ch0 = float_to_uint(head_imu_data.decoded.r, -180.0f, 180.0f, 11);
    sbus::SBUS.unpack.ch1 = float_to_uint(head_imu_data.decoded.p, -90.0f, 90.0f, 11);
    sbus::SBUS.unpack.ch2 = float_to_uint(head_imu_data.decoded.y, -180.0f, 180.0f, 11);

    sbus::SbusSendData();
}

void StateControlTask(void * pvParameters)
{
    Setup();
    // task loop
    while (true) {
        Loop();
        // yield to other tasks
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}