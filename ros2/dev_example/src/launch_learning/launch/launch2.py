from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
    Node1=Node(
        package="example_parameters_rclcpp",
        executable="parameters_basic",
       namespace="rclcpp",
        parameters=[{'rcl_log_level':40}]

    )
    Node2=Node(
        package="example_parameters_rclpy",
        executable="parameters_basic",
        namespace="rclpy",
        parameters=[{'rcl_log_level':50}]
    )
    launch_description=LaunchDescription([Node1,Node2])
    return launch_description
