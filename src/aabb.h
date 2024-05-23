#ifndef AABB_H
#define AABB_H

#include "rtweekend.h"
//实现了aabb类，构造函数包括通过三坐标线段、对角线两点、两个小box；实现hit函数检测光线是否与aabb相交
class aabb
{
public:
    interval x, y, z;

    aabb() = default;
    aabb(const interval& x, const interval& y, const interval& z) :x{ x }, y{ y }, z{ z } {
        pad_to_mininums();
    };
    aabb(const point3& a, const point3& b) {
        x = a.x() <= b.x() ? interval(a.x(), b.x()) : interval(b.x(), a.x());
        y = a.y() <= b.y() ? interval(a.y(), b.y()) : interval(b.y(), a.y());
        z = a.z() <= b.z() ? interval(a.z(), b.z()) : interval(b.z(), a.z());
        pad_to_mininums();
    }
    aabb(const aabb& box1, const aabb& box2) {
        x = interval(box1.x, box2.x);
        y = interval(box1.y, box2.y);
        z = interval(box1.z, box2.z);
    }

    const interval& axis_interval(int axis) const {
        if (axis == 1)return y;
        if (axis == 2)return z;
        return x;
    }

    bool hit(const ray& r, interval ray_t) const {
        point3 ori = r.origin();
        vec3 dir = r.direction();
        for (int axis = 0; axis < 3; ++axis) {
            interval ax = axis_interval(axis);
            double dir_inv = 1.0 / dir[axis];
            double t0 = (ax.min - ori[axis]) * dir_inv;
            double t1 = (ax.max - ori[axis]) * dir_inv;
            if (t0 < t1) {
                if (t0 > ray_t.min) {
                    ray_t.min = t0;
                }
                if (t1 < ray_t.max) {
                    ray_t.max = t1;
                }
            }
            else {
                if (t1 > ray_t.min) {
                    ray_t.min = t1;
                }
                if (t0 < ray_t.max) {
                    ray_t.max = t0;
                }
            }
            if (ray_t.min >= ray_t.max) return false;
        }
        return true;
    }

    int longest_axis() const {
        if (x.size() > y.size()) {
            return x.size() > z.size() ? 0 : 2;
        }
        else {
            return y.size() > z.size() ? 1 : 2;
        }
    }

    static const aabb empty, universe;

private:
    void pad_to_mininums() {
        double delta = 0.0001;
        if (x.size() < delta) x = x.expand(delta);
        if (y.size() < delta) y = y.expand(delta);
        if (z.size() < delta) z = z.expand(delta);
    }
};

const aabb aabb::empty = aabb(interval::empty, interval::empty, interval::empty);
const aabb aabb::universe = aabb(interval::universe, interval::universe, interval::universe);

aabb operator+ (const aabb& box, const vec3& offset) {
    return aabb(box.x + offset.x(), box.y + offset.y(), box.z + offset.z());
}

aabb operator+ (const vec3& offset, const aabb& box) {
    return box + offset;
}


#endif

