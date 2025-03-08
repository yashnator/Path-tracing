#ifndef SCENE_HPP
#define SCENE_HPP

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <vector>

using color = glm::vec3;

class Ray;
class Interval;
class Shape;
class HitRecord;
class Object;
class Material;
class PointLight;
class Camera;

class Scene {
public:
    Camera *camera;
    std::vector<Object*> objects;
    std::vector<PointLight> lights;
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
};

class Camera {
public:
    Ray make_ray(float x, float y) const; // screen coordinates in [-1, 1]
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
};

class Object {
public:
    Shape *shape;
    Material *mat;
    Object(Shape *shape, Material *mat, glm::mat4 M=glm::mat4(1.0)):
        shape(shape),
        mat(mat) {
    }
    bool hit(Ray ray, Interval t_range, HitRecord &rec) const;
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
    virtual bool hit(Ray ray, Interval t_range, HitRecord &rec) const;
};

// class Plane: public Shape {
// };

// class Box: public Shape {
// };

class Material {
public:
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
    virtual color brdf(const HitRecord &rec, glm::vec3 l, glm::vec3 v) const;
    virtual bool reflection(const HitRecord &rec, glm::vec3 v,
                            glm::vec3 &r, color &kr) {
        return false;
    }
};

// class Metallic: public Material {
// };

// class Emissive: public Material {
// };

class PointLight {
public:
    glm::vec3 location;
    color intensity;
};

#endif
