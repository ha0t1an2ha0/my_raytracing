#ifndef MATERIAL_H
#define MATERIAL_H

#include "rtweekend.h"
#include "texture.h"

class hit_record;

//实现了material虚拟类，需要一个虚拟析构函数和scatter函数，增加光源材质后需要加入emitted函数表示发射光线，默认为黑色
class material
{
public:
    virtual bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered) const {
        return false;
    }
    virtual color emitted(double u, double v, const point3& p) const {
        return color(0.0, 0.0, 0.0);
    }
    virtual ~material() = default;
};

//实现郎伯(lambertian)材质类，字段包括反射率albedo
class lambertian : public material {
private:
    shared_ptr<texture> tex;

public:
    lambertian(const color& al) :tex{ make_shared<solid_color>(al) } {}
    lambertian(shared_ptr<texture> t) :tex{ t } {}
    bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered) const override {
        vec3 scattered_direction = rec.normal + random_unit_vector();
        if (scattered_direction.near_zero()) {
            scattered_direction = rec.normal;
        }
        scattered = ray(rec.p, scattered_direction,r_in.time());
        attenuation = tex->value(rec.u, rec.v, rec.p);
        return true;
    }
};

//实现了金属类，字段fuzz实现了反射的粗糙感
class metal : public material {
private:
    color albedo;
    double fuzz;

public:
    metal(const color& al, double f) :albedo{ al }, fuzz{ f < 1 ? f : 1 } {}
    bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered) const override {
        vec3 reflected = normalize(reflect(r_in.direction(), rec.normal)) + fuzz * random_unit_vector();
        scattered = ray(rec.p, reflected,r_in.time());
        attenuation = albedo;
        return (dot(reflected, rec.normal) > 0);
    }
};

//实现了电介质类，字段包括折射率
class dielectric : public material {
private:
    double refraction_index;
    static double schilick_reflectance(double refraction_index, double cos_theta) {
        double r0 = (refraction_index - 1.0) / (refraction_index + 1.0);
        r0 = r0 * r0;
        return r0 + (1.0 - r0) * pow(1 - cos_theta, 5);
    }

public:
    dielectric(double re) :refraction_index{ re } {}
    bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered) const override {
        attenuation = color(1.0, 1.0, 1.0);
        //折射率是真空光速与该材质光速的比，是大于1的常数。如果光从外部(默认空气)射入，折射率比应为1/index
        double ri = rec.front_face ? (1.0 / refraction_index) : refraction_index;
        vec3 r_in_direction = normalize(r_in.direction());
        double cos_theta = fmin(-dot(r_in_direction, rec.normal), 1);
        double sin_theta = sqrt(1.0 - cos_theta * cos_theta);

        bool cannot_refract = (ri * sin_theta > 1.0);
        if (cannot_refract || schilick_reflectance(ri, cos_theta) > random_double()) {
            vec3 scattered_direction = reflect(r_in_direction, rec.normal);
            scattered = ray(rec.p, scattered_direction,r_in.time());
        }
        else {
            vec3 scattered_direction = refract(r_in_direction, rec.normal, ri);
            scattered = ray(rec.p, scattered_direction,r_in.time());
        }        
        return true;
    }
};

class diffuse_light : public material {
private:
    shared_ptr<texture> tex;

public:
    diffuse_light(shared_ptr<texture> t) :tex{ t } {}
    diffuse_light(const color& c) :tex{ make_shared<solid_color>(c) } {}

    color emitted(double u, double v, const point3& p) const override {
        return tex->value(u, v, p);
    }
};

class isotropic : public material {
  public:
    isotropic(const color& albedo) : tex(make_shared<solid_color>(albedo)) {}
    isotropic(shared_ptr<texture> tex) : tex(tex) {}

    bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered)
    const override {
        scattered = ray(rec.p, random_unit_vector(), r_in.time());
        attenuation = tex->value(rec.u, rec.v, rec.p);
        return true;
    }

  private:
    shared_ptr<texture> tex;
};

#endif