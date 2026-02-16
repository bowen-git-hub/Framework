#include "robot_cmd.h"
#include "usart.h"
#include "sbus_ht10a.h"
#include <stdio.h>
#include "cmsis_os.h"
#include "pwm_motor.h"
#include "stdlib.h"
#include "string.h"

UniRemote_HandleTypeDef hur = {0};
extern Robot_cmd *cmd;

Robot_cmd *robot_cmd_init(void)
{
    Robot_cmd *obj = (Robot_cmd *)malloc(sizeof(Robot_cmd));
    memset(obj, 0, sizeof(Robot_cmd));

    // 初始化UART DMA接收
    SBUS_Init(&huart5);

    //电机初始化
    motor_init();
    obj->robot_ready = Robot_stop;

    //蜂鸣器初始化

    return obj;
}

void robot_cmd_update(Robot_cmd *obj)
{
    SBUS_To_UniRemote(&hur, hsbus1);
    //printf("robot_cmd_update: %d, %d.\n\r", hur.left_x, hur.left_y);
    if (obj->robot_ready == Robot_stop)
    {
        stop_all_motors();
    }
    else
    {
        int16_t left_front_speed = 0;
        int16_t left_back_speed = 0;
        int16_t right_front_speed = 0;
        int16_t right_back_speed = 0;

        left_front_speed = hur.left_y + hur.right_x;
        left_back_speed = hur.left_y + hur.right_x;
        right_front_speed = hur.left_y - hur.right_x;
        right_back_speed = hur.left_y - hur.right_x;

        set_motor_speed(motor_left_back, left_back_speed);
        set_motor_speed(motor_left_front, left_front_speed);
        set_motor_speed(motor_right_back, right_back_speed);
        set_motor_speed(motor_right_front, right_front_speed);
        osDelay(100);
    }
}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    // printf("UARTEx_RxEventCallbackaaaaaaaaaaaaaaaaaaa: %d, %s.\n\r", Size, "debug");

    if (huart->Instance == UART5) // 判断是否是遥控器接收UART
    {
        SBUS_RxEventCallback(huart, Size);
    }
}

// 使能printf浮点数支持
asm(".global _printf_float");

// 重定向printf到UART1,方便调试
int _write(int file, char *ptr, int len)
{
    HAL_UART_Transmit_IT(&huart1, (uint8_t *)ptr, len);
    return len;
}