cmake_minimum_required(VERSION 3.13)
project(mandelbrot-gl)

file(GLOB sources *.cpp)
file(GLOB headers *.hpp)
file (GLOB shaders *.glsl)

set (INCLUDE_DIR "${CMAKE_SOURCE_DIR}/include")
source_group("Shaders" FILES "${shaders}")

add_executable(mandelbrot-gl Shader.cpp mandelbrot-gl.cpp ${headers} ${shaders} "glad.c")
target_include_directories(mandelbrot-gl PUBLIC
        "${INCLUDE_DIR}/"
        "${INCLUDE_DIR}/CoreCpp/")
target_link_directories(mandelbrot-gl PUBLIC "${CMAKE_SOURCE_DIR}/lib")
target_link_libraries( mandelbrot-gl glew32s glfw3 opengl32)

add_executable(pendulum pendulums.cpp)
target_include_directories(pendulum PUBLIC
        "${INCLUDE_DIR}/"
        "${INCLUDE_DIR}/CoreCpp/"
        "${CMAKE_SOURCE_DIR}/../Eigen/eigen")
#target_link_libraries(pendulum)