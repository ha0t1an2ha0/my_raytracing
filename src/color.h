#ifndef COLOR_H
#define COLOR_H

#include "vec3.h"
#include "rtweekend.h"

using color = vec3;

//gamma校正

inline double linear_to_gamma(double linear_component) {
    if (linear_component > 0.0) {
        return sqrt(linear_component);
    }
    return 0.0;
}

void writecolor(std::ostream& out, const color& pixel_color) {
    auto r{ pixel_color.x() };
    auto g{ pixel_color.y() };
    auto b{ pixel_color.z() };

    if (r != r) r = 0.0;
    if (g != g) g = 0.0;
    if (b != b) b = 0.0;

    r = linear_to_gamma(r);
    g = linear_to_gamma(g);
    b = linear_to_gamma(b);

    static const interval intensity{ 0.0, 0.999 };
    int rint = int(255.999 * intensity.clamp(r));
    int gint = int(255.999 * intensity.clamp(g));
    int bint = int(255.999 * intensity.clamp(b));
    out << rint << ' ' << gint << ' ' << bint << '\n';
}

#endif