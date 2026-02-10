#include "pid.h"

void PID_Init(PID_TypeDef *pid, float kp, float ki, float kd, float max_out, float int_limit)
{
	pid->Kp=kp;
	pid->Ki=ki;
	pid->Kd=kd;
	pid->MaxOutput=max_out;
	pid->IntegralLimit=int_limit;
	
	PID_Reset(pid);
}

void PID_Reset(PID_TypeDef *pid)
{
	pid->Target=0;
	pid->Actual = 0;
	pid->Error = 0;
	pid->PrevError = 0;
	pid->SumError = 0;
	pid->Output = 0;
}


/*
	taeget:串口得到的rad/s
	actual：encoder得到的rad/s
	输出：pwm
*/
float PID_Calculate(PID_TypeDef *pid, float target, float actual)
{
	pid->Target = target;
	pid->Actual = actual;
  pid->Error = pid->Target - pid->Actual; // Error = Goal - Current

	float p_out = pid->Kp * pid->Error;
	
	pid->SumError += pid->Error;
	//积分限幅
	if(pid->SumError > pid->IntegralLimit)  pid->SumError = pid->IntegralLimit;
  if(pid->SumError < -pid->IntegralLimit) pid->SumError = -pid->IntegralLimit;
	
	float i_out=pid->Ki*pid->SumError;
	
	float d_out = pid->Kd * (pid->Error - pid->PrevError);
	//总输出
	pid->Output=p_out+i_out+d_out;
	//输出限幅
	if(pid->Output > pid->MaxOutput)  pid->Output = pid->MaxOutput;
  if(pid->Output < -pid->MaxOutput) pid->Output = -pid->MaxOutput;
	
	//存入先前误差
	pid->PrevError=pid->Error;
	
	return pid->Output;

}






