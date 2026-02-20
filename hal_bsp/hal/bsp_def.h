#ifndef _BSP_DEF_H
#define _BSP_DEF_H

#include "main.h"

// 模块调试打印开关
#define DEBUG_MOVIE_DETECT 0  // 传感器检测调试:1启用/0关闭
#define DEBUG_LIDAR_DETECT 0
#define DEBUG_MOTOR_CONTROL 0  // 运动控调试
#define DEBUG_UNI_REMOTE 0
#define DEBUG_SBUS_REMOTE 0
#define DEBUG_MOTOR_DETECT 0
#define DEBUG_PID_CONTROL 0
#define DEBUG_CAN_CONTROL 1

//  轮速度监测

/*******************************************接口板定义*********************************/


//  CAN总线
#define MOTOR_HCAN_PTR &hcan1

// 拟人语音
#define BSP_VOICE_UART &huart3

// SPI OLED接口
#define BSP_OLED_DC_GPIO GPIOA
#define BSP_OLED_DC_PIN GPIO_PIN_4

#define BSP_OLED_RST_GPIO GPIOA
#define BSP_OLED_RST_PIN GPIO_PIN_6
#define BSP_OLED_SPI &hspi1

// LED灯光
#define BSP_LED_RED_GPIO GPIOD
#define BSP_LED_RED_PIN GPIO_PIN_3
#define BSP_LED_GREEN_GPIO GPIOD
#define BSP_LED_GREEN_PIN GPIO_PIN_4
#define BSP_LED_OFF GPIO_PIN_SET
#define BSP_LED_ON GPIO_PIN_RESET

// MPU6050传感器IIC接口
#define BSP_MPU6050_I2C &hi2c1  // 接在PB6-SCL 和 PB7-SDA 引脚

// USART1 调试输出  230400

// 激光雷达传感器UART
#define BSP_LIDAR_DETECT_UART &huart2
#define BSP_LIDAR_DETECT_INSTANCE USART2

// HT10接收器接口
#define BSP_REMOTE_RECEIVE_UART &huart5
#define BSP_REMOTE_RECEIVE_INSTANCE UART5

// 四轮霍尔传感器TIM
#define BSP_LF_DETECT_TIM &htim2
#define BSP_RF_DETECT_TIM &htim3
#define BSP_LB_DETECT_TIM &htim5
#define BSP_RB_DETECT_TIM &htim4

// 传感器通道号
#define BSP_LF_DETECT_TIM_CHANNEL TIM_CHANNEL_1
#define BSP_RF_DETECT_TIM_CHANNEL TIM_CHANNEL_1
#define BSP_LB_DETECT_TIM_CHANNEL TIM_CHANNEL_1
#define BSP_RB_DETECT_TIM_CHANNEL TIM_CHANNEL_1

// 电池电压探测ADC接口
#define BSP_BATTERY_ADC &hadc1

// BUZZER 扬声器喇叭有源
#define BSP_BUZZER_GPIO GPIOE
#define BSP_BUZZER_PIN GPIO_PIN_1
#define BSP_BUZZER_ON GPIO_PIN_SET     // 高电平发声
#define BSP_BUZZER_OFF GPIO_PIN_RESET  // 低电平关闭

// 电机四个控制器引脚定义 前后各两个
#define BSP_LF_FORWARD_TIM &htim1
#define BSP_LF_FORWARD_CH TIM_CHANNEL_2
#define BSP_LF_BACKWARD_TIM &htim1
#define BSP_LF_BACKWARD_CH TIM_CHANNEL_1

#define BSP_RF_FORWARD_TIM &htim1
#define BSP_RF_FORWARD_CH TIM_CHANNEL_4
#define BSP_RF_BACKWARD_TIM &htim1
#define BSP_RF_BACKWARD_CH TIM_CHANNEL_3

#define BSP_LB_FORWARD_TIM &htim9
#define BSP_LB_FORWARD_CH TIM_CHANNEL_1
#define BSP_LB_BACKWARD_TIM &htim9
#define BSP_LB_BACKWARD_CH TIM_CHANNEL_2

#define BSP_RB_FORWARD_TIM &htim12
#define BSP_RB_FORWARD_CH TIM_CHANNEL_1
#define BSP_RB_BACKWARD_TIM &htim12
#define BSP_RB_BACKWARD_CH TIM_CHANNEL_2

//编码器
#define WHEEL_COUNT 4               // 轮子数量
#define TIM_LOOP_MARK 10000         // 重载计数器用于麦轮,起始基准值 方便计算倒转时不会反向0~65535溢出
#define ENCODER_COUNTS_PER_REV 390  // 编码器每圈计数 设置每圈触发390个脉冲


#endif
