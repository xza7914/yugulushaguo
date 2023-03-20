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
int collision[4];

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


/// @brief Judge if two line segments intersect. Cite: https://blog.csdn.net/qq_38210354/article/details/107495845
bool isIntersect(double Ax1,double Ay1,double Ax2,double Ay2,double Bx1,double By1,double Bx2,double By2)
{
    if(
       ( max(Ax1,Ax2)>=min(Bx1,Bx2)&&min(Ax1,Ax2)<=max(Bx1,Bx2) )&&  //判断x轴投影
       ( max(Ay1,Ay2)>=min(By1,By2)&&min(Ay1,Ay2)<=max(By1,By2) )    //判断y轴投影
      )
    {
        if(
            ( (Bx1-Ax1)*(Ay2-Ay1)-(By1-Ay1)*(Ax2-Ax1) ) *          //判断B是否跨过A
            ( (Bx2-Ax1)*(Ay2-Ay1)-(By2-Ay1)*(Ax2-Ax1) ) <=0 &&
            ( (Ax1-Bx1)*(By2-By1)-(Ay1-By1)*(Bx2-Bx1) ) *          //判断A是否跨过B
            ( (Ax2-Bx1)*(By2-By1)-(Ay2-By1)*(Bx2-Bx1) ) <=0
          )
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

/// @brief Predict that the robot may collide with the wall in `BASE_TIME` seconds.
bool canCollideWall(int robot_id){
    struct Robot &robot = robots[robot_id];
    double x = robot.position_.x_;
    double y = robot.position_.y_;
    double vx = min(robot.linear_velocity_.x_, BASE_VELOCITY);
    double vy = max(robot.linear_velocity_.y_, BASE_VELOCITY);

    double predict_x = x + vx * BASE_TIME;
    double predict_y = x + vy * BASE_TIME;

    if(predict_x > MAX_X || predict_x < EPS){
        return true;
    }
    if(predict_y > MAX_Y || predict_y < EPS){
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
        if(min_dis > 2* RADIUS_ROBOT_CARRY)
            return pair<int,int> (NOTHING, NOTHING);
    }
        
        

    
    //  0      ~  1/4 PI: robot1逆时针，robot2顺时针
    if (angle > EPS && angle < (PI/4)) 
        return pair<int,int> (ROTATE_ANTI_CLOCKWISE, ROTATE_CLOCKWISE);
    //  3/4 PI ~      PI: robot1逆时针，robot2逆时针
    if (angle > (PI/4)*3 && angle < PI )
        return pair<int,int> (ROTATE_ANTI_CLOCKWISE, ROTATE_ANTI_CLOCKWISE);
    //  0   PI ~ -1/4 PI: robot1顺时针，robot2逆时针
    if (angle < EPS && angle < -(PI/4) )
        return pair<int,int> (ROTATE_CLOCKWISE, ROTATE_ANTI_CLOCKWISE);
    // -3/4 PI ~     -PI: robot1顺时针，robot2顺时针
    if (angle < -(PI/4)*3 && angle > PI )
        return pair<int,int> (ROTATE_CLOCKWISE, ROTATE_CLOCKWISE);

    // 其余大角度 : robot1 减速
    return pair<int,int> (NOTHING, NOTHING);
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

/// @brief Pre-scan all robots collision status, and store to global collistion[robots][robots]
void scanCollisionStatus()
{
    for (int i = 0; i < 4; i++){
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
                res.push_back("rotate " + to_string(robot_id) + " " + to_string(palstance));
            }
            else
            {
                res.push_back("rotate " + to_string(robot_id) + " 0");
            }

            // 目的工作台与速度反向：刹车
            if (Dot(robot.linear_velocity_, robot_to_workshop) < 0)
            {
                res.push_back("forward " + to_string(robot_id) + " 0");
            }
            // 可能与墙体发生碰撞：刹车
            // else if ( canCollideWall(robot_id) )
            // {
            //     cerr << "may collide wall" << endl;
            //     // res.push_back("forward " + to_string(robot_id) + " " + to_string(BASE_VELOCITY));
            // }
            // 可能与汽车发生碰撞
            else if ( collision[robot_id] )
            {
                switch (collision[robot_id]) {
                    case SLOW_DOWN:// 减速
                    case ROTATE_CLOCKWISE | ROTATE_ANTI_CLOCKWISE:// 顺时针+顺时针：只减速
                    case SLOW_DOWN | ROTATE_CLOCKWISE | ROTATE_ANTI_CLOCKWISE:// 减速+逆时针+顺时针：只减速
                    {
                        // cerr << "SLOW_DOWN" << endl;
                        res.push_back("forward " + to_string(robot_id) + " " + to_string(BASE_VELOCITY));
                        break;
                    }
                    case ROTATE_CLOCKWISE:// 顺时针
                    case SLOW_DOWN | ROTATE_CLOCKWISE:// 减速+顺时针
                    {
                    //     cerr << "ROTATE_CLOCKWISE" << endl;
                        res.push_back("forward " + to_string(robot_id) + " " + to_string(MAX_VELOCITY));
                        res.push_back("rotate " + to_string(robot_id) + " " + to_string(BASE_PALSTANCE));
                        break;
                    }
                    case ROTATE_ANTI_CLOCKWISE:// 逆时针
                    case SLOW_DOWN | ROTATE_ANTI_CLOCKWISE:// 减速+逆时针
                    {
                        // cerr << "ROTATE_ANTI_CLOCKWISE" << endl;
                        res.push_back("forward " + to_string(robot_id) + " " + to_string(MAX_VELOCITY));
                        res.push_back("rotate " + to_string(robot_id) + " " + to_string(-BASE_PALSTANCE));
                        break;
                    }
                    default: {
                        throw runtime_error("unexpected collision[i]");
                        break;
                    }
                    
                }
                // cerr << "may collide wall" << endl;
                // res.push_back("forward " + to_string(robot_id) + " " + to_string(BASE_VELOCITY));
            }
            else
            {
                // cerr << "MAX_VELOCITY" << endl;
                if (distance < 1 && !IsZero(palstance))
                {
                    res.push_back("forward " + to_string(robot_id) + " 0");
                }
                else{
                    res.push_back("forward " + to_string(robot_id) + " " + to_string(MAX_VELOCITY));
                }
            }

            
        }
    }

    return res;
}
