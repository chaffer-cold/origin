#include <Arduino.h>
#include "micro_ros_platformio.h"
#include <WiFi.h>
#include "Esp32McpwmMotor.h"
#include "Esp32PcntEncoder.h"
#include "rcl/rcl.h"
#include "rclc/rclc.h"
#include "rclc/executor.h"

#include "micro_ros_utilities/type_utilities.h"
#include "geometry_msgs/msg/twist.h"
#include "std_msgs/msg/float32_multi_array.h"
#include "std_msgs/msg/float64.h"
#include "pid.hpp"

#define pid_ts 1
#if pid_ts
struct Mymotor
{
  //发布速度
  double speed[2];
  float out_speed[2];
  //float last_speed[2];
  int64_t last_tick[2];
  int64_t last_updatetime;
  int32_t pt[2];
  //控制速度
  Esp32McpwmMotor pwm;
  Esp32PcntEncoder Encoder[2];
  pid_contrl PID[2];
};
Mymotor* motor_p;
Mymotor motor;
//初始化
rcl_allocator_t allocator;
rclc_support_t support;
rcl_node_t node;
rclc_executor_t executor;

//消息相关
rcl_subscription_t subscription;
rcl_subscription_t subscription_pid;
geometry_msgs__msg__Twist msgs_data;
std_msgs__msg__Float64 msg_pub[2];
std_msgs__msg__Float32MultiArray kpid_in;
//定时器发布速度
rcl_timer_t timer;
rcl_publisher_t pub;
rcl_publisher_t pub2;

void timer_callback(rcl_timer_t *timer, int64_t last_call_time);
void msg_callback(const void *msg_in);
void pid_topic_callback(const void *msg_in);
void cal_speed(void);
void micro_ros_task(void* param)
{
  
  IPAddress IP;
  //IP.fromString("192.168.57.114");
  //set_microros_wifi_transports("elaina","cnmnjust",IP,8888);
  IP.fromString("192.168.1.29");
  set_microros_wifi_transports("ChinaNet-QM5D","apzdkfxw",IP,8888);
  
  delay(2000);
  //初始化ros2
  allocator=rcl_get_default_allocator();
  rclc_support_init(&support,0,NULL,&allocator);
  rclc_node_init_default(&node,"elaina_twist","",&support);
  rclc_subscription_init_default(&subscription,&node,ROSIDL_GET_MSG_TYPE_SUPPORT(geometry_msgs,msg,Twist),"cmd_vel");
  rclc_subscription_init_default(&subscription_pid,&node,ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs,msg,Float32MultiArray),"pid_param");
  rclc_timer_init_default(&timer,&support,10,timer_callback);
  rclc_publisher_init_default(&pub,&node,ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs,msg,Float64),"speed");
  rclc_publisher_init_default(&pub2,&node,ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs,msg,Float32MultiArray),"topic_ts");
  //添加到executor
  micro_ros_utilities_create_message_memory(ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs,msg,Float32MultiArray),&kpid_in,{0});
  rclc_executor_init(&executor,&support.context,3,&allocator);
  rclc_executor_add_subscription(&executor,&subscription,&msgs_data,msg_callback,ON_NEW_DATA);
  rclc_executor_add_subscription(&executor,&subscription_pid,&kpid_in,pid_topic_callback,ON_NEW_DATA);
  rclc_executor_add_timer(&executor,&timer);
  while(true)
  {
    rcl_publish(&pub2,&kpid_in,NULL);
    rclc_executor_spin_some(&executor,RCL_MS_TO_NS(100));
  }

}
void setup() {
  Serial.begin(115200);
  motor.last_tick[0]=0;motor.last_tick[1]=0;
  motor_p=&motor;
  motor_p->pwm.attachMotor(0,22,23);
  motor_p->pwm.attachMotor(1,12,13);
  motor_p->Encoder[0].init(0, 32, 33); // 初始化第一个编码器，使用GPIO 32和33连接
  motor_p->Encoder[1].init(1, 26, 25); // 初始化第二个编码器，使用GPIO 26和25连接
  // put your setup code here, to run once:
  //设置pid
  motor_p->PID[0].pid_update(0.625,0.105,0.0);
  motor_p->PID[1].pid_update(0.625,0.105,0.0);

  motor_p->PID[0].out_limit(-100,100);
  motor_p->PID[1].out_limit(-100,100);
  
  //限幅
  xTaskCreatePinnedToCore(micro_ros_task,"micro_ros",10240,NULL,1,NULL,0);

  
}

