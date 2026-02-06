import os
from launch import LaunchDescription
from launch.actions import IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node
from ament_index_python import get_package_share_directory

def generate_launch_description():
    # 1. 找路径
    pkg_share = get_package_share_directory('mbot_description')
    slam_params_file = os.path.join(pkg_share, 'config', 'mapper_params_online_async.yaml')

    # 2. 引用自带的 launch 文件
    slam_launch = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(get_package_share_directory('slam_toolbox'), 'launch', 'online_async_launch.py')
        ),
        launch_arguments={
            'use_sim_time': 'true',  # 强调仿真时间
            'slam_params_file': slam_params_file
        }.items()
    )

    return LaunchDescription([
        slam_launch,
    ])