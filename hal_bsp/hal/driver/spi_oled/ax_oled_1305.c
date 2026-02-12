/**
 *
 ******************************************************************************
 * @作  者  修改2026.1.18
 * @内  容  OLED显示驱动程序
 *
 * GND：接地引脚
 * VCC：接电源引脚，接3.3V直流电源
 * D0：SPI的时钟线SCLK（SCK）
 * D1：SPI的数据线SDIN (MOSI)
 * RES：复位接口，低电平初始化（复位），正常时高电平
 * DC：用来选择命令or数据，低电平命令，高电平数据（0:命令 1:数据）
 * CS：片选线，内部有上拉电阻，要做下拉使能低电平有效
 * CUBEMX 选择SPI接口，MODE:Transmit Only Master,16分频为5Mbps左右 MOSI:PA7/CSK:PB3
 * 需要增加两个GPIO，默认高电平，配置为下拉 定义DC和RES接口DC:PB4/RES:PB5
 *
 * 调用方法:
 * #include "ax_oled_1305.h"
 * AX_OLED_Init();
 * char* ch = "hello motor!";
 * AX_OLED_DispStr(0, 0, (uint8_t*)ch, 0);  // OLED指定位置显示ASCII字符串（6X8）
 ******************************************************************************
 */

#include "ax_oled_1305.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

#include "ax_oled_ascii.h"
#include "spi.h"
// OLED句柄定义
static AX_OLED_Config ax_oled_config = {0};

//  函数定义
static void OLED_Set_GPIO_DC(uint8_t dc);
static void OLED_Set_GPIO_RST(uint8_t rst);
static uint8_t SPI_Send_Byte(uint8_t dat);
static void OLED_WriteCmd(uint8_t data);
static void OLED_WriteData(uint8_t data);
static void OLED_Set_Pos(uint8_t x, uint8_t y);

static void OLED_Set_GPIO_DC(uint8_t dc) {
    if (dc) {
        HAL_GPIO_WritePin(ax_oled_config.dc_gpio_port, ax_oled_config.dc_gpio_pin, GPIO_PIN_SET);
    } else {
        HAL_GPIO_WritePin(ax_oled_config.dc_gpio_port, ax_oled_config.dc_gpio_pin, GPIO_PIN_RESET);
    }
}

static void OLED_Set_GPIO_RST(uint8_t rst) {
    if (rst) {
        HAL_GPIO_WritePin(ax_oled_config.reset_gpio_port, ax_oled_config.reset_gpio_pin, GPIO_PIN_SET);
    } else {
        HAL_GPIO_WritePin(ax_oled_config.reset_gpio_port, ax_oled_config.reset_gpio_pin, GPIO_PIN_RESET);
    }
}

/**
 * @简  述  OLED初始化
 * @参  数  无
 * @返回值  无
 */
