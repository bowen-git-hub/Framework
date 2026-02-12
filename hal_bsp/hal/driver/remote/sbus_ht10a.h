#ifndef SBUS_HT10A_H
#define SBUS_HT10A_H

#include <stdint.h>
#include <stdbool.h>
#include "usart.h"

// SBUS协议常量定义
#define SBUS_FRAME_LENGTH       25      // SBUS帧总长度
#define SBUS_RECEIVE_BUFFER     SBUS_FRAME_LENGTH
#define SBUS_HEADER             0x0F    // 帧头标识
#define SBUS_FOOTER             0x00    // 帧尾标识

// SBUS通道值范围定义
#define SBUS_CH_MIN             192     // 最小通道值
#define SBUS_CH_MID             992     // 中间通道值
#define SBUS_CH_MAX             1792    // 最大通道值

// 标志位定义
#define SBUS_FLAG_CH17          0x01    // 通道17状态
#define SBUS_FLAG_CH18          0x02    // 通道18状态
#define SBUS_FLAG_FRAME_LOST    0x04    // 帧丢失标志
#define SBUS_FLAG_FAILSAFE      0x08    // 故障安全标志

// 连接超时定义 (ms)
#define SBUS_TIMEOUT_MS         100     // 超时时间


// 通道值范围定义
#define RBUS_CH_MIN -100
#define RBUS_CH_MID 0
#define RBUS_CH_MAX 100

// 开关状态定义 (基于通道值映射)
#define RBUS_SWITCH_UP 1
#define RBUS_SWITCH_MID 0
#define RBUS_SWITCH_DOWN 2

// 开关状态定义 (基于通道值映射)
#define RBUS_SWITCH_OFF 0
#define RBUS_SWITCH_ON 2


// SBUS原始数据结构
#pragma pack(push, 1)
typedef struct
{
    uint8_t header;    // 帧头
    uint8_t data[22];  // 通道数据区
    uint8_t flags;     // 标志位
    uint8_t footer;    // 帧尾
} SBUS_RawData;
#pragma pack(pop)

// SBUS解析器句柄
typedef struct
{
    uint8_t uart_rx_buffer[32];
    SBUS_RawData raw_data;              // 原始SBUS数据
    uint32_t last_update;               // 最后更新时间戳
    uint16_t channels_raw[16];          // 16个通道的原始值 (11位，0-2047)
    bool ch17;                          // 数字通道17状态
    bool ch18;                          // 数字通道18状态
    bool frame_lost;                    // 帧丢失标志
    bool failsafe;                      // 故障安全标志
} SBUS_HandleTypeDef;

// 解析状态
typedef struct {
    uint8_t is_valid;  // 是否有效 0-无效 1-有效 计算值

    // 原始数据转换位标准信息
    uint8_t frame_lost;       // 是否遥控信号丢失 0-未连接 1-已连接 逻辑值
    uint8_t frame_connected;  // 是否遥控已经连接 0-未连接 1-已连接 逻辑值
    uint32_t last_update;

    // 解析后的通道值用 (-660 ~ 660)
    int16_t right_x;       // 通道1: 右摇杆X
    int16_t right_y;       // 通道2: 右摇杆Y
    int16_t left_y;        // 通道3: 左摇杆Y
    int16_t left_x;        // 通道4: 左摇杆X
    uint8_t switch_sound;  // 声音开关
    uint8_t switch_mode;   // 模式转换  0/1 - 避障
    uint8_t switch_power;  // 0 停止，1 麦轮 2:胶轮
    uint8_t switch_pid;    // PID开关
    int16_t wheel_rate;    // 0~100 速度调节  预留
} UniRemote_HandleTypeDef;
;  // 都转成标准控制


// 函数声明
extern SBUS_HandleTypeDef* hsbus1;
void SBUS_Init(UART_HandleTypeDef* huart_ptr);
bool SBUS_Update(SBUS_HandleTypeDef* hsbus, const uint8_t* data, uint32_t len);
void SBUS_ParseChannels(SBUS_HandleTypeDef* hsbus);
int16_t SBUS_To_RBUS_Range(uint16_t sbus_value);
uint8_t SBUS_Map_RBUS_Switch(uint16_t channel_value);
void SBUS_PrintChannels(const SBUS_HandleTypeDef* hsbus);
void SBUS_To_UniRemote(UniRemote_HandleTypeDef* hur, const SBUS_HandleTypeDef* hsbus);
uint16_t SBUS_GetChannelRaw(const SBUS_HandleTypeDef* hsbus, uint8_t channel);
bool SBUS_IsConnected(const SBUS_HandleTypeDef* hsbus);
bool SBUS_IsFrameLost(const SBUS_HandleTypeDef* hsbus);
bool SBUS_IsFailsafe(const SBUS_HandleTypeDef* hsbus);
bool SBUS_GetDigitalChannel(const SBUS_HandleTypeDef* hsbus, uint8_t channel);
void SBUS_RxEventCallback(UART_HandleTypeDef* huart, uint16_t Size);

#endif