void loop() {
    delay(10);
    //rcl_publish(&pub2,&kpid_in,NULL);
    cal_speed();
    motor_p->out_speed[0]=motor_p->PID->update(motor_p->speed[0]);
    motor_p->out_speed[1]=motor_p->PID->update(motor_p->speed[1]);
    motor_p->pwm.updateMotorSpeed(0,motor_p->out_speed[0]);
    motor_p->pwm.updateMotorSpeed(1,motor_p->out_speed[1]);
    
}

// put function definitions here:
void msg_callback(const void *msg_in)
{
  geometry_msgs__msg__Twist* msg_p=(geometry_msgs__msg__Twist*)msg_in;
  float linear_x=msg_p->linear.x;
  float angular_z=msg_p->angular.z;
  if(linear_x==0&&angular_z==0)
  {
    motor_p->PID[0].target_=0;
    motor_p->PID[1].target_=0;
    motor_p->pwm.updateMotorSpeed(0,0);
    motor_p->pwm.updateMotorSpeed(1,0);
    
  }
  #if 0
  else if(angular_z>0)
  {
    motor_p->PID[0].target_=0;
    motor_p->PID[1].target_=0;
    motor_p->pwm.updateMotorSpeed(0, -70);
    motor_p->pwm.updateMotorSpeed(1, 70);
  }
  else if (angular_z < 0)
  {
    motor_p->PID[0].target_=0;
    motor_p->PID[1].target_=0;
    motor_p->pwm.updateMotorSpeed(0, 70);
    motor_p->pwm.updateMotorSpeed(1, -70);
  }
  #endif
  if(linear_x!=0)
  {
    //mm/s作为控制单位
    motor_p->PID[0].target_update(linear_x*1000);
    motor_p->PID[1].target_update(linear_x*1000);
  }
  #if false
  if(linear_x==0&&angular_z==0)
  {
    motor_p->pwm.updateMotorSpeed(0,0);
    motor_p->pwm.updateMotorSpeed(1,0);
  }
  else if(linear_x>0)
  {
    motor_p->pwm.updateMotorSpeed(0,70);
    motor_p->pwm.updateMotorSpeed(1,70);
  }
  else if(linear_x<0)
  {
    motor_p->pwm.updateMotorSpeed(0,-70);
    motor_p->pwm.updateMotorSpeed(1,-70);
  }
  else if(angular_z>0)
  {
    motor_p->pwm.updateMotorSpeed(0, -70);
    motor_p->pwm.updateMotorSpeed(1, 70);
  }
  else if (angular_z < 0)
  {
    motor_p->pwm.updateMotorSpeed(0, 70);
    motor_p->pwm.updateMotorSpeed(1, -70);
  }
  #endif
   Serial.printf("recv spped(%f,%f)\n", linear_x, angular_z);
}
void pid_topic_callback(const void* msg_in)
{
   std_msgs__msg__Float32MultiArray* msg_p=(std_msgs__msg__Float32MultiArray*)msg_in;
   float data[3];
   Serial.print("receive pid");
   for (int i=0;i<3;i++)
   {
      data[i]=msg_p->data.data[i];
      Serial.print(data[i]);
   }
   motor_p->PID[0].pid_update(data[0],data[1],data[2]);
   motor_p->PID[1].pid_update(data[0],data[1],data[2]);
   motor_p->PID[0].out_limit(-100,100);
  motor_p->PID[1].out_limit(-100,100);


}

void timer_callback(rcl_timer_t *timer, int64_t last_call_time)
{
  RCLC_UNUSED(last_call_time);
  //cal_speed();
  msg_pub[0].data=motor_p->speed[0];
  msg_pub[1].data=motor_p->speed[1];
  rcl_publish(&pub,&msg_pub[0],NULL);
  //rcl_publish(&pub,&msg_pub[1],NULL);
  //rcl_publish(&pub,msg_pub,NULL);
  //Serial.print(motor_p->speed[0],motor_p->speed[1]);
  //Serial.print("真的3");
}
void cal_speed(void)
{
  //计算速度
  int64_t dt=millis()-motor_p->last_updatetime;
  motor_p->pt[0]=motor_p->Encoder[0].getTicks()-motor_p->last_tick[0];
  motor_p->pt[1]=motor_p->Encoder[1].getTicks()-motor_p->last_tick[1];
  
  motor_p->speed[0]=(motor_p->pt[0]*0.1051566)/dt*1000;
  motor_p->speed[1]=(motor_p->pt[1]*0.1051566)/dt*1000;
  //更新速度
  motor_p->last_updatetime=millis();
  motor_p->last_tick[0]=motor_p->Encoder[0].getTicks();
  motor_p->last_tick[1]=motor_p->Encoder[1].getTicks();
}
#endif


