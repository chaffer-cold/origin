#include "example_action/robot.h"


float Robot::move_step()
{
    int direct=move_distance/fabs(move_distance);
    float step=direct *fabs(target_pose-current_pose)*0.1;
    current_pose=current_pose+step;
    std::cout<<"移动了:"<<step<<"当前位置:"<<current_pose<<std::endl;
    return current_pose;
}

bool Robot::set_goal(float distance)
{
    move_distance=distance;
    target_pose+=move_distance;
    if(close_goal())
    {
        status=MoveRobot::Feedback::STATUS_STOP;
        return false;
    }
    else
    {
        status=MoveRobot::Feedback::STATUS_MOVING;
        return true;
    }

}
float Robot::get_current_pose() { return current_pose; }
int Robot::get_status() { return status; }
/*是否接近目标*/
bool Robot::close_goal() { return fabs(target_pose - current_pose) < 0.01; }
void Robot::stop_move() 
{
    status = MoveRobot::Feedback::STATUS_STOP;
} /*停止移动*/

