#ifndef RTWEEKEND_H
#define RTWEEKEND_H
//通用头文件，保存using缩写，常量，角度/弧度转化函数，自订头文件等

#include <cmath>
#include <limits>
#include <memory>
#include <cstdlib>
#include <iostream>
#include <vector>
#include <random>
#include <omp.h>
#include <sstream>
#include <chrono>
#include <atomic>
#include <thread>

using std::fabs;
using std::sqrt;
using std::make_shared;
using std::shared_ptr;
using std::vector;

const double infinity{ std::numeric_limits<double>::infinity() };
const double pi{ 3.1415926535897932385 };

inline double degree_to_radian(double degree) {
    return degree / 180.0 * pi;
}

/* inline double random_double() {
    return rand() / (RAND_MAX + 1.0);
} */

inline double random_double() {
    static std::uniform_real_distribution<double> dis(0.0, 1.0);
    static std::mt19937 generator;
    return dis(generator);
}

inline double random_double(double min, double max) {
    return min + (max - min) * random_double();
}

inline int random_int(int min, int max) {
    return int(random_double(min, max + 1.0));
}

#include "ray.h"
#include "vec3.h"
#include "interval.h"
#include "color.h"
#include "onb.h"
#include "pdf.h"

#endif