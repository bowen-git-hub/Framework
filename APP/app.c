#include "app.h"
#include "robot_cmd.h"
#include <stdio.h>
#include "cmsis_os.h"
#include "hal.h"
#include "pwm_motor.h"
#include "tim.h"
#include "buzzer.h"

Robot_cmd *cmd;
void app_init(void)
{

    cmd = robot_cmd_init();
    bsp_voice_init(&huart3);
    bsp_voice_send_msg("大家好，我是刘小帅，来看我表演喽");
    printf("app_init\r\n");
    

}

void app_loop(void)
{
    robot_cmd_update(cmd);
}

void app_debug(void)
{
    bsp_voice_loop_process();
    printf("app_debug\r\n");
}
