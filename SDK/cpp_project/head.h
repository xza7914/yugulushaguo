
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
    int stage_;
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
double Dot(const Vector &, const Vector &);
double Cross(const Vector &, const Vector &);
double Length(const Vector &, const Vector &);
Vector getUnitVector(const Vector &);
double Angle(const Vector &, const Vector &);

// 获取某工作台的产品类型
int getProductId(int);

// 预定原料
void reserveStuff(int, int);

// 取消预订原料
void cancelReserveStuff(int, int);

// 预定产品
void reserveProduct(int);

// 取消预订产品
void cancelReserveProduct(int);

// 获取某项工作的紧急程度
int getUrgency(int);

// 预测某一帧时某机器人的位置与目标
int predictPosAndDes(int, int, Position &, Position &);

// 判断机器人是否会与其他机器人迎面相撞
bool willCollideOther(int, struct Task &);

// 判断某工作台是否能在到达之前准备好产品
bool canProductReady(int, double);

// 判断当前 第workshop_id个工作台是否可以接收product_id号产品
bool canRecvStuff(int, int);

// 获取某个机器人此时的目的地
int getDestination(int);

// 初始化并读入地图
void init();

// 设置当前frame_id
void setNowFrameId(int);

// 读取当前状态
void readAndSetStatus();

// 按照当前状态为机器人安排任务
void getNextDes(int);

// 避免撞墙
void reduceCollideWall();

// 为机器人设置具体指令
void setInsToDes(int);

#endif