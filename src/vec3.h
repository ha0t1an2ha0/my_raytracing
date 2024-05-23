#ifndef VEC3_H
#define VEC3_H

//实现了vec3类，类定义中包括了默认和一般构建函数，查看函数，重载取负，两种[]查看，+=，*=，/=，模长

//之后的内联函数实现了重载输出流，+，-，逐项乘*，数乘*，/。定义了dot点乘，cross叉乘，正规化

#include "rtweekend.h"

class vec3 {
public:
    double e[3]{ 0.0,0.0,0.0 };

    vec3() = default;
    vec3(double f1, double f2, double f3) :e{ f1,f2,f3 } {}

    double x() const { return e[0]; }
    double y() const { return e[1]; }
    double z() const { return e[2]; }

    vec3 operator- () const { return vec3(-e[0], -e[1], -e[2]); }

    double operator[] (int i) const { return e[i]; }
    double& operator[] (int i) { return e[i]; }

    vec3& operator+= (const vec3& v) {
        e[0] += v.e[0];
        e[1] += v.e[1];
        e[2] += v.e[2];
        return *this;
    }

    vec3& operator*= (double f) {
        e[0] *= f;
        e[1] *= f;
        e[2] *= f;
        return *this;
    }

    vec3& operator/= (double f) {
        e[0] /= f;
        e[1] /= f;
        e[2] /= f;
        return *this;
    }

    double length() const { return sqrt(e[0] * e[0] + e[1] * e[1] + e[2] * e[2]); }

    double length_squared() const { return (e[0] * e[0] + e[1] * e[1] + e[2] * e[2]); }

    bool near_zero() const {
        double s = 1e-8;
        return (fabs(e[0]) < s) && (fabs(e[1]) < s) && (fabs(e[2]) < s);
    }

    static vec3 random() { return vec3(random_double(), random_double(), random_double()); }

    static vec3 random(double min, double max) { return vec3(random_double(min, max), random_double(min, max), random_double(min, max)); }

};

using point3 = vec3;

inline std::ostream& operator<<(std::ostream& out, const vec3& v) {
    return out << v.e[0] << ' ' << v.e[1] << ' ' << v.e[2];
}

inline vec3 operator+ (const vec3& v1, const vec3& v2) { return vec3(v1.e[0] + v2.e[0], v1.e[1] + v2.e[1], v1.e[2] + v2.e[2]); }

inline vec3 operator- (const vec3& v1, const vec3& v2) { return vec3(v1.e[0] - v2.e[0], v1.e[1] - v2.e[1], v1.e[2] - v2.e[2]); }

inline vec3 operator* (const vec3& v1, const vec3& v2) { return vec3(v1.e[0] * v2.e[0], v1.e[1] * v2.e[1], v1.e[2] * v2.e[2]); }

inline vec3 operator* (double f, const vec3& v) { return vec3(f * v.e[0], f * v.e[1], f * v.e[2]); }

inline vec3 operator* (const vec3& v, double f) { return vec3(f * v.e[0], f * v.e[1], f * v.e[2]); }

inline vec3 operator/ (const vec3& v, double f) { return vec3(v.e[0] / f, v.e[1] / f, v.e[2] / f); }

inline double dot(const vec3& v1, const vec3& v2) { return (v1.e[0] * v2.e[0] + v1.e[1] * v2.e[1] + v1.e[2] * v2.e[2]); }

inline vec3 cross(const vec3& v1, const vec3& v2) { return vec3(v1.e[1] * v2.e[2] - v1.e[2] * v2.e[1], v1.e[2] * v2.e[0] - v1.e[0] * v2.e[2], v1.e[0] * v2.e[1] - v1.e[1] * v2.e[0]); }

inline vec3 normalize(const vec3& v) { return v / v.length(); }

//实现在单位圆盘内部随机向量
inline vec3 random_in_unit_disk() {
    while (true) {
        vec3 v = vec3(random_double(-1.0, 1.0), random_double(-1.0, 1.0), 0.0);
        if (v.length_squared() < 1) return v;
    }
}

//实现在单位球内部随机向量，单位化，结合法线方向随机生成正确半球内的随机向量
inline vec3 random_in_unit_sphere() {
    while (true) {
        vec3 v = vec3::random(-1.0, 1.0);
        if (v.length_squared() < 1) return v;
    }
}

inline vec3 random_unit_vector() {
    return normalize(random_in_unit_sphere());
}

inline vec3 random_on_hemisphere(const vec3& normal) {
    vec3 v = random_unit_vector();
    return (dot(v, normal) > 0) ? v : -v;
}
//实现光的反射和折射计算，两种计算中假设均是单位化的
inline vec3 reflect(const vec3& v, const vec3& n) {
    return v - 2 * dot(v, n) * n;
}

inline vec3 refract(const vec3& v, const vec3& n, double eta_over_etap) {
    double cos_ans = fmin(1.0, -dot(v, n));
    vec3 refract_orth = eta_over_etap * (v + cos_ans * n);
    vec3 refract_para = -sqrt(fabs(1 - refract_orth.length_squared())) * n;
    return refract_orth + refract_para;
}

#endif