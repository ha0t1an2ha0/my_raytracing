#ifndef ONB_H
#define ONB_H

#include "rtweekend.h"

class onb
{
private:
    vec3 axis[3];
public:

    onb() = default;

    vec3 operator[] (int i) const { return axis[i]; }
    vec3& operator[] (int i) { return axis[i]; }

    vec3 u() const { return axis[0]; }
    vec3 v() const { return axis[1]; }
    vec3 w() const { return axis[2]; }

    point3 local(double x, double y, double z) const {
        return x * axis[0] + y * axis[1] + z * axis[2];
    }

    point3 local(const vec3& v) const {
        return v[0] * axis[0] + v[1] * axis[1] + v[2] * axis[2];
    }

    void build_from_w(const vec3& w) {
        vec3 unit_w = normalize(w);
        vec3 a = (fabs(unit_w.x()) > 0.9) ? vec3(0.0, 1.0, 0.0) : vec3(1.0, 0.0, 0.0);
        vec3 v = normalize(cross(unit_w, a));
        vec3 u = cross(unit_w, v);
        axis[0] = u;
        axis[1] = v;
        axis[2] = unit_w;
    }
};

#endif