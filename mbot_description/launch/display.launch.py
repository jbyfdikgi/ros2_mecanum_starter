import os
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.substitutions import Command
from launch_ros.actions import Node


def generate_launch_description():
    
    # 1. 找到 mbot_description 这个包安装在哪里
    pkg_share=get_package_share_directory("mbot_description")

    # 2. 拼接出 xacro 文件的绝对路径
    default_model_path=os.path.join(pkg_share,"urdf/mbot.urdf.xacro")

    #3.处理xacro文件，翻译成xml格式
    robot_description_content=Command(["xacro ",default_model_path])

    #4.机器人状态发布节点(RSP)
    #Sub:joint_states
    #Pub:tf  tf_static   robot_description
    node_robot_state_publisher=Node(
        package="robot_state_publisher",
        executable="robot_state_publisher",
        output="screen",
        #传参给rviz2
        parameters=[{"robot_description": robot_description_content}]

    )

    #5.机器人关节发布节点
    joint_state_publisher=Node(
        package="joint_state_publisher",
        executable="joint_state_publisher",
        name="joint_state_publisher"

    )

    node_rviz2=Node(
        package="rviz2",
        executable="rviz2",
        name='rviz2',
        output='screen'        

    )

    return LaunchDescription([
        node_robot_state_publisher,
        joint_state_publisher,
        node_rviz2

    ])