void AX_OLED_Init(AX_OLED_Config* config) {
    // 深拷贝配置
    ax_oled_config.hspi = config->hspi;
    ax_oled_config.reset_gpio_port = config->reset_gpio_port;
    ax_oled_config.reset_gpio_pin = config->reset_gpio_pin;
    ax_oled_config.dc_gpio_port = config->dc_gpio_port;
    ax_oled_config.dc_gpio_pin = config->dc_gpio_pin;

    //__HAL_SPI_ENABLE(&hspi1);//已经自动开启

    // 启动传输
    SPI_Send_Byte(0xff);

    // 配置DC和RES引脚为输出
    OLED_Set_GPIO_RST(1);
    HAL_Delay(100);
    OLED_Set_GPIO_RST(0);
    HAL_Delay(200);
    OLED_Set_GPIO_RST(1);

    OLED_WriteCmd(0xAE);  //--turn off oled panel
    OLED_WriteCmd(0x00);  //---set low column address
    OLED_WriteCmd(0x10);  //---set high column address
    OLED_WriteCmd(0x40);  //--set start line address  Set Mapping RAM Display Start Line (0x00~0x3F)
    OLED_WriteCmd(0x81);  //--set contrast control register
    OLED_WriteCmd(0xEF);  // Set SEG Output Current Brightness
    OLED_WriteCmd(0xA1);  //--Set SEG/Column Mapping     0xa0左右反置 0xa1正常
    OLED_WriteCmd(0xC8);  // Set COM/Row Scan Direction   0xc0上下反置 0xc8正常
    OLED_WriteCmd(0xA6);  //--set normal display
    OLED_WriteCmd(0xA8);  //--set multiplex ratio(1 to 64)
    OLED_WriteCmd(0x3f);  //--1/64 duty
    OLED_WriteCmd(0xD3);  //-set display offset	Shift Mapping RAM Counter (0x00~0x3F)
    OLED_WriteCmd(0x00);  //-not offset
    OLED_WriteCmd(0xd5);  //--set display clock divide ratio/oscillator frequency
    OLED_WriteCmd(0x80);  //--set divide ratio, Set Clock as 100 Frames/Sec
    OLED_WriteCmd(0xD9);  //--set pre-charge period
    OLED_WriteCmd(0xF1);  // Set Pre-Charge as 15 Clocks & Discharge as 1 Clock
    OLED_WriteCmd(0xDA);  //--set com pins hardware configuration
    OLED_WriteCmd(0x12);
    OLED_WriteCmd(0xDB);  //--set vcomh
    OLED_WriteCmd(0x40);  // Set VCOM Deselect Level
    OLED_WriteCmd(0x20);  //-Set Page Addressing Mode (0x00/0x01/0x02)
    OLED_WriteCmd(0x02);  //
    OLED_WriteCmd(0x8D);  //--set Charge Pump enable/disable
    OLED_WriteCmd(0x14);  //--set(0x10) disable
    OLED_WriteCmd(0xA4);  // Disable Entire Display On (0xa4/0xa5)
    OLED_WriteCmd(0xA6);  // Disable Inverse Display On (0xa6/a7)
    OLED_WriteCmd(0xAF);  //--turn on oled panel

    AX_OLED_ClearScreen();
}

/**
 * @简  述  OLED屏幕清除
 * @参  数  无
 * @返回值	 无
 */
void AX_OLED_ClearScreen(void) {
    uint8_t i, n;
    for (i = 0; i < 8; i++) {
        OLED_WriteCmd(0xb0 + i);  // 设置页地址（0~7）
        OLED_WriteCmd(0x00);      // 设置显示位置—列低地址
        OLED_WriteCmd(0x10);      // 设置显示位置—列高地址
        for (n = 0; n < 128; n++)
            OLED_WriteData(0);
    }  // 更新显示
}

/**
 * @简  述  OLED指定位置显示一个ASCII字符（6X8）
 * @参  数  x：横坐标 0~123，超出范围不显示
 *          y：纵坐标 0~6，超出范围不显示
 *          ch：显示字符
 *          mode：显示模式，0-正常显示，1-反白显示
 * @返回值
 */
void AX_OLED_DispChar(uint8_t x, uint8_t y, uint8_t ch, uint8_t mode) {
    uint8_t c = 0, i = 0;

    c = ch - ' ';  // 得到偏移后的值

    // 显示范围控制，超出部分不显示
    if ((x < 123) && (y < 8)) {
        if (mode != 0) {
            OLED_Set_Pos(x, y);
            for (i = 0; i < 6; i++)
                OLED_WriteData(~ASCII6X8[c * 6 + i]);
        } else {
            OLED_Set_Pos(x, y);
            for (i = 0; i < 6; i++)
                OLED_WriteData(ASCII6X8[c * 6 + i]);
        }
    }
}

/**
 * @简  述  OLED指定位置显示ASCII字符串（8X16）,自动换行。
 * @参  数  x：横坐标 0~123，超出范围不显示
 *          y：纵坐标 0~7，超出范围不显示
 *          ch：字符串指针
 *          mode：显示模式，0-正常显示，1-反白显示
 * @返回值
 */
void AX_OLED_DispStr(uint8_t x, uint8_t y, uint8_t* ch, uint8_t mode) {
    uint8_t j = 0;

    if ((x < 123) && (y < 8)) {
        while (ch[j] != '\0') {
            AX_OLED_DispChar(x, y, ch[j], mode);
            x += 6;

            if (x > 122) {
                x = 0;
                y += 1;  // 当y>6时，超出部分不再显示
            }
            j++;
        }
    }
}
/**
 * @简  述  OLED指定位置显示一个数字（8X16）
 * @参  数  x：横坐标 0~120，超出范围不显示
 *          y：纵坐标 0~6，超出范围不显示
 *          num：显示的数字
 *          mode：显示模式，0-正常显示，1-反白显示
 * @返回值
 */
