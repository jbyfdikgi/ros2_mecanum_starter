#ifndef MBOT_SYSTEM_HPP_
#define MBOT_SYSTEM_HPP_

#include <memory>
#include <string>
#include <vector>
#include <cstdint>

#include "hardware_interface/handle.hpp"
#include "hardware_interface/hardware_info.hpp"
#include "hardware_interface/system_interface.hpp"
#include "hardware_interface/types/hardware_interface_return_values.hpp"

#include "rclcpp/macros.hpp"
#include "rclcpp_lifecycle/node_interfaces/lifecycle_node_interface.hpp"
#include "rclcpp_lifecycle/state.hpp"

#include "libserial/SerialPort.h"

namespace mbot_hardware
{

#pragma pack(1)

//下发包（PC->STM32）
typedef struct
{
    uint8_t head1;  //0x55
    uint8_t head2;  //0xAA
    float target_vel[4]; //目标速度（LF,RF,LB,RB）
    uint8_t tail1;  //0x0D
    uint8_t tail2;  //0x0A
} RobotCommand;

//联合体union

typedef union
{
    RobotCommand data;
    uint8_t buffer[sizeof(RobotCommand)];
} CommandPacket;


//上传包 (STM32 -> PC)
typedef struct
{
  uint8_t head1;      // 0x55
  uint8_t head2;      // 0xAA
  float   actual_pos[4]; // 累积位置 (用于里程计)
  float   actual_vel[4]; // 实时速度 (用于监视)
  uint8_t tail1;      // 0x0D
  uint8_t tail2;      // 0x0A
} RobotFeedback;

typedef union
{
  RobotFeedback data;
  uint8_t       buffer[sizeof(RobotFeedback)];
} FeedbackPacket;

// 恢复默认对齐设置，不影响后面的代码
#pragma pack()


class MbotSystemHardware :public hardware_interface::SystemInterface
{
public:
    RCLCPP_SHARED_PTR_DEFINITIONS(MbotSystemHardware)
    //1.初始化读取urdf参数
    hardware_interface::CallbackReturn on_init(
        const hardware_interface::HardwareInfo & info) override;
    //2.配置（打开串口）
    hardware_interface::CallbackReturn on_configure(
        const rclcpp_lifecycle::State & previous_state)override;
    //3.激活（电机上电，发送指令）
    hardware_interface::CallbackReturn on_activate(
        const rclcpp_lifecycle::State & prevous_state)override;
    //4.停用（电机断电，断开连接）
    hardware_interface::CallbackReturn on_deactivate(
        const rclcpp_lifecycle::State & prevous_state)override;
    //循环读取 上位机->stm32
    hardware_interface::return_type read(
        const rclcpp::Time & time, const rclcpp::Duration & period)override;
    //循环读取 stm32->上位机
    hardware_interface::return_type write(
        const rclcpp::Time & time, const rclcpp::Duration & period)override;

    // 暴露接口：告诉 ROS有哪些数据可以被借用（位置、速度）
    std::vector<hardware_interface::StateInterface> export_state_interfaces() override;
    std::vector<hardware_interface::CommandInterface> export_command_interfaces() override;

private:
    //串口驱动对象
    LibSerial::SerialPort serial_conn_;
    //配置参数
    std::string port_name_;
    int baud_rate_;
    //数据缓冲区
    std::vector<double> hw_commands_; //目标速度
    std::vector<double> hw_positions_; //实际位置（编码器）
    std::vector<double> hw_velocities_; //实际速度
    //串口实例化
    CommandPacket  cmd_packet_;
    FeedbackPacket fb_packet_;
};

}

#endif