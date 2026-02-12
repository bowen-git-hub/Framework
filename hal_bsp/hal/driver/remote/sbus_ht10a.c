/**
 ******************************************************************************
 * @文件    sbus_ht10a.c
 * @作者    SBUS解析器
 * @版本    V1.0
 * @日期    2025-11-09
 * @概要    SBUS协议解析器实现文件
 *
 * 通道值对应遥感映射关系
 * channels_raw[0] 通道1: 右摇杆X 192~992~1792
 * channels_raw[1] 通道2: 右摇杆Y 192~992~1792
 * channels_raw[2] 通道3: 左摇杆Y 192~992~1792
 * channels_raw[3] 通道4: 左摇杆X 192~992~1792
 * channels_raw[4] 通道5: SWA-5  192/992/1792   三档自弹
 * channels_raw[5] 通道6: SWB-6  192/1792       两档
 * channels_raw[6] 通道7: SWC-7  192/1792       两档
 * channels_raw[7] 通道8: SWD-8  192/992/1792   三档
 * channels_raw[8] 通道9: VRA    192~1792       旋钮
 * channels_raw[9] 通道10: VRB    192~1792       旋钮
 * channels_raw[x] 通道11~16                    保留
 *
 */

#include "sbus_ht10a.h"
#include <stdio.h>
#include <string.h>

// #include "main.h"  // HAL_GetTick

static SBUS_HandleTypeDef sbus1 = {0};
SBUS_HandleTypeDef* hsbus1 = &sbus1;

static uint8_t uart_rx_buffer[32] = {0};
static UART_HandleTypeDef* huart = NULL;

/**
 * @brief  初始化SBUS解析器
 * @param  hsbus: SBUS句柄
 */
void SBUS_Init(UART_HandleTypeDef* huart_ptr) {
    huart = huart_ptr;
    hsbus1->last_update = 0;

    // 初始化UART DMA接收
    HAL_UARTEx_ReceiveToIdle_DMA(huart, uart_rx_buffer, sizeof(uart_rx_buffer));  // 启动DMA接收,用来接收遥控器数据
    __HAL_DMA_DISABLE_IT(huart->hdmarx, DMA_IT_HT);                               // 关闭DMA接收缓冲区半满中断
    printf("SBUS_Init cussessful.\r");
}

/**
 * @brief  更新SBUS数据
 * @param  hsbus: SBUS句柄
 * @param  data: 原始数据指针
 * @param  len: 数据长度
 * @retval 解析是否成功
 */
bool SBUS_Update(SBUS_HandleTypeDef* hsbus, const uint8_t* data, uint32_t len) {
    if (hsbus == NULL || data == NULL || len != SBUS_FRAME_LENGTH) {
        return false;
    }

    // 检查帧头和帧尾
    if (data[0] != SBUS_HEADER || data[SBUS_FRAME_LENGTH - 1] != SBUS_FOOTER) {
        return false;
    }

    // 拷贝原始数据
    memcpy(&hsbus->raw_data, data, sizeof(SBUS_RawData));

    // 解析通道数据

    SBUS_ParseChannels(hsbus);

    return true;
}

/**
 * @brief  解析通道数据
 * @param  hsbus: SBUS句柄
 */
void SBUS_ParseChannels(SBUS_HandleTypeDef* hsbus) {
    if (hsbus == NULL) {
        return;
    }

    const uint8_t* data = hsbus->raw_data.data;

    // 解析16个通道的11位数据
    hsbus->channels_raw[0] = ((data[0] | data[1] << 8) & 0x07FF);
    hsbus->channels_raw[1] = ((data[1] >> 3 | data[2] << 5) & 0x07FF);
    hsbus->channels_raw[2] = ((data[2] >> 6 | data[3] << 2 | data[4] << 10) & 0x07FF);
    hsbus->channels_raw[3] = ((data[4] >> 1 | data[5] << 7) & 0x07FF);
    hsbus->channels_raw[4] = ((data[5] >> 4 | data[6] << 4) & 0x07FF);
    hsbus->channels_raw[5] = ((data[6] >> 7 | data[7] << 1 | data[8] << 9) & 0x07FF);
    hsbus->channels_raw[6] = ((data[8] >> 2 | data[9] << 6) & 0x07FF);
    hsbus->channels_raw[7] = ((data[9] >> 5 | data[10] << 3) & 0x07FF);
    hsbus->channels_raw[8] = ((data[11] | data[12] << 8) & 0x07FF);
    hsbus->channels_raw[9] = ((data[12] >> 3 | data[13] << 5) & 0x07FF);
    hsbus->channels_raw[10] = ((data[13] >> 6 | data[14] << 2 | data[15] << 10) & 0x07FF);
    hsbus->channels_raw[11] = ((data[15] >> 1 | data[16] << 7) & 0x07FF);
    hsbus->channels_raw[12] = ((data[16] >> 4 | data[17] << 4) & 0x07FF);
    hsbus->channels_raw[13] = ((data[17] >> 7 | data[18] << 1 | data[19] << 9) & 0x07FF);
    hsbus->channels_raw[14] = ((data[19] >> 2 | data[20] << 6) & 0x07FF);
    hsbus->channels_raw[15] = ((data[20] >> 5 | data[21] << 3) & 0x07FF);

    // 解析标志位
    const uint8_t flags = hsbus->raw_data.flags;
    hsbus->ch17 = (flags & SBUS_FLAG_CH17) != 0;
    hsbus->ch18 = (flags & SBUS_FLAG_CH18) != 0;
    hsbus->frame_lost = (flags & SBUS_FLAG_FRAME_LOST) != 0;
    hsbus->failsafe = (flags & SBUS_FLAG_FAILSAFE) != 0;
    hsbus->last_update = HAL_GetTick();
    // printf("hbus last update set %ld,SBUS_ParseChannels cussessful.\r", hsbus->last_update);
}

