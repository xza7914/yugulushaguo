
// 头文件

#ifndef __HEAD_H__
#define __HEAD_H__

#include <math.h>
#include <vector>
#include <string>
#include <algorithm>
using namespace std;

const double PI = acos(-1);
const double EPS = 1e-2;
const double TIME_FRAME = 1.0 / 50;
const int MAX_FRAME_ID = 9000;
const double MAX_VELOCITY= 6; 
const double BASE_VELOCITY = 4; // 预设最低速度（为0会导致永远停止）
const double BASE_PALSTANCE = 0.5; // 预设最低角速度
const double BASE_TIME = 0.45; // 预设最低预测时间，低于则发出碰撞警告，需要减速/转向等措施干预
const double BASE_DISTANCE = 3; // 预设最低距离碰撞限制，低于则发出碰撞警告，需要减速/转向等措施干预


const double MAX_X = 50;
const double MAX_Y = 50;
const double RADIUS_ROBOT = 0.43;
const double RADIUS_ROBOT_CARRY = 0.53;

// 碰撞应急策略
const int NOTHING               = 0;
const int SLOW_DOWN             = 1 << 0;
const int ROTATE_CLOCKWISE      = 1 << 1;
const int ROTATE_ANTI_CLOCKWISE = 1 << 2;

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
bool isIntersect(double,double,double,double,double,double,double,double);
double PointToSegDist(double, double, double, double, double, double);

void init();
void scanCollisionStatus();
int getProductId(int);
void reserveStuff(int, int);
void cancelReserveStuff(int, int);
void reserveProduct(int);
void cancelReserveProduct(int);
int getUrgency(int);
bool canRecvStuff(int, int);
bool canCollideWall(int);
pair<int,int> canCollideRobot(int, int);
void readAndSetStatus();
vector<string> reduceCollide();
void getNextDes(int, int);
vector<string> setInsToDes(int);

#endif