#if !pid_ts
struct Mymotor
{
  //发布速度
  double speed[2];
  float out_speed[2];
  //float last_speed[2];
  int64_t last_tick[2];
  int64_t last_updatetime;
  int32_t pt[2];
  //控制速度
  Esp32McpwmMotor pwm;
  Esp32PcntEncoder Encoder[2];
  pid_contrl PID[2];
};
Mymotor* motor_p;
Mymotor motor;
//初始化
rcl_allocator_t allocator;
rclc_support_t support;
rcl_node_t node;
rclc_executor_t executor;

//消息相关
rcl_subscription_t subscription;
rcl_subscription_t subscription_pid;
geometry_msgs__msg__Twist msgs_data;
std_msgs__msg__Float64 msg_pub[2];
std_msgs__msg__Float32MultiArray kpid_in;
//定时器发布速度
rcl_timer_t timer;
rcl_publisher_t pub;


//电机
//Esp32McpwmMotor motor_pwm;
//Esp32PcntEncoder motor_Encoder[2];

// put function declarations here

void timer_callback(rcl_timer_t *timer, int64_t last_call_time);
void msg_callback(const void *msg_in);
void pid_topic_callback(const void *msg_in);
void cal_speed(void);
void micro_ros_task(void* param)
{
  
  IPAddress IP;
  IP.fromString("192.168.57.114");
  set_microros_wifi_transports("elaina","cnmnjust",IP,8888);
  delay(2000);
  //初始化ros2
  allocator=rcl_get_default_allocator();
  rclc_support_init(&support,0,NULL,&allocator);
  rclc_node_init_default(&node,"elaina_twist","",&support);
  rclc_subscription_init_default(&subscription,&node,ROSIDL_GET_MSG_TYPE_SUPPORT(geometry_msgs,msg,Twist),"cmd_vel");
  rclc_subscription_init_default(&subscription_pid,&node,ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs,msg,Float32MultiArray),"pid_param");
  rclc_timer_init_default(&timer,&support,RCL_MS_TO_NS(5),timer_callback);
  rclc_publisher_init_default(&pub,&node,ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs,msg,Float64),"speed");
  //添加到executor
  rclc_executor_init(&executor,&support.context,3,&allocator);
  rclc_executor_add_subscription(&executor,&subscription,&msgs_data,msg_callback,ON_NEW_DATA);
  rclc_executor_add_subscription(&executor,&subscription_pid,&kpid_in,pid_topic_callback,ON_NEW_DATA);
  rclc_executor_add_timer(&executor,&timer);
  while(true)
  {
    
    rclc_executor_spin_some(&executor,RCL_MS_TO_NS(100));
  }

}
void setup() {
  Serial.begin(115200);
  motor.last_tick[0]=0;motor.last_tick[1]=0;
  motor_p=&motor;
  motor_p->pwm.attachMotor(0,22,23);
  motor_p->pwm.attachMotor(1,12,13);
  motor_p->Encoder[0].init(0, 32, 33); // 初始化第一个编码器，使用GPIO 32和33连接
  motor_p->Encoder[1].init(1, 26, 25); // 初始化第二个编码器，使用GPIO 26和25连接
  // put your setup code here, to run once:
  //设置pid
  motor_p->PID[0].pid_update(0.625,0.105,0.0);
  motor_p->PID[1].pid_update(0.625,0.105,0.0);

  motor_p->PID[0].out_limit(-100,100);
  motor_p->PID[1].out_limit(-100,100);
  
  //限幅
  xTaskCreatePinnedToCore(micro_ros_task,"micro_ros",10240,NULL,1,NULL,0);

  
}

void loop() {
    delay(10);
    cal_speed();
    motor_p->out_speed[0]=motor_p->PID->update(motor_p->speed[0]);
    motor_p->out_speed[1]=motor_p->PID->update(motor_p->speed[1]);
    motor_p->pwm.updateMotorSpeed(0,motor_p->out_speed[0]);
    motor_p->pwm.updateMotorSpeed(1,motor_p->out_speed[1]);
    
}

