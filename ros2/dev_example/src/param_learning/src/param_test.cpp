#include "rclcpp/rclcpp.hpp"

class Myparam_test_node:public rclcpp::Node
{
    public:
    Myparam_test_node(std::string name):Node(name)
    {
        RCLCPP_INFO(this->get_logger(),"%s已上线",name.c_str());
        timer_handle=this->create_wall_timer(std::chrono::milliseconds(1000),std::bind(&Myparam_test_node::timer_callback,this));
        this->declare_parameter("rcl_log_level",0);


    }



    private:
    int log_level=0;

    rclcpp::TimerBase::SharedPtr timer_handle;
    void timer_callback(void)
    {
        this->get_parameter("rcl_log_level",log_level);
        this->get_logger().set_level((rclcpp::Logger::Level)log_level);
        std::cout<<"======================================================"<<std::endl;
        RCLCPP_DEBUG(this->get_logger(), "我是DEBUG级别的日志,我被打印出来了!");
        RCLCPP_INFO(this->get_logger(), "我是INFO级别的日志,我被打印出来了!");
        RCLCPP_WARN(this->get_logger(), "我是WARN级别的日志,我被打印出来了!");
        RCLCPP_ERROR(this->get_logger(), "我是ERROR级别的日志,我被打印出来了!");
        RCLCPP_FATAL(this->get_logger(), "我是FATAL级别的日志,我被打印出来了!");


    }

};



int main(int argv,char**argc)
{   
    rclcpp::init(argv,argc);
    auto param_test_handle=std::make_shared<Myparam_test_node>("param_test_node");
    rclcpp::spin(param_test_handle);
    rclcpp::shutdown();


    return 0;
}