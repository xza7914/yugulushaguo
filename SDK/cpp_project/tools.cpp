#include "head.h"
#include "macro.h"
#include <cassert>
#include <stdexcept>
#include <iostream>
#include <cstring>
using namespace std;

int workshop_num;
int now_frame_id;
struct Workshop workshops[55];
struct Robot robots[4];

// 由坐标计算出向量
Vector operator-(const Point &a, const Point &b)
{
    return Vector(a.x_ - b.x_, a.y_ - b.y_);
}

// 向量放缩
Vector operator*(const Vector &a, const double &b)
{
    return Vector(a.x_ * b, a.y_ * b);
}

// 点加向量
Position operator+(const Point &a, const Vector &b)
{
    return Position(a.x_ + b.x_, a.y_ + b.y_);
}

// 判断浮点数是否近似为0
double IsZero(double a) { return fabs(a) < EPS; }

// 点积
double Dot(const Vector &a, const Vector &b)
{
    return a.x_ * b.x_ + a.y_ * b.y_;
}

// 叉积
double Cross(const Vector &a, const Vector &b)
{
    return a.x_ * b.y_ - a.y_ * b.x_;
}

// 向量长度
double Length(const Vector &a)
{
    return sqrt(Dot(a, a));
}

// 获取单位向量
Vector getUnitVector(const Vector &a) {
    double t = 1 / Length(a);
    return a * t;
}

// 两向量之间夹角
double Angle(const Vector &a, const Vector &b)
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

// 获取某工作台的产品类型
int getProductId(int workshop_id)
{
    Workshop &workshop = workshops[workshop_id];

    if (workshop.type_ <= 7)
        return workshop.type_;

    return 0;
}

// 预定原料
void reserveStuff(int workshop_id, int stuff_id)
{
    if (workshops[workshop_id].type_ >= 8)
        return;
    assert((workshops[workshop_id].reserve_stuff_status_ & (1 << stuff_id)) == 0);
    workshops[workshop_id].reserve_stuff_status_ |= (1 << stuff_id);
}

// 取消预订原料
void cancelReserveStuff(int workshop_id, int stuff_id)
{
    if (workshops[workshop_id].type_ >= 8)
        return;
    assert((workshops[workshop_id].reserve_stuff_status_ & (1 << stuff_id)) != 0);
    workshops[workshop_id].reserve_stuff_status_ &= ~(1 << stuff_id);
}

// 预定产品
void reserveProduct(int workshop_id)
{
    assert(workshops[workshop_id].reserve_product_status_ == 0);
    workshops[workshop_id].reserve_product_status_ = 1;
}

// 取消预订产品
void cancelReserveProduct(int workshop_id)
{
    assert(workshops[workshop_id].reserve_product_status_ == 1);
    workshops[workshop_id].reserve_product_status_ = 0;
}

// 获取某项工作的紧急程度
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

// 预测某一帧时某机器人的位置与目标
int predictPosAndDes(int robot_id, int frame_id, Position &pos, Position &des)
{
    if (frame_id == now_frame_id)
    {
        int des_id = getDestination(robot_id);
        if (des_id == -1)
            return -1;

        pos = robots[robot_id].position_;
        des = workshops[des_id].position_;
        return 0;
    }
    else
    {
        int des_id = getDestination(robot_id);
        if (des_id == -1)
            return -1;

        Position pos_1 = robots[robot_id].position_;
        Position des_1 = workshops[des_id].position_;
        Vector direction = des_1 - pos_1;
        int frames_to_des = int(Length(direction) / 6 * 50 + 10);
        int remain_frames = frame_id - now_frame_id;

        if (remain_frames <= frames_to_des)
        {
            pos = pos_1 + getUnitVector(direction) * 6 * (remain_frames) * TIME_FRAME;
            des = des_1;
            return 0;

        } else {
            Task &task = robots[robot_id].task_;
            if (task.stage_ == 2) return -1;

            Position des_2 = workshops[task.sell_workshop_id_].position_;
            remain_frames -= frames_to_des;
            
            if (remain_frames > int(Length(des_2 - des_1) / 6 * 50)) return -1;

            pos = des_1 + getUnitVector(des_2 - des_1) * 6 * remain_frames * TIME_FRAME;
            des = des_2;
            return 0;
        }
    }
}

// 判断机器人是否会与其他机器人迎面相撞
bool willCollideOther(int robot_id, struct Task &task)
{

    for (int j = 0; j < 4; ++j)
    {
        if (j == robot_id)
            continue;

        Position position_1, destination_1, position_2, destination_2;

        int buy_from_id = task.buy_workshop_id_;
        int sell_to_id = task.sell_workshop_id_;
        int frame_id = now_frame_id;

        position_1 = robots[robot_id].position_;
        destination_1 = workshops[buy_from_id].position_;

        auto willCollide = [&]() -> bool
        {
            if (predictPosAndDes(j, frame_id, position_2, destination_2) == 0)
            {
                double angle1 = Angle(destination_1 - position_1, destination_2 - position_2);
                double angle2 = Angle(destination_1 - position_1, position_2 - position_1);
                double angle3 = Angle(position_2 - position_1, destination_2 - destination_1);
                if ((angle1 > PI - 0.2 || angle1 < -PI + 0.2) && fabs(angle2) < 0.2 && (angle3 > PI - 0.2 || angle3 < -PI + 0.2))
                {
                    return true;
                }
            }
            return false;
        };

        if (willCollide())
            return true;

        frame_id = now_frame_id + int(Length(destination_1 - position_1) / 6 * 50 + 10);
        position_1 = destination_1;
        destination_1 = workshops[sell_to_id].position_;

        if (willCollide())
            return true;
    }
    return false;
}

