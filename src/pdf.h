#ifndef PDF_H
#define PDF_H

#include "rtweekend.h"
#include "onb.h"
#include "hittable_list.h"
//实现了概率密度函数类，需要给出指定方向上的概率密度，以及能够根据概率密度分布生成随机向量。每种概率分布作为一个继承类。
class pdf
{
public:
    virtual double value(const vec3& direction) const = 0;
    virtual vec3 generate() const = 0;
    virtual ~pdf() = default;
};

class sphere_pdf : public pdf {
public:
    sphere_pdf() = default;
    double value(const vec3& direction) const override {
        return 1.0 / (4 * pi);
    }
    vec3 generate() const override {
        return random_unit_vector();
    }
};

class cosine_pdf : public pdf {
private:
    onb uvw;
public:
    cosine_pdf(const vec3& w) {
        uvw.build_from_w(w);
    }

    double value(const vec3& direction) const override {
        double cosine_theta = dot(normalize(direction), uvw.w());
        return fmax(0, cosine_theta / pi);
    }

    vec3 generate() const override {
        return uvw.local(random_cosine_direction());
    }
};

class hittable_pdf : public pdf {
private:
    const hittable& objects;
    point3 origin;

public:
    hittable_pdf(const hittable& ob, const point3& o) :objects{ ob }, origin{ o } {}

    double value(const vec3& direction) const override {
        return objects.pdf_value(origin, direction);
    }

    vec3 generate() const override {
        return objects.random(origin);
    }
};

class mixture_pdf : public pdf {
  public:
    mixture_pdf(shared_ptr<pdf> p0, shared_ptr<pdf> p1) {
        p[0] = p0;
        p[1] = p1;
    }

    double value(const vec3& direction) const override {
        return 0.5 * p[0]->value(direction) + 0.5 *p[1]->value(direction);
    }

    vec3 generate() const override {
        if (random_double() < 0.5)
            return p[0]->generate();
        else
            return p[1]->generate();
    }

  private:
    shared_ptr<pdf> p[2];
};

#endif

