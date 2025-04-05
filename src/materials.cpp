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
    // return glm::vec3(0.0f);
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

float randf() {
    static thread_local std::mt19937 gen(std::random_device{}());
    static thread_local std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    return dist(gen);
}

float GGX_PDF(const glm::vec3& n, const glm::vec3& h, glm::vec3& v, float alpha) {
    float NdotH = glm::max(glm::dot(n, h), 0.0f);
    float D = (alpha * alpha) / 
              (glm::pi<float>() * 
               std::pow(NdotH * NdotH * (alpha * alpha - 1.0f) + 1.0f, 2));
    return D * NdotH / (4.0f * glm::dot(h, v));
}

glm::vec3 sampleGGXVNDF(const glm::vec3& n, const glm::vec3& v, float roughness) {
    float alpha = roughness * roughness;

    // Transform view direction to hemisphere space (aligned with z-axis)
    glm::vec3 up = std::abs(n.z) < 0.999f ? glm::vec3(0, 0, 1) : glm::vec3(1, 0, 0);
    glm::vec3 tangentX = glm::normalize(glm::cross(up, n));
    glm::vec3 tangentY = glm::cross(n, tangentX);

    // Random numbers
    float xi1 = randf();  // Uniform [0, 1)
    float xi2 = randf();

    // GGX sampling
    float theta = std::atan(alpha * std::sqrt(xi1) / std::sqrt(1.0f - xi1));
    float phi = 2.0f * glm::pi<float>() * xi2;

    float sinTheta = std::sin(theta);
    float cosTheta = std::cos(theta);

    glm::vec3 h = glm::normalize(
        sinTheta * std::cos(phi) * tangentX +
        sinTheta * std::sin(phi) * tangentY +
        cosTheta * n
    );
    return h;
}
bool TorrenceSparrow::reflection(const HitRecord &rec, glm::vec3 v, glm::vec3 &r, color &kr) const
{
    // std::cout<<"called"<<std::endl;
    float cos_theta = glm::dot(rec.n, glm::normalize(v));
    if(cos_theta < 0) {return false;}

    glm::vec3 d = glm::normalize(-1.0f * v);
    r = glm::normalize(d - 2.0f * glm::dot(rec.n, d) * rec.n);

    glm::vec3 h = sampleGGXVNDF(rec.n, v, roughness);
    float pdf = GGX_PDF(rec.n, h, v, roughness);
    r = glm::reflect(d,h)/pdf;
    std::cout<<to_string(r)<<std::endl;

    //Apply the schlick's approximation
    kr = parallelReflection;
    if(kr.x<0 || kr.y<0 || kr.z<0) std::cout<<"heavy sudai"<<std::endl;
    kr += (glm::vec3(1.0f) - kr) * glm::pow(1.0f - cos_theta, 5.0f);
    if(kr.x<0 || kr.y<0 || kr.z<0) std::cout<<"kr is negative"<<std::endl;
    // std::cout<<"successful ref "<<to_string(r)<<std::endl;
    return true;
}


color TorrenceSparrow::brdf(const HitRecord &rec, glm::vec3 l, glm::vec3 v) const
{
    glm::vec3 n = rec.n;
    glm::vec3 h = glm::normalize(glm::normalize(l) + glm::normalize(v)); // Halfway vector

    //Obtain the ggx function
    float alphasq = roughness * roughness;
    float cos_theta_m = glm::dot(h,rec.n);
    float cos_theta_4 = cos_theta_m * cos_theta_m * cos_theta_m * cos_theta_m;
    float tan_theta_2 = (1.0f - cos_theta_m * cos_theta_m) / (cos_theta_m * cos_theta_m);
    float D = alphasq / (glm::pi<float>() * cos_theta_4 * (alphasq + tan_theta_2) * (alphasq + tan_theta_2));

    // //Use the torrance sparrow model to get G
    // float G = 1.0;
    // G = glm::min(G,2.0f*(glm::dot(rec.n,h)*glm::dot(rec.n,v)) / glm::dot(v,h));
    // G = glm::min(G,2.0f*(glm::dot(rec.n,h)*glm::dot(rec.n,l)) / glm::dot(l,h));
    float cos_theta_i = glm::dot(rec.n, l);
    float cos_theta_r = glm::dot(rec.n, v);

    if(cos_theta_i<0.0f || cos_theta_r<0.0f) return glm::vec3(0.0f);
    // std::cout<<D<<" "<<(D)/
    //     (4.0f * cos_theta_i * cos_theta_r)<<std::endl;
    
    return (albedo * D)/
           (4.0f * cos_theta_i * cos_theta_r);
    glm::vec3 bisector = glm::normalize(glm::normalize(l) + glm::normalize(v));
    float cos_theta = glm::dot(rec.n, bisector);
    if(cos_theta < 0.0f) return glm::vec3(0.0f);
    cos_theta = std::pow(cos_theta, 200.0);
    return (albedo * cos_theta) / glm::pi<float>();
}