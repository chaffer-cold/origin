#include "rclcpp/rclcpp.hpp"
#include "tf2/utils.h"
#include "tf2_ros/transform_broadcaster.h"


class TF2_Pub:public rclcpp::Node
{
    public:
        TF2_Pub(std::string name):Node(name)
        {
            tf_broadcaster=std::make_unique<tf2_ros::TransformBroadcaster>(this);
            tim_handle=this->create_wall_timer(std::chrono::milliseconds(100),std::bind(&TF2_Pub::tim_callback,this));
        }
    private:
            geometry_msgs::msg::TransformStamped transform;
            std::unique_ptr<tf2_ros::TransformBroadcaster> tf_broadcaster;
            std::shared_ptr<rclcpp::TimerBase> tim_handle;
            void tim_callback(void)
            {   
                transform.header.frame_id="base";
                transform.header.stamp=now();
                transform.child_frame_id="child";
                auto quat=tf2::Quaternion();
                quat.setRPY(3.14,0.0,0.0);
                transform.transform.rotation.x=quat.x();
                transform.transform.rotation.y=quat.y();
                transform.transform.rotation.z=quat.z();
                transform.transform.rotation.w=quat.w();
                tf_broadcaster->sendTransform(transform);

            }
};


int main(int argv,char**argc)
{
    rclcpp::init(argv,argc);
    auto Node_handle=std::make_shared<TF2_Pub>("TF2_Pub");
    rclcpp::spin(Node_handle);
    rclcpp::shutdown();



    return 0;
}