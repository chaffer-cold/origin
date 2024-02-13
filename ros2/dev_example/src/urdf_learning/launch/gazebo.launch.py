from launch import LaunchDescription
from launch_ros.actions import Node
from launch.actions import ExecuteProcess

def generate_launch_description():
    launchdescription=LaunchDescription()
    robot_name_in_model="elaina"
    urdf_model_path="/home/fishros/dev_learning/src/urdf_learning/urdf/elaina_gazebo.urdf"
    gazebo_world_path='/home/fishros/.gazebo/world/elaina.world'
     # Start Gazebo server
    start_gazebo_cmd =  ExecuteProcess(
        cmd=['gazebo', '--verbose','-s', 'libgazebo_ros_init.so', '-s', 'libgazebo_ros_factory.so',gazebo_world_path],
        output='screen')

    # Launch the robot
    spawn_entity_cmd = Node(
        package='gazebo_ros', 
        executable='spawn_entity.py',
        arguments=['-entity', robot_name_in_model,'-file', urdf_model_path ], output='screen')
    
    
    robot_state_publisher_node=Node(
        package="robot_state_publisher",
        executable="robot_state_publisher",
        arguments=[urdf_model_path]
        )
   
   
   
    rviz2_node=Node(
        package="rviz2",
        executable="rviz2",
        name="rviz2",
        output="screen"

    )

    launchdescription.add_action(start_gazebo_cmd)
    launchdescription.add_action(spawn_entity_cmd)
    launchdescription.add_action(robot_state_publisher_node)
    launchdescription.add_action(rviz2_node)
    return launchdescription
