#ifndef __ROBOT_CTRL_H__
#define __ROBOT_CTRL_H__

#include "robot_def.h"
#include "pid.h"

// 全局 PID 实例
extern PID_TypeDef pid_LF;
extern PID_TypeDef pid_RF;
extern PID_TypeDef pid_LB;
extern PID_TypeDef pid_RB;

// 初始化
void Robot_Init(void);

void Robot_SetTargetSpeeds(float w_lf, float w_rf, float w_lb, float w_rb);

// 控制闭环任务 (每 20ms 调用)
void Robot_ControlLoop(void);

#endif



