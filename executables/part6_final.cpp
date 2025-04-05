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
    Material* lsrc = new Emissive(glm::vec3(500.0f,500.0f,500.0f) / 10.0f);
    // Sphere* ls1 = new Sphere

    //Make materials
    PointLight pl1 = PointLight(glm::vec3(-5.0,5.0,0.0), glm::vec3(1.0f,1.0f,1.0f));
    pl1.intensity*=200;
    // scene.lights.push_back(pl1);
    PointLight pl2 = PointLight(glm::vec3(5.0,5.0,0.0), glm::vec3(1.0f,1.0f,1.0f));
    pl2.intensity*=200;
    // scene.lights.push_back(pl2);
    PointLight pl3 = PointLight(glm::vec3(0.0,10.0,-20.0),glm::vec3(1.0f,1.0f,1.0f));
    pl3.intensity*=1000;
    // scene.lights.push_back(pl3);

    //Lambertian materials
    Material* mat1 = new Lambertian(glm::vec3(1.0f, 1.0f, 1.0f));
    Material* blue_mat = new Lambertian(glm::vec3(0.0f, 0.0f, 1.0f));
    Material* red_mat = new Lambertian(glm::vec3(1.0f, 0.0f, 0.0f));
    Material* green_mat = new Lambertian(glm::vec3(0.0f, 1.0f, 0.0f));
    Material* purple_mat = new Lambertian(glm::vec3(1.0f, 0.0f, 1.0f));
    // mat1->ambientColor = glm::vec3(0.0,0.01,0.0);

    Material* mat2 = new Lambertian(glm::vec3(1.0f,1.0f,1.0f));
    // mat2->ambientColor = glm::vec3(0.55,0.27,0.07);

    //Metallic materials
    color albedo = glm::vec3(1.0f, 1.0f, 1.0f);
    color parallelReflection = glm::vec3(0.5f, 0.2f, 0.5f); //The F0 value
    int expo = 200;
    Material* mat3 = new Metallic(parallelReflection, expo, albedo);
    
    // Add spheres
    Sphere* s1 = new Sphere(glm::vec3(-5.0,5.0,0.0), 1.5f);
    Object* o1 = new Object(s1, lsrc);
    // o7->setTransform(glm::rotate(glm::mat4(1.0f), glm::radians(100.0f), glm::vec3(0.0f, 1.0f, 0.0f))*glm::scale(glm::mat4(1.0f), glm::vec3(2.0f,1.0f,1.0f)));

    //Add planes
    Plane* p1 = new Plane(glm::vec3(0, -1.0, 0), glm::vec3(0.0, 1.0, 0.0));
    Object* p1_obj = new Object(p1, mat2);

    //Add boxes
    // Box* b1 = new Box(glm::vec3(-0.8f, -0.25f, -2.5f), glm::vec3(-0.5f, 0.25f, -2.0f));
    // Object* o4 = new Object(b1, blue_mat);
    Object* o5 = new Object(new Box(glm::vec3(0.8f, -1.0f, -5.0f), glm::vec3(1.75f, 0.25f, -3.0f)), red_mat);
    Object* o7 = new Object(new Sphere(glm::vec3(-0.8f, 0.0, -4.0), 1.0f), mat3);
    // o5->setTransform(glm::rotate(glm::mat4(1.0f), glm::radians(45.0f), glm::vec3(1.0f, 0.0f, 0.0f)));

    // Box* b3 = new Box(glm::vec3(-0.2f, -1.0f, -3.5f), glm::vec3(0.4f, -0.75f, -4.0f));
    // Object* o6 = new Object(b3, green_mat);
    // o6->setTransform(glm::rotate(glm::mat4(1.0f), glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f)));

    // scene.objects.push_back(o2);
    scene.objects.push_back(o1);
    // scene.objects.push_back(o3);
    scene.objects.push_back(p1_obj);
    // scene.objects.push_back(o4);
    scene.objects.push_back(o5);
    // scene.objects.push_back(o6);
    scene.objects.push_back(o7);

    scene.sky = glm::vec3(0.69,0.77,0.87) / 50.0f;
    // scene.sky = glm::vec3(0.0,0.0,0.0);
    scene.ambientLight = glm::vec3(1.0,1.0,1.0);

    for (int j = 0; j < h; j++) {
        for (int i = 0; i < w; i++) {
            float x = 2*(i+0.5)/w - 1;
            float y = 1 - 2*(j+0.5)/h;
            Ray ray = scene.camera->make_ray(x, y);


            // trace the ray and get the colour
            // color c = glm::normalize(ray.d)*0.5f + 0.5f;;
            color c = scene.tracePath(ray, 1000, 5);
            // color c = scene.getColor(ray);
            // std::cout<<x<<" "<<y<<" "<<to_string(c)<<std::endl;
            image.pixel(i, j) = c;

        }
    }
    SDL_Surface *out = SDL_CreateRGBSurface(0, w, h, 32, 0, 0, 0, 0);
    tonemap(image, out, 1, 2.2f);
    std::string filename = "path_tracing.png";
    for(auto x: scene.objects)
    {
        delete x;
    }
    IMG_SavePNG(out, filename.data());
    openImage(filename.data());

    delete scene.camera;

}