#include "Kinematic.hpp"
void Kinematic::set_motor_param(uint8_t id,uint16_t reducation_ratio,uint16_t pulse_diameter,float wheel_diameter)
{
    /*确定参数*/
    motor_param[id].id=id;
    motor_param[id].reducation_ratio=reducation_ratio;
    motor_param[id].pulse_ration=pulse_diameter;
    motor_param[id].wheel_diameter=wheel_diameter;

    //motor_param[id].last_update_time=micros();


    motor_param[id].per_pulse_distance=(wheel_diameter*3.1415926535)/(pulse_diameter*reducation_ratio);
    motor_param[id].speed_factor=Kinematic::motor_param[id].per_pulse_distance*1000*1000;//将单位由s转化成us

     Serial.printf("init motor param %d: %f=%f*PI/(%d*%d) speed_factor=%d\n", id, motor_param[id].per_pulse_distance, wheel_diameter, reducation_ratio,motor_param[id].pulse_ration, motor_param[id].speed_factor);   // 打印调试信息
}
void Kinematic::set_kinematic_param(float wheel_distance)
{
    Kinematic::wheel_distance=wheel_distance;
}
double Kinematic::get_speed(uint8_t id)

//float Kinematic::get_speed(uint8_t id)
{
    return Kinematic::motor_param[id].motor_speed;
}
void Kinematic::update_motor_ticks(uint64_t current_time,int64_t left_wheel_ticks,int64_t right_wheel_ticks)
{
    //要保持单位一样
    int32_t delat_t=current_time-motor_param[0].last_update_time;//计算时间差
    
    int32_t left_wheel_ticks_delat=left_wheel_ticks-motor_param[0].last_encoder_tick;
    int32_t right_wheel_ticks_delat=right_wheel_ticks-motor_param[1].last_encoder_tick;
    
    motor_param[0].motor_speed=(float)(left_wheel_ticks_delat*motor_param[0].speed_factor)/(float)delat_t;
    motor_param[1].motor_speed=(float)(right_wheel_ticks_delat*motor_param[1].speed_factor)/(float)delat_t;

    motor_param[0].last_update_time=current_time;
    motor_param[0].last_encoder_tick=left_wheel_ticks;
    motor_param[1].last_update_time=current_time;
    motor_param[1].last_encoder_tick=right_wheel_ticks;
    
    //更新odom
    update_odom(delat_t);
}
void Kinematic::kinematic_forward(float left_wheel_speed,float right_wheel_speed,float& line_speed,float& angle_speed)
{
    line_speed=(left_wheel_speed+right_wheel_speed)/2.0;
    angle_speed=(right_wheel_speed-left_wheel_speed)/wheel_distance;
}

void Kinematic::kinematic_inverse(float line_speed,float  angle_speed,float& left_wheel_speed,float& right_wheel_speed)
{
    left_wheel_speed=(2.0*line_speed-wheel_distance*angle_speed)/2.0;
    right_wheel_speed=(2.0*line_speed+wheel_distance*angle_speed)/2.0;
}

/*里程计相关函数
*
*
*
*/
void Kinematic::TransAngleInPI(float angle, float &out_angle)
{
    if(angle>PI)
    {
        out_angle-=2*PI;
    }
    else if(angle< -PI)
    {
        out_angle+=2*PI;
    }

}
void Kinematic::update_odom(int32_t dt)
{
    static float linear_x,angular_z;
    float dt_s=(float)(dt/1000)/1000;
    
    kinematic_forward(get_speed(0),get_speed(1),linear_x,angular_z);
    odom_.linear_speed=linear_x/1000; //mm/s换单位为m/s
    odom_.angular_speed=angular_z;
    
    odom_.yaw+=odom_.angular_speed*dt_s;
    //Serial.printf("%f",odom_.yaw);
    TransAngleInPI(odom_.yaw,odom_.yaw);
    //一小段直线距离
    
    float ds=odom_.linear_speed*dt_s;
    odom_.x+=ds*std::cos(odom_.yaw);
    odom_.y+=ds*std::sin(odom_.yaw);
}

odom_t &Kinematic::odom(void)
{
    Euler2Quaternion(0.0,0.0,odom_.yaw,odom_.quaternion);
    return odom_;
}

void Kinematic::Euler2Quaternion(float roll, float pitch, float yaw, quaternion_t &q)
{
    double cr = cos(roll * 0.5);
    double sr = sin(roll * 0.5);
    double cy = cos(yaw * 0.5);
    double sy = sin(yaw * 0.5);
    double cp = cos(pitch * 0.5);
    double sp = sin(pitch * 0.5);
    // 计算出四元数的四个分量 q.w、q.x、q.y、q.z
    q.w = cy * cp * cr + sy * sp * sr;
    q.x = cy * cp * sr - sy * sp * cr;
    q.y = sy * cp * sr + cy * sp * cr;
    q.z = sy * cp * cr - cy * sp * sr;
}