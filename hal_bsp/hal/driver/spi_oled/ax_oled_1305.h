 

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef _AX_OLED_1305_H
#define _AX_OLED_1305_H

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include "spi.h"

// OLED配置结构体
typedef struct {
    SPI_HandleTypeDef* hspi;
    GPIO_TypeDef* reset_gpio_port;   // GPIO端口
    uint16_t reset_gpio_pin;          //PIN引脚
    GPIO_TypeDef* dc_gpio_port;   // GPIO端口
    uint16_t dc_gpio_pin;          //PIN引脚
} AX_OLED_Config;


/***  OLED操作函数 **********/
void AX_OLED_Init(AX_OLED_Config* config);    //OLED初始化
void AX_OLED_ClearScreen(void);    //OLED清除屏幕
/* 6X8显示 */
void AX_OLED_DispChar(uint8_t x, uint8_t y, uint8_t ch, uint8_t mode);    // OLED指定位置显示一个ASCII字符（6X8）
void AX_OLED_DispStr(uint8_t x, uint8_t y, uint8_t *ch, uint8_t mode);    //OLED指定位置显示ASCII字符串（6X8）
void AX_OLED_DispNum(uint8_t x, uint8_t y, uint8_t num, uint8_t mode);    //OLED指定位置显示一个数字（6X8）
void AX_OLED_DispInteger(uint8_t x, uint8_t y, int32_t value, uint8_t digits, uint8_t mode);  //OLED显示int32_t 整数
void AX_OLED_DispFloat(uint8_t x, uint8_t y, float value, uint8_t inte, uint8_t deci, uint8_t mode);
void AX_OLED_DispFloatAuto(uint8_t x, uint8_t y, float value, uint8_t deci, uint8_t mode);  //自动计算整数长度
/* 8X16显示 */
void AX_OLED_Disp16Char(uint8_t x, uint8_t y, uint8_t ch, uint8_t mode);    // OLED指定位置显示一个ASCII字符（8X16）
void AX_OLED_Disp16Str(uint8_t x, uint8_t y, uint8_t *ch, uint8_t mode);    //OLED指定位置显示ASCII字符串（8X16）
void AX_OLED_Disp16Num(uint8_t x, uint8_t y, uint8_t num, uint8_t mode);    //OLED指定位置显示一个数字（8X16）
void AX_OLED_Disp16Integer(uint8_t x, uint8_t y, int32_t value, uint8_t digits, uint8_t mode);
/* 图形及文字 */
void AX_OLED_DispPicture(uint8_t x, uint8_t y, uint8_t xsize, uint8_t ysize, const uint8_t *pbuf, uint8_t mode);    //OLED指定位置显示一个指定尺寸照片
void AX_OLED_DispChinese(uint8_t x, uint8_t y, const uint8_t *pbuf, uint8_t mode);    //OLED指定位置显示一个汉字（16X16）
void AX_OLED_ClearLine(uint8_t line);
#endif

/******************* (C) 版权 2026 ****************************/
