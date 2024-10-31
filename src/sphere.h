#ifndef SPHERE_H
#define SPHERE_H

#include "rtweekend.h"
#include "hittable.h"
//实现了sphere类继承自hittalbe，需要完善构造函数和求交函数。
class sphere : public hittable
{
private:
    point3 center1;
    vec3 center_vec{ 0.0,0.0,0.0 };
    double radius;
    shared_ptr<material> mat;
    bool is_moving;
    aabb bbox;

    point3 sphere_center(double time) const {
        return center1 + time * center_vec;
    }
    //通过单位球上三维左边p，转化为uv坐标
/*     static void get_sphere_uv(const point3& p, double& u, double& v) {
        double phi = atan2(p.y(), p.x()) + pi;
        double theta = acos(p.z());
        u = phi / (2.0 * pi);
        v = theta / pi;
    } */
    static void get_sphere_uv(const point3& p, double& u, double& v) {
        // p: a given point on the sphere of radius one, centered at the origin.
        // u: returned value [0,1] of angle around the Y axis from X=-1.
        // v: returned value [0,1] of angle from Y=-1 to Y=+1.
        //     <1 0 0> yields <0.50 0.50>       <-1  0  0> yields <0.00 0.50>
        //     <0 1 0> yields <0.50 1.00>       < 0 -1  0> yields <0.50 0.00>
        //     <0 0 1> yields <0.25 0.50>       < 0  0 -1> yields <0.75 0.50>

        auto theta = acos(-p.y());
        auto phi = atan2(-p.z(), p.x()) + pi;

        u = phi / (2 * pi);
        v = theta / pi;
    }

    static vec3 random_to_sphere(double radius, double distance_squared) {
        auto r1 = random_double();
        auto r2 = random_double();
        auto z = 1 + r2*(sqrt(1-radius*radius/distance_squared) - 1);

        auto phi = 2*pi*r1;
        auto x = cos(phi)*sqrt(1-z*z);
        auto y = sin(phi)*sqrt(1-z*z);

        return vec3(x, y, z);
    }

public:
    //静止球
    sphere(const point3& p, double r, shared_ptr<material> m) :center1{ p }, radius{ fmax(r,0) }, mat{ m }, is_moving{ false } {
        vec3 rvec = vec3(radius, radius, radius);
        bbox = aabb(center1 - rvec, center1 + rvec);
    }
    //运动球
    sphere(const point3& p1, const point3& p2, double r, shared_ptr<material> m) :center1{ p1 }, radius{ fmax(r,0) }, mat{ m }, is_moving{ true } {
        center_vec = p2 - p1;
        vec3 rvec = vec3(radius, radius, radius);
        aabb box1 = aabb(p1 - rvec, p1 + rvec);
        aabb box2 = aabb(p2 - rvec, p2 + rvec);
        aabb bbox = aabb(box1, box2);
    };

    bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
        point3 center = is_moving ? sphere_center(r.time()) : center1;
        vec3 oc = center - r.origin();
        double a = r.direction().length_squared();
        double h = dot(r.direction(), oc);
        double c = oc.length_squared() - radius * radius;
        double discriminant = h * h - a * c;
        if (discriminant < 0) {
            return false;
        }

        double sqrtd = sqrt(discriminant);
        double root = (h - sqrtd) / a;
        if (!ray_t.surrounds(root)) {
            root = (h + sqrtd) / a;
            if (!ray_t.surrounds(root)) {
                return false;
            }
        }

        rec.t = root;
        rec.p = r.at(root);
        vec3 outward_normal = (rec.p - center) / radius;
        rec.set_face_normal(r, outward_normal);
        get_sphere_uv(outward_normal, rec.u, rec.v);
        rec.mat = mat;

        return true;
    }

    aabb bounding_box() const override {
        return bbox;
    }

    double pdf_value(const point3& origin, const vec3& direction) const override {
        // This method only works for stationary spheres.

        hit_record rec;
        if (!this->hit(ray(origin, direction), interval(0.001, infinity), rec))
            return 0;

        auto cos_theta_max = sqrt(1 - radius*radius/(center1 - origin).length_squared());
        auto solid_angle = 2*pi*(1-cos_theta_max);

        return  1 / solid_angle;
    }

    vec3 random(const point3& origin) const override {
        vec3 direction = center1 - origin;
        auto distance_squared = direction.length_squared();
        onb uvw;
        uvw.build_from_w(direction);
        return uvw.local(random_to_sphere(radius, distance_squared));
    }
};

#endif