#ifndef HITTABLE_LIST_H
#define HITTABLE_LIST_H


#include "rtweekend.h"
#include "hittable.h"
#include "aabb.h"

//实现hittable_list类，利用智能指针和vector实现对每一个hittable物体，进行动态数组管理。实现clear和add功能
class hittable_list :public hittable
{
private:
    aabb bbox;
public:
    vector<shared_ptr<hittable>> objects;

    hittable_list() = default;
    hittable_list(shared_ptr<hittable> object) { add(object); }
    void clear() { objects.clear(); }
    void add(shared_ptr<hittable> object) {
        objects.push_back(object);
        bbox = aabb(bbox, object->bounding_box());
    }
    
    //还需要一个hit成员函数,遍历所有物体，记录最近处交点的hit_record
    bool hit(const ray& r, interval ray_t, hit_record& rec) const override{
        hit_record rec_temp;
        double close_so_far{ ray_t.max };
        bool hit_anything = false;
        for (const shared_ptr<hittable>& object : objects) {
            if (object->hit(r, interval(ray_t.min,close_so_far), rec_temp)) {
                hit_anything = true;
                close_so_far = rec_temp.t;
                rec = rec_temp;
            }
        }        
        return hit_anything;
    }

    aabb bounding_box() const override {
        return bbox;
    }
};

#endif