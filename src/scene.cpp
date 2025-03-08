#include "scene.hpp"

Ray Camera::make_ray(float x, float y) const {
    // FIXME!
    return Ray(glm::vec3(0,0,0), glm::vec3(x, y, -1));
}
