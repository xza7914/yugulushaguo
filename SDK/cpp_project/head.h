
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
const int MAX_FRAME_ID = 9000;

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
    int type_;
    Position position_;
    int remainder_produce_time_;
    int stuff_status_;
    int product_status_;
    // 代表被预定的状态
    int reserve_stuff_status_;
    int reserve_product_status_;
};

enum TASK_TYPE
{
    EMPTY,
    DESTROY,
    SELL,
    BUY
};

// 任务分为两段，一阶段买，二阶段卖
struct Task
{
    int buy_workshop_id_;
    int sell_workshop_id_;
    int product_id_;
    TASK_TYPE task_type_;
};

// 机器人状态，每帧更新一次
struct Robot
{
    int robot_id_;
    int workshop_id_;
    int product_id_;
    double time_rate_;
    double collide_rate_;
    double palstance_;
    LinearVelocity linear_velocity_;
    double direction_;
    Position position_;

    bool has_task_; // 当前机器人是否有任务
    Task task_;     // 当前机器人的任务类型
};

double IsZero(double);
double Dot(Vector, Vector);
double Length(Vector, Vector);
double Angle(Vector, Vector);

void init();
int getProductId(int);
void reserveStuff(int, int);
void cancelReserveStuff(int, int);
void reserveProduct(int);
void cancelReserveProduct(int);
bool canRecvStuff(int, int);
void readAndSetStatus();
void getNextDes(int, int);
vector<string> setInsToDes(int);

#endif