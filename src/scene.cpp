#include "scene.hpp"

//Camera functions
Camera::Camera(float fov, float width, float height) : fov(fov), width(width), height(height){};
//Default constructor
Camera::Camera(): fov(60), width(800), height(600){};

Ray Camera::make_ray(float x, float y) const {
    float pi = glm::pi<float>();
    float aspect_ratio = float(width) / height;   // Correct aspect ratio
    float scale = tan(fov * 0.5f * pi / 180.0f); // Convert FOV to radians and compute scaling
    // Compute ray direction in camera space
    glm::vec3 ray_dir(
        x * aspect_ratio * scale,  // Adjust x for aspect ratio
        y * scale,                 // Adjust y for FOV scaling
        -1                         // Always points into the scene (negative z-direction)
    );

    return Ray(glm::vec3(0, 0, 0), glm::normalize(ray_dir));
    // return Ray(glm::vec3(0,0,0), glm::vec3(x, y, -1));
}
