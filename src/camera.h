#ifndef CAMERA_H
#define CAMERA_H

#include "rtweekend.h"
#include "hittable.h"
#include "material.h"
#include <iomanip>
#include <sstream>

//实现了相机类，公共接口包括render函数供main调用，render函数首先运行initialize()初始化参数。ray_color着色也在private部分

class camera
{
public:
    //默认宽高比和默认图片宽度，默认像素采样数量，默认最大递归深度
    double aspect_ratio{ 1.0 };
    int    image_width{ 100 };
    int    samples_per_pixel{ 10 };
    int    depth_max{ 10 };
    color  background{ 0.0,0.0,0.0 };

    //使用垂直视角+焦距定义视口垂直范围，默认90度
    double vfov{ 90 };

    //默认焦距，默认光圈幅角
    double focus_dist{ 10 };
    double defocus_degree{ 0 };

    //默认相机位置
    point3 lookfrom{ 0.0,0.0,0.0 };
    point3 lookat{ 0.0,0.0,-1.0 };
    vec3 vup{ 0.0,1.0,0.0 };

    /* void render(const hittable& world) {
        //初始化相机参数
        initialize();
        //渲染
        std::cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";
        for (int j{ 0 }; j < image_height; ++j) {
            std::clog << "\rScanlines remaining: " << (image_height - j) << ' ' << std::flush;
            for (int i{ 0 }; i < image_width; ++i) {
                color pixel_color{ 0.0,0.0,0.0 };
                for (int sample{ 0 }; sample < samples_per_pixel; ++sample) {
                    ray r = get_ray(i, j);
                    pixel_color += ray_color(r, depth_max, world);
                }
                pixel_color *= pixel_samples_scale;
                writecolor(std::cout, pixel_color);
            }
        }
        std::clog << "\rDone.                 \n";
    } */

    void render(const hittable& world, const hittable& lights) {
        // 初始化相机参数
        initialize();

        // 记录渲染开始的时间点
        auto start_time = std::chrono::steady_clock::now();

        // 安全地更新完成的像素数
        std::atomic<int> pixels_done(0);
        const int total_pixels = image_width * image_height;
        const int bar_width = 70; // 进度条的宽度    

        // 创建进度监视器线程
        std::thread progress_monitor([&]() {
            std::string spinner = "/-\\|";
            size_t spin_idx = 0;
            while (pixels_done < total_pixels) {
                // 计算已经持续的时间
                auto current_time = std::chrono::steady_clock::now();
                auto elapsed_seconds = std::chrono::duration_cast<std::chrono::seconds>(current_time - start_time).count();

                // 计算渲染的平均速度
                float average_speed = static_cast<float>(pixels_done) / elapsed_seconds;

                // 计算剩余工作量所需的时间
                float remaining_seconds = (total_pixels - pixels_done) / average_speed;
                int remaining_minutes = static_cast<int>(remaining_seconds) / 60;
                int remaining_seconds_remaining = std::max(0, static_cast<int>(remaining_seconds) % 60);

                // 更新进度条和显示估计的剩余时间
                float progress = static_cast<float>(pixels_done) / total_pixels;
                int pos = bar_width * progress;
                std::ostringstream eta_stream;
                eta_stream << std::setw(3) << remaining_minutes << " min " << std::setw(3) << remaining_seconds_remaining << " s";
                std::string eta_str = eta_stream.str();
                // 确保ETA字符串长度为固定长度
                eta_str.resize(14, ' ');
                std::clog << "[";
                for (int i = 0; i < bar_width; ++i) {
                    if (i < pos) std::clog << "=";
                    else if (i == pos) std::clog << ">";
                    else std::clog << " ";
                }
                std::clog << "] " << int(progress * 100.0) << "% " << spinner[spin_idx++ % spinner.size()] << " ETA: " << eta_str << "\r";
                std::clog.flush();

                std::this_thread::sleep_for(std::chrono::milliseconds(100)); // 更新频率更快，以便观察到旋转效果
            }
            });
        std::cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";

        vector<color> framebuffer(image_height * image_width);

#pragma omp parallel for schedule(dynamic, 1) collapse(2)
        for (int j = 0; j < image_height; ++j) {
            for (int i = 0; i < image_width; ++i) {
                color pixel_color{ 0.0,0.0,0.0 };
                for (int s_i = 0; s_i < sqrt_spp; ++s_i) {
                    for (int s_j = 0; s_j < sqrt_spp; ++s_j) {
                        ray r = get_ray(i, j, s_i, s_j);
                        pixel_color += ray_color(r, depth_max, world, lights);
                    }
                }
                pixel_color *= pixel_samples_scale;
                framebuffer[j * image_width + i] = pixel_color;
                ++pixels_done;
            }
        }

        progress_monitor.join();
        for (const color& pixel_color : framebuffer) {
            writecolor(std::cout, pixel_color);
        }
        std::clog << "\nDone.                 \n";
    }

private:
    int    image_height;        //图片高度
    point3 center;              //相机坐标
    point3 pixel00_loc;         //左上像素中心点坐标
    vec3   pixel_delta_u;       //图片向右一个像素对应向量
    vec3   pixel_delta_v;       //图片向下一个像素对应向量
    double pixel_samples_scale; //像素采样系数
    int sqrt_spp;               //像素采样数平方根
    double recip_sqrt_spp;      //上个变量的倒数
    //相机坐标系，v是worldup或vup在视口平面的投影，-w是相机指向方向，u是视口向右方向
    vec3 u, v, w;
    //光圈在u，v方向的向量长度
    vec3 defocus_disk_u, defocus_disk_v;

