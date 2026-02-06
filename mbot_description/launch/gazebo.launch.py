import os
from launch import LaunchDescription
from launch.actions import  ExecuteProcess , IncludeLaunchDescription ,RegisterEventHandler
from launch.event_handlers import OnProcessExit
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import Command , LaunchConfiguration 
from launch_ros.actions import Node
from ament_index_python import get_package_share_directory

def generate_launch_description():

    #找路径
    pkg_share=get_package_share_directory("mbot_description")
    xacro_path=os.path.join(pkg_share,"urdf","mbot.urdf.xacro")
    world_path=os.path.join(pkg_share,"world","my_room.world")
    rviz_config_path = os.path.join(pkg_share, 'config', 'mbot.rviz')
    slam_params_path = os.path.join(pkg_share, 'config', 'mapper_params_online_async.yaml')

    use_sim_time = LaunchConfiguration('use_sim_time', default='true')

    #解析xacro
    robot_description_config=Command(["xacro ",xacro_path ," use_sim:=true"])

    #节点xacro内容发布，发布到rviz2的robot_description
    node_robot_state_publisher=Node(
        package="robot_state_publisher",
        executable="robot_state_publisher",
        parameters=[{
            'robot_description': robot_description_config,
            'use_sim_time': use_sim_time 
        }],
        output="screen"

    )

    #加载gazebo仿真环境----引用写好的launch文件
    gazebo_launch=IncludeLaunchDescription(
        PythonLaunchDescriptionSource([
            get_package_share_directory("gazebo_ros"),"/launch/gazebo.launch.py"
        ]),
        launch_arguments={'world': world_path}.items()
    )

    #spawn生成机器人到gazebo世界里
    spawn_entity=Node(
        package="gazebo_ros",
        executable="spawn_entity.py",
        arguments=["-topic","robot_description",
                   "-entity","mbot"],
        output="screen"

    )
    # 加载关节状态广播器 (负责发布 /joint_states)
    load_joint_state_broadcaster = ExecuteProcess(
        cmd=['ros2', 'control', 'load_controller', '--set-state', 'active',
             'joint_state_broadcaster'],
        output='screen'
    )

    # 加载麦轮控制器 (负责移动)
    load_mbot_controller = ExecuteProcess(
        cmd=['ros2', 'control', 'load_controller', '--set-state', 'active',
             'mbot_base_controller'],
        output='screen'
    )

    #加载红绿灯控制，必须先spawn机器人模型，才能执行 load_joint_state_broadcaster load_mbot_controller
    load_broadcaster_controllers = RegisterEventHandler(
        event_handler=OnProcessExit(
            target_action=spawn_entity,
            on_exit=[load_joint_state_broadcaster, load_mbot_controller],
        )
    )
    # Nav2 发出 /cmd_vel，我们把它转给麦轮控制器
    cmd_vel_relay = Node(
        package='topic_tools',
        executable='relay',
        name='cmd_vel_relay',
        output='screen',
        arguments=['/cmd_vel', '/mbot_base_controller/reference_unstamped'],
        parameters=[{'use_sim_time': True}]
    )

    #启动补丁脚本，手动拼接odom到baseprintfoot
    odom_tf_node = ExecuteProcess(
        cmd=['python3', os.path.join(pkg_share, 'launch', 'odom_tf.py')],
        output='screen'
    )
    #启动rviz
    node_rviz = Node(
        package='rviz2',
        executable='rviz2',
        name='rviz2',
        output='screen',
        # 加载保存好的配置文件，如果文件不存在，去掉这一行 arguments 即可
        arguments=['-d', rviz_config_path], 
        parameters=[{'use_sim_time': use_sim_time}]
    )

        # 2. 引用自带的 launch 文件
    slam_launch = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(get_package_share_directory('slam_toolbox'), 'launch', 'online_async_launch.py')
        ),
        launch_arguments={
            'use_sim_time': 'true',  # 强调仿真时间
            'slam_params_file': slam_params_path
        }.items()
    )

    return LaunchDescription([
        node_robot_state_publisher,
        gazebo_launch,
        spawn_entity,
        load_broadcaster_controllers,
        cmd_vel_relay,
        odom_tf_node,
        node_rviz,
        slam_launch

    ])