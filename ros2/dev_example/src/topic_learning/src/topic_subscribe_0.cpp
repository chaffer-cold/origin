#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"
class MySubscribe:public rclcpp::Node
{
    public:
    MySubscribe(std::string name):Node(name)
    {
        RCLCPP_INFO(this->get_logger(),"%s success started",name.c_str());
        subscribe0_handle=this->create_subscription<std_msgs::msg::String>("test_topic",10,std::bind(&MySubscribe::subscribe0_callback,this,std::placeholders::_1));
    } 
    private:
    rclcpp::Subscription<std_msgs::msg::String>::SharedPtr subscribe0_handle;
    void subscribe0_callback(std_msgs::msg::String::SharedPtr message)
    {
        RCLCPP_INFO(this->get_logger(),"收到消息,消息为%s",message->data.c_str());
    }
};
       
int main(int argv,char**argc)
{
    rclcpp::init(argv,argc);
    auto Subscribe0=std::make_shared<MySubscribe>("Subscribe0");
    rclcpp::spin(Subscribe0);
    rclcpp::shutdown();
    return 0;    
}
