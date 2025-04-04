#include "scene.hpp"

//Probability
bool probability(float p) {
    static std::random_device rd;
    static std::mt19937 gen(rd()); // Mersenne Twister RNG
    static std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    return dist(gen) < p;
}

// Cosine-weighted sample in local space (+Z is the normal)
glm::vec3 sampleCosineHemisphereLocal() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    float u1 = dist(gen);
    float u2 = dist(gen);

    float r = sqrt(u1);
    float theta = 2.0f * glm::pi<float>() * u2;

    float x = r * cos(theta);
    float y = r * sin(theta);
    float z = sqrt(1.0f - u1);

    return glm::vec3(x, y, z); // local space
}

// Build an orthonormal basis around the normal
glm::vec3 toWorldSpace(const glm::vec3& local, const glm::vec3& normal) {
    glm::vec3 up = (fabs(normal.z) < 0.999f) ? glm::vec3(0, 0, 1) : glm::vec3(1, 0, 0);
    glm::vec3 tangent = glm::normalize(glm::cross(up, normal));
    glm::vec3 bitangent = glm::cross(normal, tangent);

    return glm::normalize(
        tangent * local.x +
        bitangent * local.y +
        normal * local.z
    );
}

// Full cosine-weighted sample in world space at a point with a given normal
glm::vec3 sampleCosineHemisphere(const glm::vec3& normal) {
    glm::vec3 local = sampleCosineHemisphereLocal();
    glm::vec3 dir = toWorldSpace(local, normal);
    //Importance sampling, so need to divide by pdf
    dir/=cosineHemispherePDF(normal, dir);
    return dir;
}

float cosineHemispherePDF(const glm::vec3& normal, const glm::vec3& dir) {
    float cosTheta = glm::dot(normal, glm::normalize(dir));
    return (cosTheta > 0.0f) ? cosTheta / glm::pi<float>() : 0.0f;
}