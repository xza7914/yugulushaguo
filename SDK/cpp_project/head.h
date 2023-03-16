
// 头文件

#ifndef __HEAD_H__
#define __HEAD_H__

#include <math.h>
#include <vector>
#include <string>
using namespace std;

const double PI = acos(-1);
const double EPS = 1e-2;
const double TIME_FRAME = 1.0 / 50;

// 用于坐标计算
struct Point
{
    double x_;
    double y_;
    Point(double x, double y) : x_(x), y_(y) {}
    Point() : x_(0), y_(0) {}
};

typedef Point LinearVelocity;
typedef Point Position;
typedef Point Vector;

// 工作台状态，每帧更新一次
struct Workshop
{
    // 通用状态（题目所给）
    int type_;
    Position position_;
    int remainder_produce_time_;
    int stuff_status_;
    char product_status_;
    // 专用状态
    bool assigned_ [10]; // assigned_[i] 表示产品 i 已经被分配给了机器人。（分配时判断并赋值，结算时消除）
};

enum TASK_TYPE
{
    EMPTY,
    DESTROY,
    SELL,
    BUY
};

struct Task
{
    int workshop_id_;
    TASK_TYPE task_type_;
};

// 机器人状态，每帧更新一次
struct Robot
{
    // 通用状态（题目所给）
    int robot_id_;
    int workshop_id_;
    int product_id_;
    double time_rate_;
    double collide_rate_;
    double palstance_;
    LinearVelocity linear_velocity_;
    double direction_;
    Position position_;
    // 专用状态
    bool has_task_; // 当前机器人是否有任务
    Task task_;     // 当前机器人的任务类型
};

double IsZero(double);
double Dot(Vector, Vector);
double Length(Vector, Vector);
double Angle(Vector, Vector);

void init();
bool canRecv(int, int);
void readAndSetStatus();
void getNextDes(int);
vector<string> setInsToDes(int);

#endif