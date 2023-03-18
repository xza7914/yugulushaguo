#include "head.h"
#include "macro.h"
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

int getProductId(int workshop_id)
{
    Workshop &workshop = workshops[workshop_id];

    if (workshop.type_ <= 7)
        return workshop.type_;

    return 0;
}

// 预定stuff
void reserveStuff(int workshop_id, int stuff_id)
{
    if (workshops[workshop_id].type_ >= 8)
        return;
    assert((workshops[workshop_id].reserve_stuff_status_ & (1 << stuff_id)) == 0);
    workshops[workshop_id].reserve_stuff_status_ |= (1 << stuff_id);
}

// 取消预订
void cancelReserveStuff(int workshop_id, int stuff_id)
{
    if (workshops[workshop_id].type_ >= 8)
        return;
    assert((workshops[workshop_id].reserve_stuff_status_ & (1 << stuff_id)) != 0);
    workshops[workshop_id].reserve_stuff_status_ &= ~(1 << stuff_id);
}

// 预定product
void reserveProduct(int workshop_id)
{
    assert(workshops[workshop_id].reserve_product_status_ == 0);
    workshops[workshop_id].reserve_product_status_ = 1;
}

// 取消预订
void cancelReserveProduct(int workshop_id)
{
    assert(workshops[workshop_id].reserve_product_status_ == 1);
    workshops[workshop_id].reserve_product_status_ = 0;
}

int getUrgency(int workshop_id)
{
    int res = 0;
    int t = workshops[workshop_id].stuff_status_;
    while (t)
    {
        if (t & 1)
            ++res;
        t >>= 1;
    }
    return res;
}

// 判断当前 第workshop_id个工作台是否可以接收product_id号产品
bool canRecvStuff(int workshop_id, int product_id)
{
    Workshop &workshop = workshops[workshop_id];
    switch (workshop.type_)
    {
    case 1:
    case 2:
    case 3:
        return false;

    case 4:
        if (product_id == 1 || product_id == 2)
        {
            if (!(workshop.stuff_status_ & (1 << product_id)) && !(workshop.reserve_stuff_status_ & (1 << product_id)))
                return true;
        }
        return false;

    case 5:
        if (product_id == 1 || product_id == 3)
        {
            if (!(workshop.stuff_status_ & (1 << product_id)) && !(workshop.reserve_stuff_status_ & (1 << product_id)))
                return true;
        }
        return false;

    case 6:
        if (product_id == 2 || product_id == 3)
        {
            if (!(workshop.stuff_status_ & (1 << product_id)) && !(workshop.reserve_stuff_status_ & (1 << product_id)))
                return true;
        }
        return false;

    case 7:
        if (product_id == 4 || product_id == 5 || product_id == 6)
        {
            if (!(workshop.stuff_status_ & (1 << product_id)) && !(workshop.reserve_stuff_status_ & (1 << product_id)))
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
void getNextDes(int robot_id, int framd_id)
{
    struct Robot &robot = robots[robot_id];
    struct Task &task = robot.task_;

    if (robot.has_task_)
        return;

    // 下列算法寻找产品号最大的，且总距离最短的产品

    double max_priority = -1e100;
    int tar_product_id = 0;
    int buy_from = 0;
    int sell_to = 0;

    for (int i = 0; i < workshop_num; ++i)
    {
        if (!workshops[i].product_status_ || workshops[i].reserve_product_status_)
            continue;

        int product_id = getProductId(i);

        for (int j = 0; j < workshop_num; ++j)
        {
            if (canRecvStuff(j, product_id))
            {
                double distance = Length(workshops[i].position_ - workshops[j].position_) +
                                  Length(workshops[i].position_ - robot.position_);

                // 计算剩余时间内机器人可行驶的最大距离，并排除掉不可能的任务。
                double max_length = 5.0 * (MAX_FRAME_ID - framd_id) * TIME_FRAME;
                if (max_length < distance)
                    continue;

                // 优先级
                double priority = INIT_PRIORITY;
                priority += product_id * PRODUCT_ID;
                if (product_id > 3)
                    priority += LEVEL_1;
                if (product_id == 7)
                    priority += LEVEL_2;
                priority += getUrgency(j) * URGENCY;
                if (workshops[j].type_ == 9)
                    priority -= NINE_WORKSHOP;

                priority = priority * 200 / distance;

                if (priority > max_priority)
                {
                    max_priority = priority;
                    tar_product_id = product_id;
                    buy_from = i;
                    sell_to = j;
                }
            }
        }
    }

    if (tar_product_id != 0)
    {
        task.buy_workshop_id_ = buy_from;
        task.sell_workshop_id_ = sell_to;
        task.product_id_ = tar_product_id;
        robot.has_task_ = true;

        reserveProduct(buy_from);
        reserveStuff(sell_to, tar_product_id);
    }
}

vector<string> reduceCollide()
{
    vector<string> res;
    for (int i = 0; i < 4; ++i)
    {
        for (int j = i + 1; j < 4; ++j)
        {
            double distance = Length(robots[i].position_ - robots[j].position_);
            double angle = Angle(robots[i].linear_velocity_, robots[j].linear_velocity_);
            if (distance < 2 && (angle > PI - 0.3 || angle < -PI + 0.3))
            {
                res.push_back("rotate " + to_string(i) + " 1");
                res.push_back("rotate " + to_string(j) + " 1");
            }
        }
    }
    return res;
}

// 为机器人设置具体指令
vector<string> setInsToDes(int robot_id)
{
    struct Robot &robot = robots[robot_id];
    struct Task &task = robot.task_;
    vector<string> res;

    if (!robot.has_task_)
    {
        res.push_back("forward " + to_string(robot_id) + " 0");
        res.push_back("rotate " + to_string(robot_id) + " 0");
    }
    else if (task.task_type_ == DESTROY)
    {
        // 目前的策略不会销毁产品
        throw runtime_error("unexpected condition");
    }
    else
    {
        int target_workshop_id;

        // 根据机器人此时是否携带产品，来判定任务的阶段。
        if (robot.product_id_)
        {
            target_workshop_id = task.sell_workshop_id_;
        }
        else
        {
            target_workshop_id = task.buy_workshop_id_;
        }

        if (target_workshop_id == robot.workshop_id_)
        {
            if (robot.product_id_)
            {
                res.push_back("sell " + to_string(robot_id));
                cancelReserveStuff(target_workshop_id, robot.product_id_);
                robot.has_task_ = false;
            }
            else
            {
                res.push_back("buy " + to_string(robot_id));
                cancelReserveProduct(target_workshop_id);
            }
        }
        else
        {
            int workshop_id = target_workshop_id;

            Vector direction = Vector(cos(robot.direction_), sin(robot.direction_));
            Vector robot_to_workshop = workshops[workshop_id].position_ - robot.position_;

            double angle = Angle(direction, robot_to_workshop);
            double distance = Length(robot_to_workshop);
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
    }

    return res;
}
