#include <protocol.h>
#include "robot_def.h"  // 定义数据包结构体
#include "robot_ctrl.h" // 设置目标speed
#include "bsp_encoder.h"// 获取实际speed pos
#include "usart.h"      // 发送串口数据 (huart1)


/*
	使用状态机接收数据包
*/
static uint8_t rx_state=0;   //0:找包头 1：找包头 2，接收数据
static uint8_t rx_cnt=0;  //数据计数

ControlPacket_t rx_packet; //接收数据联合体

//串口接收数据处理
void Protocol_ProcessByte(uint8_t data)
{
	switch(rx_state)
	{
		case 0:
			if(data==0x55)
			{
				rx_packet.buffer[0]=data;
				rx_state=1;
			}break;
		case 1:
				if(data==0xAA)
			{
				rx_packet.buffer[1]=data;
				rx_state=2;
				rx_cnt = 2; // 已经收了2个字节
      } 
			else {
				rx_state = 0; // 找错了，重来  
			}break;
		case 2:
				rx_packet.buffer[rx_cnt++] = data; //剩余数据放进数组里
				if(rx_cnt>=sizeof(ControlCmd_t))
				{
					// 校验包尾 (0x0D, 0x0A)
					uint8_t tail1 = rx_packet.buffer[sizeof(ControlCmd_t)-2];
					uint8_t tail2 = rx_packet.buffer[sizeof(ControlCmd_t)-1];				
					if(tail1 == 0x0D && tail2 == 0x0A)
					{
						// 把速度发送到机器人控制层
						Robot_SetTargetSpeeds(
								rx_packet.data.speed_lf,
								rx_packet.data.speed_rf,
								rx_packet.data.speed_lb,
								rx_packet.data.speed_rb);
					}			
					// 复位状态机，准备收下一包
					rx_state = 0;
					rx_cnt = 0;
				}break;
		default:
				rx_state=0;break;
	}
}

//串口发送数据处理
void Protocol_SendFeedback(void)
{
	static FeedbackPacket_t tx_packet;
	
	//防止DMA还在搬运数据时，函数修改导致出错
	if (huart1.gState != HAL_UART_STATE_READY) return;
	//处理包头包尾
	tx_packet.data.head[0]=0x55;
	tx_packet.data.head[1]=0xAA;
	tx_packet.data.tail[0]=0x0D;
	tx_packet.data.tail[1]=0x0A;

	//填充位置数据（Rad）
	tx_packet.data.position[LF]=BSP_Encoder_GetPos_Rad(LF);
	tx_packet.data.position[RF]=BSP_Encoder_GetPos_Rad(RF);
	tx_packet.data.position[LB]=BSP_Encoder_GetPos_Rad(LB);
	tx_packet.data.position[RB]=BSP_Encoder_GetPos_Rad(RB);

	//填充速度数据（Rad/s）	
	tx_packet.data.velocity[LF] = BSP_Encoder_GetSpeed_Rad(LF);
	tx_packet.data.velocity[RF] = BSP_Encoder_GetSpeed_Rad(RF);
	tx_packet.data.velocity[LB] = BSP_Encoder_GetSpeed_Rad(LB);
	tx_packet.data.velocity[RB] = BSP_Encoder_GetSpeed_Rad(RB);
	
	//使用DMA发送
	HAL_UART_Transmit_DMA(&huart1, tx_packet.buffer, sizeof(FeedbackPacket_t));
}











