cmake_minimum_required(VERSION 3.0)

project(a3)
set(CMAKE_CXX_STANDARD 11)
cmake_policy(SET CMP0072 NEW)

find_package(glm REQUIRED)
find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)

add_library(ray_tracer src/scene.cpp src/image.cpp src/camera.cpp src/objects.cpp src/materials.cpp src/path_tracing_source.cpp)
target_link_libraries(ray_tracer glm::glm)

add_compile_options(-O3 -Wall)

add_executable(example executables/example.cpp)
add_executable(p3 executables/p3.cpp)
add_executable(p5 executables/p5.cpp)
add_executable(pathtr executables/path_tracing.cpp)
target_link_libraries(example ray_tracer SDL2::SDL2 SDL2_image::SDL2_image)
target_link_libraries(p3 ray_tracer SDL2::SDL2 SDL2_image::SDL2_image)
target_link_libraries(p5 ray_tracer SDL2::SDL2 SDL2_image::SDL2_image)
target_link_libraries(pathtr ray_tracer SDL2::SDL2 SDL2_image::SDL2_image)