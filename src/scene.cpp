#include "scene.hpp"

//Scene functions
bool Scene::inShadow(glm::vec3 p, PointLight light) const
{
    Ray shadow_ray(p, normalize(light.location-p));
    HitRecord rec = HitRecord();
    float light_t = (glm::length(light.location - shadow_ray.o) / glm::length(shadow_ray.d));
    Interval t_range = Interval(0.0f, light_t);
    float bias = 0.001f, min_hit = light_t;
    // for(auto const &obj:objects)
    shadow_ray.o = shadow_ray.o + bias * shadow_ray.d;
    for(int i = 0; i < objects.size(); ++i)
    {
        auto const&obj = objects[i];
        if(obj->hit(shadow_ray, t_range, rec) and !obj->shape->isRectangle) {
            min_hit = std::min(min_hit, std::max(rec.t, 2 * bias));
            // if(rec.t < 0) std::cout << "negative min_hit: " << i << " " << std::endl;
        }
    }
    // std::cout << "lt: " << light_t <<  " " << min_hit << std::endl;
    return (min_hit > bias && min_hit < light_t) ? true : false;
}

glm::vec3 Scene::irradiance(HitRecord &rec, PointLight light) const
{
    float r_square = glm::dot(light.location - rec.p, light.location - rec.p);
    glm::vec3 l = light.location-rec.p;
    float cos_theta = glm::dot(rec.n, glm::normalize(l));

    // std::cout << "normal: " << std::endl;

    if(cos_theta < 0) return glm::vec3(0.0);
    // if(cos_theta < 0) cos_theta = -cos_theta;
    return (light.intensity * cos_theta) / r_square;
}

color Scene::radiance(HitRecord &rec) const
{
    color totalRadiance = glm::vec3(0.0);
    for(auto const &light:lights)
    {
        if(!inShadow(rec.p, light))
        {
            // std::cout << "not in shadow" << std::endl;
            glm::vec3 v = camera->getLocation()-rec.p;
            glm::vec3 l = light.location-rec.p;
            color brdf = rec.mat->brdf(rec, l, v);
            // std::cout << to_string(brdf) << std::endl;
            // auto ir = irradiance(rec, light);
            totalRadiance += irradiance(rec, light) * brdf;
            // std::cout << "irradience: " <<  to_string(ir) << std::endl;
        } else {
            // std::cout << "outgoing" <<std::endl;
        }
    }
    // std::cout << to_string(totalRadiance) << std::endl;
    return totalRadiance;
}

float random_float_01() {
    static std::random_device rd;
    static std::mt19937 gen(rd()); // static: only initialized once
    static std::uniform_real_distribution<float> dis(0.0f, 1.0f);
    return dis(gen);
}

