#ifndef __PID_HPP
#define __PID_HPP
class pid_contrl
{
    public:
        pid_contrl();
        pid_contrl(float kp, float ki, float kd);
        float target_;
        float kp_;          // 比例系数
        float ki_;          // 积分系数
        float kd_;          // 微分系数
        float last_output_; // 上一次输出值
    
        float update(float contrl);  //更新输出
        void reset(void);              //重置pid控制器
        void target_update(float target);
        void pid_update(float kp,float ki,float kd); //更新pid
        void out_limit(float out_min,float out_max); //限制
    private:
        
        float error;        //误差
        float error_sum;    //累计误差
        float error_sum_max=2500; //积分上限;

        float error_delta;  //误差微分
        float error_last;   //上一次的误差
        float error_pre;    //前次的误差
        
        
        float out_min_;     // 输出下限
        float out_max_;     // 输出上限

};




//输入的contrl是当前的速度
//error_pre与last_output没有用到



#endif