    void initialize() {
        //图片参数
        image_height = int(image_width / aspect_ratio);
        image_height = (1 > image_height) ? 1 : image_height;

        //像素采样系数
        pixel_samples_scale = 1.0 / samples_per_pixel;
        sqrt_spp = int(sqrt(samples_per_pixel));
        recip_sqrt_spp = 1.0 / sqrt_spp;

        //定义相机位置
        center = lookfrom;

        vec3 look_direction = lookat - lookfrom;

        //视口参数（坐标系概念下）,改用垂直视角定义高度
        double theta = degree_to_radian(vfov);
        double viewport_height = 2.0 * tan(theta / 2.0) * focus_dist;
        double viewport_width = viewport_height * (double(image_width) / image_height);

        //计算u,v,w
        w = -normalize(look_direction);
        u = normalize(cross(vup, w));
        v = cross(w, u);

        //计算光圈
        double defocus_radius = tan(degree_to_radian(defocus_degree / 2.0)) * focus_dist;
        defocus_disk_u = u * defocus_radius;
        defocus_disk_v = v * defocus_radius;

        //视口上向右，向下总向量，根据uvw
        vec3 viewport_u = viewport_width * u;
        vec3 viewport_v = viewport_height * -v;

        //计算逐像素向量
        pixel_delta_u = viewport_u / image_width;
        pixel_delta_v = viewport_v / image_height;

        //计算左上像素中心点位置，根据uvw
        point3 viewport_left_up_loc = center - (focus_dist * w) - viewport_u / 2 - viewport_v / 2;
        pixel00_loc = viewport_left_up_loc + 0.5 * (pixel_delta_u + pixel_delta_v);
    }
    //像素点上确定采样光线
    ray get_ray(int i, int j, int s_i, int s_j) const {
        vec3 offset = sample_square_layer(s_i, s_j);
        point3 pixel_sample{ pixel00_loc
                            + (i + offset.x()) * pixel_delta_u
                            + (j + offset.y()) * pixel_delta_v };
        point3 ray_start;
        ray_start = (defocus_degree <= 0.0) ? center : get_ray_start();
        vec3 ray_direction = pixel_sample - ray_start;
        double ray_time = random_double();
        return ray(ray_start, ray_direction, ray_time);
    }
    //正方形采样规则
    vec3 sample_square() const {
        return vec3(random_double() - 0.5, random_double() - 0.5, 0.0);
    }

    //分层采样
    vec3 sample_square_layer(int s_i, int s_j) const {
        double s_x = (s_i + random_double()) * recip_sqrt_spp - 0.5;
        double s_y = (s_j + random_double()) * recip_sqrt_spp - 0.5;
        return vec3(s_x, s_y, 0.0);
    }

    //光圈采样光线起点
    point3 get_ray_start() const {
        vec3 offset = random_in_unit_disk();
        return center + (offset.x() * defocus_disk_u) + (offset.y() * defocus_disk_v);
    }

    //着色
    //规定了递归深度,忽略精度误差导致的过近的交点
    color ray_color(const ray& r, int depth, const hittable& world, const hittable& lights) const {
        if (depth <= 0)return color(0.0, 0.0, 0.0);
        hit_record rec{};
        //光线不与任何物体有交点，返回背景色
        if (!world.hit(r, interval(0.001, infinity), rec)) {
            return background;
        }

        scatter_record srec;

        color color_from_emission = rec.mat->emitted(r, rec, rec.u, rec.v, rec.p);
        //光线不产生反射光，说明射入光源，返回光源照亮
        if (!rec.mat->scatter(r, rec, srec)) {
            return color_from_emission;
        }

        if (srec.skip_pdf) {
            return srec.attenuation * ray_color(srec.skip_pdf_ray, depth - 1, world, lights);
        }

        auto light_ptr = make_shared<hittable_pdf>(lights, rec.p);
        mixture_pdf p(light_ptr, srec.pdf_ptr);

        ray scattered = ray(rec.p, p.generate(), r.time());
        auto pdf_val = p.value(scattered.direction());

        double scattering_pdf = rec.mat->scattering_pdf(r, rec, scattered);
        //double pdf = scattering_pdf;

        color color_from_scatter = (srec.attenuation * scattering_pdf * ray_color(scattered, depth - 1, world, lights)) / pdf_val;
        return color_from_emission + color_from_scatter;
    }
    /*     color ray_color(const ray& r, int depth, const hittable& world) const {
            // If we've exceeded the ray bounce limit, no more light is gathered.
            if (depth <= 0)
                return color(0,0,0);

            hit_record rec;

            // If the ray hits nothing, return the background color.
            if (!world.hit(r, interval(0.001, infinity), rec))
                return background;

            ray scattered;
            color attenuation;
            double pdf;
            color color_from_emission = rec.mat->emitted(r, rec, rec.u, rec.v, rec.p);

            if (!rec.mat->scatter(r, rec, attenuation, scattered, pdf))
                return color_from_emission;

            auto on_light = point3(random_double(213,343), 554, random_double(227,332));
            auto to_light = on_light - rec.p;
            auto distance_squared = to_light.length_squared();
            to_light = normalize(to_light);

            if (dot(to_light, rec.normal) < 0)
                return color_from_emission;

            double light_area = (343-213)*(332-227);
            auto light_cosine = fabs(to_light.y());
            if (light_cosine < 0.000001)
                return color_from_emission;

            pdf = distance_squared / (light_cosine * light_area);
            scattered = ray(rec.p, to_light, r.time());

            double scattering_pdf = rec.mat->scattering_pdf(r, rec, scattered);

            color color_from_scatter =
                (attenuation * scattering_pdf * ray_color(scattered, depth-1, world)) / pdf;

            return color_from_emission + color_from_scatter;
        } */
};

#endif