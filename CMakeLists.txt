cmake_minimum_required(VERSION 3.25)

cmake_policy(SET CMP0069 NEW) # enforce INTERPROCEDURAL_OPTIMIZATION
set(CMAKE_POLICY_DEFAULT_CMP0069 NEW)

project(cpu_raytracer)

set(CMAKE_CXX_STANDARD 20)
#set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)

find_package(SDL2 REQUIRED)

add_compile_options(-march=native -mtune=native -Wno-deprecated-volatile)

add_executable(cpu_raytracer src/main.cpp src/types.h src/scene.h src/render.h src/render.cpp src/camera.h src/camera.cpp src/rng.h src/rng.cpp src/timer.h src/timer.cpp src/queue.h src/scene.cpp src/ray.h src/material.h src/3rdparty/stb_image_write.h)

target_compile_definitions(cpu_raytracer PUBLIC GLM_FORCE_DEPTH_ZERO_TO_ONE)
target_include_directories(cpu_raytracer PUBLIC 3rdparty/glm)
