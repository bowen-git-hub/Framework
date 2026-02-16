#ifndef ROBOT_CMD_H
#define ROBOT_CMD_H
#include <stdint.h>
#include "ax_oled_1305.h"
#include "sbus_ht10a.h"
#include "buzzer.h"


typedef enum Robot_mode_e
{
    Robot_stop = 0,
    Robot_run
} Robot_mode;

typedef struct Robot_cmd_t
{

    // 标志量
    // 机器人状态标志量
    Robot_mode mode;
    
    // 机器人启动好标志量
    uint8_t robot_ready;
    
    // 外设
    Voice *voice;
    UniRemote_HandleTypeDef *remote;
    AX_OLED_Config *led;

   /*  Publisher *gimbal_cmd_puber;
    Subscriber *gimbal_upload_suber; */
} Robot_cmd;



Robot_cmd *robot_cmd_init(void);
void robot_cmd_update(Robot_cmd *cmd);



#endif