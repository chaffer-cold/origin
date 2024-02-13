#include <Arduino.h>
#include "WiFi.h"
#include "Esp32PcntEncoder.h"
#include "Esp32McpwmMotor.h"

#include "micro_ros_platformio.h"
#include "rcl/rcl.h"
#include "rclc/rclc.h"
#include "rclc/executor.h"
#include "geometry_msgs/msg/twist.h"

#include "Kinematic.hpp"
#include "pid.hpp"

#define use_pub_odom 1
#define usart_trans_odom_data 0

#if use_pub_odom
  #include "nav_msgs/msg/odometry.h"
  #include "micro_ros_utilities/string_utilities.h"
#endif
//ros2相关
rcl_allocator_t allocator;
rclc_support_t support;
rcl_node_t node;
rclc_executor_t executor;
//ros2消息
rcl_subscription_t subscription;
geometry_msgs__msg__Twist msg_in;

#if use_pub_odom
  rcl_publisher_t publish;
  nav_msgs__msg__Odometry odom_msg;
#endif
struct Motor_t
{
  Esp32McpwmMotor pwm;
  Esp32PcntEncoder Encoder[2];
  Kinematic kinematic;
  pid_contrl PID[2];

};
Motor_t Motor;
void micro_ros_task(void* param);
void msg_in_callback(const void* msg_in);
void setup() {
  Serial.begin(115200);
  //初始化encoder与pwm
  Motor.Encoder[0].init(0,32,33);
  Motor.Encoder[1].init(1,26,25);
  Motor.pwm.attachMotor(0,22,23);
  Motor.pwm.attachMotor(1,12,13);
  //初始化pid
  Motor.PID[0].pid_update(0.625,0.103,0.03);
  Motor.PID[1].pid_update(0.625,0.103,0.03);
  Motor.PID[0].out_limit(-100,100);
  Motor.PID[1].out_limit(-100,100);

  //初始化Kinematic
  Motor.kinematic.set_motor_param(0,45,44,65);
  Motor.kinematic.set_motor_param(1,45,44,65);

  Motor.kinematic.set_kinematic_param(175);
 

  xTaskCreatePinnedToCore(micro_ros_task,"task",12000,NULL,1,NULL,0);
  Serial.print("set up finish");
}

void loop() {
  #if 1
  delay(10);
  
  float out_put_speed[2];
  uint64_t current_time=millis();
  
  
  Motor.kinematic.update_motor_ticks(micros(),Motor.Encoder[0].getTicks(),Motor.Encoder[1].getTicks());
  
  out_put_speed[0]= Motor.PID[0].update(Motor.kinematic.get_speed(0));
  out_put_speed[1]= Motor.PID[1].update(Motor.kinematic.get_speed(1));
  
  Motor.pwm.updateMotorSpeed(0,out_put_speed[0]);
  Motor.pwm.updateMotorSpeed(1,out_put_speed[1]);

  #endif


  #if usart_trans_odom_data
  static unsigned long interval=1000;  
  static unsigned long previousMillis=0;
    
    unsigned long currentMillis = millis(); // 获取当前时间
  if (currentMillis - previousMillis >= interval)
  {                                 // 判断是否到达间隔时间
    previousMillis = currentMillis; // 记录上一次打印的时间
    float linear_speed, angle_speed;
    Motor.kinematic.kinematic_forward(Motor.kinematic.get_speed(0), Motor.kinematic.get_speed(1), linear_speed, angle_speed);
    Serial.printf("[%ld] linear:%f angle:%f\n", currentMillis, linear_speed, angle_speed);                       // 打印当前时间
    Serial.printf("[%ld] x:%f y:%f yaml:%f\n", currentMillis,Motor.kinematic.odom().x, Motor.kinematic.odom().y, Motor.kinematic.odom().yaw); // 打印当前时间
  }
  
  #endif
  #if use_pub_odom
  int64_t stamp=rmw_uros_epoch_millis();
  odom_t odom=Motor.kinematic.odom();
  odom_msg.header.stamp.sec=stamp/1000;
  odom_msg.header.stamp.sec=stamp%1000*1000000;
  odom_msg.pose.pose.position.x = odom.x;
  odom_msg.pose.pose.position.y = odom.y;
  odom_msg.pose.pose.orientation.w = odom.quaternion.w;
  odom_msg.pose.pose.orientation.x = odom.quaternion.x;
  odom_msg.pose.pose.orientation.y = odom.quaternion.y;
  odom_msg.pose.pose.orientation.z = odom.quaternion.z; 
  odom_msg.twist.twist.angular.z = odom.angular_speed;
  odom_msg.twist.twist.linear.x = odom.linear_speed;
  
 // rcl_publish(&publish,&odom_msg,NULL);
  #endif
}
void msg_in_callback(const void * msg_in)
{
  geometry_msgs__msg__Twist*msg_p=(geometry_msgs__msg__Twist*)msg_in;
  float linear_x=msg_p->linear.x;
  float angular_z=msg_p->angular.z;
  float target_left_wheel_speed,target_right_wheel_speed;
  Motor.kinematic.kinematic_inverse(linear_x*1000,angular_z,target_left_wheel_speed,target_right_wheel_speed);
  Motor.PID[0].target_update(target_left_wheel_speed);
  Motor.PID[1].target_update(target_right_wheel_speed);

  Serial.printf("recv spped(%f,%f)\n", linear_x, angular_z);
}

void micro_ros_task(void* param)
{
  IPAddress IP;
  IP.fromString("192.168.1.29");
  set_microros_wifi_transports("ChinaNet-QM5D","apzdkfxw",IP,8888);
  delay(1000);
  allocator=rcl_get_default_allocator();
  rclc_support_init(&support,0,NULL,&allocator);
  rclc_node_init_default(&node,"elaina_car","",&support);
  rclc_subscription_init_default(&subscription,&node,ROSIDL_GET_MSG_TYPE_SUPPORT(geometry_msgs,msg,Twist),"cmd_vel");

#if use_pub_odom
  odom_msg.header.frame_id=micro_ros_string_utilities_set(odom_msg.header.frame_id,"odom");
  odom_msg.child_frame_id=micro_ros_string_utilities_set(odom_msg.child_frame_id,"base_link");
  rclc_publisher_init_best_effort(&publish,&node,ROSIDL_GET_MSG_TYPE_SUPPORT(nav_msgs,msg,Odometry),"odom");
#endif

  rclc_executor_init(&executor,&support.context,1,&allocator);
  rclc_executor_add_subscription(&executor,&subscription,&msg_in,msg_in_callback,ON_NEW_DATA);

  while(true)
  {
    #if use_pub_odom
    if (!rmw_uros_epoch_synchronized())
       {
         rmw_uros_sync_session(1000);
         // 如果时间同步成功，则将当前时间设置为MicroROS代理的时间，并输出调试信息。
         delay(10);
       }
    #endif
    delay(100);
    rclc_executor_spin_some(&executor,RCL_MS_TO_NS(100));
    rcl_publish(&publish,&odom_msg,NULL);
  }
}
