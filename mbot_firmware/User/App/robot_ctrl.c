#include <robot_ctrl.h>
#include <bsp_motor.h>
#include <bsp_encoder.h>
//四个轮子pid实例
PID_TypeDef pid_LF;
PID_TypeDef pid_RF;
PID_TypeDef pid_LB;
PID_TypeDef pid_RB;

static float target_speed[4] = {0.0f, 0.0f, 0.0f, 0.0f};

void Robot_Init(void)
{
	BSP_Motor_Init();
	BSP_Encoder_Init();

	float kp = 800.0f; 
  float ki = 5.0f;
  float kd = 0.0f;

	PID_Init(&pid_LF, kp, ki, kd, 7199, 2000);
  PID_Init(&pid_RF, kp, ki, kd, 7199, 2000);
  PID_Init(&pid_LB, kp, ki, kd, 7199, 2000);
  PID_Init(&pid_RB, kp, ki, kd, 7199, 2000);

}

//放入串口的目标速度
void Robot_SetTargetSpeeds(float w_lf, float w_rf, float w_lb, float w_rb)
{
    target_speed[LF] = w_lf;
    target_speed[RF] = w_rf;
    target_speed[LB] = w_lb;
    target_speed[RB] = w_rb;
}

void Robot_ControlLoop(void)
{
	//更新编码器数据
	BSP_Encoder_Update();

	//给每一个轮子单独计算actual，target，pid计算，最后发布pwm
	for(int i=0;i<4;i++)
	{
		PID_TypeDef *pid_ptr;
		switch(i)
		{
			case LF:pid_ptr=&pid_LF; break;
			case RF:pid_ptr=&pid_RF; break;
			case LB:pid_ptr=&pid_LB; break;
			case RB:pid_ptr=&pid_RB; break;			
			default: continue;		
		}
		//获取实际速度
		float actual_rad_s=BSP_Encoder_GetSpeed_Rad(i);
	
		//获取目标速度
    float target_rad_s = target_speed[i];
		
		//计算PID，输出pwm
		float pwm_out=PID_Calculate(pid_ptr,target_rad_s,actual_rad_s);
		
		//输出pwm
		BSP_Motor_SetPWM(i,(int16_t)pwm_out);
		
	}

}



