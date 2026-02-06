#include "mbot_hardware/mbot_system.hpp"
#include <chrono>
#include <cmath>
#include <vector>
#include "rclcpp/rclcpp.hpp"
// 1. 引入插件注册宏
#include "pluginlib/class_list_macros.hpp"


namespace mbot_hardware
{
//重写on_init
hardware_interface::CallbackReturn MbotSystemHardware::on_init(
    const hardware_interface::HardwareInfo & info)
{
    // 父类负责把 XML 解析成 info 结构体
    if(hardware_interface::SystemInterface::on_init(info) != hardware_interface::CallbackReturn::SUCCESS)
    {
        return hardware_interface::CallbackReturn::ERROR;
    }
    // 读取串口名，（三元运算符 if-else 条件判断 ? 真的选我 : 假的选我）默认给 /dev/ttyUSB0
    port_name_ = info_.hardware_parameters.count("port_name") ? 
                info_.hardware_parameters.at("port_name") : "/dev/ttyUSB0";

    // 读取波特率 同理，默认115200
    baud_rate_ = info_.hardware_parameters.count("baud_rate") ? 

                std::stoi(info_.hardware_parameters.at("baud_rate")) : 115200;

    //给内存向量分配空间,麦克纳姆轮，有 4 个关节。
    hw_positions_.resize(info_.joints.size(),std::numeric_limits<double>::quiet_NaN());
    hw_velocities_.resize(info_.joints.size(),std::numeric_limits<double>::quiet_NaN());
    hw_commands_.resize(info_.joints.size(),std::numeric_limits<double>::quiet_NaN());

    // 确认一下 URDF 里是不是真的写了 4 个轮子
    if (info_.joints.size() != 4)
    {
        RCLCPP_FATAL(
        rclcpp::get_logger("MbotSystemHardware"),
        "System has %zu joints. 4 expected.", info_.joints.size());
        return hardware_interface::CallbackReturn::ERROR;
    }

    RCLCPP_INFO(rclcpp::get_logger("MbotSystemHardware"), "Initialized with Port: %s, Baud: %d", port_name_.c_str(), baud_rate_);
    return hardware_interface::CallbackReturn::SUCCESS;
}

//重写on_configure
hardware_interface::CallbackReturn MbotSystemHardware::on_configure(
    const rclcpp_lifecycle::State & /*previous_state*/)
{
    //打印开机日志
    RCLCPP_INFO(rclcpp::get_logger("MbotSystemHardware"),"Configuring... Opening Serial Port %s", port_name_.c_str());
    //进行串口配置
    try
    {
        //打开串口
       serial_conn_.Open(port_name_);
       // B1. 波特率  115200
       serial_conn_.SetBaudRate(LibSerial::BaudRate::BAUD_115200);
       // B2. 数据位 8位 (标准 uint8_t)
       serial_conn_.SetCharacterSize(LibSerial::CharacterSize::CHAR_SIZE_8);
       // B3. 校验位  无校验 (None)
       serial_conn_.SetParity(LibSerial::Parity::PARITY_NONE);
       // B4. 停止位  1位
       serial_conn_.SetStopBits(LibSerial::StopBits::STOP_BITS_1);
       // B5. 流控  无流控 (None) - 只要有数据就发，不管对方死活
       serial_conn_.SetFlowControl(LibSerial::FlowControl::FLOW_CONTROL_NONE);
       serial_conn_.SetVMin(0); 
       serial_conn_.SetVTime(1);
    }
    catch(...)
    {
        RCLCPP_FATAL(
            rclcpp::get_logger("MbotSystemHardware"),
            "Failed to open serial port: %s. Check your USB connection!", port_name_.c_str());
        //返回错误
        return hardware_interface::CallbackReturn::ERROR;
    }
    
    RCLCPP_INFO(rclcpp::get_logger("MbotSystemHardware"), "Successfully opened serial port!");

   // 6. 返回成功：系统状态切换为 Inactive (就绪)
    return hardware_interface::CallbackReturn::SUCCESS;

}

//重写on_activate
hardware_interface::CallbackReturn MbotSystemHardware::on_activate(
    const rclcpp_lifecycle::State & /*previous_state*/)
{
    RCLCPP_INFO(rclcpp::get_logger("MbotSystemHardware"),"Activating...System is starting");

    //清理之前数据
    for(auto i=0u; i < hw_commands_.size(); i++)
    {
        hw_commands_[i] = 0.0;     // 目标速度归零
        hw_positions_[i] = 0.0;    // 编码器位置归零 (假设每次上电都算从0开始)
        hw_velocities_[i] = 0.0;   // 实际速度归零
    }
    //发送开始指令 (可选) 
    try
    {

    }
    catch (...)
    {
        RCLCPP_FATAL(rclcpp::get_logger("MbotSystemHardware"), "Failed to activate system!");
        return hardware_interface::CallbackReturn::ERROR;
    }

    RCLCPP_INFO(rclcpp::get_logger("MbotSystemHardware"), "System Activated!");
    return hardware_interface::CallbackReturn::SUCCESS;
}

//重写on_deactivate
hardware_interface::CallbackReturn MbotSystemHardware::on_deactivate(
    const rclcpp_lifecycle::State & /*previous_state*/)
{
    RCLCPP_INFO(rclcpp::get_logger("MbotSystemHardware"), "Deactivating... Stopping motors!");

    //安全停车 
    try
    {
        // 以后会封装一个 send_command 函数，这里暂时先空着
    }
    catch (...)
    {
        RCLCPP_WARN(rclcpp::get_logger("MbotSystemHardware"), "Error while deactivating!");
    }

    RCLCPP_INFO(rclcpp::get_logger("MbotSystemHardware"), "System Deactivated!");
    return hardware_interface::CallbackReturn::SUCCESS;
}

//重新write
hardware_interface::return_type MbotSystemHardware::write(
    const rclcpp::Time & /*time*/, const rclcpp::Duration & /*period*/)
{
    //包头
    cmd_packet_.data.head1 = 0x55;
    cmd_packet_.data.head2 = 0xAA;
    //数据内容
    cmd_packet_.data.target_vel[0]=static_cast<float>(hw_commands_[0]);
    cmd_packet_.data.target_vel[1]=static_cast<float>(hw_commands_[1]);
    cmd_packet_.data.target_vel[2]=static_cast<float>(hw_commands_[2]);
    cmd_packet_.data.target_vel[3]=static_cast<float>(hw_commands_[3]);
    //包尾
    cmd_packet_.data.tail1 = 0x0D;
    cmd_packet_.data.tail2 = 0x0A;

    //包装为vector
    std::vector<uint8_t> data_to_send(
        cmd_packet_.buffer, 
        cmd_packet_.buffer + sizeof(cmd_packet_.buffer)
    );
    try
    {
        serial_conn_.Write(data_to_send);
    }
    catch(...)
    {
        RCLCPP_ERROR(rclcpp::get_logger("MbotSystemHardware"), "Failed to send command!");
        return hardware_interface::return_type::ERROR;
    }
    
    return hardware_interface::return_type::OK;
}

//重新read
hardware_interface::return_type MbotSystemHardware::read(
    const rclcpp::Time & /*time*/, const rclcpp::Duration & /*period*/)
{
    if(static_cast<size_t>(serial_conn_.GetNumberOfBytesAvailable())< sizeof(FeedbackPacket))
    {
        return hardware_interface::return_type::OK;
    }
    bool header_found=false;

    while (static_cast<size_t>(serial_conn_.GetNumberOfBytesAvailable()) >= sizeof(FeedbackPacket))
    {
        char byte1=0;
        serial_conn_.ReadByte(byte1);
        //找到包头了
        if(static_cast<uint8_t>(byte1) == 0x55)
        {
            char byte2=0;
            serial_conn_.ReadByte(byte2);
            //第二个包头也没问题
            if(static_cast<uint8_t>(byte2) == 0xAA)
            {
                header_found=true;
                break;
            }
        }   
    }
    //跳出while循环，但是还是没发现整包数据
    if (!header_found)
    {
        return hardware_interface::return_type::OK;
    }    
    try
    {
        size_t remaining_size=sizeof(FeedbackPacket)-2;
        std::vector<uint8_t> remaining_data;
        remaining_data.resize(remaining_size);
        //读剩下的34个字节
        serial_conn_.Read(remaining_data, remaining_size);
        //拼包
        fb_packet_.data.head1=0x55;
        fb_packet_.data.head2=0xAA;
        //将数据后移放到fb_packet_里面
        std::memcpy(fb_packet_.buffer + 2, remaining_data.data(), remaining_size);

        //校验尾部最后两个位置
        if (fb_packet_.data.tail1 != 0x0D || fb_packet_.data.tail2 != 0x0A)
        {
            // 尾部不对，说明中间有错位，放弃这一包
            return hardware_interface::return_type::OK;
        }
        hw_positions_[0] = fb_packet_.data.actual_pos[0];
        hw_positions_[1] = fb_packet_.data.actual_pos[1];
        hw_positions_[2] = fb_packet_.data.actual_pos[2];
        hw_positions_[3] = fb_packet_.data.actual_pos[3];

        hw_velocities_[0] = fb_packet_.data.actual_vel[0];
        hw_velocities_[1] = fb_packet_.data.actual_vel[1];
        hw_velocities_[2] = fb_packet_.data.actual_vel[2];
        hw_velocities_[3] = fb_packet_.data.actual_vel[3];      


    }
    catch(...)
    {
       RCLCPP_ERROR(rclcpp::get_logger("MbotSystemHardware"), "Error reading serial!");
    }
    return hardware_interface::return_type::OK;
}

//导出状态接口 (State Interfaces)
std::vector<hardware_interface::StateInterface> MbotSystemHardware::export_state_interfaces()
{
    std::vector<hardware_interface::StateInterface> state_interfaces;

    for (auto i = 0u; i < info_.joints.size(); i++)
    {
        // 1. 导出位置状态 (Position) -> 绑定到 hw_positions_[i]
        state_interfaces.emplace_back(hardware_interface::StateInterface(
        info_.joints[i].name, "position", &hw_positions_[i]));

        // 2. 导出速度状态 (Velocity) -> 绑定到 hw_velocities_[i]
        state_interfaces.emplace_back(hardware_interface::StateInterface(
        info_.joints[i].name, "velocity", &hw_velocities_[i]));
    }
    return state_interfaces;
}

//导出命令接口 (Command Interfaces)
std::vector<hardware_interface::CommandInterface> MbotSystemHardware::export_command_interfaces()
{
  std::vector<hardware_interface::CommandInterface> command_interfaces;

  // 遍历 4 个关节
  for (auto i = 0u; i < info_.joints.size(); i++)
  {
    // 导出速度命令 (Velocity) -> 绑定到 hw_commands_[i]
    // 意思就是：ROS 控制器往这个接口写数据，就会直接改写 hw_commands_[i] 的值
    command_interfaces.emplace_back(hardware_interface::CommandInterface(
      info_.joints[i].name, "velocity", &hw_commands_[i]));
  }

  return command_interfaces;
}



}




// 注册插件
// 参数1: 类名 (带命名空间)
// 参数2: 父类名 (这是必须要写的固定格式)
PLUGINLIB_EXPORT_CLASS(
  mbot_hardware::MbotSystemHardware, 
  hardware_interface::SystemInterface
)




