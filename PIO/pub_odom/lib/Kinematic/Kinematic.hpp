#ifndef __KINEMATIC_HPP
#define __KINEMATIC_HPP
#include "Arduino.h"
struct Mymotor_param_t
{
    uint8_t id;                // 电机编号
    uint16_t reducation_ratio; // 减速器减速比，轮子转一圈，电机需要转的圈数
    uint16_t pulse_ration;     // 脉冲比，电机转一圈所产生的脉冲数
    float wheel_diameter;      // 轮子的外直径，单位mm

    double per_pulse_distance;  // 无需配置，单个脉冲轮子前进的距离，单位mm，设置时自动计算
                               // 单个脉冲距离=轮子转一圈所行进的距离/轮子转一圈所产生的脉冲数
                               // per_pulse_distance= (wheel_diameter*3.1415926)/(pulse_ration*reducation_ratio)
    
    int32_t speed_factor;     // 无需配置，计算速度时使用的速度因子，设置时自动计算，speed_factor计算方式如下
                               // 设 dt（单位us,1s=1000ms=10^6us）时间内的脉冲数为dtick
                               // 速度speed = per_pulse_distance*dtick/(dt/1000/1000)=(per_pulse_distance*1000*1000)*dtic/dt
                               // 记 speed_factor = (per_pulse_distance*1000*1000)
    
    //float motor_speed;       // 无需配置，当前电机速度mm/s，计算时使用
    double motor_speed;
    int64_t last_encoder_tick; // 无需配置，上次电机的编码器读数
    uint64_t last_update_time; // 无需配置，上次更新数据的时间，单位us

};
/*四元数*/
struct quaternion_t
{
    float w;
    float x;
    float y;
    float z;

};
struct odom_t
{
    float x;
    float y;
    float yaw;
    float linear_speed;
    float angular_speed;
    quaternion_t quaternion;
};
class Kinematic
{
    public:
        /*设定电机基本参数*/
        void set_motor_param(uint8_t id,uint16_t reducation_ratio,uint16_t pulse_diameter,float wheel_diameter);
        /*设定运动学相关参数*/
        void set_kinematic_param(float wheel_distance);
        /*运动学解逆,输入线速度角速度得到当前左右轮速度*/
        void kinematic_inverse(float line_speed,float angle_speed,float& left_wheel_speed,float& right_wheel_speed);
        /*运动学正解,输入左右轮速度得到当前的线速度角速度*/
        void kinematic_forward(float left_wheel_speed,float right_wheel_speed,float& line_speed,float& angle_speed);
        /*更新轮子的速度与tick(编码器计数)*/
        void update_motor_ticks(uint64_t current_time,int64_t left_wheel_ticks,int64_t right_wheel_ticks);
        /*获得轮子的速度*/
        //float get_speed(uint8_t id);
        double get_speed(uint8_t id);

        /*转换欧拉变成四元角*/
         void Euler2Quaternion(float roll, float pitch, float yaw, quaternion_t &q);

        
        //里程计相关
        odom_t & odom(void);
        void update_odom(int32_t dt);
        //将角度限制在-PI~PI
        void TransAngleInPI(float angle, float &out_angle);

        //姿态相关

    private:
        Mymotor_param_t motor_param[2];
        float wheel_distance;    //轮子间距
        odom_t odom_;
        
};









#endif
