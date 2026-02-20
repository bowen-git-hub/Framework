#include "hal.h"

#include "buzzer.h"
#include "speaker.h"
#include "voltage.h"

void HAL_Layer_Init() {
    BSP_Voltage_Init();
}

void HAL_Voice_loop() { bsp_voice_loop_process(); }