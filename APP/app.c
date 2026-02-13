#include "app.h"
#include "robot_cmd.h"
#include <stdio.h>
#include "cmsis_os.h"
#include "hal.h"
#include "pwm_motor.h"
#include "tim.h"

void app_init(void)
{
    // printf("app_init\r\n");
    robot_cmd_init();
    motor_init();




}

void app_loop(void)
{
    robot_cmd_update();
}

void app_debug(void)
{

}
