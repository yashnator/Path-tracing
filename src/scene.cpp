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
    // return glm::normalize(totalRadiance + ambientLight*rec.mat->ambientColor);
    return glm::normalize(totalRadiance);
}

color Scene::computeColor(HitRecord& rec, glm::vec3 direction, int numberOfBounces) const
{
    float prob = 1.0 - 1.0/(float)numberOfBounces;
    glm::vec3 v = direction;
    color emittedRadiance = glm::vec3(0.0f), otherSources = glm::vec3(0.0f);
    float cos_theta = glm::dot(glm::normalize(v), rec.n);
    if(rec.mat) emittedRadiance += rec.mat->emission(rec,v); //The direct emitted radiance from the object
    if(rec.mat) emittedRadiance+=ambientLight*rec.mat->ambientColor; //Ambient color of the object itself
    //Sample a direction from importance sampling
    glm::vec3 sampledNormal = glm::vec3(0.0f), kr = glm::vec3(0.0f);
    rec.mat->reflection(rec,v,sampledNormal, kr);
    float pdfinverse = glm::length(sampledNormal);
    sampledNormal = glm::normalize(sampledNormal);
    
    if(probability(prob) && cos_theta>0 && sampledNormal!=glm::vec3(0.0f))
    {
        //Trace the ray
        Ray sampledRay=Ray(rec.p, sampledNormal); HitRecord sampledHit = HitRecord();
        Interval t_range = Interval(0.001f, std::numeric_limits<float>::max());
        int no_of_hits=0;
        for(auto const &obj:objects)
        {
            if(obj->hit(sampledRay,t_range,sampledHit)) 
            {
                no_of_hits++;
                t_range.max=std::min(t_range.max,rec.t);
            }
        }
        glm::vec3 l = glm::normalize(sampledHit.p - rec.p);
        if(no_of_hits==0) ;//Decide what to do
        else
        {
            //Recurse and solve, get the pdf
            otherSources = (computeColor(sampledHit, -1.0f*l, numberOfBounces)*cos_theta*rec.mat->brdf(rec, l, v))*pdfinverse;
        }
    }
    return emittedRadiance + otherSources/prob;
}

color Scene::tracePath(Ray ray, int numberOfSamples, int numberOfBounces) const
{
    HitRecord rec = HitRecord();
    Interval t_range = Interval(0.001f, std::numeric_limits<float>::max());
    color res = glm::vec3(0);
    int no_of_hits = 0;
    for(auto const &obj:objects)
    {
        if(obj->hit(ray, t_range, rec)) 
        {
            no_of_hits++;
            t_range.max=std::min(t_range.max, rec.t);
        }
    }
    if(!no_of_hits) return sky;
    for(int i=0;i<numberOfSamples;i++)
    {
        res+=computeColor(rec, camera->getLocation()-rec.p, numberOfBounces);
    }
    return (float)(1.0/(float)numberOfSamples)*res;
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
            }
            no_of_hits++;
            t_range.max=std::min(t_range.max, rec.t);
        }
    }
    if(!no_of_hits) c = sky;
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
        }

        //Calculate the final radiance
        glm::vec3 inherent = glm::vec3(1.0f)-kr;
        glm::vec3 reflected = kr;
        c = inherent * c + reflected * reflectedColor;
    }
    return glm::normalize(c);
}