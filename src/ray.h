#ifndef RAY_H
#define RAY_H

#include "vec3.h"
//实现了光线类结构，包含一个point3字段和vec3字段，成员函数包括读取函数，构建函数和确认点位置函数。新增了时间字段
class ray {
private:
    point3 orig;
    vec3 dir;
    double tm;
public:
    ray() = default;
    ray(const point3& origin, const vec3& direction) :orig{ origin }, dir{ direction }, tm{ 0 } {}
    ray(const point3& origin, const vec3& direction, double time) :orig{ origin }, dir{ direction }, tm{ time } {}
    const point3& origin() const { return orig; }
    const vec3& direction() const { return dir; }
    double time()const { return tm; }
    point3 at(double t) const { return (orig + t * dir); }
};


#endif