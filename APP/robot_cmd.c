#include "robot_cmd.h"
#include "usart.h"
#include "sbus_ht10a.h"
#include <stdio.h>
#include "cmsis_os.h"
#include "pwm_motor.h"
UniRemote_HandleTypeDef hur = {0};

void robot_cmd_init(void) {
    // 初始化UART DMA接收
    SBUS_Init(&huart5);
}

void robot_cmd_update(void)
{
    SBUS_To_UniRemote(&hur, hsbus1);
    printf("robot_cmd_update: %d, %d.\n\r", hur.left_x, hur.left_y);

    int16_t left_front_speed = 0;
    int16_t left_back_speed = 0;
    int16_t right_front_speed = 0;
    int16_t right_back_speed = 0;

    left_front_speed = hur.left_y +  hur.right_x;
    left_back_speed = hur.left_y +  hur.right_x;
    right_front_speed = hur.left_y -  hur.right_x;
    right_back_speed = hur.left_y -  hur.right_x;

    set_motor_speed(motor_left_back, left_back_speed);
    set_motor_speed(motor_left_front, left_front_speed);
    set_motor_speed(motor_right_back, right_back_speed);
    set_motor_speed(motor_right_front, right_front_speed);
    osDelay(100);
    
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