from nav2_simple_commander.robot_navigator import BasicNavigator
import rclpy
from copy import deepcopy
from geometry_msgs.msg import PoseStamped
from nav2_msgs.action import NavigateToPose
from nav2_msgs.action import _navigate_to_pose
rclpy.init()
nav = BasicNavigator()
navigator.waitUntilNav2Active()
# ======================初始化位置，代替rviz2的2D Pose Estimate===============================
initial_pose = PoseStamped()
initial_pose.header.frame_id = 'map'
initial_pose.header.stamp = navigator.get_clock().now().to_msg()
initial_pose.pose.position.x = 0.0
initial_pose.pose.position.y = 0.0
initial_pose.pose.orientation.w = 1.0
navigator.setInitialPose(initial_pose)
#========================导航到目标点1===========================================
goal_pose1 = deepcopy(initial_pose)
goal_pose1.pose.position.x = 1.5
nav.goToPose(goal_pose1)
while not nav.isNavComplete():
  feedback = nav.getFeedback()
  #检查是否超时，超时则停止导航到点   
  if feedback.navigation_duration > 600:
    nav.cancelNav()


#================================导航到目标点2==================================
goal_pose2 = deepcopy(initial_pose)
goal_pose2.pose.position.x = -1.5

nav.goToPose(goal_pose2)
while not nav.isNavComplete():
  feedback = nav.getFeedback()
  #检查是否超时，超时则停止导航到点   
  if feedback.navigation_duration > 600:
    nav.cancelNav()

#===============================查看返回结果=====================================
result = nav.getResult()
if result == NavigationResult.SUCCEEDED:
    print('Goal succeeded!')
elif result == NavigationResult.CANCELED:
    print('Goal was canceled!')
elif result == NavigationResult.FAILED:
    print('Goal failed!')
