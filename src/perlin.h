#ifndef PERLIN_H
#define PERLIN_H

#include "rtweekend.h"
//实现了perlin类，可以形成perlin噪声
class perlin
{
private:

    static const int point_count = 256;
    vec3* randvec;
    int* x_perm;
    int* y_perm;
    int* z_perm;

    static int* get_generated_perm() {
        int* perm = new int[point_count];
        for (int i = 0; i < point_count; ++i) {
            perm[i] = i;
        }
        permute(perm, point_count);
        return perm;
    }

    static void permute(int* perm, int n) {
        for (int i = n - 1; i > 0; --i) {
            int j = random_int(0, i);
            int term = perm[j];
            perm[j] = perm[i];
            perm[i] = term;
        }
    }

    static double perlin_interp(const vec3 c[2][2][2], double u, double v, double w) {
        auto uu = u*u*(3-2*u);
        auto vv = v*v*(3-2*v);
        auto ww = w*w*(3-2*w);
        auto accum = 0.0;

        for (int i=0; i < 2; i++)
            for (int j=0; j < 2; j++)
                for (int k=0; k < 2; k++) {
                    vec3 weight_v(u-i, v-j, w-k);
                    accum += (i*uu + (1-i)*(1-uu))
                           * (j*vv + (1-j)*(1-vv))
                           * (k*ww + (1-k)*(1-ww))
                           * dot(c[i][j][k], weight_v);
                }

        return accum;
    }

public:
    perlin() {
        randvec = new vec3[point_count];
        for (int i = 0; i < point_count; ++i) {
            randvec[i] = normalize(vec3::random(-1.0, 1.0));
        }
        x_perm = get_generated_perm();
        y_perm = get_generated_perm();
        z_perm = get_generated_perm();
    }

    ~perlin() {
        delete[] randvec;
        delete[] x_perm;
        delete[] y_perm;
        delete[] z_perm;
    }

    double noise(const point3& p) const {
        auto u = p.x() - floor(p.x());
        auto v = p.y() - floor(p.y());
        auto w = p.z() - floor(p.z());

        auto i = int(floor(p.x()));
        auto j = int(floor(p.y()));
        auto k = int(floor(p.z()));
        vec3 c[2][2][2];

        for (int di = 0; di < 2; di++)
            for (int dj = 0; dj < 2; dj++)
                for (int dk = 0; dk < 2; dk++)
                    c[di][dj][dk] = randvec[
                        x_perm[(i + di) & 255] ^
                            y_perm[(j + dj) & 255] ^
                            z_perm[(k + dk) & 255]
                    ];

        return perlin_interp(c, u, v, w);
    }
    
    double turb(const point3& p, int depth) const {
        auto accum = 0.0;
        auto temp_p = p;
        auto weight = 1.0;

        for (int i = 0; i < depth; i++) {
            accum += weight * noise(temp_p);
            weight *= 0.5;
            temp_p *= 2;
        }

        return fabs(accum);
    }
};

#endif