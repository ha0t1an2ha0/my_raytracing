#ifndef INTERVAL_H
#define INTERVAL_H

#include "rtweekend.h"
//实现间隔类，要求默认构造函数生成空的间隔，实现两个函数检测值x是否在间隔的开区间和闭区间中。静态预生成两个实现，空和无穷间隔

class interval
{
public:
    double min;
    double max;

    interval() :min{ infinity }, max{ -infinity } {};
    interval(double a, double b) :min{ a }, max{ b } {};
    interval(const interval& a, const interval& b) {
        min = a.min <= b.min ? a.min : b.min;
        max = a.max >= b.max ? a.max : b.max;
    }

    double size() const { return max - min; }

    bool contains(double x) const {
        return x >= min && x <= max;
    }

    bool surrounds(double x) const {
        return x > min && x < max;
    }

    double clamp(double x) const {
        if (x < min)return min;
        if (x > max)return max;
        return x;
    }

    interval expand(double delta) const {
        double padding = delta / 2.0;
        return interval(min - padding, max + padding);
    }

    static const interval universe, empty;
};

const interval interval::universe = interval(-infinity, infinity);
const interval interval::empty = interval(infinity, -infinity);

interval operator+ (const interval& p, double d) {
    return interval(p.min + d, p.max + d);
}

interval operator+ (double d, const interval& p) {
    return p + d;
}

#endif