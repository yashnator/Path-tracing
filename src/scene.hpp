#ifndef SCENE_HPP
#define SCENE_HPP

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtx/string_cast.hpp>
#include <random>
#include <vector>
#include <iostream>
#include <algorithm>

using color = glm::vec3;

class Ray;
class Interval;
class Shape;
class HitRecord;
class Object;
class Material;
class PointLight;
class Camera;

bool probability(float p);
glm::vec3 sampleCosineHemisphere(const glm::vec3& normal);
glm::vec3 sampleCosineHemisphereLocal();
glm::vec3 toWorldSpace(const glm::vec3& local, const glm::vec3& normal);
float cosineHemispherePDF(const glm::vec3& normal, const glm::vec3& dir);

class Scene {
public:
    Camera *camera;
    std::vector<Object*> objects;
    std::vector<PointLight> lights;
    color sky = glm::vec3(0.0f);
    color ambientLight = glm::vec3(0.0f);
    color getColor(Ray ray, int depth = 2) const;
    color tracePath(Ray ray, int numberOfSamples, int numberOfBounces) const;
    color computeColor(Ray ray, int numberOfBounces) const;
    bool inShadow(glm::vec3 p, PointLight light) const;
    glm::vec3 irradiance(HitRecord &rec, PointLight light) const;
    color radiance(HitRecord &rec) const;
    color radianceFromEmissive(HitRecord &rec) const;
    std::pair<HitRecord,int> traceRay(Ray ray) const;
};

class Ray {
public:
    glm::vec3 o, d;
    Ray(glm::vec3 origin, glm::vec3 direction):
        o(origin),
        d(direction) {
    }
    glm::vec3 at(float t) const {
        return o + t*d;
    }
    void debugRay() {
        std::cout << "Ray: " << glm::to_string(o) << " " << glm::to_string(d) << std::endl;
    }
};

class Camera {
public:
    float fov;
    float width, height;
    glm::vec3 center, view, up, right;
    Camera();
    Camera(float fov, float width, float height);
    Ray make_ray(float x, float y) const; // screen coordinates in [-1, 1]
    glm::vec3 getLocation();
    void transformCamera(glm::mat4 transform);
    void debugCamera();
};

class Interval {
public:
    float min, max;
    Interval(float min, float max):
        min(min),
        max(max) {
    }
    bool contains(float x) const {
        return min <= x && x <= max;
    }
};

class HitRecord {
public:
    float t;
    glm::vec3 p, n;
    Material *mat;
    HitRecord();
};

class Object {
public:
    Shape *shape;
    Material *mat;
    glm::mat4 transform, normalTransform, inverse;    //Metallic materials
    Object(Shape *shape, Material *mat, glm::mat4 M=glm::mat4(1.0)):
        shape(shape),
        mat(mat) {
            transform = glm::mat4(1.0);
            normalTransform = glm::mat4(1.0);
            inverse = glm::mat4(1.0);
    }
    bool hit(Ray ray, Interval t_range, HitRecord &rec) const;
    void setTransform(glm::mat4 M);
    void debugTransform();
    ~Object() {
    }
};

class Shape {
public:
    bool isRectangle = false;
    glm::vec3 center = glm::vec3(0.0f);
    virtual bool hit(Ray ray, Interval t_range, HitRecord &rec, glm::mat4 tf, glm::mat4 itf, glm::mat4 ntf) const = 0;
};

class Sphere: public Shape {
public:
    glm::vec3 c;
    float r;
    Sphere(glm::vec3 cent, float radius):
        c(cent),
        r(radius) {
            center = cent;
    }
    bool hit(Ray ray, Interval t_range, HitRecord &rec, glm::mat4 tf, glm::mat4 itf, glm::mat4 ntf) const;
};

class Plane: public Shape {
public: 
    glm::vec3 point, normal;
    Plane(glm::vec3 pt, glm::vec3 n): point(pt), normal(n) { center = point; };
    bool hit(Ray ray, Interval t_range, HitRecord &rec, glm::mat4 tf, glm::mat4 itf, glm::mat4 ntf) const override;
};

