#ifndef ROBOT_CMD_H
#define ROBOT_CMD_H
#include <stdint.h>


typedef struct Robot_cmd_t
{
    /* data */
    uint8_t cmd_buf[8];
} Robot_cmd;


void robot_cmd_init(void);
void robot_cmd_update(void);



#endif