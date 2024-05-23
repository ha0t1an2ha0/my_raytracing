#ifndef HITTABLE_H
#define HITTABLE_H

#include "rtweekend.h"
#include "aabb.h"
//实现hit_record类以及hittable虚拟基类,新增了材质类

class material;

//需要存储交点，法线方向，交点的根t
class hit_record {
public:
    point3 p;
    vec3 normal;
    shared_ptr<material> mat;
    double t;
    //纹理坐标
    double u;
    double v;
    //额外需要一个布尔量记录正反面，一个成员函数根据光线在内部或者外部决定法向量朝内还是外。假设外部计算的outside_normal都是单位化的
    bool front_face;

    void set_face_normal(const ray& r, const vec3& outward_normal) {
        front_face = (dot(r.direction(), outward_normal) < 0);
        normal = (front_face ? outward_normal : -outward_normal);
    }
};

//需要虚拟的析构函数，虚拟的求解相交的hit函数
class hittable {
public:
    virtual bool hit(const ray& r, interval ray_t, hit_record& rec) const = 0;
    virtual ~hittable() = default;
    virtual aabb bounding_box() const = 0;
};

//实现物体的平移，用坐标变换实现，先将光线从世界坐标变到物体坐标，再将交点变回世界坐标
class translate : public hittable {
private:
    shared_ptr<hittable> object;
    vec3 offset;
    aabb bbox;

public:
    translate(shared_ptr<hittable> o, const vec3& off) :object{ o }, offset{ off } {
        bbox = o->bounding_box() + offset;
    }

    bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
        ray r_offset{ r.origin() - offset,r.direction(),r.time() };
        if (!object->hit(r_offset, ray_t, rec)) {
            return false;
        }
        rec.p += offset;
        return true;
    }

    aabb bounding_box() const override {
        return bbox;
    }
};

class rotate_y : public hittable {
private:
    shared_ptr<hittable> object;
    double sin_theta;
    double cos_theta;
    aabb bbox;

public:
    rotate_y(shared_ptr<hittable> o, double angle) :object{ o } {
        double radians = degree_to_radian(angle);
        sin_theta = sin(radians);
        cos_theta = cos(radians);
        bbox = object->bounding_box();
        point3 min(infinity, infinity, infinity);
        point3 max(-infinity, -infinity, -infinity);

        for (int i = 0; i < 2; i++) {
            for (int j = 0; j < 2; j++) {
                for (int k = 0; k < 2; k++) {
                    auto x = i * bbox.x.max + (1 - i) * bbox.x.min;
                    auto y = j * bbox.y.max + (1 - j) * bbox.y.min;
                    auto z = k * bbox.z.max + (1 - k) * bbox.z.min;

                    auto newx = cos_theta * x + sin_theta * z;
                    auto newz = -sin_theta * x + cos_theta * z;

                    vec3 tester(newx, y, newz);

                    for (int c = 0; c < 3; c++) {
                        min[c] = fmin(min[c], tester[c]);
                        max[c] = fmax(max[c], tester[c]);
                    }
                }
            }
        }

        bbox = aabb(min, max);
    }

    bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
        // Change the ray from world space to object space
        auto origin = r.origin();
        auto direction = r.direction();

        origin[0] = cos_theta * r.origin()[0] - sin_theta * r.origin()[2];
        origin[2] = sin_theta * r.origin()[0] + cos_theta * r.origin()[2];

        direction[0] = cos_theta * r.direction()[0] - sin_theta * r.direction()[2];
        direction[2] = sin_theta * r.direction()[0] + cos_theta * r.direction()[2];

        ray rotated_r(origin, direction, r.time());

        // Determine whether an intersection exists in object space (and if so, where)
        if (!object->hit(rotated_r, ray_t, rec))
            return false;

        // Change the intersection point from object space to world space
        auto p = rec.p;
        p[0] = cos_theta * rec.p[0] + sin_theta * rec.p[2];
        p[2] = -sin_theta * rec.p[0] + cos_theta * rec.p[2];

        // Change the normal from object space to world space
        auto normal = rec.normal;
        normal[0] = cos_theta * rec.normal[0] + sin_theta * rec.normal[2];
        normal[2] = -sin_theta * rec.normal[0] + cos_theta * rec.normal[2];

        rec.p = p;
        rec.normal = normal;

        return true;
    }

    aabb bounding_box() const override {
        return bbox;
    }
};

#endif