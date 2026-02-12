# 使用方法
 
## oled

AX_OLED_Config oled_config = {
    .hspi = &hspi1,
    .reset_gpio_port = GPIOA,
    .reset_gpio_pin = GPIO_PIN_6,
    .dc_gpio_port = GPIOA,
    .dc_gpio_pin = GPIO_PIN_4,
};

uint8_t oled_ch[] = "hello";

AX_OLED_Init(&oled_config); // OLED初始化

osDelay(100);

AX_OLED_DispStr(0, 0, oled_ch, 0);