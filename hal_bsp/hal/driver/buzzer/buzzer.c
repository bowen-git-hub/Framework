#include "buzzer.h"
#include "usart.h"
#include "string.h"

// 语音消息队列（环形缓冲区）
static VoiceQueue_t g_voice_queue;

static UART_HandleTypeDef* voc_uart_ptr = NULL;

// 内部函数声明
static bool voice_queue_init(VoiceQueue_t* queue);
static bool voice_queue_send(VoiceQueue_t* queue, const VoiceMsg_t* msg);
static bool voice_queue_receive(VoiceQueue_t* queue, VoiceMsg_t* msg);
static bool voice_queue_is_empty(const VoiceQueue_t* queue);
static bool voice_queue_is_full(const VoiceQueue_t* queue);
static uint8_t generate_voice_command(char* output, const char* msg, uint16_t msg_len);

/**
 * @brief  语音模块初始化
 * @return true - 初始化成功, false - 初始化失败
 */
bool bsp_voice_init(UART_HandleTypeDef* uart_ptr) {
    // 初始化语音消息队列
    if (!voice_queue_init(&g_voice_queue)) {
        return false;
    }

    // 保存UART指针
    voc_uart_ptr = uart_ptr;

    return true;
}


/**
 * @brief  发送语音消息到队列
 * @param  msg - 要发送的语音消息
 * @return true - 发送成功, false - 发送失败
 */
bool bsp_voice_send_msg(const char* msg) {
    // 参数检查
    if (msg == NULL) {
        return false;
    }

    VoiceMsg_t voice_msg;
    // 检查是否距离上次发送过1秒
    static uint32_t last_send_time = 0;
    
    if (HAL_GetTick() - last_send_time < 1000) {
        return false;
    }
    last_send_time = HAL_GetTick();

    // 复制消息内容，确保不超过最大长度
    strncpy(voice_msg.msg, msg, VOICE_MSG_MAX_LENGTH - 1);
    voice_msg.msg[VOICE_MSG_MAX_LENGTH - 1] = '\0';

    // 发送消息到队列
    if (!voice_queue_send(&g_voice_queue, &voice_msg)) {
        return false;
    }

    return true;
}

/**
 * @brief  语音消息处理函数
 * @note   需要在主循环中定期调用此函数
 */
void bsp_voice_loop_process(void) {

    if (voc_uart_ptr == NULL) {
        return;
    }
    
    VoiceMsg_t voice_msg;
    char send_buf[128] = {0};
    static uint32_t last_send_time = 0;
    // 检查队列是否有消息
    if (!voice_queue_is_empty(&g_voice_queue) && (HAL_GetTick() - last_send_time >= 2000)) {
        // 从队列接收语音消息
        if (voice_queue_receive(&g_voice_queue, &voice_msg)) {
            // 生成语音命令
            uint16_t len = generate_voice_command(send_buf, voice_msg.msg, strlen(voice_msg.msg));

            // 发送语音命令到语音模块到串口
            HAL_UART_Transmit(voc_uart_ptr, (uint8_t*)send_buf, len, 1000);
            last_send_time = HAL_GetTick();
        }
    }
}


/*****************************************************************************/
//队列相关函数
/**
 * @brief  初始化语音消息队列
 * @param  queue - 队列指针
 * @return true - 初始化成功, false - 初始化失败
 */
static bool voice_queue_init(VoiceQueue_t* queue) {
    if (queue == NULL) {
        return false;
    }

    queue->head = 0;
    queue->tail = 0;
    queue->count = 0;

    return true;
}

/**
 * @brief  检查队列是否为空
 * @param  queue - 队列指针
 * @return true - 队列为空, false - 队列不为空
 */
static bool voice_queue_is_empty(const VoiceQueue_t* queue) {
    return (queue->count == 0);
}

/**
 * @brief  检查队列是否已满
 * @param  queue - 队列指针
 * @return true - 队列已满, false - 队列未满
 */
static bool voice_queue_is_full(const VoiceQueue_t* queue) {
    return (queue->count == VOICE_QUEUE_LENGTH);
}

/**
 * @brief  发送消息到队列
 * @param  queue - 队列指针
 * @param  msg - 要发送的消息
 * @return true - 发送成功, false - 发送失败
 */
static bool voice_queue_send(VoiceQueue_t* queue, const VoiceMsg_t* msg) {
    if (queue == NULL || msg == NULL) {
        return false;
    }

    // 检查队列是否已满
    if (voice_queue_is_full(queue)) {
        return false;
    }

    // 复制消息到队列
    memcpy(&queue->buffer[queue->tail], msg, sizeof(VoiceMsg_t));

    // 更新队列尾指针
    queue->tail = (queue->tail + 1) % VOICE_QUEUE_LENGTH;

    // 更新队列计数
    queue->count++;

    return true;
}

/**
 * @brief  从队列接收消息
 * @param  queue - 队列指针
 * @param  msg - 接收消息的缓冲区
 * @return true - 接收成功, false - 接收失败
 */
static bool voice_queue_receive(VoiceQueue_t* queue, VoiceMsg_t* msg) {
    if (queue == NULL || msg == NULL) {
        return false;
    }

    // 检查队列是否为空
    if (voice_queue_is_empty(queue)) {
        return false;
    }

    // 从队列复制消息
    memcpy(msg, &queue->buffer[queue->head], sizeof(VoiceMsg_t));

    // 更新队列头指针
    queue->head = (queue->head + 1) % VOICE_QUEUE_LENGTH;

    // 更新队列计数
    queue->count--;

    return true;
}

/***************************************************************************/

/**
 * 生成语音命令数据包
 * 
 * 该函数构建一个符合特定协议格式的语音命令数据包，用于语音模块通信
 * 数据包格式：帧头(1字节) + 长度字段(2字节) + 命令类型(1字节) + 编码格式(1字节) + 消息内容(N字节)
 * 
 * @param output 输出缓冲区指针，用于存储生成的数据包
 * @param msg 要发送的消息字符串
 * @param msg_len 消息长度
 * @return uint8_t 返回生成的数据包总长度，如果参数无效则返回0
 */
static uint8_t generate_voice_command(char* output, const char* msg, uint16_t msg_len) {
    // 参数检查
    if (output == NULL || msg == NULL) {
        return 0;
    }

    // 计算总长度
    uint16_t total_len = msg_len + 5;

    // 构建数据包
    output[0] = 0xFD;  // 帧头

    // 使用按位与 &
    output[1] = ((msg_len + 2) >> 8) & 0xFF;  // 长度高8位
    output[2] = (msg_len + 2) & 0xFF;         // 长度低8位

    output[3] = 0x01;  // 命令类型
    output[4] = 0x05;  // 编码格式 (UTF-8)

    // 复制消息内容
    memcpy(&output[5], msg, msg_len);

    return total_len;
}