// put function definitions here:
void msg_callback(const void *msg_in)
{
  geometry_msgs__msg__Twist* msg_p=(geometry_msgs__msg__Twist*)msg_in;
  float linear_x=msg_p->linear.x;
  float angular_z=msg_p->angular.z;
  if(linear_x==0&&angular_z==0)
  {
    motor_p->PID[0].target_=0;
    motor_p->PID[1].target_=0;
    motor_p->pwm.updateMotorSpeed(0,0);
    motor_p->pwm.updateMotorSpeed(1,0);
  }
  #if 0
  else if(angular_z>0)
  {
    motor_p->PID[0].target_=0;
    motor_p->PID[1].target_=0;
    motor_p->pwm.updateMotorSpeed(0, -70);
    motor_p->pwm.updateMotorSpeed(1, 70);
  }
  else if (angular_z < 0)
  {
    motor_p->PID[0].target_=0;
    motor_p->PID[1].target_=0;
    motor_p->pwm.updateMotorSpeed(0, 70);
    motor_p->pwm.updateMotorSpeed(1, -70);
  }
  #endif
  if(linear_x!=0)
  {
    //mm/s作为控制单位
    motor_p->PID[0].target_update(linear_x*1000);
    motor_p->PID[1].target_update(linear_x*1000);
  }
  #if false
  if(linear_x==0&&angular_z==0)
  {
    motor_p->pwm.updateMotorSpeed(0,0);
    motor_p->pwm.updateMotorSpeed(1,0);
  }
  else if(linear_x>0)
  {
    motor_p->pwm.updateMotorSpeed(0,70);
    motor_p->pwm.updateMotorSpeed(1,70);
  }
  else if(linear_x<0)
  {
    motor_p->pwm.updateMotorSpeed(0,-70);
    motor_p->pwm.updateMotorSpeed(1,-70);
  }
  else if(angular_z>0)
  {
    motor_p->pwm.updateMotorSpeed(0, -70);
    motor_p->pwm.updateMotorSpeed(1, 70);
  }
  else if (angular_z < 0)
  {
    motor_p->pwm.updateMotorSpeed(0, 70);
    motor_p->pwm.updateMotorSpeed(1, -70);
  }
  #endif
   Serial.printf("recv spped(%f,%f)\n", linear_x, angular_z);
}
void pid_topic_callback(const void* msg_in)
{
   std_msgs__msg__Float32MultiArray* msg_p=(std_msgs__msg__Float32MultiArray*)msg_in;
   float data[3];
   Serial.print("receive pid");
   for (int i=0;i<3;i++)
   {
      data[i]=msg_p->data.data[i];
      Serial.print(data[i]);
   }
   motor_p->PID[0].pid_update(data[0],data[1],data[2]);
   motor_p->PID[1].pid_update(data[0],data[1],data[2]);


}

void timer_callback(rcl_timer_t *timer, int64_t last_call_time)
{
  RCLC_UNUSED(last_call_time);
  //cal_speed();
  msg_pub[0].data=motor_p->speed[0];
  msg_pub[1].data=motor_p->speed[1];
  rcl_publish(&pub,&msg_pub[0],NULL);
//  rcl_publish(&pub,&msg_pub[1],NULL);
  //rcl_publish(&pub,msg_pub,NULL);
  //Serial.print(motor_p->speed[0],motor_p->speed[1]);
  //Serial.print("真的3");
}
void cal_speed(void)
{
  //计算速度
  int64_t dt=millis()-motor_p->last_updatetime;
  motor_p->pt[0]=motor_p->Encoder[0].getTicks()-motor_p->last_tick[0];
  motor_p->pt[1]=motor_p->Encoder[1].getTicks()-motor_p->last_tick[1];
  
  motor_p->speed[0]=(motor_p->pt[0]*0.1051566)/dt*1000;
  motor_p->speed[1]=(motor_p->pt[1]*0.1051566)/dt*1000;
  //更新速度
  motor_p->last_updatetime=millis();
  motor_p->last_tick[0]=motor_p->Encoder[0].getTicks();
  motor_p->last_tick[1]=motor_p->Encoder[1].getTicks();
}
#endif