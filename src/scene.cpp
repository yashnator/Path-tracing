#include "scene.hpp"

//Camera functions
Camera::Camera(float fov, float width, float height) : fov(fov), width(width), height(height){};
//Default constructor
Camera::Camera(): fov(60), width(800), height(600){};

Ray Camera::make_ray(float x, float y) const {
    float pi = glm::pi<float>();
    float aspect_ratio = float(width) / height;   // Correct aspect ratio
    float scale = tan(fov * 0.5f * pi / 180.0f); // Convert FOV to radians and compute scaling
    // Compute ray direction in camera space
    glm::vec3 ray_dir(
        x * aspect_ratio * scale,  // Adjust x for aspect ratio
        y * scale,                 // Adjust y for FOV scaling
        -1                         // Always points into the scene (negative z-direction)
    );

    return Ray(glm::vec3(0, 0, 0), glm::normalize(ray_dir));
    // return Ray(glm::vec3(0,0,0), glm::vec3(x, y, -1));
}

//Scene functions
color Scene::getColor(Ray ray) const
{
    HitRecord rec = HitRecord();
    Interval t_range = Interval(0.001f, std::numeric_limits<float>::max());
    color c = glm::vec3(0);
    int no_of_hits = 0;
    for(auto const &obj:objects)
    {
        if(obj->hit(ray, t_range, rec)) 
        {
            c = (float)0.5 * (rec.n + glm::vec3(1));
            no_of_hits++;
            t_range.max=std::min(t_range.max, rec.t);
            // std::cout<<rec.t<<" ";
        }
        // if(no_of_hits>1) std::cout<<"some error ";
    }
    // if(no_of_hits)std::cout<<std::endl;
    return c;
}

//Object functions
bool Object::hit(Ray ray, Interval t_range, HitRecord &rec) const
{
    return shape->hit(ray, t_range, rec);
}


//HitRecord functions
HitRecord::HitRecord(): t(std::numeric_limits<float>::max()), p(glm::vec3(0)), n(glm::vec3(0)), mat(nullptr) {};


//Hit functions
bool Sphere::hit(Ray ray, Interval t_range, HitRecord &rec) const
{
    float qa = glm::dot(ray.d, ray.d);
    float qb = glm::dot(2.0f * ray.d, ray.o - c);
    float qc = glm::dot(ray.o - c, ray.o - c) - r * r;

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
        rec.n = glm::normalize(rec.p - c);
        // rec.mat = mat;
        t_range.max = t1; // Update the t_range to reflect the hit
        return true;
    }
    if(t_range.contains(t2) && t2>0)
    {
        rec.t = t2;
        rec.p = ray.at(t2);
        rec.n = glm::normalize(rec.p - c);
        // rec.mat = mat;
        t_range.max = t2; // Update the t_range to reflect the hit
        return true;
    }
    else return false;
}

bool Plane::hit(Ray ray, Interval t_range, HitRecord &rec) const 
{
    float dnr = glm::dot(normal, ray.d);
    if(dnr == 0) 
        return false;
    float nr = glm::dot(normal, point - ray.o);
    float t = nr / dnr;
    if(t < 0 or (not t_range.contains(t))) 
        return false;
    rec.t = t;
    rec.p = ray.at(t);
    rec.n = glm::normalize(normal);
    t_range.max = t;
    return true;
}

bool Box::hit(Ray ray, Interval t_range, HitRecord &rec) const 
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
    std::cout << "Found a collision at " << tmin << " " << tmax << std::endl;
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