/**
 * @brief  将SBUS数据转换为统一遥控器格式
 * @param  hur: 统一遥控器句柄
 * @param  hsbus: SBUS句柄
 */
void SBUS_To_UniRemote(UniRemote_HandleTypeDef* hur, const SBUS_HandleTypeDef* hsbus) {
    if (hur == NULL || hsbus == NULL) {
        return;
    }

    //  遥控信号是否丢失
    hur->frame_lost = hsbus->frame_lost;  // 丢失遥控信号
    //  遥控模块是否连接
    if (HAL_GetTick() - hsbus->last_update > 200) {
        hur->frame_connected = false;
    } else {
        hur->frame_connected = true;
    }

    hur->last_update = hsbus->last_update;

    if (hur->frame_connected && !hur->frame_lost) {
        // 转换成标准范围数据
        hur->right_x = SBUS_To_RBUS_Range(hsbus->channels_raw[0]);  // 通道1: 右摇杆X
        hur->right_y = SBUS_To_RBUS_Range(hsbus->channels_raw[1]);  // 通道2: 右摇杆Y
        hur->left_y = SBUS_To_RBUS_Range(hsbus->channels_raw[2]);   // 通道3: 左摇杆Y
        hur->left_x = SBUS_To_RBUS_Range(hsbus->channels_raw[3]);   // 通道4: 左摇杆X

        // 映射开关状态
        hur->switch_sound = SBUS_Map_RBUS_Switch(hsbus->channels_raw[4]);  // 通道5: 左1开关（喇叭）
        hur->switch_pid = SBUS_Map_RBUS_Switch(hsbus->channels_raw[5]);    // 通道6: 右2开关（PID模式）
        hur->switch_mode = SBUS_Map_RBUS_Switch(hsbus->channels_raw[6]);   // 通道7: 左2开关（避障）
        hur->switch_power = SBUS_Map_RBUS_Switch(hsbus->channels_raw[7]);  // 通道8: 右1开关（运行模式）
    } else {
        // 丢失信号时，将所有数据置零
        hur->right_x = RBUS_CH_MID;
        hur->right_y = RBUS_CH_MID;
        hur->left_y = RBUS_CH_MID;
        hur->left_x = RBUS_CH_MID;
        hur->switch_mode = RBUS_SWITCH_OFF;
        hur->switch_power = RBUS_SWITCH_OFF;
        hur->switch_pid = RBUS_SWITCH_OFF;
        hur->switch_sound = RBUS_SWITCH_OFF;
    }
}

/**
 * @brief  将SBUS原始值转换RBUS标准范围
 * @param  sbus_value: SBUS原始值 (192-1792)
 * @retval RBUS标准范围值 (-100 ~ 100)
 */
int16_t SBUS_To_RBUS_Range(uint16_t sbus_value) {
    int32_t result;

    // 限制输入范围不越界
    sbus_value = (sbus_value < SBUS_CH_MIN) ? SBUS_CH_MIN : (sbus_value > SBUS_CH_MAX) ? SBUS_CH_MAX
                                                                                       : sbus_value;

    // 线性映射计算
    // 公式: output = (input - mid) * range_ratio
    if (sbus_value > SBUS_CH_MID) {
        // 正值范围计算
        result = (int32_t)(sbus_value - SBUS_CH_MID) * RBUS_CH_MAX;
        result /= (SBUS_CH_MAX - SBUS_CH_MID);
    } else if (sbus_value < SBUS_CH_MID) {
        // 负值范围计算
        result = (int32_t)(sbus_value - SBUS_CH_MID) * RBUS_CH_MAX;
        result /= (SBUS_CH_MID - SBUS_CH_MIN);
    } else {
        // 中位值
        result = RBUS_CH_MID;
    }

    return (int16_t)result;
}

/**
 * @brief  映射开关状态 192:0 992:1 1992:2
 * @param  channel_value: 通道原始值
 * @retval 开关状态 (SWITCH_UP/MID/DOWN)
 */
