#include "rclcpp/rclcpp.hpp"

class Mynode:public rclcpp::Node
{
    public:
    Mynode(const std::string name):Node(name)
    {
        RCLCPP_INFO(this->get_logger(),"Node0 is started");
    }
    

};


int main(int argv,char**argc)
{
    rclcpp::init(argv,argc);
    auto node0=std::make_shared<Mynode>("node0");
    rclcpp::spin(node0);
    rclcpp::shutdown();
    return 0;
};
