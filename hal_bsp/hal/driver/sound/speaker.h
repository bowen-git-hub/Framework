#ifndef _SPEAKER_H_
#define _SPEAKER_H_

#include <stdbool.h>
#include "stdint.h"
#include "usart.h"

// 语音消息队列配置
#define VOICE_QUEUE_LENGTH 10     // 队列长度
#define VOICE_MSG_MAX_LENGTH 128  // 每条消息的最大长度

// 语音消息结构体
typedef struct {
    char msg[VOICE_MSG_MAX_LENGTH];
} VoiceMsg_t;

// 环形缓冲区结构体
typedef struct {
    VoiceMsg_t buffer[VOICE_QUEUE_LENGTH];
    volatile uint8_t head;
    volatile uint8_t tail;
    volatile uint8_t count;
} VoiceQueue_t;

typedef struct Voice_t {
    UART_HandleTypeDef* uart_ptr;
    VoiceQueue_t queue;
} Voice;

// 函数原型
bool bsp_voice_init(UART_HandleTypeDef* uart_ptr);
bool bsp_voice_send_msg(const char* msg);
void bsp_voice_loop_process(void);// 轮询处理函数，需要在主循环中调用

#endif