void AX_OLED_DispNum(uint8_t x, uint8_t y, uint8_t num, uint8_t mode)  // 显示单个数字
{
    if (num < 10) {
        AX_OLED_DispChar(x, y, ('0' + num), mode);
    }
}

/**
 * @简  述  OLED指定位置显示整数值（可显示负数）
 * @参  数  x：横坐标 0~122，超出范围不显示
 *          y：纵坐标 0~7，超出范围不显示
 *          value：显示的整数值，32位有符号整型
 *          digits：显示的数字位数（包含负号位置，如-123需要4位）
 *          mode：显示模式，0-正常显示，1-反白显示
 * @说  明  1. 如果显示负数，digits参数需要包含负号占用的位置
 *          2. 如果实际位数少于digits，会在左侧用0填充
 *          3. 如果实际位数多于digits，只会显示最低的digits位
 * @返回值  无
 */
void AX_OLED_DispInteger(uint8_t x, uint8_t y, int32_t value,
                         uint8_t digits, uint8_t mode) {
    uint8_t num_digits[10] = {0};  // 存储每一位数字
    uint8_t i;
    uint32_t temp;
    uint8_t start_x = x;
    uint8_t actual_digits = 0;

    // 参数检查
    if (digits == 0 || digits > 10) {
        return;  // 参数错误，直接返回
    }

    // 处理负数和提取数字
    if (value < 0) {
        // 显示负号
        AX_OLED_DispChar(start_x, y, '-', mode);
        start_x += 6;  // 负号占用一个字符宽度

        // 计算实际需要显示的数字位数（不包括负号）
        uint8_t num_digits_needed = digits - 1;

        // 提取绝对值
        temp = (uint32_t)(-value);

        // 提取每一位数字（从低位到高位）
        for (i = 0; i < num_digits_needed; i++) {
            num_digits[i] = temp % 10;
            temp /= 10;
            if (temp == 0) {
                break;
            }
        }

        // 从最高位开始显示数字
        for (i = 0; i < num_digits_needed; i++) {
            uint8_t digit_index = num_digits_needed - i - 1;
            AX_OLED_DispNum(start_x + (i * 6), y,
                            num_digits[digit_index], mode);
        }
    } else {
        // 处理正数
        temp = (uint32_t)value;

        // 提取每一位数字（从低位到高位）
        for (i = 0; i < digits; i++) {
            num_digits[i] = temp % 10;
            temp /= 10;
            actual_digits++;
            if (temp == 0) {
                break;
            }
        }

        // 如果实际位数少于要求位数，用0填充
        for (i = actual_digits; i < digits; i++) {
            num_digits[i] = 0;
        }

        // 从最高位开始显示数字
        for (i = 0; i < digits; i++) {
            uint8_t digit_index = digits - i - 1;
            AX_OLED_DispNum(start_x + (i * 6), y,
                            num_digits[digit_index], mode);
        }
    }
}

/**
 * @简  述  OLED指定位置显示一个ASCII字符（8X16）
 * @参  数  x：横坐标 0~120，超出范围不显示
 *          y：纵坐标 0~6，超出范围不显示
 *          ch：显示字符
 *          mode：显示模式，0-正常显示，1-反白显示
 * @返回值
 */
void AX_OLED_Disp16Char(uint8_t x, uint8_t y, uint8_t ch, uint8_t mode) {
    uint8_t c = 0, i = 0;

    c = ch - ' ';  // 得到偏移后的值

    // 显示范围控制，超出部分不显示
    if ((x < 121) && (y < 7)) {
        if (mode != 0) {
            OLED_Set_Pos(x, y);
            for (i = 0; i < 8; i++)
                OLED_WriteData(~ASCII8X16[c * 16 + i]);
            OLED_Set_Pos(x, y + 1);
            for (i = 0; i < 8; i++)
                OLED_WriteData(~ASCII8X16[c * 16 + i + 8]);
        } else {
            OLED_Set_Pos(x, y);
            for (i = 0; i < 8; i++)
                OLED_WriteData(ASCII8X16[c * 16 + i]);
            OLED_Set_Pos(x, y + 1);
            for (i = 0; i < 8; i++)
                OLED_WriteData(ASCII8X16[c * 16 + i + 8]);
        }
    }
}

