#include "rclcpp/rclcpp.hpp"
#include "rclcpp_action/rclcpp_action.hpp"
#include "example_ros2_interfaces/action/move_robot.hpp"
class robotcontrol :public rclcpp::Node
{
    public:
    using MoveRobot = example_ros2_interfaces::action::MoveRobot;
    robotcontrol(std::string name):Node(name)
    {
        RCLCPP_INFO(this->get_logger(),"%s is on line",name.c_str());
        client=rclcpp_action::create_client<MoveRobot>(this,"move_robot");
        tim1=this->create_wall_timer(std::chrono::milliseconds(1000),std::bind(&robotcontrol::send_goal,this));
        //tim2=this->create_wall_timer(std::chrono::milliseconds(5000),std::bind(&robotcontrol::task_cancel,this));
    }

    private:
    std::shared_ptr<rclcpp_action::Client<MoveRobot>> client;
    std::shared_ptr<rclcpp::TimerBase> tim1;
    std::shared_ptr<rclcpp::TimerBase> tim2;
    
    void send_goal(void)
    {
        using namespace std::placeholders;
        tim1->cancel();
        if(!client->wait_for_action_server(std::chrono::milliseconds(2000)))
        {
            RCLCPP_ERROR(this->get_logger(),"服务器G了");
            rclcpp::shutdown();
            return;
        }
        
        auto goal_msg=MoveRobot::Goal();
        goal_msg.distance=30;
        RCLCPP_INFO(this->get_logger(),"Sending goal");
        auto send_goal_options=rclcpp_action::Client<MoveRobot>::SendGoalOptions();
        send_goal_options.goal_response_callback=std::bind(&robotcontrol::goal_response_callback,this,_1);
        send_goal_options.feedback_callback=std::bind(&robotcontrol::feedback_callback,this,_1,_2);
        send_goal_options.result_callback=std::bind(&robotcontrol::result_callback,this,_1);
        
        auto goal_handle_future= client->async_send_goal(goal_msg,send_goal_options);
        
       
    }

    void task_cancel(rclcpp_action::ClientGoalHandle<MoveRobot>::SharedPtr goal_handle)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(5000));
        client->async_cancel_goal(goal_handle,NULL);
    }
    void goal_response_callback(const rclcpp_action::ClientGoalHandle<MoveRobot>::SharedPtr goal_handle)
    {
        if (!goal_handle) 
        {
            RCLCPP_ERROR(this->get_logger(), "Goal was rejected by server");
        } 
        else 
        {
            RCLCPP_INFO(this->get_logger(),"Goal accepted by server, waiting for result");
            std::thread{std::bind(&robotcontrol::task_cancel,this,std::placeholders::_1),goal_handle}.detach();

        }
    }
    void feedback_callback(rclcpp_action::ClientGoalHandle<MoveRobot>::SharedPtr,const std::shared_ptr<const MoveRobot::Feedback>feedback)
    {
        RCLCPP_INFO(this->get_logger(), "Feedback current pose:%f", feedback->pose);
    }
    void result_callback(const rclcpp_action::ClientGoalHandle<MoveRobot>::WrappedResult& result)
    {
        switch (result.code)
        {
            case rclcpp_action::ResultCode::SUCCEEDED:
                break;
            case rclcpp_action::ResultCode::ABORTED:
                RCLCPP_ERROR(this->get_logger(), "Goal was aborted");
                return;
            case rclcpp_action::ResultCode::CANCELED:
                RCLCPP_ERROR(this->get_logger(), "Goal was canceled");
                return;
            default:
                RCLCPP_ERROR(this->get_logger(), "Unknown result code");
                return;
                    
        }
        RCLCPP_INFO(this->get_logger(), "Result received: %f", result.result->pose);
    }

};
int main(int argv,char**argc)
{
    rclcpp::init(argv,argc);
    auto robot_client=std::make_shared<robotcontrol>("robot1");
    rclcpp::spin(robot_client);
    rclcpp::shutdown();
    return 0;

}