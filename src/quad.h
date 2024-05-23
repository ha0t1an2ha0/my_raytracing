#ifndef QUAD_H
#define QUAD_H

#include "rtweekend.h"
#include "hittable.h"
#include "hittable_list.h"
//实现了四边形类，包含结构体，材质以及包围盒计算。平行四边形被定义为起点Q以及出发的相邻两边u，v
class quad : public hittable {
private:
    point3 Q;
    vec3 u;
    vec3 v;
    shared_ptr<material> mat;
    aabb bbox;
    vec3 normal;//四边形所在平面的单位法向量
    double D;//所在平面的隐式公式Ax+By+Cz=D中的D
    vec3 w;//求交计算，需要首先计算光线与所在平面的交，然后求出交点在向量uv坐标下的坐标值，判断是否落在[0,1]^2来决定是否与四边形相交，w用来方便计算坐标值

public:
    quad(point3 q, vec3 u, vec3 v, shared_ptr<material> mat) :Q{ q }, u{ u }, v{ v }, mat{ mat } {
        set_bounding_box();
        vec3 n = cross(u, v);
        normal = normalize(n);
        D = dot(normal, Q);
        w = n / dot(n, n);
    }

    virtual void set_bounding_box() {
        aabb box1 = aabb(Q, Q + u + v);
        aabb box2 = aabb(Q + u, Q + v);
        bbox = aabb(box1, box2);
    }

    aabb bounding_box() const override {
        return bbox;
    }

    bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
        double n_d = dot(normal, r.direction());
        if (fabs(n_d) < 1e-8)return false;
        double t = (D - dot(normal, r.origin())) / n_d;
        if (!ray_t.contains(t)) return false;

        point3 intersection = r.at(t);
        vec3 p = intersection - Q;
        //假设所在平面上的交点，在uv坐标系下坐标为(alpha,beta)
        double alpha = dot(w, cross(p, v));
        double beta = dot(w, cross(u, p));

        if (!is_interior(alpha, beta, rec))return false;

        rec.set_face_normal(r, normal);
        rec.mat = mat;
        rec.t = t;
        rec.p = intersection;

        return true;
    }

    virtual bool is_interior(double a, double b, hit_record& rec) const {
        interval unit_interval(0.0, 1.0);
        if ((!unit_interval.contains(a)) || (!unit_interval.contains(b))) {
            return false;
        }
        rec.u = a;
        rec.v = b;
        return true;
    }

};

inline shared_ptr<hittable_list> box(const point3& a, const point3& b, shared_ptr<material> mat) {
    auto slides = make_shared<hittable_list>();

    point3 min{ fmin(a.x(),b.x()),fmin(a.y(),b.y()),fmin(a.z(),b.z()) };
    point3 max{ fmax(a.x(),b.x()),fmax(a.y(),b.y()),fmax(a.z(),b.z()) };

    vec3 dx{ max.x() - min.x(),0.0,0.0 };
    vec3 dy{ 0.0,max.y() - min.y(),0.0 };
    vec3 dz{ 0.0,0.0,max.z() - min.z() };

    slides->add(make_shared<quad>(max, -dx, -dy, mat));//正面
    slides->add(make_shared<quad>(max, -dz, -dx, mat));//上面
    slides->add(make_shared<quad>(max, -dy, -dz, mat));//右面
    slides->add(make_shared<quad>(min, dy, dx, mat));//背面
    slides->add(make_shared<quad>(min, dx, dz, mat));//下面
    slides->add(make_shared<quad>(min, dz, dy, mat));//左面

    return slides;
}

#endif