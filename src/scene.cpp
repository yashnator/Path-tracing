#include "scene.hpp"

//Scene functions
bool Scene::inShadow(glm::vec3 p, PointLight light) const
{
    Ray shadow_ray(p, light.location-p);
    HitRecord rec;
    Interval t_range = Interval(0.0f, std::numeric_limits<float>::max());
    float bias = 0.001f, max_hit = 0.0f;
    for(auto const &obj:objects)
    {
        if(obj->hit(shadow_ray, t_range, rec)) max_hit = std::max(max_hit, rec.t);
    }
    return (max_hit > bias) ? true : false;
}

glm::vec3 Scene::irradiance(HitRecord &rec, PointLight light) const
{
    float r_square = glm::dot(light.location - rec.p, light.location - rec.p);
    glm::vec3 l = light.location-rec.p;
    float cos_theta = glm::dot(rec.n, glm::normalize(l));

    if(cos_theta < 0) return glm::vec3(0.0);
    return (light.intensity * cos_theta) / r_square;
}

color Scene::radiance(HitRecord &rec) const
{
    color totalRadiance = glm::vec3(0.0);
    for(auto const &light:lights)
    {
        if(!inShadow(rec.p, light))
        {
            glm::vec3 v = camera->getLocation()-rec.p;
            glm::vec3 l = light.location-rec.p;
            color brdf = rec.mat->brdf(rec, l, v);
            totalRadiance += irradiance(rec, light) * brdf;
        }
    }
    return totalRadiance + ambientLight*rec.mat->ambientColor;
}

//Camera functions
Camera::Camera(float fov, float width, float height) : fov(fov), width(width), height(height)
{
    center = glm::vec3(0.0f);
    view = glm::vec3(0.0f, 0.0f, -1.0f);
    up = glm::vec3(0.0f, 1.0f, 0.0f);
    right = glm::vec3(1.0f,0.0f,0.0f);
}
//Default constructor
Camera::Camera(): fov(60), width(800), height(600)
{
    center = glm::vec3(0.0f);
    view = glm::vec3(0.0f, 0.0f, -1.0f);
    up = glm::vec3(0.0f, 1.0f, 0.0f);
    right = glm::vec3(1.0f,0.0f,0.0f);
}

void Camera::transformCamera(glm::mat4 transform)
{
    center = glm::vec3(transform * glm::vec4(center, 1.0f));
    view = glm::vec3(transform * glm::vec4(view, 0.0f));
    up = glm::vec3(transform * glm::vec4(up, 0.0f));
    right = glm::vec3(transform * glm::vec4(right, 0.0f));
    
    //Normalize
    view = glm::normalize(view);
    up = glm::normalize(up);
    right = glm::normalize(right);
}

Ray Camera::make_ray(float x, float y) const {
    float pi = glm::pi<float>();
    float aspect_ratio = float(width) / height;   // Correct aspect ratio
    float scale = tan(fov * 0.5f * pi / 180.0f); // Convert FOV to radians and compute scaling
    // Compute ray direction in camera space

    glm::vec3 ray_dir = glm::vec3(0.0f);

    ray_dir += right * x * aspect_ratio * scale;    
    ray_dir += up * y * scale;
    ray_dir += view;

    // glm::vec3 ray_dir(
    //     x * aspect_ratio * scale,  // Adjust x for aspect ratio
    //     y * scale,                 // Adjust y for FOV scaling
    //     -1                         // Always points into the scene (negative z-direction)
    // );
    // std::cout<<abs(x)<<" "<<abs(y)<<std::endl;
    // if(std::abs(x)<0.01f && std::abs(y)<0.01f) std::cout<<x<<" "<<y<<" ("<<ray_dir.x<<","<<ray_dir.y<<","<<ray_dir.z<<")"<<std::endl;

    return Ray(center, normalize(ray_dir));
    // return Ray(glm::vec3(0,0,0), glm::vec3(x, y, -1));
}

glm::vec3 Camera::getLocation()
{
    return center;
}

void Camera::debugCamera()
{
    std::cout<<"Camera: "<<std::endl;
    std::cout<<"Center: "<<center.x<<","<<center.y<<","<<center.z<<std::endl;
    std::cout<<"View: "<<view.x<<","<<view.y<<","<<view.z<<std::endl;
    std::cout<<"Up: "<<up.x<<","<<up.y<<","<<up.z<<std::endl;
    std::cout<<"Right: "<<right.x<<","<<right.y<<","<<right.z<<std::endl;
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
            if(!obj->mat) {c = (float)0.5 * (rec.n + glm::vec3(1));}
            else
            {
                c = radiance(rec);
            }
            no_of_hits++;
            t_range.max=std::min(t_range.max, rec.t);
        }
    }
    if(!no_of_hits) c = sky;
    return c;
}

//Object functions
bool Object::hit(Ray ray, Interval t_range, HitRecord &rec) const
{
    glm::vec3 new_direction = glm::vec3(inverse * glm::vec4(ray.d, 0.0f));
    new_direction = glm::normalize(new_direction);
    Ray transformed_ray = Ray(glm::vec3(inverse * glm::vec4(ray.o, 1.0f)), new_direction);
    // transformed_ray.debugRay();
    if(shape->hit(transformed_ray, t_range, rec))
    {
        // std::cout<<"hit received"<<std::endl;
        rec.p = glm::vec3(transform * glm::vec4(rec.p, 1.0f));
        rec.n = glm::vec3(normalTransform * glm::vec4(rec.n, 0.0f));
        // std::cout<<glm::to_string(rec.n)<<std::endl;
        rec.mat=mat;return true;
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


//Material functions
color Lambertian::brdf(const HitRecord &rec, glm::vec3 l, glm::vec3 v) const
{
    return (albedo/glm::pi<float>());
}