uint8_t SBUS_Map_RBUS_Switch(uint16_t channel_value) {
    // 根据通道值映射三档开关 0是转换后的中间位置
    if (channel_value == SBUS_CH_MIN)
        return RBUS_SWITCH_DOWN;  // 下档
    else if (channel_value == SBUS_CH_MAX)
        return RBUS_SWITCH_UP;  // 上档
    else
        return RBUS_SWITCH_MID;  // 中档
}

/**
 * @brief  打印通道数据（详细版）
 * @param  hsbus: SBUS句柄
 */
void SBUS_PrintChannels(const SBUS_HandleTypeDef* hsbus) {
    if (hsbus == NULL) {
        printf("SBUS: Invalid handle\n");
        return;
    }

    if (!SBUS_IsConnected(hsbus)) {
        printf("SBUS: Disconnected\n");
        return;
    }

    printf("=== SBUS All Channels ===\n");
    printf("Raw Values (0-2047):\n");

    // 打印原始值
    for (int i = 0; i < 16; i++) {
        printf("CH%2d: %4d", i + 1, hsbus->channels_raw[i]);
        if ((i + 1) % 4 == 0) {
            printf("\n");
        } else {
            printf(" | ");
        }
    }

    printf("Digital: CH17=%d, CH18=%d\n", hsbus->ch17, hsbus->ch18);
    printf("Status: Frame Lost=%d, Failsafe=%d\n", hsbus->frame_lost, hsbus->failsafe);
    printf("Connection: %s, Last Update: %lu ms\n",
           SBUS_IsConnected(hsbus) ? "OK" : "Lost", hsbus->last_update);
}

/**
 * @brief  获取通道原始值
 * @param  hsbus: SBUS句柄
 * @param  channel: 通道号 (0-15)
 * @retval 通道原始值 (0-2047)，如果错误返回0
 */
uint16_t SBUS_GetChannelRaw(const SBUS_HandleTypeDef* hsbus, uint8_t channel) {
    if (hsbus == NULL || channel >= 16) {
        return 0;
    }

    return hsbus->channels_raw[channel];
}

/**
 * @brief  获取连接状态
 * @param  hsbus: SBUS句柄
 * @retval 连接状态
 */
bool SBUS_IsConnected(const SBUS_HandleTypeDef* hsbus) {
    if (hsbus == NULL) {
        return false;
    }
    // 如果超时，返回false
    return (HAL_GetTick() - hsbus->last_update < SBUS_TIMEOUT_MS) ? true : false;
}

/**
 * @brief  获取帧丢失状态
 * @param  hsbus: SBUS句柄
 * @retval 帧丢失状态
 */
bool SBUS_IsFrameLost(const SBUS_HandleTypeDef* hsbus) {
    if (hsbus == NULL) {
        return true;
    }

    return hsbus->frame_lost;
}

/**
 * @brief  获取故障安全状态
 * @param  hsbus: SBUS句柄
 * @retval 故障安全状态
 */
bool SBUS_IsFailsafe(const SBUS_HandleTypeDef* hsbus) {
    if (hsbus == NULL) {
        return true;
    }

    return hsbus->failsafe;
}

/**
 * @brief  获取数字通道状态
 * @param  hsbus: SBUS句柄
 * @param  channel: 数字通道号 (17或18)
 * @retval 数字通道状态
 */
bool SBUS_GetDigitalChannel(const SBUS_HandleTypeDef* hsbus, uint8_t channel) {
    if (hsbus == NULL) {
        return false;
    }

    if (channel == 17) {
        return hsbus->ch17;
    } else if (channel == 18) {
        return hsbus->ch18;
    }

    return false;
}

void SBUS_RxEventCallback(UART_HandleTypeDef* huart, uint16_t Size) {
    static uint32_t count = 0;
    static uint32_t error = 0;
    // printf("SBUS_RxEventCallback,Size:%d.\r", Size);
    if (SBUS_Update(hsbus1, uart_rx_buffer, Size)) {
        count++;
        //  sprintf(msg, "s:%2d,c:%5ld,e:%2ld\n", Size, count, error);
        //  AX_OLED_Disp16Str(0, 5, (u_int8_t*)msg, 0);  // OLED指定位置显示ASCII字符串（8X16）

        // HAL_UART_Transmit_IT(&huart1, (uint8_t*)msg, strlen(msg));//DEBUG
        //   OLED_ShowHexArray(0, 4, (uint8_t *)&(sbus_parser.raw_data), Size);
    } else {
        error++;

        // sprintf(msg, "s:%d, c:%ld, e:%ld", Size, count, error);
        //  OLED_ShowString(0, 2, msg);
    }

    HAL_UARTEx_ReceiveToIdle_DMA(huart, uart_rx_buffer, sizeof(uart_rx_buffer));  // 启动DMA接收
    __HAL_DMA_DISABLE_IT(huart->hdmarx, DMA_IT_HT);                               // 关闭DMA接收缓冲区溢出中断
}

/******************* (C) COPYRIGHT 2024 SBUS Parser **************************/