#!/usr/bin/env python3
import rclpy
from rclpy.node import Node
from nav_msgs.msg import Odometry
from tf2_ros import TransformBroadcaster
from geometry_msgs.msg import TransformStamped

class OdomTfBroadcaster(Node):
    def __init__(self):
        super().__init__('odom_tf_broadcaster')
        # 1. 创建 TF 广播器
        self.tf_broadcaster = TransformBroadcaster(self)
        
        # 2. 订阅里程计数据 (注意这里的话题名要和你 echo 出来的一样！)
        self.subscription = self.create_subscription(
            Odometry,
            '/mbot_base_controller/odometry',  # <--- 关键：必须对应你的真实话题
            self.handle_odom,
            10)
        self.get_logger().info('Odom TF Broadcaster Started!')

    def handle_odom(self, msg):
        t = TransformStamped()

        # 3. 填充头部信息
        t.header.stamp = msg.header.stamp
        t.header.frame_id = 'odom'        # 父坐标系
        t.child_frame_id = 'base_footprint' # 子坐标系 (你的 URDF 根节点)

        # 4. 搬运位置 (直接抄 msg 里的)
        t.transform.translation.x = msg.pose.pose.position.x
        t.transform.translation.y = msg.pose.pose.position.y
        t.transform.translation.z = msg.pose.pose.position.z

        # 5. 搬运旋转 (直接抄 msg 里的)
        t.transform.rotation = msg.pose.pose.orientation

        # 6. 发送出去！
        self.tf_broadcaster.sendTransform(t)

def main():
    rclpy.init()
    node = OdomTfBroadcaster()
    rclpy.spin(node)
    rclpy.shutdown()

if __name__ == '__main__':
    main()