color Scene::radianceFromEmissive(HitRecord &rec) const
{
    color totalRadiance = glm::vec3(0.0);
    if(rec.mat->emission(rec, camera->getLocation()-rec.p) != glm::vec3(0.0f))
    {
        return totalRadiance;
    }
    for(auto const &obj:objects) {
        if(obj->shape->isRectangle) {
            for(int i = 0; i < 5; i++) {
                Rectangle *rect = static_cast<Rectangle*>(obj->shape);
                glm::vec3 lo = rect->low, hi = rect->hi;
                float sample_x = random_float_01() * (hi.x - lo.x) + lo.x;
                float sample_z = random_float_01() * (hi.z - lo.z) + lo.z;
                glm::vec3 sample_point = glm::vec3(sample_x, lo.y, sample_z);
                if(not inShadow(rec.p + 0.001f * rec.n, PointLight(sample_point, glm::vec3(0.0f)))) {
                    totalRadiance += obj->mat->emission(rec, camera->getLocation()-rec.p) * glm::dot(rec.n, glm::normalize(sample_point-rec.p));
                    // std::cout << "sampled point: " << to_string(sample_point) << std::endl;
                } else {
                    // std::cout << "sampled point: " << to_string(sample_point) << std::endl;
                }
            }
        }
    }   

    return (totalRadiance * rec.mat->albedo) / 500.0f;

    // color totalRadiance = glm::vec3(0.0);
    // if(rec.mat->emission(rec, camera->getLocation()-rec.p) != glm::vec3(0.0f))
    // {
    //     return totalRadiance;
    // }
    // for(auto const &obj:objects)
    // {
    //     if(obj->mat && obj->mat->emission(rec, camera->getLocation()-rec.p) != glm::vec3(0.0f))
    //     {
    //         //The object must be a sphere
    //         // std::cout<<"source cum"<<std::endl;
    //         Sphere *sp = dynamic_cast<Sphere*>(obj->shape);
    //         if(!sp) std::cout<<"cooked"<<std::endl;
    //         glm::vec3 ct = glm::normalize(sp->c-rec.p);
    //         std::pair<HitRecord,int> hit = traceRay(Ray(rec.p,glm::normalize(sp->c-rec.p)));
    //         if(hit.second) totalRadiance += hit.first.mat->emission(rec, sp->c-rec.p) * rec.mat->brdf(rec,sp->c - rec.p, camera->getLocation()-rec.p);
    //     }
    // }
    // return totalRadiance;
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
    
    if(probability(prob) && sampledNormal!=glm::vec3(0.0f) && pdfinverse > 1e-5)
    {
        std::pair<HitRecord,int> hit2 = traceRay(Ray(point+0.001f*sampledNormal, sampledNormal));
        if(hit2.second)
        {
            Lr = computeColor(Ray(point+0.001f*sampledNormal, sampledNormal),numberOfBounces)*
                 cos_theta_i*pdfinverse*hit.first.mat->brdf(hit.first, sampledNormal, v)*
                (1.0f/prob); 
        }
    }
    // std::cout << "brdf: " << to_string(hit.first.mat->brdf(hit.first, sampledNormal, v)) << std::endl;
    if(kr != glm::vec3(0.0f))
    {
        Lr *= kr;
    }
    return Le + Lr;
}

color Scene::tracePath(Ray ray, int numberOfSamples, int numberOfBounces) const
{
    color c = glm::vec3(0.0f);
    for(int i=0;i<numberOfSamples;i++)
    {
        c += computeColor(ray, numberOfBounces);
    }
    color direct_light = glm::vec3(0.0f);
    std::pair<HitRecord,int> hit = traceRay(ray);
    if(hit.second) direct_light+= radiance(hit.first);
    if(hit.second) direct_light+= radianceFromEmissive(hit.first);

    return (c/(float)numberOfSamples + direct_light);
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
            // std::cout<<to_string(rec.n)<<std::endl;
            // std::cout<<to_string(rec.p)<<std::endl;
            if(!obj->mat) {c = (float)0.5 * (rec.n + glm::vec3(1));}
            else
            {
                c = radiance(rec)+rec.mat->emission(rec,ray.d); // if hit is at a light source, adjust
                // if(depth==0) std::cout<<" "<<to_string(c)<<std::endl;
                // std::cout<<to_string(radiance(rec))<<std::endl;
            }
            no_of_hits++;
            t_range.max=std::min(t_range.max, rec.t);
        }
        else
        {
            // std::cout<<"no hit"<<std::endl;
            // std::cout<<to_string(ray.o)<<" "<<to_string(ray.d)<<std::endl;
        }
    }
    // std::cout<<"check "<<to_string(c)<<std::endl;
    if(!no_of_hits) {c = sky;}
    else
    {
        //some object was hit, we now need to find if something was reflected from this object
        //Get the reflection coefficient and the reflected direction
        glm::vec3 kr = glm::vec3(0.0f), r = glm::vec3(0.0f), reflectedColor = glm::vec3(0.0f);
        if(rec.mat && rec.mat->reflection(rec,ray.o-rec.p, r, kr))
        {
            //calculate the reflected color. bias added
            Ray reflectedRay = Ray(rec.p + 0.001f*rec.n, r);
            reflectedColor = getColor(reflectedRay, depth-1);
            // reflectedColor *= rec.mat->brdf(rec, r, ray.o-rec.p);
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
        c = inherent * c + reflected * reflectedColor * rec.mat->brdf(rec, r, ray.o-rec.p);
        // if(no_of_hits) std::cout<< "cum"<<std::endl;
    }
    // if(no_of_hits){std::cout<<"final allot "<<std::endl;
    // if(no_of_hits) std::cout<<to_string(c)<<std::endl;
    // else std::cout<<"sudai moment:" << to_string(c)<<std::endl;
    return c;
}