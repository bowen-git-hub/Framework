#include "pwm_motor.h"
#include "tim.h"
#include <stdint.h>


#define MAX_PWM 100
#define MIN_PWM 56

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

static const TIM_Config_t tim_configs[] = {
    {BSP_LF_FORWARD_TIM, BSP_LF_FORWARD_CH},
    {BSP_LF_BACKWARD_TIM, BSP_LF_BACKWARD_CH},
    {BSP_RF_FORWARD_TIM, BSP_RF_FORWARD_CH},
    {BSP_RF_BACKWARD_TIM, BSP_RF_BACKWARD_CH},
    {BSP_LB_FORWARD_TIM, BSP_LB_FORWARD_CH},
    {BSP_LB_BACKWARD_TIM, BSP_LB_BACKWARD_CH},
    {BSP_RB_FORWARD_TIM, BSP_RB_FORWARD_CH},
    {BSP_RB_BACKWARD_TIM, BSP_RB_BACKWARD_CH}};

#define TIM_CONFIG_COUNT (sizeof(tim_configs) / sizeof(tim_configs[0]))

static const Motor_Config_t motor_configs[] = {
    {&motor_left_front, BSP_LF_FORWARD_TIM, BSP_LF_FORWARD_CH, BSP_LF_BACKWARD_TIM, BSP_LF_BACKWARD_CH},
    {&motor_right_front, BSP_RF_FORWARD_TIM, BSP_RF_FORWARD_CH, BSP_RF_BACKWARD_TIM, BSP_RF_BACKWARD_CH},
    {&motor_left_back, BSP_LB_FORWARD_TIM, BSP_LB_FORWARD_CH, BSP_LB_BACKWARD_TIM, BSP_LB_BACKWARD_CH},
    {&motor_right_back, BSP_RB_FORWARD_TIM, BSP_RB_FORWARD_CH, BSP_RB_BACKWARD_TIM, BSP_RB_BACKWARD_CH}};

#define MOTOR_COUNT (sizeof(motor_configs) / sizeof(motor_configs[0]))


Motor_TIM_HandleTypeDef motor_left_front, motor_right_front, motor_left_back, motor_right_back;





//限位
static int16_t constrain_PWM(int16_t value) {
    if (value > MAX_PWM)
        return MAX_PWM;
    if (value < MIN_PWM)
        return MIN_PWM;
    return value;
}

// 启动所有PWM定时器
static void start_all_pwm_timers(void) {
    for (int i = 0; i < TIM_CONFIG_COUNT; i++) {
        HAL_TIM_PWM_Start_IT(tim_configs[i].tim, tim_configs[i].channel);
    }
}

//停车
void stop_all_motors(void) {
    set_motor_speed(motor_left_front, 0);
    set_motor_speed(motor_right_front, 0);
    set_motor_speed(motor_left_back, 0);
    set_motor_speed(motor_right_back, 0);
    // printf("All motors stopped\r\n");
}

static void init_motor_structures(void) {
    for (int i = 0; i < MOTOR_COUNT; i++) {
        const Motor_Config_t* config = &motor_configs[i];
        Motor_TIM_HandleTypeDef* motor = config->motor;

        motor->forward_tim = config->forward_tim;
        motor->forward_ch = config->forward_ch;
        motor->backward_tim = config->backward_tim;
        motor->backward_ch = config->backward_ch;
    }
}

void motor_init () {
    start_all_pwm_timers();

    init_motor_structures();

    stop_all_motors();


}
void set_motor_speed(Motor_TIM_HandleTypeDef motor, int16_t speed) {
    uint16_t pwm_value;
    uint16_t forward_pwm = 0;
    uint16_t backward_pwm = 0;

    if (speed > 0) {
        // 前进
        pwm_value = constrain_PWM(speed);
        forward_pwm = pwm_value;
        backward_pwm = 0;
    } else if (speed < 0) {
        // 后退
        pwm_value = constrain_PWM(-speed);
        forward_pwm = 0;
        backward_pwm = pwm_value;
    } else {
        // 停止
        forward_pwm = 0;
        backward_pwm = 0;
    }
    // printf("test:%ld,%ld,%ld,%ld--------------\n",motor.forward_ch,forward_pwm,motor.backward_ch,backward_pwm);
    __HAL_TIM_SET_COMPARE(motor.forward_tim, motor.forward_ch, forward_pwm);
    __HAL_TIM_SET_COMPARE(motor.backward_tim, motor.backward_ch, backward_pwm);
}