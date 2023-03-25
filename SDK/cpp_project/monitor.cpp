/*
中央控制系统（主控Monitor）：用来管理、监控、预测全局状态，力求实现全局最优解。
主要包含两个系统：决策系统、监控系统。决策系统用来预测并分配任务，而监控系统用来跟踪实际状态下每个机器人的真实运行情况，进而修正决策系统。

决策系统：
    决策的核心输出：给定决策从工作台wA购买产品pA，放置于wB，潜在收益最高。一般来说潜在收益为单位时间下的金钱收获。
    决策的核心结构1： 静态产品依赖表
                    (wA) -> (wB, pA) 金钱收益和时间消耗
    决策的核心结构2： 动态决策收益表
                    考虑到当前工作台的各产品情况下
                    (wA） -> (wB, pA) 收益
                    举例：若wA就绪，则为买wA送到wB的预计收益和时间损失（用静态产品依赖表计算得到）。若未就绪，则将该预计收益加上完成pA的收益。
    决策的核心算法：对每个送货选择(wA) -> (wB, pA) ，选择最大的单位时间下的动态决策收益路线，且预计该路线能被完成。
    算法的不精确性及解决方案：静态产品依赖表没有考虑可能的碰撞、转向等时间和价值上的损失。该不精确性可以使用监控系统修正。
    全局有一个静态最优规划表，每个台都有一个完整流需要的时间和金钱，每个机器人用这个来判断去哪，这样可以去掉一堆参数。

监控系统：
    用来跟踪实际状态下每个机器人的真实运行情况，在台子之间运动的真实时间和所获得的金钱。根据这个值算出：标准差、平均值。进而修正决策系统。
    该过程可以不停重复训练、迭代，得到更精确的决策系统。
    监控的核心结构与输出：产品依赖数据表
                    (wA) -> (wB, pA) 的所有过程流的实际收益和时间消耗。
    监控的核心算法：对每个送货开始和完成进行记录，就可以得到产品依赖数据表。
    算法的不精确性及解决方案：产品依赖数据表只能反映其平均值，不能够精确到具体的决策。
                            该不精确性可以增加决策系统的实施碰撞等检测方案，得到实时修正值，该值仍然可以加入到监控系统进行训练。
*/


#include "head.h"
#include "vector"

#define MAX_ROBOT_NUM 100
#define MAX_WORKSHOP_TYPE 10
const int FRAME_OPERATION = 0; // 每次买卖操作需要的额外时间（加减速/转向/碰撞等）
int prices_buy[10] = {999999, 3000, 4400, 5800, 15400, 17200, 19200, 76000, 999999, 999999};
int prices_sell[10] = {0, 6000, 7600, 9200, 22500, 25000, 27500, 105000, 0, 0};
int global_product_produce[MAX_WORKSHOP_TYPE] = {0,1,2,3,4,5,6,7,0,0,0};
int workshop_product_produce[][] = {{0}, // Workshop 0: Not exist
                                    {0}, // Workshop 1: Can produce 
                                    {0},
                                    {0},
                                    }
double global_distance[MAX_ROBOT_NUM][MAX_ROBOT_NUM];
double global_money[MAX_ROBOT_NUM][MAX_ROBOT_NUM];



double Distance(struct Position& pos1, struct Position& pos2)
{
    Vector v1 = Vector(pos1.x_, pos1.y);
    Vector v2 = Vector(pos2.x_, pos2.y);
    return Length(v2-v1);
}

int produceProductType(int workshop_type){
    if(workshop_type >= MAX_WORKSHOP_TYPE)
        return 0;
    else
        return global_product_produce[workshop_type];
}

vector<int> consumeProductType(int workshop_type){
    switch (workshop_type){
        case 1: return vector<int>{};
        case 2: return vector<int>{};
        case 3: return vector<int>{};
        case 4: return vector<int>{1,2};
        case 5: return vector<int>{1,3};
        case 6: return vector<int>{2,3};
        case 7: return vector<int>{4,5,6};
        case 8: return vector<int>{7};
        case 9: return vector<int>{1,2,3,4,5,6,7};
        default: return vector<int>{};
    }
    return vector<int>{};
}

int getPriceWithDistance(int max_price, int frames)
{
    double x = max_price;
    double min_rate = 0.8;
    double maxX = 9000;
    if( x >= maxX)
        return min_rate;
    return max_price * (1 - sqrt(1 -  pow(1 - x / maxX , 2))) - (1 - min_rate);
}

void build_distance_table(struct Workshop &workshops[], int workshop_num)
{
    // Build table `global_distance`;
    for(int i=0; i<workshop_num; i++){
        for(int j=(i+1); j<workshop_num; j++){
            global_distance[i][j] = global_distance[j][i] = Distance(workshops[i].position_, workshops[j].position_);
        }
    }
    // Build table `global_money`
    // 1.  Get product dependency
    // 2.  Get product dependency

    int current_workshop;
    int sum = 0;
    for (product in product_needed){
        int product_type;
        int max_price = 0;
        int max_workshop = -1;
        for (workshop in workshops){
            if can_produce(workshop, product_type){
                double distance = Distance(current_workshop.position_, workshop.position_);
                int frames = distance / MAX_VELOCITY / TIME_FRAME + FRAME_OPERATION;
                int price = getPriceWithDistance(prices_sell[product], frames) - prices_buy[product];
                if( max_price < price ){
                    max_price = price;
                    max_workshop = workshop;
                }
            }
        }
        sum += max_price;
    }

}