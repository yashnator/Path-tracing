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
    return totalRadiance;
}

color Scene::computeColor(Ray ray, int numberOfBounces) const
{
    std::pair<HitRecord,int> hit = traceRay(ray);
    if(!hit.second) return sky;
    glm::vec3 point = hit.first.p;
    glm::vec3 v = glm::normalize(-1.0f*ray.d);
    color Le = hit.first.mat->emission(hit.first, v);
    color Lr = glm::vec3(0.0f);

    float prob = 1.0 - (1.0/(float)numberOfBounces);

    //Sample a direction from importance sampling
    glm::vec3 sampledNormal = glm::vec3(0.0f), kr = glm::vec3(0.0f);
    hit.first.mat->reflection(hit.first,v,sampledNormal, kr);
    float pdfinverse = glm::length(sampledNormal);
    sampledNormal = glm::normalize(sampledNormal);

    //Get cos_theta_i
    float cos_theta_i = glm::dot(hit.first.n, sampledNormal);
    
    if(probability(prob) && sampledNormal!=glm::vec3(0.0f))
    {
        std::pair<HitRecord,int> hit2 = traceRay(Ray(point+0.001f*hit.first.n, sampledNormal));
        if(hit2.second)
        {
            Lr = computeColor(Ray(point+0.001f*hit.first.n, sampledNormal),numberOfBounces)*
                 cos_theta_i*pdfinverse*hit.first.mat->brdf(hit.first, sampledNormal, v)*
                (1.0f/prob); 
        }
    }
    return Le+Lr;
}

color Scene::tracePath(Ray ray, int numberOfSamples, int numberOfBounces) const
{
    color c = glm::vec3(0.0f);
    for(int i=0;i<numberOfSamples;i++)
    {
        c += computeColor(ray, numberOfBounces);
    }
    return c/(float)numberOfSamples;
}

//Camera functions
Camera::Camera(float fov, float width, float height) : fov(fov), width(width), height(height)
{
    center = glm::vec3(0.0f);
    view = glm::vec3(0.0f, 0.0f, -1.0f);
    up = glm::vec3(0.0f, 1.0f, 0.0f);
    right = glm::vec3(1.0f,0.0f,0.0f);
}
//Scene functions
color Scene::getColor(Ray ray, int depth) const
{
    //exceeded the recursion depth
    if(depth<0) return glm::vec3(0.0f);

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
                c = radiance(rec)+rec.mat->emission(rec,ray.d); // if hit is at a light source, adjust
                if(depth==0) std::cout<<" "<<to_string(c)<<std::endl;
                // std::cout<<to_string(radiance(rec))<<std::endl;
            }
            no_of_hits++;
            t_range.max=std::min(t_range.max, rec.t);
        }
    }
    // std::cout<<"check "<<to_string(c)<<std::endl;
    if(!no_of_hits) {c = sky;}
    else
    {
        //some object was hit, we now need to find if something was reflected from this object
        //Get the reflection coefficient and the reflected direction
        glm::vec3 kr = glm::vec3(0.0f), r = glm::vec3(0.0f), reflectedColor = glm::vec3(0.0f);
        if(rec.mat->reflection(rec,ray.o-rec.p, r, kr))
        {
            //calculate the reflected color. bias added
            Ray reflectedRay = Ray(rec.p + 0.001f*rec.n, r);
            reflectedColor = getColor(reflectedRay, depth-1);
            if(reflectedColor.x < 0 || reflectedColor.y < 0 || reflectedColor.z < 0)
            {
                std::cout<<"Reflected color is negative"<<std::endl;
                // reflectedColor = glm::vec3(0.0f);
            }
            // std::cout<<"reflection happened"<<std::endl;
            // std::cout<<"reflected color "<<to_string(reflectedColor)<<std::endl;
        }

        //Calculate the final radiance
        glm::vec3 inherent = glm::vec3(1.0f)-kr;
        glm::vec3 reflected = kr;
        // std::cout<<to_string(reflectedColor)<<std::endl;
        c = inherent * c + reflected * reflectedColor;
        // if(reflected.x>0.0f && reflected.y>0.0f && reflected.z>0.0f)
        // {
        //     std::cout<<"final color "<<to_string(c)<<std::endl;
        // }
    }
    return c;
}