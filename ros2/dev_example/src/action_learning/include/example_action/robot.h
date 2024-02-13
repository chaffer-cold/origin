#ifndef __ROBOT_H
#define __ROBOT_H
#include "rclcpp/rclcpp.hpp"
#include "example_ros2_interfaces/action/move_robot.hpp"
class Robot
{
    public:
        using MoveRobot = example_ros2_interfaces::action::MoveRobot;
        //每500ms移动一小步
        float move_step();
        /*移动一小段距离*/
        bool set_goal(float distance);
        /*获取当前位置*/
        float get_current_pose();
        /*获取状态*/
        int get_status();   
        /*判断是否接近目标*/
        bool close_goal();      
        /*停止*/
        void stop_move();
    private:
        /*当前位置*/
        float current_pose=0.0;  
        /*目标位置*/
        float target_pose=0.0;
        /*目标位置*/
        float move_distance=0.0;
        /*取消标志*/
        std::atomic<bool> cancel_flag{false};
        /*状态*/
        int status=MoveRobot::Feedback::STATUS_STOP;
};  

#endif
