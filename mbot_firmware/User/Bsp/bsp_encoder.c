#include <bsp_encoder.h>
#include <tim.h>


// 记录上一次的 CNT 值 (0~65535)
static int16_t g_last_cnt[4] = {0}; 

// 记录累计的总脉冲数 (计算pos)
static int32_t g_total_pulses[4] = {0};

// 记录计算好的实时速度 (rad/s)
static float g_speed_rad[4] = {0.0f};


void BSP_Encoder_Init(void)
{

	HAL_TIM_Encoder_Start(&htim2,TIM_CHANNEL_ALL);
	HAL_TIM_Encoder_Start(&htim3,TIM_CHANNEL_ALL);
	HAL_TIM_Encoder_Start(&htim4,TIM_CHANNEL_ALL);
	HAL_TIM_Encoder_Start(&htim5,TIM_CHANNEL_ALL);
	
	__HAL_TIM_SET_COUNTER(&htim2,0);
	__HAL_TIM_SET_COUNTER(&htim3,0);
	__HAL_TIM_SET_COUNTER(&htim4,0);
	__HAL_TIM_SET_COUNTER(&htim5,0);
	
	for(int i=0;i<4;i++)
	{
		g_last_cnt[i]=0;
		g_total_pulses[i] = 0;
    g_speed_rad[i] = 0.0f;	

	}
}
/*
	这里采用差分编码器计算速度
（清零编码会产生少量误差，导致里程计漂移）

	将uint16（0~65535），转化为int16 （-32768~32767）
	（int16_t 的视界只有 -32768 到 +32767。 
		凡是超过 32767 的数，都会被减去 65536”来看待）

	这样减，怎么越过边界，也能准确读出脉冲差值
*/
void BSP_Encoder_Update(void)
{
	int16_t current_cnt=0;
	int16_t diff=0;
	
	for(int i=0;i<4;i++)
	{
		switch(i)
		{
			case LF: current_cnt=(int16_t)__HAL_TIM_GET_COUNTER(&htim2);break;
			case RF: current_cnt=(int16_t)__HAL_TIM_GET_COUNTER(&htim3);break;
      case LB: current_cnt=(int16_t)__HAL_TIM_GET_COUNTER(&htim4);break;
      case RB: current_cnt=(int16_t)__HAL_TIM_GET_COUNTER(&htim5);break;		
		}
		
		//计算差值，就是20ms里的的脉冲数
		diff=current_cnt-g_last_cnt[i];
		
		//更新上次差值
		g_last_cnt[i]=current_cnt;
		
		//累计总脉冲用来计算odom
		g_total_pulses[i]+=diff;
		// 计算速度 (rad/s)
    // 速度 = (增量脉冲 / 一圈总脉冲) * 2π / 时间 
		//假设调用周期是 20ms (0.02s)
    g_speed_rad[i] = (float)diff / PULSE_PER_ROUND * (2.0f * PI) / 0.02f;
	
	}
}


// 获取位置 (Rad)
float BSP_Encoder_GetPos_Rad(uint8_t id)
{
    // 把总脉冲数换算成弧度
    // 弧度 = (总脉冲 / 一圈总脉冲) * 2π
    return (float)g_total_pulses[id] / PULSE_PER_ROUND * (2.0f * PI);
}

// 获取速度 (Rad/s)
float BSP_Encoder_GetSpeed_Rad(uint8_t id)
{
    return g_speed_rad[id];
}


// 获取原始脉冲 (调试用)
int32_t BSP_Encoder_GetTotalPulse(uint8_t id)
{
    return g_total_pulses[id];
}













