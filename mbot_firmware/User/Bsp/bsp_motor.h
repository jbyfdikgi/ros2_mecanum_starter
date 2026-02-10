#ifndef __BSP_MOTOR_H__
#define __BSP_MOTOR_H__

#include <main.h>
#include <robot_def.h>

/*初始化电机*/
void BSP_Motor_Init(void);

/*id:轮子代号（LF RF LB RB）
	pwm: 速度 (-7199 到 +7199)*/
void BSP_Motor_SetPWM(uint8_t id ,int16_t pwm);

/*停止所有电机*/
void BSP_Motor_Stop(void);







#endif