/**
 * @简  述  OLED指定位置显示ASCII字符串（8X16）,自动换行。
 * @参  数  x：横坐标 0~120，超出范围不显示
 *          y：纵坐标 0~6，超出范围不显示
 *          ch：字符串指针
 *          mode：显示模式，0-正常显示，1-反白显示
 * @返回值
 */
void AX_OLED_Disp16Str(uint8_t x, uint8_t y, uint8_t* ch, uint8_t mode) {
    uint8_t j = 0;

    if ((x < 121) && (y < 7)) {
        while (ch[j] != '\0') {
            AX_OLED_Disp16Char(x, y, ch[j], mode);
            x += 8;

            if (x > 120) {
                x = 0;
                y += 2;  // 当y>6时，超出部分不再显示
            }
            j++;
        }
    }
}

/**
 * @简  述  OLED指定位置显示一个数字（8X16）
 * @参  数  x：横坐标 0~120，超出范围不显示
 *          y：纵坐标 0~6，超出范围不显示
 *          num：显示的数字
 *          mode：显示模式，0-正常显示，1-反白显示
 * @返回值
 */
void AX_OLED_Disp16Num(uint8_t x, uint8_t y, uint8_t num, uint8_t mode)  // 显示单个数字
{
    if (num < 10) {
        AX_OLED_Disp16Char(x, y, ('0' + num), mode);
    }
}

/**
 * @简  述  OLED指定位置像素显示整数值,可显示负数）8*16
 * @参  数  x：横坐标 0~120，超出范围不显示
 *          y：纵坐标 0~6，超出范围不显示
 *          value：显示的整数值，32位有符号整型
 *          digits：显示的数字位数（包含负号位置，如-123需要4位）
 *          mode：显示模式，0-正常显示，1-反白显示
 * @说  明  1. 如果显示负数，digits参数需要包含负号占用的位置
 *          2. 如果实际位数少于digits，会在左侧用0填充
 *          3. 如果实际位数多于digits，只会显示最低的digits位
 * @返回值  无
 */
void AX_OLED_Disp16Integer(uint8_t x, uint8_t y, int32_t value,
                           uint8_t digits, uint8_t mode) {
    uint8_t num_digits[10] = {0};  // 存储每一位数字
    uint8_t i;
    uint32_t temp;
    uint8_t start_x = x;
    uint8_t actual_digits = 0;

    // 参数检查
    if (digits == 0 || digits > 10) {
        return;  // 参数错误，直接返回
    }

    // 处理负数和提取数字
    if (value < 0) {
        // 显示负号
        AX_OLED_Disp16Char(start_x, y, '-', mode);
        start_x += 8;  // 负号占用一个字符宽度（16点阵字体通常8像素宽）

        // 计算实际需要显示的数字位数（不包括负号）
        uint8_t num_digits_needed = digits - 1;
        if (num_digits_needed == 0) {
            return;  // 只有负号没有数字，直接返回
        }

        // 提取绝对值（注意处理INT32_MIN边界情况）
        if (value == INT32_MIN) {
            temp = (uint32_t)INT32_MAX + 1;
        } else {
            temp = (uint32_t)(-value);
        }

        // 提取每一位数字（从低位到高位）
        for (i = 0; i < num_digits_needed; i++) {
            num_digits[i] = temp % 10;
            temp /= 10;
            if (temp == 0) {
                actual_digits = i + 1;
                break;
            }
        }

        // 如果循环结束但temp不为0，说明数字位数多于需要显示的位数
        if (temp > 0) {
            actual_digits = num_digits_needed;  // 显示所有请求的位数
        }

        // 如果实际位数少于要求位数，用0填充
        for (i = actual_digits; i < num_digits_needed; i++) {
            num_digits[i] = 0;
        }

        // 从最高位开始显示数字
        for (i = 0; i < num_digits_needed; i++) {
            uint8_t digit_index = num_digits_needed - i - 1;
            AX_OLED_Disp16Num(start_x + (i * 8), y,
                              num_digits[digit_index], mode);
        }
    } else {
        // 处理正数
        temp = (uint32_t)value;

        // 提取每一位数字（从低位到高位）
        for (i = 0; i < digits; i++) {
            num_digits[i] = temp % 10;
            temp /= 10;
            actual_digits++;
            if (temp == 0) {
                break;
            }
        }

        // 如果实际位数少于要求位数，用0填充
        for (i = actual_digits; i < digits; i++) {
            num_digits[i] = 0;
        }

        // 从最高位开始显示数字
        for (i = 0; i < digits; i++) {
            uint8_t digit_index = digits - i - 1;
            AX_OLED_Disp16Num(start_x + (i * 8), y,
                              num_digits[digit_index], mode);
        }
    }
}

