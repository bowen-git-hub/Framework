#include "app.h"
#include "robot_cmd.h"
#include <stdio.h>
#include "cmsis_os.h"
#include "hal.h"
#include "pwm_motor.h"
#include "voltage.h"
#include "ax_oled_1305.h"
#include "buzzer.h"

Robot_cmd *cmd;
void BSP_OLED_ShowVolte() {
    AX_OLED_DispFloat(6, 2, GetBatteryVoltage(), 2, 2, 0);
    uint8_t ch = 'v';
    AX_OLED_DispChar(36, 2, ch, 0);
}

AX_OLED_Config oled_config = {
    .hspi = &hspi1,
    .reset_gpio_port = GPIOA,
    .reset_gpio_pin = GPIO_PIN_6,
    .dc_gpio_port = GPIOA,
    .dc_gpio_pin = GPIO_PIN_4,
};

void app_init(void)
{

    cmd = robot_cmd_init();
    AX_OLED_Init(&oled_config); // OLED初始化




    BuzzerBeep(200, 3);
    printf("app_init\r\n");

    

}

void app_loop(void)
{
    robot_cmd_update(cmd);
}

void app_debug_loop(void)
{
    BSP_OLED_ShowVolte();

    //printf("app_debug\r\n");
}
