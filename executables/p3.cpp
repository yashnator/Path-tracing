#include "../src/scene.hpp"
#include "../src/image.hpp"

#include <SDL2/SDL.h>
#include <iostream>

int main() {
    int w = 800, h = 600;
    HDRImage image(w, h);
    Scene scene;
    scene.camera = new Camera();

    //Make light sources
    PointLight p1 = PointLight(glm::vec3(10.0,0.0,-2.0),glm::vec3(1.0f,1.0f,1.0f));
    p1.intensity*=100;
    // scene.lights.push_back(p1);

    PointLight p2 = PointLight(glm::vec3(-10.0,0.0,-2.0),glm::vec3(1.0f,1.0f,1.0f));
    p2.intensity*=100;
    // scene.lights.push_back(p2);

    PointLight p3 = PointLight(glm::vec3(0.0,3.0,-2.0),glm::vec3(1.0f,1.0f,1.0f));
    p3.intensity*=500;
    scene.lights.push_back(p3);

    //Make materials
    Material* mat1 = new Lambertian(glm::vec3(1.0f, 1.0f, 0.0f));
    mat1->ambientColor = glm::vec3(0.0,1.0,0.0);

    //Metallic materials
    color albedo = glm::vec3(1.0f, 1.0f, 1.0f);
    color parallelReflection = glm::vec3(0.5f, 0.5f, 0.5f); //The F0 value
    int expo = 200;
    Material* mat3 = new Metallic(parallelReflection, expo, albedo);

    Material* mat2 = new Lambertian(glm::vec3(0.55,0.27,0.07));
    // mat2->ambientColor = glm::vec3(0.55,0.27,0.07);
    
    // Add spheres
    Sphere* s1 = new Sphere(glm::vec3(0, 0, -2.0), 0.25f);
    Object* o1 = new Object(s1, mat3);
    //Add some basic transform
    glm::mat4 spTransform = glm::mat4(1.0f);
    // spTransform = glm::scale(spTransform, glm::vec3(1.0f,1.0f,1.0f));
    // spTransform = glm::translate(spTransform, glm::vec3(0.0f, 0.0f, -5.0f));
    // o1->setTransform(spTransform);
    // o1->debugTransform();

    Sphere* s2 = new Sphere(glm::vec3(0, -101, -2), 100);
    Object* o2 = new Object(s2, mat2);
    scene.objects.push_back(o2);
    scene.objects.push_back(o1);

    // scene.sky = glm::vec3(0.69,0.77,0.87);
    scene.ambientLight = glm::vec3(1.0,1.0,1.0);

    for (int j = 0; j < h; j++) {
        for (int i = 0; i < w; i++) {
            float x = 2*(i+0.5)/w - 1;
            float y = 1 - 2*(j+0.5)/h;
            Ray ray = scene.camera->make_ray(x, y);

            // trace the ray and get the colour
            // color c = glm::normalize(ray.d)*0.5f + 0.5f;;
            color c = scene.getColor(ray);
            image.pixel(i, j) = c;

        }
    }
    SDL_Surface *out = SDL_CreateRGBSurface(0, w, h, 32, 0, 0, 0, 0);
    tonemap(image, out, 1, 2.2f);
    std::string filename = "part_3.png";
    IMG_SavePNG(out, filename.data());
    openImage(filename.data());

    // Ray testRay = Ray(glm::vec3(0,0,0), glm::vec3(0,0,-1));
    // HitRecord rec = HitRecord();
    // Interval t_range = Interval(0.001f, std::numeric_limits<float>::max());
    // if(o1->hit(testRay, t_range, rec))
    // {
    //     std::cout<<"Hit at: "<<glm::to_string(rec.p)<<std::endl;
    //     std::cout<<"Normal: "<<glm::to_string(rec.n)<<std::endl;
    //     std::cout<<"Material: "<<rec.mat<<std::endl;
    // }
    // else
    // {
    //     std::cout<<"No hit"<<std::endl;
    // }
    //Memory cleanup
    // for(auto obj: scene.objects) delete obj;
    delete scene.camera;
    delete mat1;
    delete mat2;
    // delete mat1;
}
