#include "rclcpp/rclcpp.hpp"
#include "rclcpp_action/rclcpp_action.hpp"
#include "example_action/robot.h"
#include "example_ros2_interfaces/action/move_robot.hpp"

class actionrobot:public rclcpp::Node
{
    public:
    using MoveRobot=example_ros2_interfaces::action::MoveRobot;
    actionrobot(std::string name):Node(name)
    {
        using namespace std::placeholders;
        RCLCPP_INFO(this->get_logger(), "节点已启动：%s.", name.c_str());
        server=rclcpp_action::create_server<MoveRobot>(this,"move_robot",
        std::bind(&actionrobot::handle_goal,this,_1,_2),
        std::bind(&actionrobot::handle_cancel,this,_1),
        std::bind(&actionrobot::handle_accepted,this,_1));

    }

    private:
    Robot robot_example;
    rclcpp_action::Server<MoveRobot>::SharedPtr server; 
    rclcpp_action::GoalResponse handle_goal(const rclcpp_action::GoalUUID& uuid,std::shared_ptr<const MoveRobot::Goal> goal)
    {
        RCLCPP_INFO(this->get_logger(), "Received goal request with distance %f",goal->distance);
        if(fabs(goal->distance)>100)
        {   
            RCLCPP_WARN(this->get_logger(),"目标距离太远了，本机器人表示拒绝！");
            return rclcpp_action::GoalResponse::REJECT;
        }
        else
        {
            RCLCPP_INFO(this->get_logger(),"目标距离%f我可以走到，本机器人接受，准备出发！",goal->distance);
            return rclcpp_action::GoalResponse::ACCEPT_AND_EXECUTE;
        }

    }
    rclcpp_action::CancelResponse handle_cancel(const std::shared_ptr<rclcpp_action::ServerGoalHandle<MoveRobot>> goal_handle)
    {
        RCLCPP_INFO(this->get_logger(), "Received request to cancel goal");
        robot_example.stop_move();
        return rclcpp_action::CancelResponse::ACCEPT;

    
    }
    void execute_move(const std::shared_ptr<rclcpp_action::ServerGoalHandle<MoveRobot>> goal_handle)
    {
        auto goal=goal_handle->get_goal();
        RCLCPP_INFO(this->get_logger(), "开始执行移动 %f 。。。", goal->distance);
        auto result=std::make_shared<MoveRobot::Result>();
        rclcpp::Rate rate=rclcpp::Rate(2);
        robot_example.set_goal(goal->distance);
        while(rclcpp::ok()&&!robot_example.close_goal())
        {
            robot_example.move_step();
            auto feedback=std::make_shared<MoveRobot::Feedback>();
            feedback->pose=robot_example.get_current_pose();
            feedback->status=robot_example.get_status();
            goal_handle->publish_feedback(feedback);
            if(goal_handle->is_canceling())
            {
                result->pose=robot_example.get_current_pose();
                goal_handle->canceled(result);
                RCLCPP_INFO(this->get_logger(), "Goal Canceled");
                return;
            }
            RCLCPP_INFO(this->get_logger(), "Publish Feedback"); /*Publish feedback*/
            rate.sleep();
            
        }
        result->pose=robot_example.get_current_pose();
        goal_handle->succeed(result);
        RCLCPP_INFO(this->get_logger(), "Goal Succeeded");
    }
    void handle_accepted(const std::shared_ptr<rclcpp_action::ServerGoalHandle<MoveRobot>> goal_handle)
    {
        std::thread( std::bind(&actionrobot::execute_move,this,std::placeholders::_1),goal_handle).detach();
    }
};
int main(int argv,char**argc)
{
    rclcpp::init(argv,argc);
    auto robot1_serve=std::make_shared<actionrobot>("robot1");
    rclcpp::spin(robot1_serve);
    rclcpp::shutdown();
    return 0;
}
