cmake_minimum_required(VERSION 3.8)

project(CodeGraph)
set(CMAKE_CXX_STANDARD 17)

add_compile_definitions(CURRENT_PROJECT_PATH="${CMAKE_CURRENT_SOURCE_DIR}/")




set(GLFW_BUILD_EXAMPLES OFF)
set(GLFW_BUILD_TESTS OFF)
set(GLFW_BUILD_DOCS OFF)
set(GLFW_INSTALL OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})
set(ENV{EIGEN3_ROOT_DIR} "${CMAKE_CURRENT_SOURCE_DIR}/extern/Eigen")
message(STATUS "EIGEN3_ROOT_DIR: $ENV{EIGEN3_ROOT_DIR}")

include_directories(extern/spdlog/include)
add_subdirectory(extern/glad)
add_subdirectory(extern/glfw)
add_subdirectory(extern/imgui)
add_subdirectory(extern/glm)
add_subdirectory(extern/partio)

add_subdirectory(src)