/**
 * @简  述  OLED指定位置显示一个汉字（16X16）
 * @参  数  x：横坐标 0~112，超出范围不显示
 *          y：纵坐标 0~6，超出范围不显示
 *          pbuf：汉字编码数据指针
 *          mode：显示模式，0-正常显示，1-反白显示
 * @返回值
 */
void AX_OLED_DispChinese(uint8_t x, uint8_t y, const uint8_t* pbuf, uint8_t mode) {
    uint8_t t;

    if ((x < 113) && (y < 7)) {
        if (mode) {
            OLED_Set_Pos(x, y);
            for (t = 0; t < 16; t++) {
                OLED_WriteData(~*pbuf++);
            }
            OLED_Set_Pos(x, y + 1);
            for (t = 0; t < 16; t++) {
                OLED_WriteData(~*pbuf++);
            }
        } else {
            OLED_Set_Pos(x, y);
            for (t = 0; t < 16; t++) {
                OLED_WriteData(*pbuf++);
            }
            OLED_Set_Pos(x, y + 1);
            for (t = 0; t < 16; t++) {
                OLED_WriteData(*pbuf++);
            }
        }
    }
}

/**
 * @简  述  OLED指定位置显示一个指定尺寸照片
 * @参  数  x：图片起始点横坐标，0~127
 *          y：图片起始点纵坐标，0~7
 *          xsize：X轴图片尺寸，1~128
 *          ysize：Y轴图片尺寸，1~8
 *          pbuf：照片数据指针
 *          mode：显示模式，0-正常显示，1-反白显示
 * @返回值
 */
void AX_OLED_DispPicture(uint8_t x, uint8_t y, uint8_t xsize, uint8_t ysize, const uint8_t* pbuf, uint8_t mode) {
    uint8_t tx, ty;

    // 参数过滤
    if (x > 127 || y > 7)
        return;
    if ((x + xsize) > 128 || (y + ysize) > 8)
        return;

    if (mode) {
        for (ty = y; ty < ysize; ty++) {
            OLED_Set_Pos(x, ty);
            for (tx = x; tx < xsize; tx++) {
                OLED_WriteData(~*pbuf++);
            }
        }
    } else {
        for (ty = y; ty < ysize; ty++) {
            OLED_Set_Pos(x, ty);
            for (tx = x; tx < xsize; tx++) {
                OLED_WriteData(*pbuf++);
            }
        }
    }
}

/* 底层操作函数-----------------------------------------------------*/
/**
 * @简  述  SPI3写入读取一个字节函数
 * @参  数  dat：要写入的字节
 * @返回值  读出字节
 */
/* 底层操作函数-----------------------------------------------------*/
/**
 * @简  述  SPI1写入读取一个字节函数
 * @参  数  dat：要写入的字节
 * @返回值  读出字节
 */
static uint8_t SPI_Send_Byte(uint8_t dat) {
    uint8_t rx_data;

    /* 使用HAL_SPI_TransmitReceive进行全双工SPI通信 */
    HAL_StatusTypeDef status = HAL_SPI_TransmitReceive(ax_oled_config.hspi, &dat, &rx_data, 1, HAL_MAX_DELAY);

    /* 可选：检查传输状态 */
    if (status != HAL_OK) {
        // 处理错误，这里可以根据需要添加错误处理代码
        // 例如：Error_Handler();
    }

    /* 返回接收到的数据 */
    return rx_data;
}

/**
 * @简  述  设置OLED的DC引脚状态
 * @参  数  dc：DC引脚状态，0表示命令模式，1表示数据模式
 * @返回值  无
 * @说  明  DC引脚用于区分SPI传输的数据类型：
 *          - 低电平(0)：SPI发送的是命令
 *          - 高电平(1)：SPI发送的是数据
 *          此函数通过配置结构体中的GPIO端口和引脚信息来控制实际硬件引脚
 */
static void OLED_WriteCmd(uint8_t data) {
    OLED_Set_GPIO_DC(0);
    SPI_Send_Byte(data);  // 发送片擦除命令
    OLED_Set_GPIO_DC(1);
}

