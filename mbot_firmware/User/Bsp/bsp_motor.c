#include <bsp_motor.h>
#include <tim.h>
/*
	设置GPIO电平，控制旋转方向
	dir: 1=正转 -1=反转 0=停止
*/

static void Set_Dir(uint8_t id, int8_t dir)
{
	//IN线端口
	GPIO_TypeDef* port=GPIOE;
	uint16_t pin1,pin2;
	//根据ID找引脚号
	switch(id)
	{
		case(LF):
			pin1=LF_IN1_Pin;
			pin2=LF_IN2_Pin;
			break;
		case(RF):
			pin1=RF_IN1_Pin;
			pin2=RF_IN2_Pin;		
			break;	
		case(LB):
			pin1=LB_IN1_Pin;
			pin2=LB_IN2_Pin;
			break;	
		case(RB):
			pin1=RB_IN1_Pin;
			pin2=RB_IN2_Pin;
			break;	
		default: return;
	}
	//根据方向写高低电平
	if(dir==1)//正传
	{
		HAL_GPIO_WritePin(port, pin1, GPIO_PIN_SET);   // IN1 = 1
    HAL_GPIO_WritePin(port, pin2, GPIO_PIN_RESET); // IN2 = 0	
	}
	else if(dir==-1)//反转
	{
		HAL_GPIO_WritePin(port, pin1, GPIO_PIN_RESET);   // IN1 = 0
    HAL_GPIO_WritePin(port, pin2, GPIO_PIN_SET); // IN2 = 1
	}
	else//停止
	{
		HAL_GPIO_WritePin(port, pin1, GPIO_PIN_RESET); // IN1 = 0
    HAL_GPIO_WritePin(port, pin2, GPIO_PIN_RESET); // IN2 = 0	
	}
}


void BSP_Motor_Stop(void)
{
	BSP_Motor_SetPWM(LF,0);
	BSP_Motor_SetPWM(RF,0);
	BSP_Motor_SetPWM(LB,0);
	BSP_Motor_SetPWM(RB,0);
}


void BSP_Motor_Init(void)
{
	HAL_TIM_PWM_Start(&htim8,TIM_CHANNEL_1);
	HAL_TIM_PWM_Start(&htim8,TIM_CHANNEL_2);
	HAL_TIM_PWM_Start(&htim8,TIM_CHANNEL_3);
	HAL_TIM_PWM_Start(&htim8,TIM_CHANNEL_4);
	//默认停车
	BSP_Motor_Stop();
}


void BSP_Motor_SetPWM(uint8_t id ,int16_t pwm)
{
	//限幅
	if(pwm>7199) pwm=7199;
	if(pwm<-7199) pwm=-7199;

	//判断正负，设置方向
	if(pwm>0)
	{
		Set_Dir(id,1);  //
	}
	else if(pwm<0)
	{
		pwm=-pwm;
		Set_Dir(id,-1); //反转
	}
	else
	{
		Set_Dir(id,0); //停车
	}
	switch(id)
	{
		case LF: __HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_1, pwm); break;
		case RF: __HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_2, pwm); break;
		case LB: __HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_3, pwm); break;
		case RB: __HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_4, pwm); break;
	
	}
}
























