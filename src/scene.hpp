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
    color getColor(Ray ray, int depth = 1) const;
    color tracePath(Ray ray, int numberOfSamples, int numberOfBounces) const;
    color computeColor(HitRecord& rec, glm::vec3 direction, int numberOfBounces) const;
    bool inShadow(glm::vec3 p, PointLight light) const;
    glm::vec3 irradiance(HitRecord &rec, PointLight light) const;
    color radiance(HitRecord &rec) const;
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
    glm::mat4 transform, normalTransform, inverse;
    Object(Shape *shape, Material *mat, glm::mat4 M=glm::mat4(1.0)):
        shape(shape),
        mat(mat) {
            transform = glm::mat4(1.0);
            normalTransform = glm::mat4(1.0);
            inverse = glm::mat4(1.0);
    }
    bool hit(Ray ray, Interval t_range, HitRecord &rec) const;
    void setTransform(glm::mat4 M) {
        transform = M;
        normalTransform = glm::inverseTranspose(M);
        inverse = glm::inverse(M);
    }
    void debugTransform();
    ~Object() {
    }
};

class Shape {
public:
    virtual bool hit(Ray ray, Interval t_range, HitRecord &rec) const = 0;
};

class Sphere: public Shape {
public:
    glm::vec3 c;
    float r;
    Sphere(glm::vec3 center, float radius):
        c(center),
        r(radius) {
    }
    bool hit(Ray ray, Interval t_range, HitRecord &rec) const override;
};

class Plane: public Shape {
public: 
    glm::vec3 point, normal;
    Plane(glm::vec3 pt, glm::vec3 n): point(pt), normal(n) { };
    bool hit(Ray ray, Interval t_range, HitRecord &rec) const override;
};

class Box: public Shape {
public:
    glm::vec3 low, hi;
    Box(glm::vec3 minpt, glm::vec3 maxpt): low(minpt), hi(maxpt) { };
    bool hit(Ray ray, Interval t_range, HitRecord &rec) const override;
};

class Material {
public:
    color ambientColor=glm::vec3(0.0f);
    virtual color emission(const HitRecord &rec, glm::vec3 v) const {
        return glm::vec3(0.0);
    }
    virtual color brdf(const HitRecord &rec, glm::vec3 l, glm::vec3 v) const = 0;
    virtual bool reflection(const HitRecord &rec, glm::vec3 v,
                            glm::vec3 &r, color &kr) const = 0;
};

class Lambertian: public Material {
public:
    color albedo;
    Lambertian(color albedo):
        albedo(albedo) {
    }
    virtual color brdf(const HitRecord &rec, glm::vec3 l, glm::vec3 v) const;
    virtual bool reflection(const HitRecord &rec, glm::vec3 v,
                            glm::vec3 &r, color &kr) const;
};

class Metallic: public Material {
public:
    color parallelReflection; //This is the F_0 value for schlick's approximation
    int shininess = 1;
    color albedo; //specular reflection coefficient
    Metallic(color parallelReflection, int blinnPhongExponent, color albedo):
        parallelReflection(parallelReflection), shininess(blinnPhongExponent), albedo(albedo) {
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
        return glm::vec3(0.0f);
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