/**
 * @brief
 * @param  None
 * @retval None
 */
static void OLED_WriteData(uint8_t data) {
    SPI_Send_Byte(data);  // 发送片擦除命令
}

static void OLED_Set_Pos(uint8_t x, uint8_t y) {
    OLED_WriteCmd(0xb0 + y);
    OLED_WriteCmd(((x & 0xf0) >> 4) | 0x10);
    OLED_WriteCmd((x & 0x0f) | 0x01);
}

/**
 * @brief  在OLED指定位置显示浮点数（支持完整int32范围，单函数整合版）
 * @param  x：横坐标 0~122，超出范围不显示
 *         y：纵坐标 0~7，超出范围不显示
 *         value：显示的浮点数值
 *         inte：整数位数（包含负号占位），范围>0且inte+deci<=12
 *         deci：小数位数，范围inte+deci<=20
 *         mode：显示模式，0-正常显示，1-反白显示
 * @note   1. OLED为128x64分辨率，6x8字体每行最多21个字符
 *         2. int32范围：-2147483648 ~ 2147483647（10位数字）
 *         3. 整数位数需包含负号，如-12.34的整数位应设为3
 *         4. 自动处理-0.00、溢出、位数不足等异常场景
 */
void AX_OLED_DispFloat(uint8_t x, uint8_t y, float value, uint8_t inte, uint8_t deci, uint8_t mode) {
    // 核心显示缓冲区：负号(1)+整数(10)+小数点(1)+小数(10)+'\0' = 23字节，留冗余确保安全
    char disp_buf[24] = {0};
    uint8_t buf_idx = 0;  // 缓冲区写入索引

    // -------------------------- 1. 快速参数合法性检查 --------------------------
    // OLED坐标范围：x(0~122)、y(0~7)；整数位必须>0；总位数不超过20（inte+deci<=20）
    if (x > 122 || y > 7 || inte == 0 || deci > 10 || (inte + deci) > 20) {
        strcpy(disp_buf, "ERR:X>122/Y>7/LEN>20");  // 长度超限错误
        AX_OLED_DispStr(x, y, (uint8_t*)disp_buf, mode);
        return;  // 参数非法，直接返回不显示
    }

    // 计算总显示字符数：整数位 + 小数点（如果有小数）
    uint8_t total_chars = inte + (deci > 0 ? 1 : 0);
    // OLED每行最大字符数（6x8字体：128像素/6像素 per char ≈21）
    if (total_chars > 21) {
        strcpy(disp_buf, "ERR:LEN");  // 长度超限错误
        AX_OLED_DispStr(x, y, (uint8_t*)disp_buf, mode);
        return;
    }

    // -------------------------- 2. 处理特殊浮点值（NaN/INF） --------------------------
    if (isnanf(value) || isinff(value)) {
        strcpy(disp_buf, "ERR:SPE");  // 特殊值错误
        AX_OLED_DispStr(x, y, (uint8_t*)disp_buf, mode);
        return;
    }

    // -------------------------- 3. 处理接近零的负值（避免显示-0.00） --------------------------
    const float zero_threshold = 1e-8f;  // 零值判断阈值
    if (value > -zero_threshold && value < zero_threshold) {
        value = 0.0f;
    }

    // -------------------------- 4. 处理符号位 --------------------------
    uint8_t is_negative = (value < 0.0f);  // 是否为负数
    if (is_negative) {
        disp_buf[buf_idx++] = '-';  // 写入负号
        value = -value;             // 转为正数后续处理
        // 负号占用1位，整数位至少需留1位给数字（避免仅显示负号）
        if (inte == 1) {
            strcpy(disp_buf, "ERR:SIG");  // 符号位错误
            AX_OLED_DispStr(x, y, (uint8_t*)disp_buf, mode);
            return;
        }
    }

    // -------------------------- 5. 计算小数缩放因子（避免溢出） --------------------------
    uint64_t scale = 1;  // 缩放因子（如小数位2则scale=100）
    // 限制小数位最大10位，避免64位无符号数溢出
    deci = (deci > 10) ? 10 : deci;
    for (uint8_t i = 0; i < deci; i++) {
        if (scale > UINT64_MAX / 10) {  // 检查溢出，提前终止
            deci = i;
            break;
        }
        scale *= 10;
    }

    // -------------------------- 6. 数值缩放与四舍五入 --------------------------
    // 用double提升精度，避免float精度丢失；+0.5实现四舍五入
    double scaled = (double)value * (double)scale + 0.5;
    // 检查缩放后是否超出int64范围（避免后续计算溢出）
    if (scaled > INT64_MAX || scaled < INT64_MIN) {
        strcpy(disp_buf + buf_idx, "ERR:OVF!  ");  // 溢出错误
        AX_OLED_DispStr(x, y, (uint8_t*)disp_buf, mode);
        return;
    }
    int64_t scaled_value = (int64_t)scaled;

    // -------------------------- 7. 分离整数和小数部分 --------------------------
    int64_t int_part = 0;   // 整数部分
    int64_t frac_part = 0;  // 小数部分
    if (deci > 0) {
        int_part = scaled_value / (int64_t)scale;   // 取整数部分
        frac_part = scaled_value % (int64_t)scale;  // 取小数部分
        // 四舍五入后小数部分超界修正（如scale=100，frac_part=100则进1）
        if (frac_part >= (int64_t)scale) {
            int_part += 1;
            frac_part -= (int64_t)scale;
        }
    } else {
        int_part = scaled_value;  // 无小数时直接取整
        frac_part = 0;
    }

    // -------------------------- 8. 检查整数部分是否超出int32范围 --------------------------
    if (int_part > INT32_MAX || int_part < INT32_MIN) {
        strcpy(disp_buf + buf_idx, "ERR:OVF");
        AX_OLED_DispStr(x, y, (uint8_t*)disp_buf, mode);
        return;
    }

    // -------------------------- 9. 计算整数部分实际位数（查表法提升效率） --------------------------
    uint8_t actual_inte = 0;
    int32_t temp_int = (int32_t)int_part;
    if (temp_int == 0) {
        actual_inte = 1;  // 0的实际位数为1
    } else {
        // 查表法替代循环，提升执行效率（针对int32范围）
        if (temp_int >= 1000000000)
            actual_inte = 10;
        else if (temp_int >= 100000000)
            actual_inte = 9;
        else if (temp_int >= 10000000)
            actual_inte = 8;
        else if (temp_int >= 1000000)
            actual_inte = 7;
        else if (temp_int >= 100000)
            actual_inte = 6;
        else if (temp_int >= 10000)
            actual_inte = 5;
        else if (temp_int >= 1000)
            actual_inte = 4;
        else if (temp_int >= 100)
            actual_inte = 3;
        else if (temp_int >= 10)
            actual_inte = 2;
        else
            actual_inte = 1;
    }

    // -------------------------- 10. 检查整数位数是否足够（含负号） --------------------------
    uint8_t required_inte = is_negative ? (actual_inte + 1) : actual_inte;
    if (inte < required_inte) {
        strcpy(disp_buf + buf_idx, "ERR:PAR");  // 参数不足错误
        AX_OLED_DispStr(x, y, (uint8_t*)disp_buf, mode);
        return;
    }

    // -------------------------- 11. 转换整数部分为字符串（自动补前导零） --------------------------
    uint8_t inte_digits = inte - (is_negative ? 1 : 0);  // 扣除负号后的整数显示位数
    char temp_buf[12] = {0};                             // 临时缓冲区：存放逆序整数
    uint8_t temp_idx = 0;

    // 提取整数数字（逆序存入临时缓冲区）
    uint64_t uint_int = (uint64_t)int_part;  // 转为无符号避免负数处理
    if (uint_int == 0) {
        temp_buf[temp_idx++] = '0';
    } else {
        while (uint_int > 0 && temp_idx < sizeof(temp_buf) - 1) {
            temp_buf[temp_idx++] = '0' + (uint_int % 10);
            uint_int /= 10;
        }
    }

    // 填充前导零（补足指定整数位数）
    uint8_t zeros = inte_digits - temp_idx;
    for (uint8_t i = 0; i < zeros; i++) {
        disp_buf[buf_idx++] = '0';
    }

    // 反转临时缓冲区，写入最终显示缓冲区
    for (int8_t i = temp_idx - 1; i >= 0; i--) {
        disp_buf[buf_idx++] = temp_buf[i];
    }

    // -------------------------- 12. 转换小数部分为字符串 --------------------------
    if (deci > 0 && buf_idx < sizeof(disp_buf) - 2) {  // 留位置给小数点和终止符
        disp_buf[buf_idx++] = '.';                     // 写入小数点

        // 提取小数数字（逆序存入临时缓冲区）
        memset(temp_buf, 0, sizeof(temp_buf));  // 清空临时缓冲区
        temp_idx = 0;
        uint64_t uint_frac = (uint64_t)frac_part;
        if (uint_frac == 0) {
            temp_buf[temp_idx++] = '0';
        } else {
            while (uint_frac > 0 && temp_idx < sizeof(temp_buf) - 1) {
                temp_buf[temp_idx++] = '0' + (uint_frac % 10);
                uint_frac /= 10;
            }
        }

        // 填充小数前导零（补足指定小数位数）
        zeros = deci - temp_idx;
        for (uint8_t i = 0; i < zeros; i++) {
            disp_buf[buf_idx++] = '0';
        }

        // 反转临时缓冲区，写入最终显示缓冲区
        for (int8_t i = temp_idx - 1; i >= 0; i--) {
            disp_buf[buf_idx++] = temp_buf[i];
        }
    }

    // -------------------------- 13. 确保字符串终止并显示 --------------------------
    disp_buf[buf_idx] = '\0';  // 字符串终止符
    AX_OLED_DispStr(x, y, (uint8_t*)disp_buf, mode);
}

