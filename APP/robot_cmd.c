#include "robot_cmd.h"

#include <stdio.h>

#include "cmsis_os.h"
#include "pwm_motor.h"
#include "sbus_ht10a.h"
#include "stdlib.h"
#include "string.h"
#include "usart.h"



Robot_cmd* robot_cmd_init(void) {
    Robot_cmd* obj = (Robot_cmd*)malloc(sizeof(Robot_cmd));
    memset(obj, 0, sizeof(Robot_cmd));



    // 为 remote 分配内存并初始化
    obj->remote = (UniRemote_HandleTypeDef*)malloc(sizeof(UniRemote_HandleTypeDef));
    if (obj->remote == NULL) {
        // 内存分配失败，处理错误
        free(obj);
        return NULL;
    }
 

    // 初始化UART DMA接收
    SBUS_Init(&huart5);

    // 电机初始化
    motor_init();
    BSP_Wheel_Detect_Init();

    // 蜂鸣器初始化
    bsp_voice_init(&huart3);
    // bsp_voice_send_msg("大家好");

    return obj;
}

void robot_cmd_update(Robot_cmd* obj) {
    SBUS_To_UniRemote(obj->remote, hsbus1);


    //模式选择
    if (obj->remote->frame_connected == 0) { 
        obj->mode = Robot_stop; 
    } else {
        // 连接正常且未丢失帧时，根据开关状态设置模式
        if (obj->remote->switch_power == 2) { 
            obj->mode = Robot_run; 
        } else {
            obj->mode = Robot_stop; 
        }
    }
    
    
    

    //机器人控制
    if (obj->mode == Robot_stop) {
        stop_all_motors();
    } else if (obj->mode == Robot_run)
    {
        int16_t left_front_speed = 0;
        int16_t left_back_speed = 0;
        int16_t right_front_speed = 0;
        int16_t right_back_speed = 0;

        left_front_speed = obj->remote->left_y + obj->remote->right_x;
        left_back_speed = obj->remote->left_y + obj->remote->right_x;
        right_front_speed = obj->remote->left_y - obj->remote->right_x;
        right_back_speed = obj->remote->left_y - obj->remote->right_x;

        set_motor_speed(motor_left_back, left_back_speed);
        set_motor_speed(motor_left_front, left_front_speed);
        set_motor_speed(motor_right_back, right_back_speed);
        set_motor_speed(motor_right_front, right_front_speed);
    }

}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef* huart, uint16_t Size) {
    // printf("UARTEx_RxEventCallbackaaaaaaaaaaaaaaaaaaa: %d, %s.\n\r", Size, "debug");

    if (huart->Instance == UART5)  // 判断是否是遥控器接收UART
    {
        SBUS_RxEventCallback(huart, Size);
    }
}

// 使能printf浮点数支持
asm(".global _printf_float");

// 重定向printf到UART1,方便调试
int _write(int file, char* ptr, int len) {
    HAL_UART_Transmit_IT(&huart1, (uint8_t*)ptr, len);
    return len;
}