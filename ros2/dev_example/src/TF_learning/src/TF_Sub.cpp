#include "rclcpp/rclcpp.hpp"
#include "tf2/utils.h"
#include "tf2_ros/transform_listener.h"

class TF_Sub:public rclcpp::Node
{
    public:
        TF_Sub(std::string name):Node(name)
        {
            listener=std::make_shared<tf2_ros::TransformListener>(buffer);
            timer=this->create_wall_timer(std::chrono::milliseconds(1000),std::bind(&TF_Sub::timer_callback,this));
            
        }
        

    private:
        std::shared_ptr<tf2_ros::TransformListener> listener;
        tf2::BufferCore buffer;
        rclcpp::TimerBase::SharedPtr timer;
        void timer_callback(void)
        {
            geometry_msgs::msg::TransformStamped transform=buffer.lookupTransform("base", "child", tf2::TimePointZero);
            RCLCPP_INFO(this->get_logger(), "Received TF: Translation(%f, %f, %f), Rotation(%f, %f, %f, %f)",
            transform.transform.translation.x, transform.transform.translation.y, transform.transform.translation.z,
            transform.transform.rotation.x, transform.transform.rotation.y, transform.transform.rotation.z, transform.transform.rotation.w);
        } 
};


int main(int argv,char**argc)
{   
    rclcpp::init(argv,argc);
    auto Node0_handle=std::make_shared<TF_Sub>("TF_Sub");
    rclcpp::spin(Node0_handle);
    rclcpp::shutdown();
    return 0;
}