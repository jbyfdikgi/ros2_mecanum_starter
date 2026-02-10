#ifndef _ROBOT_DEF_H__
#define _ROBOT_DEF_H__

#include <stdint.h>   //uint8 uint16标准

//定义PI
#ifndef PI
#define PI 3.1415926535f
#endif

//电机参数定义
//减速比
#define MOTOR_REDUCTION_RATIO 30.0f  
//分辨率
#define ENCODER_RESOLUTION 11.0f  
//四倍频
#define ENCODER_MULTIPLE 4.0f 
//一周的脉冲
#define PULSE_PER_ROUND (MOTOR_REDUCTION_RATIO * ENCODER_RESOLUTION * ENCODER_MULTIPLE)

//麦克纳姆轮半径（单位m）
#define WHEEL_RADIUS 0.05f

//左右轮中心距离
#define ROBOT_WHEEL_TRACK 0.34f
//前后轮中心距离
#define ROBOT_WHEEL_BASE 0.36f


//定义电机枚举

typedef enum{
	LF=0,			//左前
	RF=1,			//右前
	LB=2,			//左后
	RB=3,			//右后
	MOTOR_COUNT=4,  //电机总数
} Motor_e;


//定义通讯数据包

#pragma pack(1)  //禁止编译器优化

// 反馈给上位机的数据 (STM32 -> ROS)
// 包含：包头 + 4个位置 + 4个速度 + 包尾
typedef struct {
    uint8_t  head[2];       // 0x55, 0xAA
    float    position[4];   // 单位: rad (弧度)
    float    velocity[4];   // 单位: rad/s (弧度每秒)
    uint8_t  tail[2];       // 0x0D, 0x0A
} FeedbackData_t;

// 联合体：既可以当结构体存 float，也可以当数组发串口
typedef union {
    FeedbackData_t data;
    uint8_t buffer[sizeof(FeedbackData_t)]; // 自动计算字节数 (应该约36字节)
} FeedbackPacket_t;

// 接收上位机的指令 (ROS -> STM32)
// 包含：包头 + 四轮速度 + 包尾
typedef struct {
    uint8_t  head[2];      // 0x55, 0xAA
    
    float    speed_lf;     // 左前轮目标速度 (rad/s)
    float    speed_rf;     // 右前轮目标速度 (rad/s)
    float    speed_lb;     // 左后轮目标速度 (rad/s)
    float    speed_rb;     // 右后轮目标速度 (rad/s)
    
    uint8_t  tail[2];      // 0x0D, 0x0A
} ControlCmd_t;

typedef union {
    ControlCmd_t data;
    uint8_t buffer[sizeof(ControlCmd_t)];
} ControlPacket_t;
#pragma pack() // 取消对齐设置，恢复默认










#endif



