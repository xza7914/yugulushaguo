#include "head.h"
#include "macro.h"
#include "maps.h"
#include <cassert>
#include <stdexcept>
#include <iostream>
#include <cstring>
using namespace std;

// #define TIAOCAN

int workshop_num;
int now_frame_id;
struct Workshop workshops[55];
struct Robot robots[4];
int collision[4];

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
Vector getUnitVector(const Vector &a)
{
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

// 字符串哈希，用于确定地图
unsigned long long stringHash(const string &s, unsigned long long old)
{
    for (auto x : s)
        old = old * HASH_BASE + x;
    return old;
}

/// @brief Judge if two line segments intersect. Cite: https://blog.csdn.net/qq_38210354/article/details/107495845
bool isIntersect(double Ax1, double Ay1, double Ax2, double Ay2, double Bx1, double By1, double Bx2, double By2)
{
    if (
        (max(Ax1, Ax2) >= min(Bx1, Bx2) && min(Ax1, Ax2) <= max(Bx1, Bx2)) && // 判断x轴投影
        (max(Ay1, Ay2) >= min(By1, By2) && min(Ay1, Ay2) <= max(By1, By2))    // 判断y轴投影
    )
    {
        if (
            ((Bx1 - Ax1) * (Ay2 - Ay1) - (By1 - Ay1) * (Ax2 - Ax1)) * // 判断B是否跨过A
                    ((Bx2 - Ax1) * (Ay2 - Ay1) - (By2 - Ay1) * (Ax2 - Ax1)) <=
                0 &&
            ((Ax1 - Bx1) * (By2 - By1) - (Ay1 - By1) * (Bx2 - Bx1)) * // 判断A是否跨过B
                    ((Ax2 - Bx1) * (By2 - By1) - (Ay2 - By1) * (Bx2 - Bx1)) <=
                0)
        {
            return 1;
        }
        else
            return 0;
    }
    else
        return 0;
}

/// @brief Distance between point and the segment. Cite:https://blog.csdn.net/qq_28087491/article/details/119239974
double PointToSegDist(double x, double y, double x1, double y1, double x2, double y2)
{
	double cross = (x2 - x1) * (x - x1) + (y2 - y1) * (y - y1);
	if (cross <= 0) return sqrt((x - x1) * (x - x1) + (y - y1) * (y - y1));
	  
	double d2 = (x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1);
	if (cross >= d2) return sqrt((x - x2) * (x - x2) + (y - y2) * (y - y2));
	  
	double r = cross / d2;
	double px = x1 + (x2 - x1) * r;
	double py = y1 + (y2 - y1) * r;
	return sqrt((x - px) * (x - px) + (y - py) * (y - py));
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
            pos = pos_1 + getUnitVector(direction) * 6 * (remain_frames)*TIME_FRAME;
            des = des_1;
            return 0;
        }
        else
        {
            Task &task = robots[robot_id].task_;
            if (task.stage_ == 2)
                return -1;

            Position des_2 = workshops[task.sell_workshop_id_].position_;
            remain_frames -= frames_to_des;

            if (remain_frames > int(Length(des_2 - des_1) / 6 * 50))
                return -1;

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

/// @brief Predict that the robot may collide with the wall in `BASE_TIME` seconds.
bool canCollideWall(int robot_id)
{
    struct Robot &robot = robots[robot_id];
    double x = robot.position_.x_;
    double y = robot.position_.y_;
    double vx = min(robot.linear_velocity_.x_, BASE_VELOCITY);
    double vy = max(robot.linear_velocity_.y_, BASE_VELOCITY);

    double predict_x = x + vx * BASE_TIME;
    double predict_y = x + vy * BASE_TIME;

    if (predict_x > MAX_X || predict_x < EPS)
    {
        return true;
    }
    if (predict_y > MAX_Y || predict_y < EPS)
    {
        return true;
    }
    return false;
}

string debugLineIntersect(double Ax1,double Ay1,double Ax2,double Ay2,double Bx1,double By1,double Bx2,double By2){
    string s;
    s += "(" + to_string(Ax1) + "," + to_string(Ay1) + ")";
    s += "(" + to_string(Ax2) + "," + to_string(Ay2) + ")";
    s += "(" + to_string(Bx1) + "," + to_string(By1) + ")";
    s += "(" + to_string(Bx2) + "," + to_string(By2) + ")";
    return s;
}

/// @brief Predict that the robot may collide with the wall in `BASE_TIME` seconds.
/*
    Strategy (If might collide):
        Make far robot Slow down when collide angle ranges 45~135
        Both avoid collide through turning angles when collide angle ranges 0~45 135~180 (Each for different directions)
*/
pair<int,int> canCollideRobot(int robot1_id, int robot2_id){
    struct Robot &robot1 = robots[robot1_id];
    struct Robot &robot2 = robots[robot2_id];
    double x1 = robot1.position_.x_;
    double y1 = robot1.position_.y_;
    double x2 = robot2.position_.x_;
    double y2 = robot2.position_.y_;
    // double vx1 = max(robot1.linear_velocity_.x_, BASE_VELOCITY);
    // double vy1 = max(robot1.linear_velocity_.y_, BASE_VELOCITY);
    // double vx2 = max(robot2.linear_velocity_.x_, BASE_VELOCITY);
    // double vy2 = max(robot2.linear_velocity_.y_, BASE_VELOCITY);
    double vx1 = robot1.linear_velocity_.x_;
    double vy1 = robot1.linear_velocity_.y_;
    double vx2 = robot2.linear_velocity_.x_;
    double vy2 = robot2.linear_velocity_.y_;
    double angle = Angle({vx1, vy1}, {vx2, vy2});
    double relative_vx = vx2 - vx1; 
    double relative_vy = vy2 - vy1; 
    double relative_v = sqrt((relative_vx*relative_vx) + (relative_vy*relative_vy)); 
    double base_time = min(BASE_DISTANCE/relative_v, BASE_TIME);
    // cerr << base_time << endl;
    double predict_x1 = x1 + vx1 * base_time;
    double predict_y1 = x1 + vy1 * base_time;
    double predict_x2 = x2 + vx2 * base_time;
    double predict_y2 = x2 + vy2 * base_time;
    

    // 预测不碰撞条件1：预计轨道不相交
    if( ! isIntersect(x1, y1, predict_x1, predict_y1, x2, y2, predict_y2, predict_y2) ){
        // cerr << "intersect:" << debugLineIntersect(x1, y1, predict_x1, predict_y1, x2, y2, predict_y2, predict_y2);

        // if(
        //     isIntersect(x1 +RADIUS_ROBOT_CARRY , y1+RADIUS_ROBOT_CARRY
        //         , predict_x1+RADIUS_ROBOT_CARRY, predict_y1+RADIUS_ROBOT_CARRY
        //         , x2-RADIUS_ROBOT_CARRY, y2-RADIUS_ROBOT_CARRY
        //         , predict_y2-RADIUS_ROBOT_CARRY, predict_y2-RADIUS_ROBOT_CARRY) ||
        //     isIntersect(x1 -RADIUS_ROBOT_CARRY , y1-RADIUS_ROBOT_CARRY
        //         , predict_x1-RADIUS_ROBOT_CARRY, predict_y1-RADIUS_ROBOT_CARRY
        //         , x2+RADIUS_ROBOT_CARRY, y2+RADIUS_ROBOT_CARRY
        //         , predict_y2+RADIUS_ROBOT_CARRY, predict_y2+RADIUS_ROBOT_CARRY)
        // )
        //     return 1 << 0;
        // else{
        //     return 0;
        // }

        
        // 预测不碰撞条件2：预计轨道间最短距离大于直径
        /*      考虑是否会因为半径而相碰，而非质心相碰。
                方法：使用相对距离和相对速度，算出相对轨迹过程中最短距离
        */   
        double relative_x = x2 - x1; 
        double relative_y = y2 - y1; 
        double relative_predict_x = predict_x2 - predict_x1; 
        double relative_predict_y = predict_y2 - predict_y1;
        double min_dis = PointToSegDist(0,0,relative_x,relative_y,relative_predict_x,relative_predict_y);
        // cerr << min_dis << endl;
        if(min_dis > 2* RADIUS_ROBOT_CARRY)
            return pair<int,int> (NOTHING, NOTHING);
    }
        
        

    
    // //  0      ~  1/4 PI: robot1逆时针，robot2顺时针
    // if (angle > EPS && angle < (PI/4)) 
    //     return pair<int,int> (ROTATE_ANTI_CLOCKWISE, ROTATE_CLOCKWISE);
    //  3/4 PI ~      PI: robot1逆时针，robot2逆时针
    if (angle > (PI/4)*3 && angle < PI )
        return pair<int,int> (ROTATE_CLOCKWISE, ROTATE_CLOCKWISE);
        // return pair<int,int> (ROTATE_ANTI_CLOCKWISE, ROTATE_ANTI_CLOCKWISE);
    // //  0   PI ~ -1/4 PI: robot1顺时针，robot2逆时针
    // if (angle < EPS && angle < -(PI/4) )
    //     return pair<int,int> (ROTATE_CLOCKWISE, ROTATE_ANTI_CLOCKWISE);
    // -3/4 PI ~     -PI: robot1顺时针，robot2顺时针
    if (angle < -(PI/4)*3 && angle > -PI )
        return pair<int,int> (ROTATE_ANTI_CLOCKWISE, ROTATE_ANTI_CLOCKWISE);
        // return pair<int,int> (ROTATE_CLOCKWISE, ROTATE_CLOCKWISE);

    // cerr << angle << endl;

    // 其余大角度 : robot1 减速
    return pair<int,int> (NOTHING, NOTHING);
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

    unsigned long long hash = 0;

    // 读入地图
    string s;
    for (int i = 0; i < 100; ++i)
    {
        cin >> s;
        hash = stringHash(s, hash);
    }

#ifdef TIAOCAN
    // 调参时，所有地图共用参数
#else
    // 根据地图选择相应的参数
    switch (hash)
    {
    case HASH_OF_MAP1:
        INIT_PRIORITY = INIT_PRIORITY_1;
        PRODUCT_ID = PRODUCT_ID_1;
        LEVEL = LEVEL_1;
        URGENCY = URGENCY_1;
        NINE_WORKSHOP = NINE_WORKSHOP_1;
        COLLIDE = COLLIDE_1;
        break;

    case HASH_OF_MAP2:
        INIT_PRIORITY = INIT_PRIORITY_2;
        PRODUCT_ID = PRODUCT_ID_2;
        LEVEL = LEVEL_2;
        URGENCY = URGENCY_2;
        NINE_WORKSHOP = NINE_WORKSHOP_2;
        COLLIDE = COLLIDE_2;
        break;

    case HASH_OF_MAP3:
        INIT_PRIORITY = INIT_PRIORITY_3;
        PRODUCT_ID = PRODUCT_ID_3;
        LEVEL = LEVEL_3;
        URGENCY = URGENCY_3;
        NINE_WORKSHOP = NINE_WORKSHOP_3;
        COLLIDE = COLLIDE_3;
        break;

    case HASH_OF_MAP4:
        INIT_PRIORITY = INIT_PRIORITY_4;
        PRODUCT_ID = PRODUCT_ID_4;
        LEVEL = LEVEL_4;
        URGENCY = URGENCY_4;
        NINE_WORKSHOP = NINE_WORKSHOP_4;
        COLLIDE = COLLIDE_4;
        break;

    default:
        throw runtime_error("unexcepted map");
    }
#endif

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
                double max_length = 6 * (MAX_FRAME_ID - now_frame_id - 20) * TIME_FRAME;
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
                if (willCollideOther(robot_id, task))
                {
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

/// @brief Pre-scan all robots collision status, and store to global collistion[robots][robots]
void scanCollisionStatus()
{
    for (int i = 0; i < 4; i++)
    {
        collision[i] = 0;
    }
    for (int i = 0; i < 4; i++){
        for (int j = (i+1); j < 4 ; j++){
            pair<int,int> results = canCollideRobot(i, j);
            collision[i] |= results.first;
            collision[j] |= results.second;
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
            // 操作：前往工作台
            int workshop_id = target_workshop_id;

            Vector direction = Vector(cos(robot.direction_), sin(robot.direction_));
            Vector robot_to_workshop = workshops[workshop_id].position_ - robot.position_;

            // 策略：若非直线前往工作台，需要转向。
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

            // 目的工作台与速度反向：刹车
            if (Dot(robot.linear_velocity_, robot_to_workshop) < 0)
            {
                cout << "forward " + to_string(robot_id) + " 0" << '\n';
            }
            // 可能与墙体发生碰撞：刹车
            // else if ( canCollideWall(robot_id) )
            // {
            //     cerr << "may collide wall" << endl;
            //     // res.push_back("forward " + to_string(robot_id) + " " + to_string(BASE_VELOCITY));
            // }
            // 可能发生碰撞
            else
            {
                if (distance < 1 && fabs(angle) > 0.3)
                {
                    cout << "forward " + to_string(robot_id) + " 0" << '\n';
                }
                else{
                    cout << "forward " + to_string(robot_id) + " 6" << '\n';
                    // if ( collision[robot_id] )
                    // {
                    //     switch (collision[robot_id]) {
                    //         // Now, it's impssible to slow down
                    //         case SLOW_DOWN:// 减速
                    //         case ROTATE_CLOCKWISE | ROTATE_ANTI_CLOCKWISE:// 顺时针+顺时针：只减速
                    //         case SLOW_DOWN | ROTATE_CLOCKWISE | ROTATE_ANTI_CLOCKWISE:// 减速+逆时针+顺时针：只减速
                    //         {
                    //             // cerr << "SLOW_DOWN" << endl;
                    //             cout << "forward " + to_string(robot_id) + " " + to_string(BASE_VELOCITY)<< '\n';
                    //             break;
                    //         }
                    //         case ROTATE_CLOCKWISE:// 顺时针
                    //         case SLOW_DOWN | ROTATE_CLOCKWISE:// 减速+顺时针
                    //         {
                    //             // cerr << "ROTATE_CLOCKWISE" << endl;
                    //             cout << "rotate " + to_string(robot_id) + " " + to_string(BASE_PALSTANCE)<< '\n';
                    //             break;
                    //         }
                    //         case ROTATE_ANTI_CLOCKWISE:// 逆时针
                    //         case SLOW_DOWN | ROTATE_ANTI_CLOCKWISE:// 减速+逆时针
                    //         {
                    //             // cerr << "ROTATE_ANTI_CLOCKWISE" << endl;
                    //             cout << "rotate " + to_string(robot_id) + " " + to_string(-BASE_PALSTANCE)<< '\n';
                    //             break;
                    //         }
                    //         default: {
                    //             throw runtime_error("unexpected collision[i]");
                    //             break;
                    //         }
                    //     }
                    // }
                }
            }
        }
    }
}
