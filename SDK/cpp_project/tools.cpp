#include "head.h"
#include <cassert>
#include <stdexcept>
#include <iostream>
#include <cstring>
using namespace std;

int workshop_num;
struct Workshop workshops[55];
struct Robot robots[4];

// 由坐标计算出向量
Vector operator-(Point a, Point b)
{
    return Vector(a.x_ - b.x_, a.y_ - b.y_);
}

double IsZero(double a) { return fabs(a) < EPS; }

// 点积
double Dot(Vector a, Vector b)
{
    return a.x_ * b.x_ + a.y_ * b.y_;
}

// 叉积
double Cross(Vector a, Vector b)
{
    return a.x_ * b.y_ - a.y_ * b.x_;
}

// 向量长度
double Length(Vector a)
{
    return sqrt(Dot(a, a));
}

// 两向量之间夹角
double Angle(Vector a, Vector b)
{
    if (IsZero(Length(a)) || IsZero(Length(b)))
        return 0;

    double cross_ = Cross(a, b), dot_ = Dot(a, b);
    double res = asin(cross_ / Length(a) / Length(b));

    // 点积为负代表为钝角
    if (dot_ < 0)
    {
        if (cross_ > 0)
        {
            res = PI - res;
        }
        else
        {
            res = -PI - res;
        }
    }

    return res;
}

// 判断当前 第workshop_id个工作台是否可以接收product_id号产品
bool canRecv(int workshop_id, int product_id)
{
    Workshop &workshop = workshops[workshop_id];
    // 已经分配给其他机器人了
    if (workshop.assigned_[product_id])
        return false;
    
    switch (workshop.type_)
    {
    case 1:
    case 2:
    case 3:
        return false;

    case 4:
        if (product_id == 1 || product_id == 2)
        {
            if (!(workshop.stuff_status_ & (1 << product_id)))
                return true;
        }
        return false;

    case 5:
        if (product_id == 1 || product_id == 3)
        {
            if (!(workshop.stuff_status_ & (1 << product_id)))
                return true;
        }
        return false;

    case 6:
        if (product_id == 2 || product_id == 3)
        {
            if (!(workshop.stuff_status_ & (1 << product_id)))
                return true;
        }
        return false;

    case 7:
        if (product_id == 4 || product_id == 5 || product_id == 6)
        {
            if (!(workshop.stuff_status_ & (1 << product_id)))
                return true;
        }
        return false;

    case 8:
        if (product_id == 7)
            return true;
        return false;

    case 9:
        return true;
    }
    return false;
}

// 初始化
void init()
{
    for (int i = 0; i < 4; ++i)
    {
        robots[i].robot_id_ = i;
        robots[i].task_.task_type_ = EMPTY;
    }

    // 设置随机数种子
    srand(time(0));

    // 读入地图
    string s;
    for (int i = 0; i < 100; ++i)
    {
        cin >> s;
    }

    cin >> s;
    assert(strcmp(s.c_str(), "OK") == 0);
    cout << "OK" << endl;
}

// 读取当前状态
void readAndSetStatus()
{
    cin >> workshop_num;
    for (int i = 0; i < workshop_num; ++i)
    {
        Workshop &workshop = workshops[i];

        cin >> workshop.type_;
        cin >> workshop.position_.x_ >> workshop.position_.y_;
        cin >> workshop.remainder_produce_time_;
        cin >> workshop.stuff_status_;
        cin >> workshop.product_status_;
    }

    for (int i = 0; i < 4; ++i)
    {
        Robot &robot = robots[i];

        cin >> robot.workshop_id_;
        cin >> robot.product_id_;
        cin >> robot.time_rate_;
        cin >> robot.collide_rate_;
        cin >> robot.palstance_;
        cin >> robot.linear_velocity_.x_ >> robot.linear_velocity_.y_;
        cin >> robot.direction_;
        cin >> robot.position_.x_ >> robot.position_.y_;
    }

    string s;
    cin >> s;
}

