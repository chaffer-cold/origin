#include "pid.hpp"
pid_contrl::pid_contrl()
{

    kp_=0.6;
    kd_=0.2;
    target_=0;
    last_output_=0;
}
pid_contrl::pid_contrl(float kp, float ki, float kd)
{
    reset(); // 初始化控制器
    pid_update(kp, ki, kd); // 更新PID参数
}
void pid_contrl::pid_update(float kp,float ki,float kd)
{
    reset();
    kp_=kp;
    ki_=ki;
    kd_=kd;
}
void pid_contrl::out_limit(float out_min,float out_max )
{
    out_min_=out_min;
    out_max_=out_max;
}
void pid_contrl::reset(void)
{
    last_output_ = 0.0f; // 上一次的控制输出值
    target_ = 0.0f; // 控制目标值
    out_min_ = 0.0f; // 控制输出最小值
    out_max_ = 0.0f; // 控制输出最大值
    
    
    kp_=0.0;
    ki_=0.0;
    kd_=0.0;

    error=0.0;
    error_delta=0.0;
    error_last=0.0;
    error_sum=0.0;
    error_pre=0.0;
}
void pid_contrl::target_update(float target)
{
    target_=target;
}
float pid_contrl::update(float contrl)
{
    //计算误差
    error=target_-contrl;
    //积分微分
    error_sum+=error;
    error_delta=error_last-error;
    
    //更新误差
    error_last=error;
    
    //积分限幅
    if(error_sum>error_sum_max)
    {
        error_sum=error_sum_max;
    }
    //输出计算
    float output=kp_*error+ki_*error_sum+kd_*error_delta;
    //输出限幅
    if(output>out_max_)
    {
        output=out_max_;
    }
    else if(output<out_min_)
    {
        output=out_min_;
    }

    last_output_=output;
    return output;
}

