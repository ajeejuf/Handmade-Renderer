cmake_minimum_required(VERSION 3.13)

project(test_webgpu)

find_package(Dawn REQUIRED)
add_executable(test_webgpu src/webgpu/main.c)

target_link_libraries(test_webgpu dawn::webgpu_dawn)

add_subdirectory(lib/dawn)
add_subdirectory(lib/glfw)
add_subdirectory(lib/glfw3webgpu)
target_link_libraries(test_webgpu glfw)