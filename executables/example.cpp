#include "../src/scene.hpp"
#include "../src/image.hpp"

#include <SDL2/SDL.h>
#include <iostream>

int main() {
    int w = 800, h = 600;
    HDRImage image(w, h);
    Scene scene;
    scene.camera = new Camera();

    //Add spheres
    // Sphere* s1 = new Sphere(glm::vec3(0, 0, -2), 1);
    // Object* o1 = new Object(s1, nullptr);

    // Sphere* s2 = new Sphere(glm::vec3(0, -101, -2), 100);
    // Object* o2 = new Object(s2, nullptr);
    // scene.objects.push_back(o2);
    // scene.objects.push_back(o1);

    // Plane* p1 = new Plane(glm::vec3(0, 0, -2), glm::vec3(1, 1, -1));
    // Plane* p2 = new Plane(glm::vec3(0, 0, -2), glm::vec3(0, 0, -1));
    // scene.objects.emplace_back(new Object(p1, nullptr));
    // scene.objects.emplace_back(new Object(p2, nullptr));

    // Box* bx1 = new Box(glm::vec3(0.5, 0.5, -5), glm::vec3(-0.5, -0.5, -5.5));
    // scene.objects.emplace_back(new Object(bx1, nullptr));

    // Second scene
    Sphere* s1 = new Sphere(glm::vec3(0, 0, -2), 1);
    scene.objects.emplace_back(new Object(s1, nullptr));
    Plane* p1 = new Plane(glm::vec3(0, -1, 0), glm::vec3(0, -1, 0));
    scene.objects.emplace_back(new Object(p1, nullptr));


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
    tonemap(image, out, 1, 1);
    IMG_SavePNG(out, "out.png");

    //Memory cleanup
    for(auto obj: scene.objects) delete obj;
    delete scene.camera;
}