// 按照当前状态为机器人安排任务
// 目前的策略为随机挑选
void getNextDes(int robot_id)
{
    struct Robot &robot = robots[robot_id];
    struct Task &task = robot.task_;

    if (robot.has_task_)
        return;

    for (int i = 0; i < 3000; ++i)
    {
        int t = rand() % workshop_num;

        // 机器人未持有物品
        if (robot.product_id_ == 0)
        {
            int type = workshops[t].type_;
            // 选择了只能卖东西的工作台
            if(type > 8)
                continue ;
            // 已经有机器申请了取得该东西
            if(workshops[t].assigned_[type])
                continue ;
            // 工作台有待取的产品
            if (workshops[t].product_status_ )
            {
                task.workshop_id_ = t;
                task.task_type_ = BUY;
                robot.has_task_ = true;
                workshops[t].assigned_[type] = true;
                break;
            }
        }
        // 机器人持有物品
        else
        {
            if (canRecv(t, robot.product_id_))
            {
                task.workshop_id_ = t;
                task.task_type_ = SELL;
                robot.has_task_ = true;
                workshops[t].assigned_[robot.product_id_] = true;
                break;
            }
        }
    }

    // 无法为持有物品的机器人找到出售点， 则销毁物品
    // 经测试，此行为会出现销毁价值较大的物品
    // 如果未出现死锁，尽量不要销毁物品
    if (robot.product_id_ && !robot.has_task_)
    {
        if (rand() % 100 == 0)
        {
            task.task_type_ = DESTROY;
            robot.has_task_ = true;
        }
    }
}

// 为机器人设置具体指令
vector<string> setInsToDes(int robot_id)
{
    struct Robot &robot = robots[robot_id];
    struct Task &task = robot.task_;
    int target_workshop_id = task.workshop_id_;
    struct Workshop &workshop = workshops[target_workshop_id];
    int product_id = robot.product_id_;
    int workshop_type = workshop.type_;
    vector<string> res;

    if (!robot.has_task_)
    {
        res.push_back("forward " + to_string(robot_id) + " 0");
        res.push_back("rotate " + to_string(robot_id) + " 0");
    }
    else if (task.task_type_ == DESTROY)
    {
        res.push_back("destroy " + to_string(robot_id));
        res.push_back("forward " + to_string(robot_id) + " 0");
        robot.has_task_ = false;
        workshop.assigned_[product_id] = false;
    }
    else if (robot.workshop_id_ == task.workshop_id_)
    {
        if (task.task_type_ == SELL)
        {
            res.push_back("sell " + to_string(robot_id));
            robot.has_task_ = false;
            workshop.assigned_[product_id] = false;
        }
        else if (task.task_type_ == BUY)
        {
            res.push_back("buy " + to_string(robot_id));
            robot.has_task_ = false;
            workshop.assigned_[workshop_type] = false;
        }

        res.push_back("forward " + to_string(robot_id) + " 0");
    }
    else
    {
        int workshop_id = task.workshop_id_;

        Vector direction = Vector(cos(robot.direction_), sin(robot.direction_));
        Vector robot_to_workshop = workshops[workshop_id].position_ - robot.position_;

        double angle = Angle(direction, robot_to_workshop);
        double palstance = 0;
        if (!IsZero(angle))
        {
            palstance = angle / TIME_FRAME;
        }

        if (!IsZero(palstance))
        {
            res.push_back("rotate " + to_string(robot_id) + " " + to_string(palstance));
        }
        else
        {
            res.push_back("rotate " + to_string(robot_id) + " 0");
        }

        if (Dot(robot.linear_velocity_, robot_to_workshop) < 0)
        {
            res.push_back("forward " + to_string(robot_id) + " 0");
        }
        else
        {
            res.push_back("forward " + to_string(robot_id) + " 6");
        }
    }
    return res;
}