// 判断某工作台是否能在到达之前准备好产品
bool canProductReady(int workshop_id, double distance)
{
    struct Workshop &workshop = workshops[workshop_id];
    if (workshop.reserve_product_status_)
    {
        return false;
    }

    if (workshop.product_status_)
        return true;

    if (workshop.remainder_produce_time_ > 0)
    {
        if (workshop.remainder_produce_time_ * TIME_FRAME * 6 < distance - 0.5)
        {
            return true;
        }
    }

    return false;
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

// 获取某个机器人此时的目的地, 若为空闲，则返回-1
int getDestination(int robot_id)
{
    if (!robots[robot_id].has_task_)
        return -1;
    Task &task = robots[robot_id].task_;

    if (task.stage_ == 1)
        return task.buy_workshop_id_;
    else
        return task.sell_workshop_id_;
}

// 初始化并读入地图
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

// 设置当前frame_id
void setNowFrameId(int frame_id)
{
    now_frame_id = frame_id;
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
void getNextDes(int robot_id)
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
        double distance = Length(workshops[i].position_ - robot.position_);

        if (!canProductReady(i, distance))
            continue;

        int product_id = getProductId(i);

        for (int j = 0; j < workshop_num; ++j)
        {
            if (canRecvStuff(j, product_id))
            {
                double distance = Length(workshops[i].position_ - workshops[j].position_) +
                                  Length(workshops[i].position_ - robot.position_);

                // 计算剩余时间内机器人可行驶的最大距离，并排除掉不可能的任务。
                double max_length = 5.0 * (MAX_FRAME_ID - now_frame_id) * TIME_FRAME;
                if (max_length < distance)
                    continue;

                // 优先级
                double priority = INIT_PRIORITY;
                priority += product_id * PRODUCT_ID;

                if (product_id > 3)
                    priority += LEVEL;

                if (product_id == 7)
                    priority += LEVEL;

                priority += getUrgency(j) * URGENCY;

                if (workshops[j].type_ == 9 && product_id != 7)
                    priority -= NINE_WORKSHOP;

                Task task;
                task.buy_workshop_id_ = i;
                task.sell_workshop_id_ = j;
                if (willCollideOther(robot_id, task)) {
                    priority -= COLLIDE;
                }

                priority = priority / distance;

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
        task.stage_ = 1;
        robot.has_task_ = true;

        reserveProduct(buy_from);
        reserveStuff(sell_to, tar_product_id);
    }
}

// 避免撞墙
void reduceCollideWall()
{
    for (int i = 0; i < 4; ++i)
    {
        Position new_position = robots[i].position_ + robots[i].linear_velocity_ * 0.5;
        if (new_position.x_ < 0 || new_position.x_ > 50 || new_position.y_ < 0 || new_position.y_ > 50)
        {
            cout << "forward " + to_string(i) + " 3" << '\n';
        }
    }
}

// 为机器人设置具体指令
void setInsToDes(int robot_id)
{
    struct Robot &robot = robots[robot_id];
    struct Task &task = robot.task_;

    if (!robot.has_task_)
    {
        cout << "forward " + to_string(robot_id) + " 0" << '\n';
        cout << "rotate " + to_string(robot_id) + " 0" << '\n';
    }
    else if (task.task_type_ == DESTROY)
    {
        // 目前的策略不会销毁产品
        throw runtime_error("unexpected condition");
    }
    else
    {
        int target_workshop_id = getDestination(robot_id);

        if (target_workshop_id == robot.workshop_id_)
        {
            if (task.stage_ == 1)
            {
                cout << "buy " + to_string(robot_id) << '\n';
                cancelReserveProduct(target_workshop_id);
                task.stage_ = 2;
                // 执行完购买指令后，立即前往售卖地
                setInsToDes(robot_id);
            }
            else
            {
                cout << "sell " + to_string(robot_id) << '\n';
                cancelReserveStuff(target_workshop_id, robot.product_id_);
                robot.has_task_ = false;
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
                cout << "rotate " + to_string(robot_id) + " " + to_string(palstance) << '\n';
            }
            else
            {
                cout << "rotate " + to_string(robot_id) + " 0" << '\n';
            }

            if (Dot(robot.linear_velocity_, robot_to_workshop) < 0)
            {
                cout << "forward " + to_string(robot_id) + " 0" << '\n';
            }
            else
            {
                if (distance < 1 && !IsZero(palstance))
                {
                    cout << "forward " + to_string(robot_id) + " 0" << '\n';
                }
                else
                {
                    cout << "forward " + to_string(robot_id) + " 6" << '\n';
                }
            }
        }
    }
}
