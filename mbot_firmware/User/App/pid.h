#ifndef __PID_H__
#define __PID_H__

#include <stdint.h>

typedef struct {
    
    float Kp;           
    float Ki;           
    float Kd;           
    
    
    float MaxOutput;    //PWM最大输出7199
    float IntegralLimit;  //防止I项过大
    
		//状态参数
    float Target;      //目标rad/s
    float Actual;      //实际rad/s
    float Error;       //误差=目标值-实际值
		float PrevError;   //先前误差
    float SumError;    //误差总和-计算Ki
    float Output;       //最后输出的pwm
    
} PID_TypeDef;


void PID_Init(PID_TypeDef *pid, float kp, float ki, float kd, float max_out, float int_limit);

//清空状态参数
void PID_Reset(PID_TypeDef *pid);

//返回pwm到motor
float PID_Calculate(PID_TypeDef *pid, float target, float actual);



#endif
