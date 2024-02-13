#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"
class Mypublisher:public rclcpp::Node
{
    public:
    Mypublisher(const std::string name):Node(name)
    {
        RCLCPP_INFO(this->get_logger(),"%s is work",name.c_str());
        command_publisher=this->create_publisher<std_msgs::msg::String>("test_topic",10);
        tim_handle=this->create_wall_timer(std::chrono::milliseconds(500),std::bind(&Mypublisher::timer_callback,this));
        
    }



    private:
        rclcpp::Publisher<std_msgs::msg::String>::SharedPtr command_publisher;
        rclcpp::TimerBase::SharedPtr tim_handle;
        void timer_callback(void)
        {
            //auto message=std::make_shared<std_msgs::msg::String>();
            //message->data="oh fuck njust";
            std_msgs::msg::String message;
            message.data="oh fk njust";
            RCLCPP_INFO(this->get_logger(),"is working"); 
            command_publisher->publish(message);
        }
};

int main(int argv,char**argc)
{
    rclcpp::init(argv,argc);
    auto publisher0=std::make_shared<Mypublisher>("publisher0");
    rclcpp::spin(publisher0);
    rclcpp::shutdown();
    return 0;
}
