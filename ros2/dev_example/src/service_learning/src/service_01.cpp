#include "rclcpp/rclcpp.hpp"
#include "example_interfaces/srv/add_two_ints.hpp"

class Myservice:public rclcpp::Node
{
    public:
    Myservice(std::string name):Node(name)
    {
        RCLCPP_INFO(this->get_logger(),"%s successfully start",name.c_str());
        service0_handle=this->create_service<example_interfaces::srv::AddTwoInts>("add_two_ints_srv",std::bind(&Myservice::service0_callback,this,std::placeholders::_1,std::placeholders::_2));
    }
    private:
    rclcpp::Service<example_interfaces::srv::AddTwoInts>::SharedPtr service0_handle;
    
    
    void service0_callback(std::shared_ptr<example_interfaces::srv::AddTwoInts::Request> request,
    std::shared_ptr<example_interfaces::srv::AddTwoInts::Response> response)
    {
        RCLCPP_INFO(this->get_logger(),"收到了a:%ld b:%ld",request->a,request->b);
        //模拟服务器处理数据
        rclcpp::sleep_for(std::chrono::milliseconds(3000));
        response->sum=request->a+request->b;

    } 
  
};


int main(int argv,char**argc)
{
    rclcpp::init(argv,argc);
    auto service_0=std::make_shared<Myservice>("service_01");
    rclcpp::spin(service_0);
    rclcpp::shutdown();
    return 0;

}