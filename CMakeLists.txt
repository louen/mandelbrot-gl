cmake_minimum_required(VERSION 3.13)
project(mandelbrot-gl)

file(GLOB sources *.cpp)
file(GLOB headers *.hpp)


set (INCLUDE_DIR "${CMAKE_SOURCE_DIR}/../include")

add_executable(mandelbrot-gl ${sources} ${headers} "glad.c")
target_include_directories(mandelbrot-gl PUBLIC "${INCLUDE_DIR}/eigen/"  "${INCLUDE_DIR}/stb/" "${INCLUDE_DIR}/core/" "${CMAKE_SOURCE_DIR}/include")
target_link_directories(mandelbrot-gl PUBLIC "${CMAKE_SOURCE_DIR}/lib")
target_link_libraries( mandelbrot-gl glew32s glfw3 opengl32)