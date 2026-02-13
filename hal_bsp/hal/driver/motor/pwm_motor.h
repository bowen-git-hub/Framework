#ifndef PWM_MOTOR_H
#define PWM_MOTOR_H
#include "tim.h"
typedef struct {
    TIM_HandleTypeDef *tim;
    uint32_t channel;
} TIM_Config_t;

typedef struct {
    TIM_HandleTypeDef* forward_tim;
    uint32_t forward_ch;
    TIM_HandleTypeDef* backward_tim;  
    uint32_t backward_ch;
} Motor_TIM_HandleTypeDef;

typedef struct {
    Motor_TIM_HandleTypeDef *motor;
    TIM_HandleTypeDef *forward_tim;
    uint32_t forward_ch;
    TIM_HandleTypeDef *backward_tim;
    uint32_t backward_ch;
} Motor_Config_t;

void motor_init ();
void set_motor_speed(Motor_TIM_HandleTypeDef motor, int16_t speed);
void stop_all_motors();

extern Motor_TIM_HandleTypeDef motor_left_front, motor_right_front, motor_left_back, motor_right_back;
#endif