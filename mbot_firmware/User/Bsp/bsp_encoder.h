#ifndef __BSP_ENCODER_H__
#define __BSP_ENCODER_H__

#include <main.h>
#include <robot_def.h>


//初始化编码器定时器TIM2,3,4,5
void BSP_Encoder_Init(void);

// 读取硬件、计算差值、累加总脉冲
void BSP_Encoder_Update(void);

// 3. 获取累计位置 (单位: rad, 给 ROS 用)
float BSP_Encoder_GetPos_Rad(uint8_t id);

// 4. 获取实时速度 (单位: rad/s, 给 ROS 用)
float BSP_Encoder_GetSpeed_Rad(uint8_t id);

#endif

