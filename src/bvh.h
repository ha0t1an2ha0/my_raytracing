#ifndef BVH_H
#define BVH_H

#include "rtweekend.h"
#include "hittable.h"
#include "hittable_list.h"
#include "aabb.h"
#include <algorithm>

//实现了bvh类树结构，字段包括左右子节点和bbox。成员函数有构造函数和hit函数
class bvh_node : public hittable
{
private:
    shared_ptr<hittable> left;
    shared_ptr<hittable> right;
    aabb bbox;

    static bool compare_x(const shared_ptr<hittable> a, const shared_ptr<hittable> b) {
        return a->bounding_box().x.min < b->bounding_box().x.min;
    }

    static bool compare_y(const shared_ptr<hittable> a, const shared_ptr<hittable> b) {
        return a->bounding_box().y.min < b->bounding_box().y.min;
    }

    static bool compare_z(const shared_ptr<hittable> a, const shared_ptr<hittable> b) {
        return a->bounding_box().z.min < b->bounding_box().z.min;
    }

public:
    bvh_node(hittable_list& list) :bvh_node(list.objects, 0, list.objects.size()) {}
    bvh_node(vector<shared_ptr<hittable>>& objects, size_t start, size_t end) {
        bbox = aabb::empty;
        for (size_t object_index = start; object_index < end; ++object_index) {
            bbox = aabb(bbox, objects[object_index]->bounding_box());
        }
        int random_com = bbox.longest_axis();

        auto comparator = random_com == 0 ? compare_x
                        : random_com == 1 ? compare_y
                                          : compare_z;
        size_t slide = end - start;
        if (slide == 1) {
            left = right = objects[start];
        }
        else if (slide == 2)
        {
            left = objects[start];
            right = objects[start+1];
        }
        else {
            std::sort(objects.begin() + start, objects.begin() + end, comparator);
            size_t mid = start + slide / 2;
            left = make_shared<bvh_node>(objects, start, mid);
            right = make_shared<bvh_node>(objects, mid, end);
        }        

    }

    bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
        if (!bbox.hit(r, ray_t)) return false;
        bool hit_left = left->hit(r, ray_t, rec);
        //如果hit_left为真，那么左边递归结果为真说明已经和一个hittable相交，那么被这个物体遮挡的后续光线不需要考虑，因此可以缩短ray_t.max
        bool hit_right = right->hit(r, interval(ray_t.min, hit_left ? rec.t : ray_t.max), rec);
        return hit_left || hit_right;
    }
    aabb bounding_box() const override {
        return bbox;
    }
};

#endif

