#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__

#include "main.h"

// 1. 处理接收到的字节 (在串口中断里调用)
// data: 接收到的 1 个字节
void Protocol_ProcessByte(uint8_t data);

// 2. 发送反馈数据给 ROS (在定时器里调用)
void Protocol_SendFeedback(void);

#endif

