import os
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import Command, FindExecutable, PathJoinSubstitution ,LaunchConfiguration
from launch_ros.actions import Node 

def generate_launch_description():
    
    # 1. 定位功能包路径
    pkg_share = get_package_share_directory("mbot_description")

    # 2. 准备文件路径
    # URDF 文件
    model_path = os.path.join(pkg_share, "urdf/mbot.urdf.xacro")
    # YAML 配置文件 (上一把我们写的那个)
    controller_config_path = os.path.join(pkg_share, "config/mbot_controller.yaml")

    use_sim_time_arg = DeclareLaunchArgument(
        'use_sim_time',
        default_value='false',
        description='Use simulation (Gazebo) clock if true'
    )
    # 获取参数的值
    use_sim_time = LaunchConfiguration('use_sim_time')  

    # 3. 解析 Xacro 
    robot_description_content = Command(
        [
            PathJoinSubstitution([FindExecutable(name="xacro")]),
            " ",
            model_path,
            " ",
            "use_sim:=",use_sim_time, 
        ]
    )
    
    # 包装成参数字典
    robot_description = {"robot_description": robot_description_content}

    # 4. 节点：Robot State Publisher
    # 负责发布机器人的静态 TF 变换和模型描述
    node_robot_state_publisher = Node(
        package="robot_state_publisher",
        executable="robot_state_publisher",
        output="screen",
        parameters=[
            robot_description,
            {'use_sim_time': use_sim_time} 
        ]
    )

    # 5. 节点：Controller Manager (ROS 2 Control 的大脑)
    # 它负责加载 C++ 硬件驱动，并读取 YAML 里的参数
    node_controller_manager = Node(
        package="controller_manager",
        executable="ros2_control_node",
        parameters=[
            robot_description,      # 把 URDF 喂给它
            controller_config_path,  # 把 YAML 喂给它
            {'use_sim_time': use_sim_time}
        ],
        output="screen",
    )

    # 6. 节点：加载关节状态广播器 (Joint State Broadcaster)
    # 负责从硬件读取编码器数据 -> 发布 /joint_states
    spawn_joint_state_broadcaster = Node(
        package="controller_manager",
        executable="spawner",
        arguments=["joint_state_broadcaster", "--controller-manager", "/controller_manager"],
        output="screen",
    )

    # 7. 节点：加载底盘控制器 (Mecanum Controller)
    # 负责接收 /cmd_vel -> 计算轮速 -> 发送给硬件
    # 【注意】名字 "mbot_base_controller" 必须和你 YAML 文件里的一层名字一致
    spawn_mbot_controller = Node(
        package="controller_manager",
        executable="spawner",
        arguments=["mbot_base_controller", "--controller-manager", "/controller_manager"],
        output="screen",
    )

    # 8. (可选) 节点：Rviz2
    # 为了方便调试，默认启动 Rviz。如果在无屏幕的树莓派上跑，可以注释掉这部分。
    node_rviz2 = Node(
        package="rviz2",
        executable="rviz2",
        name='rviz2',
        output='screen',
        # 如果你有保存好的 rviz 配置，可以在这里加载
        # arguments=['-d', os.path.join(pkg_share, 'rviz', 'display.rviz')]
    )

    # 返回 Launch 描述
    return LaunchDescription([
        use_sim_time_arg,
        node_robot_state_publisher,
        node_controller_manager,
        spawn_joint_state_broadcaster,
        spawn_mbot_controller,
        # node_rviz2, # 如果不想自动弹窗，就把这行注释掉
    ])