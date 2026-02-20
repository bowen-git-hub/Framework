#include "buzzer.h"
#include "bsp_def.h"
#include "cmsis_os.h"

void BuzzerBeep(uint16_t delay, uint8_t times) {
    for (uint8_t i = 0; i < times; i++) {
        HAL_GPIO_WritePin(BSP_BUZZER_GPIO, BSP_BUZZER_PIN, BSP_BUZZER_ON);  // SPEAKER 高电平有效
        osDelay(delay);
        HAL_GPIO_WritePin(BSP_BUZZER_GPIO, BSP_BUZZER_PIN, BSP_BUZZER_OFF);  // SPEAKER close
        if (i != times - 1) {
            osDelay(delay);
        }
    }
    
}