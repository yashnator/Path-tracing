#include "scene.hpp"
Camera::Camera(): fov(60), width(800), height(600)
{
    center = glm::vec3(0.0f);
    view = glm::vec3(0.0f, 0.0f, -1.0f);
    up = glm::vec3(0.0f, 1.0f, 0.0f);
    right = glm::vec3(1.0f,0.0f,0.0f);
}

void Camera::transformCamera(glm::mat4 transform)
{
    center = glm::vec3(transform * glm::vec4(center, 1.0f));
    view = glm::vec3(transform * glm::vec4(view, 0.0f));
    up = glm::vec3(transform * glm::vec4(up, 0.0f));
    right = glm::vec3(transform * glm::vec4(right, 0.0f));
    
    //Normalize
    view = glm::normalize(view);
    up = glm::normalize(up);
    right = glm::normalize(right);
}

Ray Camera::make_ray(float x, float y) const {
    float pi = glm::pi<float>();
    float aspect_ratio = float(width) / height;   // Correct aspect ratio
    float scale = tan(fov * 0.5f * pi / 180.0f); // Convert FOV to radians and compute scaling
    // Compute ray direction in camera space

    glm::vec3 ray_dir = glm::vec3(0.0f);

    ray_dir += right * x * aspect_ratio * scale;    
    ray_dir += up * y * scale;
    ray_dir += view;
    return Ray(center, normalize(ray_dir));
    // return Ray(glm::vec3(0,0,0), glm::vec3(x, y, -1));
}

glm::vec3 Camera::getLocation()
{
    return center;
}

void Camera::debugCamera()
{
    std::cout<<"Camera: "<<std::endl;
    std::cout<<"Center: "<<center.x<<","<<center.y<<","<<center.z<<std::endl;
    std::cout<<"View: "<<view.x<<","<<view.y<<","<<view.z<<std::endl;
    std::cout<<"Up: "<<up.x<<","<<up.y<<","<<up.z<<std::endl;
    std::cout<<"Right: "<<right.x<<","<<right.y<<","<<right.z<<std::endl;
}