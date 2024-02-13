from launch_ros.actions import Node
from launch import LaunchDescription

def generate_launch_description():
    action_control=Node(
        package="action_learning",
        executable="action_control"

    )
    action_robot=Node(
        package="action_learning",
        executable="action_robot"
    )
    launch_description=LaunchDescription([action_control,action_robot])



    return launch_description
