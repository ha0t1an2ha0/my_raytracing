#ifndef TEXTURE_H
#define TEXTURE_H

#include "rtweekend.h"
#include "rtw_stb_image.h"
#include "perlin.h"
//实现texture虚拟类，以及solid_color类
class texture
{
public:

    virtual color value(double u, double v, const point3& p) const = 0;
    virtual ~texture() = default;
};

class solid_color : public texture {
private:
    color albedo;

public:
    solid_color(const color& c) :albedo{ c } {};
    solid_color(double red, double green, double blue) :albedo{ color(red,green,blue) } {};
    color value(double u, double v, const point3& p) const override {
        return albedo;
    }
};

class checker_texture : public texture {
private:
    double inv_scale;
    shared_ptr<texture> even;
    shared_ptr<texture> odd;

public:
    checker_texture(double scale, shared_ptr<texture> e, shared_ptr<texture> o)
        :inv_scale{ 1.0 / scale }, even{ e }, odd{ o } {};
    checker_texture(double scale, const color& e, const color& o)
        :inv_scale{ 1.0 / scale }, even{ make_shared<solid_color>(e) }, odd{ make_shared<solid_color>(o) } {};
    color value(double u, double v, const point3& p) const override {
        int x_scaled_int = int(std::floor(inv_scale * p.x()));
        int y_scaled_int = int(std::floor(inv_scale * p.y()));
        int z_scaled_int = int(std::floor(inv_scale * p.z()));
        bool is_even = (x_scaled_int + y_scaled_int + z_scaled_int) % 2 == 0;
        return is_even ? even->value(u, v, p) : odd->value(u, v, p);
    }
};

class image_texture : public texture {
private:
    rtw_image image;

public:
    image_texture(const char* filmname) :image{ filmname } {};

    color value(double u, double v, const point3& p) const override {
        if (image.height() <= 0) return color(0.0, 1.0, 1.0);
        u = interval(0.0, 1.0).clamp(u);
        v = 1.0 - interval(0.0, 1.0).clamp(v);
        int i = int(u * image.width());
        int j = int(v * image.height());
        auto c = image.pixel_data(i, j);
        double scale = 1.0 / 255.0;
        return color(c[0] * scale, c[1] * scale, c[2] * scale);
    }
};

class noise_texture : public texture {
private:
    perlin noise;
    double scale;

public:
    noise_texture() {}
    noise_texture(double s) :scale{ s } {}
    color value(double u, double v, const point3& p) const override {
        return color(.5, .5, .5) * (1 + sin(scale * p.z() + 10 * noise.turb(p, 7)));
    }
};

#endif