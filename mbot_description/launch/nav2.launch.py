import os
from ament_index_python import get_package_share_directory
from launch import LaunchDescription
from launch.actions import IncludeLaunchDescription 
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import  Node


def generate_launch_description():

    #获取包路径
    pkg_mbot_share_path=get_package_share_directory("mbot_description")
    pkg_nav_bringup_share_path=get_package_share_directory("nav2_bringup")

    #拼接成完整路径
    map_path=os.path.join(pkg_mbot_share_path,"maps","first_map.yaml")
    my_params_file = os.path.join(pkg_mbot_share_path, 'config', 'my_nav2_params.yaml')
    #启动一键导航launch,自定义参数配置
    nav2_launch=IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(pkg_nav_bringup_share_path,"launch","bringup_launch.py")
        ),
        launch_arguments={
            "map":map_path,
            "use_sim_time":"true",
            "params_file":my_params_file
        }.items()


    )
    #启动bringup默认配置好的rviz界面
    rviz2_node=Node(
        package="rviz2",
        executable="rviz2",
        name="rviz2",
        arguments=["-d", os.path.join(pkg_nav_bringup_share_path,"rviz","nav2_default_view.rviz")],
        parameters=[{"use_sim_time":True}]

    )
    return LaunchDescription([
        nav2_launch,
        rviz2_node,

    ])