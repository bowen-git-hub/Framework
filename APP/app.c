#include "app.h"
#include "usart.h"
#include "dma.h"
#include <stdint.h>
uint32_t debug_count = 0;
asm(".global _printf_float"); // 使能printf浮点数支持
uint8_t uart_rx_buffer[32] = {0};
UART_HandleTypeDef *uart_handle = &huart5;
void app_init(void)
{
    printf("app_init\r\n");
    // 初始化UART DMA接收
    HAL_UARTEx_ReceiveToIdle_DMA(uart_handle, uart_rx_buffer, sizeof(uart_rx_buffer)); // 启动DMA接收,用来接收遥控器数据
    __HAL_DMA_DISABLE_IT(uart_handle->hdmarx, DMA_IT_HT);                              // 关闭DMA接收缓冲区半满中断
    printf("SBUS_Init cussessful.\n\r");
}

void app_loop(void)
{
}

// 重定向printf到UART1,方便调试
int _write(int file, char *ptr, int len)
{
    HAL_UART_Transmit(&huart1, (uint8_t *)ptr, len, HAL_MAX_DELAY);
    return len;
}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
   // printf("UARTEx_RxEventCallbackaaaaaaaaaaaaaaaaaaa: %d, %s.\n\r", Size, "debug");

    if (huart->Instance == UART5) // 判断是否是遥控器接收UART
    {
       // printf("SBUS_RxEventCallback: %d\n\r", Size);
        // SBUS_RxEventCallback(huart, Size);
        //  初始化UART DMA接收
        //HAL_Delay(1);
        debug_count = Size;
        HAL_UARTEx_ReceiveToIdle_DMA(uart_handle, uart_rx_buffer, sizeof(uart_rx_buffer)); // 启动DMA接收,用来接收遥控器数据
        __HAL_DMA_DISABLE_IT(uart_handle->hdmarx, DMA_IT_HT);
    }
}