class Box: public Shape {
public:
    glm::vec3 low, hi;
    Box(glm::vec3 minpt, glm::vec3 maxpt): low(minpt), hi(maxpt) { center = hi+(low-hi)/2.0f; };
    bool hit(Ray ray, Interval t_range, HitRecord &rec, glm::mat4 tf, glm::mat4 itf, glm::mat4 ntf) const override;
};

class Rectangle: public Shape {
public:
    glm::vec3 low, hi;
    Rectangle(glm::vec3 minpt, glm::vec3 maxpt): low(minpt), hi(maxpt) { center = hi+(low-hi)/2.0f; isRectangle=true; };
    bool hit(Ray ray, Interval t_range, HitRecord &rec, glm::mat4 tf, glm::mat4 itf, glm::mat4 ntf) const override;
};

class Material {
public:
    color ambientColor=glm::vec3(0.0f);
    color albedo = glm::vec3(0.0f);
    virtual color emission(const HitRecord &rec, glm::vec3 v) const {
        return glm::vec3(0.0);
    }
    virtual color brdf(const HitRecord &rec, glm::vec3 l, glm::vec3 v) const = 0;
    virtual bool reflection(const HitRecord &rec, glm::vec3 v,
                            glm::vec3 &r, color &kr) const = 0;
};

class Lambertian: public Material {
public:
    Lambertian(color _albedo) { albedo = _albedo; }
    virtual color brdf(const HitRecord &rec, glm::vec3 l, glm::vec3 v) const;
    virtual bool reflection(const HitRecord &rec, glm::vec3 v,
                            glm::vec3 &r, color &kr) const;
};

class Metallic: public Material {
public:
    color parallelReflection; //This is the F_0 value for schlick's approximation
    int shininess = 1;
    Metallic(color parallelReflection, int blinnPhongExponent, color _albedo): parallelReflection(parallelReflection), shininess(blinnPhongExponent) {
        albedo = _albedo;
    }
    virtual color brdf(const HitRecord &rec, glm::vec3 l, glm::vec3 v) const;
    virtual bool reflection(const HitRecord &rec, glm::vec3 v, glm::vec3 &r, color &kr) const;
};

class TorrenceSparrow: public Material {
public: 
    color parallelReflection;
    float roughness;
    color albedo;
    TorrenceSparrow(color parallelReflection, float roughness, color albedo):
        parallelReflection(parallelReflection), roughness(roughness), albedo(albedo) {
    }
    virtual color brdf(const HitRecord &rec, glm::vec3 l, glm::vec3 v) const;
    virtual bool reflection(const HitRecord &rec, glm::vec3 v, glm::vec3 &r, color &kr) const;
};

class Emissive: public Material {
public:
    color emittedRadiance=glm::vec3(0.0f);
    Emissive(color emittedRadiance):
        emittedRadiance(emittedRadiance) {
    }
    virtual color emission(const HitRecord &rec, glm::vec3 v) const;
    virtual color brdf(const HitRecord &rec, glm::vec3 l, glm::vec3 v) const {
        return glm::vec3(1.0f);
    }
    virtual bool reflection(const HitRecord &rec, glm::vec3 v,
                            glm::vec3 &r, color &kr) const;
};

class EmissiveRectangle: public Material {
public:
    color emittedRadiance=glm::vec3(0.0f);
    EmissiveRectangle(color emittedRadiance):
        emittedRadiance(emittedRadiance) {
    }
    virtual color emission(const HitRecord &rec, glm::vec3 v) const;
    virtual color brdf(const HitRecord &rec, glm::vec3 l, glm::vec3 v) const {
        return glm::vec3(1.0f);
    }
    virtual bool reflection(const HitRecord &rec, glm::vec3 v,
                            glm::vec3 &r, color &kr) const;
};

class PointLight {
public:
    glm::vec3 location;
    color intensity;
    PointLight(glm::vec3 location, color intensity): location(location),intensity(intensity) {}
};

#endif
