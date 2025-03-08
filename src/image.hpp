#ifndef IMAGE_HPP
#define IMAGE_HPP

#include <glm/glm.hpp>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <vector>

using color = glm::vec3;

class HDRImage {
public:
    int w, h;
    std::vector<color> pixels;
    HDRImage(int w, int h):
        w(w),
        h(h),
        pixels(w*h) {
    }
    const color &pixel(int i, int j) const {
        return pixels[i + j*w];
    }
    color &pixel(int i, int j) {
        return pixels[i + j*w];
    }
};

void tonemap(const HDRImage &hdri, SDL_Surface* ldri,
             float exposure=1, float gamma=2.2);

#endif
