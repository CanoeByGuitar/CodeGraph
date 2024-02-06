cmake_minimum_required(VERSION 3.8)

project(CodeGraph)
set(CMAKE_CXX_STANDARD 17)

add_compile_definitions(CURRENT_PROJECT_PATH="${CMAKE_CURRENT_SOURCE_DIR}/")

set(GLFW_BUILD_EXAMPLES OFF)
set(GLFW_BUILD_TESTS OFF)
set(GLFW_BUILD_DOCS OFF)
set(GLFW_INSTALL OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)

include_directories(extern/spdlog/include)

add_subdirectory(extern/glad)
add_subdirectory(extern/glfw)
add_subdirectory(extern/imgui)
add_subdirectory(extern/glm)

add_subdirectory(src)



