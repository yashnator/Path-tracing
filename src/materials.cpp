#include "scene.hpp"

//Material functions
color Lambertian::brdf(const HitRecord &rec, glm::vec3 l, glm::vec3 v) const
{
    return (albedo/glm::pi<float>());
}
bool Lambertian::reflection(const HitRecord &rec, glm::vec3 v, glm::vec3 &r, color &kr) const
{
    kr = glm::vec3(0.0f);
    r = sampleCosineHemisphere(rec.n);
    return false;
}

color Metallic::brdf(const HitRecord &rec, glm::vec3 l, glm::vec3 v) const
{
    glm::vec3 bisector = glm::normalize(glm::normalize(l) + glm::normalize(v));
    float cos_theta = glm::dot(rec.n, bisector);
    if(cos_theta < 0.0f) return glm::vec3(0.0f);
    cos_theta = std::pow(cos_theta, shininess);
    return (albedo * cos_theta) / glm::pi<float>();
}

bool Metallic::reflection(const HitRecord &rec, glm::vec3 v, glm::vec3 &r, color &kr) const
{
    // std::cout<<"called"<<std::endl;
    float cos_theta = glm::dot(rec.n, glm::normalize(v));
    if(cos_theta < 0) {return false;}

    glm::vec3 d = glm::normalize(-1.0f * v);
    r = glm::normalize(d - 2.0f * glm::dot(rec.n, d) * rec.n);

    //Apply the schlick's approximation
    kr = parallelReflection;
    if(kr.x<0 || kr.y<0 || kr.z<0) std::cout<<"heavy sudai"<<std::endl;
    kr += (glm::vec3(1.0f) - kr) * glm::pow(1.0f - cos_theta, 5.0f);
    if(kr.x<0 || kr.y<0 || kr.z<0) std::cout<<"kr is negative"<<std::endl;
    // std::cout<<"successful ref "<<to_string(r)<<std::endl;
    return true;
}
bool Emissive::reflection(const HitRecord &rec, glm::vec3 v, glm::vec3 &r, color &kr) const
{
    return false;
}
color Emissive::emission(const HitRecord &rec, glm::vec3 v) const
{
    return emittedRadiance;
}