/**
 * @简  述  在OLED指定位置显示浮点数（自动计算整数位数，无位数限制）
 * @参  数  x：横坐标 0~122，超出范围不显示
 *         y：纵坐标 0~7，超出范围不显示
 *         value：显示的浮点数值
 *         deci：期望显示的小数位数，最终会限制为最大6位
 *         mode：显示模式，0-正常显示，1-反白显示
 */
void AX_OLED_DispFloatAuto(uint8_t x, uint8_t y, float value, uint8_t deci, uint8_t mode) {
    // 1. 坐标越界检查，超出范围直接返回0（不显示）
    if (x > 122 || y > 7) {
        return;
    }

    // 2. 限制小数位数最大为6位（满足需求：小数点后最多6位）
    if (deci > 6) {
        deci = 6;
    }

    uint8_t inte = 1;  // 默认整数位数为1（至少显示0）
    float abs_value = (value < 0) ? -value : value;
    // 提取整数部分（float的整数部分最大约2^24，用uint32_t足够存储）
    uint32_t int_part = (uint32_t)abs_value;

    // 3. 动态计算整数位数（去掉8位限制，适配float最大范围）
    if (int_part > 0) {
        inte = 0;
        uint32_t temp = int_part;
        while (temp > 0) {
            temp /= 10;
            inte++;
        }
    }

    // 4. 符号位处理（负数需额外占1位显示负号）
    uint8_t sign_bit = (value < 0) ? 1 : 0;
    inte = inte + sign_bit;  // 整数位包括符号位
    // 总显示位数 = 整数位 + 符号位 + 小数位 + 小数点（1位）
    uint8_t total_digits = inte + sign_bit + deci + 1;

    // 5. 仅做小数位的保底防护（确保小数位非负，无整数位限制）
    int temp_deci = deci;
    // 若总位数超OLED单行长（仅做降级，不限制整数位）
    while (total_digits > 16 && temp_deci > 0) {  // 16为OLED常规单行最大显示位数
        temp_deci--;
        total_digits = inte + sign_bit + temp_deci + 1;
    }
    deci = (uint8_t)temp_deci;

    // 6. 调用底层显示函数（传入实际计算的整数位数，无+1错误）
    // printf("debug:%d,%d,%lf,%d,%d,%d\n", x, y, value, inte, deci, mode);
    AX_OLED_DispFloat(x, y, value, inte, deci, mode);
}

/**
 * @简  述  擦除OLED指定行（页）
 * @参  数  line: 要擦除的行号（0~7，对应8行）
 * @返回值	 无
 */
void AX_OLED_ClearLine(uint8_t line) {
    uint8_t n;

    // 参数检查
    if (line > 7) return;

    // 设置页地址
    OLED_WriteCmd(0xb0 + line);  // 设置页地址
    OLED_WriteCmd(0x00);         // 设置列低地址
    OLED_WriteCmd(0x10);         // 设置列高地址

    // 清除该页所有数据（128列）
    for (n = 0; n < 128; n++) {
        OLED_WriteData(0);
    }
}

/******************* (C) 版权 2023 ************************/
