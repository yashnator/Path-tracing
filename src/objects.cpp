#include "scene.hpp"
//Object functions
void Object::setTransform(glm::mat4 M)
{
    glm::mat4 N = glm::translate(glm::mat4(1.0f), shape->center);
    transform = N * M * glm::inverse(N);
    normalTransform = glm::inverseTranspose(transform);
    inverse = glm::inverse(transform);
}

bool Object::hit(Ray ray, Interval t_range, HitRecord &rec) const
{
    glm::vec3 new_direction = glm::vec3(inverse * glm::vec4(ray.d, 0.0f));
    new_direction = glm::normalize(new_direction);
    Ray transformed_ray = Ray(glm::vec3(inverse * glm::vec4(ray.o, 1.0f)), new_direction);
    // transformed_ray.debugRay();
    if(shape->hit(transformed_ray, t_range, rec, transform, inverse, normalTransform))
    {
        rec.p = glm::vec3(transform * glm::vec4(rec.p, 1.0f));
        rec.n = glm::normalize(glm::vec3(normalTransform * glm::vec4(rec.n, 0.0f)));
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
    glm::vec3 object_space_c = glm::vec3(glm::vec4(this->c, 1.0f));
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
    glm::vec3 tlow = glm::vec3(glm::vec4(low, 1.0f));
    glm::vec3 thi = glm::vec3(glm::vec4(hi, 1.0f));

    // glm::vec4 miz = glm::vec4(0.0, 0.0, tlow.z, 1.0f);
    // glm::vec4 maxz = glm::vec4(0.0, 0.0, thi.z, 1.0f);
    // tlow.z = (itf*miz).z;
    // thi.z = (itf*maxz).z;

    // std::cout<<"tlow: "<<to_string(tlow)<<" thi: "<<to_string(thi)<<std::endl;

    float tminx = ((tlow.x - ray.o.x) / ray.d.x);
    float tmaxx = ((thi.x - ray.o.x) / ray.d.x);
    float tminy = ((tlow.y - ray.o.y) / ray.d.y);
    float tmaxy = ((thi.y - ray.o.y) / ray.d.y);
    float tminz = ((tlow.z - ray.o.z) / ray.d.z);
    float tmaxz = ((thi.z - ray.o.z) / ray.d.z);
    float tmin = std::max(std::min(tminx, tmaxx), std::max(std::min(tminy, tmaxy), std::min(tminz, tmaxz)));
    float tmax = std::min(std::max(tminx, tmaxx), std::min(std::max(tminy, tmaxy), std::max(tminz, tmaxz)));
    if(tmax < 0) 
        {
        // std::cout<<"tmax < 0"<<std::endl;
        return false;
        }
    if(tmin > tmax) {
        // std::cout<<"tmin > tmax"<<std::endl;
        return false;
    }
    if(tmin < 0 or !t_range.contains(tmin)) {
        return false;
    }
    rec.t = tmin;
    rec.p = ray.at(tmin);
    rec.n = glm::vec3(0.0);
    if(tmin == std::min(tminx, tmaxx)) {
        if(tminx==tmin) rec.n.x = -1;
        else rec.n.x = 1;
    } else if(tmin == std::min(tminy, tmaxy)) {
        if (tminy==tmin) rec.n.y = -1;
        else rec.n.y = 1;
    } else {
        if(tminz==tmin) rec.n.z = -1;
        else rec.n.z = 1;
    }
    if(glm::dot(rec.n, ray.d) > 0) rec.n = -rec.n;
    t_range.max = tmin;
    return true;
}

bool Rectangle::hit(Ray ray, Interval t_range, HitRecord &rec, glm::mat4 tf, glm::mat4 itf, glm::mat4 ntf) const
{
    if(ray.d.y == 0) return false;
    // std::cout << "Here" << std::endl;
    float t = (low.y - ray.o.y) / ray.d.y;
    glm::vec3 p = ray.at(t);
    if(p.x < std::min(low.x, hi.x) || p.x > std::max(low.x, hi.x) || p.z < std::min(low.z, hi.z) || p.z > std::max(low.z, hi.z)) return false;
    if(not t_range.contains(t)) {
        return false;
    }
    rec.t = t;
    rec.p = ray.at(t);
    rec.n = glm::vec3(0.0f, -1.0f, 0.0f);
    if(glm::dot(rec.n, ray.d) > 0) rec.n = -rec.n;
    return true;
}