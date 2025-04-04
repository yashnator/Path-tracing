#include "image.hpp"

void tonemap(const HDRImage &hdri, SDL_Surface* ldri,
             float exposure, float gamma) {
    int w = hdri.w, h = hdri.h;
    SDL_PixelFormat *format = ldri->format;
    Uint32 *pixels = (Uint32*)ldri->pixels;
    for (int j = 0; j < h; j++) {
        for (int i = 0; i < w; i++) {
            color c = hdri.pixel(i, j);
            c = glm::pow(c * exposure, color(1/gamma));
            c = glm::clamp(c, color(0.0), color(1.0))*255.f;
            pixels[i + w*j] = SDL_MapRGBA(format, c.r, c.g, c.b, 255);
        }
    }
}

void openImage(const char* filename) {
    #if defined(_WIN32) || defined(_WIN64)
        std::string command = "start " + std::string(filename);
    #elif defined(__APPLE__)
        std::string command = "open " + std::string(filename);
    #else // Linux / Unix
        std::string command = "xdg-open " + std::string(filename);
    #endif
        std::system(command.c_str());
    }