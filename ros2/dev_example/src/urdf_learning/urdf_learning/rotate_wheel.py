import rclpy                           # 初始化 ROS
from rclpy.node import Node
from sensor_msgs.msg import JointState
import time
import threading

class rotate_wheel_node(Node):
    def __init__(self, name):           # 初始化节点,输出日志信息
        super().__init__(name)
        self.get_logger().info(f"node{name} init")
        
        self.publisher = self.create_publisher(JointState, "joint_states", 10)  # 创建 ROS发布器
        self.pub_rate = self.create_rate(5)                                    # 创建 ROS速度
        
        self.init_jointstate()            # 初始化机器人状态
        
        self.thread = threading.Thread(target=self.thread_pub)  # 创建线程,用于 ROS发布
        self.thread.start()               # 启动线程

    def init_jointstate(self):
        self.joint_speeds = [0.0, 0.0]        # 设置机器人状态初始值
        self.joint_states = JointState()  # 创建 ROS消息
        self.joint_states.header.stamp = self.get_clock().now().to_msg()       # 设置时间戳
        self.joint_states.header.frame_id = ''                                # 设置消息的 frame_id和 name
        self.joint_states.name = ["left_wheel_joint", "right_wheel_joint"]
        self.joint_states.position = [0.0, 0.0]                                   # 设置位置
        self.joint_states.velocity = self.joint_speeds                        # 设置速度
        self.joint_states.effort = []                                        # 设置 effort为空
    
    def update_speed(self, speeds):         # 更新速度
        self.joint_speeds = speeds
    
    def thread_pub(self):
        last_time = time.time()            # 记录最后时间
        while rclpy.ok():                   # 循环执行
            delta_time = time.time() - last_time                             # 计算时间差
            self.joint_states.position[0] += delta_time * self.joint_states.velocity[0]  # 更新位置和速度
            self.joint_states.position[1] += delta_time * self.joint_states.velocity[1]
            self.joint_states.velocity = self.joint_speeds                    # 更新速度
            self.joint_states.header.stamp = self.get_clock().now().to_msg()  # 更新时间戳
            self.publisher.publish(self.joint_states)                         # 发布消息
            self.pub_rate.sleep()                                             # 控制线程睡眠
def main(args=None):
    rclpy.init(args=args)                 # 初始化 ROS
    node = rotate_wheel_node("rotate_wheel_node")  # 创建节点
    node.update_speed([15.0, -15.0])          # 更新速度
    rclpy.spin(node)                      # 运行节点
    rclpy.shutdown()                      # 关闭 ROS
