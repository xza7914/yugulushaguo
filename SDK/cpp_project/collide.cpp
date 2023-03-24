
// 一些碰撞函数
#include "head.h"
#include <iostream>

// 判断是否会相撞
bool willCollide(const Position &pos1, const Position &des1, const Position &pos2, const Position &des2)
{
    Vector direction1 = des1 - pos1;
    Vector direction2 = des2 - pos2;

    double len1 = Length(direction1) - 0.4;
    double len2 = Length(direction2) - 0.4;

    double len = 0;
    for (; len < 5; len += 0.5)
    {
        Position tpos1 = pos1 + getUnitVector(direction1) * len;
        Position tpos2 = pos2 + getUnitVector(direction2) * len;

        if (len > len1 || len > len2)
            return false;

        if (Length(tpos1 - tpos2) < 0.9)
            return true;
    }

    return false;
}

// 计算临时目标点
// 临时目标点与实际位置组成菱形
// pos1------------temp1
//   | *              |
//   |    *           |
//   |       *        |
//   |          *     |
//   |             *  |
// temp2------------pos2
void getTempDes(const Position &pos1, const Position &pos2, Position &temdes1, Position &temdes2) {
    Position mid = Position{(pos1.x_ + pos2.x_) / 2, (pos1.y_ + pos2.y_) / 2};
    Vector direction = pos2 - pos1;
    Vector vertical_direction = Vector{-direction.y_, direction.x_};

    double len = Length(direction);

    // 目前是向两侧偏移单位向量长度， 可以修改， 例如乘以1.5倍
    temdes1 = mid + getUnitVector(vertical_direction) * 1.0;
    temdes2 = mid - getUnitVector(vertical_direction) * 1.0; 
}