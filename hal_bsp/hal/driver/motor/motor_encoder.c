#include "motor_encoder.h"
#include "tim.h"
#include "bsp_def.h"
#include "ax_oled_1305.h"


/**
 * @brief 轮子配置结构体
 * @details 定义每个轮子对应的定时器和旋转方向
 */
typedef struct {
    TIM_HandleTypeDef* timer;
    int8_t direction;
} WheelConfig_t;

// 四轮配置配置描述
/**
 * @brief 四轮配置数组
 * @details 定义四个轮子的定时器和旋转方向配置
 */
static const WheelConfig_t wheel_configs[] = {
    {BSP_LF_DETECT_TIM, 1},   // 左前轮
    {BSP_RF_DETECT_TIM, -1},  // 右前轮
    {BSP_LB_DETECT_TIM, 1},   // 左后轮
    {BSP_RB_DETECT_TIM, -1},  // 右后轮
};

BSP_Wheel_Sensor_HandleTypeDef wheel_sensor = {0};
BSP_Wheel_Sensor_HandleTypeDef* hws = &wheel_sensor;

void BSP_Wheel_Detect_Init(void) {
    // 电机编码器读取初始化
    HAL_TIM_Encoder_Start(BSP_LF_DETECT_TIM, BSP_LF_DETECT_TIM_CHANNEL);  // 左前轮编码器启动
    HAL_TIM_Encoder_Start(BSP_RF_DETECT_TIM, BSP_RF_DETECT_TIM_CHANNEL);  // 右前轮编码器启动
    HAL_TIM_Encoder_Start(BSP_LB_DETECT_TIM, BSP_LB_DETECT_TIM_CHANNEL);  // 左后轮编码器启动
    HAL_TIM_Encoder_Start(BSP_RB_DETECT_TIM, BSP_RB_DETECT_TIM_CHANNEL);  // 右后轮编码器启动

    // 初始化编码器计数器
    for (int i = 0; i < WHEEL_COUNT; i++) {
        __HAL_TIM_SET_COUNTER(wheel_configs[i].timer, TIM_LOOP_MARK);  // 设置计数器初始值
        wheel_sensor.wheelSpeeds[i].tick_time = HAL_GetTick();         // 记录当前时间
    }
    printf("Motor Encoder Init OK!\r\n");
}

/**
 * @brief 霍尔传感器轮速检测主循环
 * @details 循环计算四个轮子的转速，更新速度数据并显示
 * @param 无
 * @retval 无
 * @note 该函数应在FreeRTOS任务中定期调用
 * @warning 调用前确保已调用BSP_Wheel_Detect_Init()
 */
void BSP_Wheel_Detect_Loop(void) {
    // 处理所有轮子当前速度存入count
    for (int i = 0; i < WHEEL_COUNT; i++) {
        // 计算当前速度
        int32_t current_count = __HAL_TIM_GET_COUNTER(wheel_configs[i].timer) - TIM_LOOP_MARK;  // 取得当前计数器--TIM_LOOP_MARK 取得当前计数值
        int32_t times = (HAL_GetTick() - wheel_sensor.wheelSpeeds[i].tick_time);


        if (times > 0) {
            // 速度计算公式：每分钟圈速度 = 当前计数* 1000（转秒）/（现在时间/ms-过去时间/ms）* 60 /每圈计数器 * 方向符号
            wheel_sensor.wheelSpeeds[i].speed = (current_count * 1000 / times * 60 / ENCODER_COUNTS_PER_REV * wheel_configs[i].direction);
        } else {
            wheel_sensor.wheelSpeeds[i].speed = 0;
        }

        // 下次计算使用当前时间
        wheel_sensor.wheelSpeeds[i].tick_time = HAL_GetTick();
        // 重置计数器
        __HAL_TIM_SET_COUNTER(wheel_configs[i].timer, TIM_LOOP_MARK);
    }
    char display_text[24] = {0};
    sprintf(display_text, "LF:%4ld,RF:%4ld", wheel_sensor.wheelSpeeds[0].speed, wheel_sensor.wheelSpeeds[1].speed);
    AX_OLED_DispStr(5, 4, (uint8_t*)display_text, 0);  // OLED输出速度
    sprintf(display_text, "LB:%4ld,RB:%4ld", wheel_sensor.wheelSpeeds[2].speed, wheel_sensor.wheelSpeeds[3].speed);
    AX_OLED_DispStr(5, 5, (uint8_t*)display_text, 0);  // OLED输出速度


}






