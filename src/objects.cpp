#include "scene.hpp"
//Object functions
bool Object::hit(Ray ray, Interval t_range, HitRecord &rec) const
{
    glm::vec3 new_direction = glm::vec3(inverse * glm::vec4(ray.d, 0.0f));
    new_direction = glm::normalize(new_direction);
    Ray transformed_ray = Ray(glm::vec3(inverse * glm::vec4(ray.o, 1.0f)), new_direction);
    // transformed_ray.debugRay();
    if(shape->hit(transformed_ray, t_range, rec, transform, inverse, normalTransform))
    {
        // std::cout<<"hit received"<<std::endl;
        // std::cout<<"at: "<<glm::to_string(rec.p)<<std::endl;
        rec.p = glm::vec3(transform * glm::vec4(rec.p, 1.0f));
        rec.n = glm::normalize(glm::vec3(normalTransform * glm::vec4(rec.n, 0.0f)));
        // std::cout<<glm::to_string(rec.n)<<std::endl;
        rec.mat=mat;
        return true;
    }
    else return false;
}

void Object::debugTransform()
{
    std::cout<<"Transform: "<<glm::to_string(transform)<<std::endl;
    std::cout<<"Normal Transform: "<<glm::to_string(normalTransform)<<std::endl;
    std::cout<<"Inverse: "<<glm::to_string(inverse)<<std::endl;
}


//HitRecord functions
HitRecord::HitRecord(): t(std::numeric_limits<float>::max()), p(glm::vec3(0)), n(glm::vec3(0)), mat(nullptr) {};

//Hit functions
bool Sphere::hit(Ray ray, Interval t_range, HitRecord &rec, glm::mat4 tf, glm::mat4 itf, glm::mat4 ntf) const
{
    // std::cout<<to_string(ray.o)<<" "<<to_string(ray.d)<<std::endl;
    glm::vec3 object_space_c = glm::vec3(itf * glm::vec4(this->c, 1.0f));
    float qa = glm::dot(ray.d, ray.d);
    float qb = glm::dot(2.0f * ray.d, ray.o - object_space_c);
    float qc = glm::dot(ray.o - object_space_c, ray.o - object_space_c) - r * r;

    float discriminant = qb * qb - 4 * qa * qc;
    if (discriminant < 0)
        return false;
    
    float t1 = (-qb - sqrt(discriminant)) / (2 * qa);
    float t2 = (-qb + sqrt(discriminant)) / (2 * qa);

    if(t1>t2) std::swap(t1, t2);
    if(t_range.contains(t1) && t1>0)
    {
        rec.t = t1;
        rec.p = ray.at(t1);
        rec.n = glm::normalize(rec.p - object_space_c);
        t_range.max = t1; // Update the t_range to reflect the hit
        return true;
    }
    if(t_range.contains(t2) && t2>0)
    {
        rec.t = t2;
        rec.p = ray.at(t2);
        rec.n = glm::normalize(rec.p - object_space_c);
        // rec.mat = mat;
        float cos_theta = glm::dot(rec.n, ray.o - rec.p);
        t_range.max = t2; // Update the t_range to reflect the hit
        // std::cout<<"hit recv"<<std::endl;
        return true;
    }
    else return false;
}

bool Plane::hit(Ray ray, Interval t_range, HitRecord &rec,glm::mat4 tf, glm::mat4 itf, glm::mat4 ntf) const 
{
    glm::vec3 os_point = glm::vec3(itf * glm::vec4(point, 1.0f));
    float dnr = glm::dot(normal, ray.d);
    if(dnr == 0) 
        return false;
    float nr = glm::dot(normal, os_point - ray.o);
    float t = nr / dnr;
    if(t < 0 or (not t_range.contains(t))) 
        return false;
    rec.t = t;
    rec.p = ray.at(t);
    rec.n = glm::normalize(normal);
    t_range.max = t;
    return true;
}

bool Box::hit(Ray ray, Interval t_range, HitRecord &rec, glm::mat4 tf, glm::mat4 itf, glm::mat4 ntf) const 
{
    float tminx = ray.d.x != 0 ? ((low.x - ray.o.x) / ray.d.x) : std::numeric_limits<float>::min();
    float tmaxx = ray.d.x != 0 ? ((hi.x - ray.o.x) / ray.d.x) : std::numeric_limits<float>::max();
    float tminy = ray.d.y != 0 ? ((low.y - ray.o.y) / ray.d.y) : std::numeric_limits<float>::min();
    float tmaxy = ray.d.y != 0 ? ((hi.y - ray.o.y) / ray.d.y) : std::numeric_limits<float>::max();
    float tminz = ray.d.z != 0 ? ((low.z - ray.o.z) / ray.d.z) : std::numeric_limits<float>::min();
    float tmaxz = ray.d.z != 0 ? ((hi.z - ray.o.z) / ray.d.z) : std::numeric_limits<float>::max();
    // if(tminx > tmaxx) std::swap(tminx, tmaxx);
    // if(tminy > tmaxy) std::swap(tminy, tmaxy);
    // if(tminz > tmaxz) std::swap(tminz, tmaxz);
    float tmin = std::max(tminx, std::max(tminy, tminz));
    float tmax = std::min(tmaxx, std::min(tmaxy, tmaxz));
    if(tmax > 0 and tmin <= tmax) {
        rec.t = tmin;
    // std::cout << "Found a collision at " << tmin << " " << tmax << std::endl;
        rec.p = ray.at(tmin);
        rec.n = glm::vec3(0.0);
        if(tmin == tminx) {
            rec.n.x = tmin >= 0 ? 1 : -1;
        } else if(tmin == tminy) {
            rec.n.y = tmin >= 0 ? 1 : -1;
        } else {
            rec.n.z = tmin >= 0 ? 1 : -1;
        }
        t_range.max = tmin;
        return true;
    }
    return false;
}
