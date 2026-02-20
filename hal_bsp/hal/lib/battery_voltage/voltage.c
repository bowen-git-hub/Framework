#include "voltage.h"

#include "adc.h"
#include "stdint.h"
#include "bsp_def.h"

static float ADC_ToVoltage(uint16_t adc_value);

void BSP_Voltage_Init() {
    HAL_ADC_Start(BSP_BATTERY_ADC);  // 开启ADC用于电压读取
}

static float ADC_ToVoltage(uint16_t adc_value) {
    // STM32F1 ADC为12位，最大值为4095
    // 参考电压一般为3.3V
    const float VREF = 3.3f;
    return (adc_value * VREF) / 4095.0f;
}

float GetBatteryVoltage() {
    HAL_ADC_Start(BSP_BATTERY_ADC);
    HAL_ADC_PollForConversion(BSP_BATTERY_ADC, 10);  // 等待转换完成
    volatile uint32_t adc_value = HAL_ADC_GetValue(BSP_BATTERY_ADC);
    return ADC_ToVoltage(adc_value * 11.0);  // 10:1分压
}