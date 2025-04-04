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

    PointLight p3 = PointLight(glm::vec3(0.0,3.0,-2.0),glm::vec3(1.0f,0.0f,1.0f));
    p3.intensity*=1000;
    scene.lights.push_back(p3);

    //Make materials
    Material* mat1 = new Lambertian(glm::vec3(1.0f, 1.0f, 1.0f));

    // Add spheres
    Sphere* s1 = new Sphere(glm::vec3(0, 0, -2.0), 0.25f);
    Object* o1 = new Object(s1, mat1);
    //Add some basic transform
    glm::mat4 spTransform = glm::mat4(1.0f);
    spTransform = glm::scale(spTransform, glm::vec3(2.0f, 1.0, 1.0));
    o1->setTransform(spTransform);

    Sphere* s2 = new Sphere(glm::vec3(0, -101, -2), 100);
    Object* o2 = new Object(s2, mat1);
    scene.objects.push_back(o2);
    scene.objects.push_back(o1);

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
    IMG_SavePNG(out, "out.png");

    //Memory cleanup
    // for(auto obj: scene.objects) delete obj;
    delete scene.camera;
    // delete mat1;
}
