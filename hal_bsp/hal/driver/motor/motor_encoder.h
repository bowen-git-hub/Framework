#ifndef __MOTOR_ENCODER_H__
#define __MOTOR_ENCODER_H__

#include <stdint.h>
#include "bsp_def.h"


typedef struct {
    int32_t count;  //脉冲计数器
    int32_t speed;
    uint32_t tick_time;//取样时间
} WheelSpeed_t;

typedef struct {
    uint8_t wheelCount; //轮子数量
    WheelSpeed_t wheelSpeeds[WHEEL_COUNT];//四轮状态
} BSP_Wheel_Sensor_HandleTypeDef;

void BSP_Wheel_Detect_Init(void);
void BSP_Wheel_Detect_Loop(void);

extern   BSP_Wheel_Sensor_HandleTypeDef *hws; 


#endif