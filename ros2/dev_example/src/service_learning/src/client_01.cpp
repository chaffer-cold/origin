#include "rclcpp/rclcpp.hpp"
#include <iostream>
#include "example_interfaces/srv/add_two_ints.hpp"
class Myclient:public rclcpp::Node
{
    public:
    Myclient(std::string name):Node(name)
    {
        RCLCPP_INFO(this->get_logger(),"%s successfully started",name.c_str());
        client_handle=this->create_client<example_interfaces::srv::AddTwoInts>("add_two_ints_srv");
        while(!client_handle->wait_for_service(std::chrono::seconds(2)))
        {
            if(!rclcpp::ok())
            {
                RCLCPP_ERROR(this->get_logger(), "等待服务的过程中被打断...");
                return;
            }
            else
            {
                RCLCPP_INFO(this->get_logger(),"等待服务器上线，小崽子汗流浃背了吧");
            }
        }
        
    }
    
    void send_request(int a,int b)
    {   
        auto request=std::make_shared<example_interfaces::srv::AddTwoInts::Request>();    
        
        request->a=a; 
        request->b=b;
        client_handle->async_send_request(request,std::bind(&Myclient::send_request_callback,this,std::placeholders::_1));
    }

    private:
    //std::shared_ptr<example_interfaces::srv::AddTwoInts_Request> request;
    rclcpp::Client<example_interfaces::srv::AddTwoInts>::SharedPtr client_handle;
    void send_request_callback(rclcpp::Client<example_interfaces::srv::AddTwoInts>::SharedFuture future)
    {
        auto response=future.get();
        
        RCLCPP_INFO(this->get_logger(),"数据收到了,为%d",response->sum);

    }

};

int main(int argv,char**argc)
{
    rclcpp::init(argv,argc);
    auto client_01=std::make_shared<Myclient>("client_01");
    client_01->send_request(std::stoi(argc[1]),std::stoi(argc[2]));
    rclcpp::spin(client_01);
    rclcpp::shutdown();
    
    return 0;
}