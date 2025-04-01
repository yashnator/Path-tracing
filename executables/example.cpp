#include "../src/scene.hpp"
#include "../src/image.hpp"

#include <SDL2/SDL.h>
#include <iostream>

int main() {
    int w = 800, h = 600;
    HDRImage image(w, h);
    Scene scene;
    scene.camera = new Camera();
    for (int j = 0; j < h; j++) {
        for (int i = 0; i < w; i++) {
            float x = 2*(i+0.5)/w - 1;
            float y = 1 - 2*(j+0.5)/h;
            // std::cout<<"cum"<<std::endl;
            Ray ray = scene.camera->make_ray(x, y);
            // std::cout<<"ray cum"<<std::endl;
            // trace the ray and get the colour
            color c = glm::normalize(ray.d)*0.5f + 0.5f;;
            image.pixel(i, j) = c;

        }
    }
    delete scene.camera;
    SDL_Surface *out = SDL_CreateRGBSurface(0, w, h, 32, 0, 0, 0, 0);
    tonemap(image, out, 1, 1);
    IMG_SavePNG(out, "out.png");
}
