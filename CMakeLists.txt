cmake_minimum_required(VERSION 3.0)

project(a3)
set(CMAKE_CXX_STANDARD 11)
cmake_policy(SET CMP0072 NEW)

find_package(glm REQUIRED)
find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)

add_library(ray_tracer src/scene.cpp src/image.cpp)
target_link_libraries(ray_tracer glm::glm)

add_compile_options(-O3 -Wall)

add_executable(example executables/example.cpp)
target_link_libraries(example ray_tracer SDL2::SDL2 SDL2_image::SDL2_image)
