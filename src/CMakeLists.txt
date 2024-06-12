file(GLOB SOURCE_FILES
        Draw.cpp
        imgui_impl_glfw.cpp
        imgui_impl_opengl3.cpp

        Draw.h
        HierarchyCallStack.h
        HybridDraw.h
        imgui_impl_glfw.h
        imgui_impl_opengl3.h
        )


add_executable(main ${SOURCE_FILES} main.cpp)
target_include_directories(main PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} Eigen3::Eigen)
target_link_libraries(main PUBLIC glfw imgui glad glm Eigen3::Eigen)


find_package(Eigen3 REQUIRED)

add_executable(sph ${SOURCE_FILES} sph_particle.cpp)
target_include_directories(sph PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} Eigen3::Eigen)
target_link_libraries(sph PUBLIC glfw imgui glad glm Eigen3::Eigen partio)

add_executable(svd svd.cpp)
target_include_directories(svd PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} Eigen3::Eigen)
target_link_libraries(svd PUBLIC  Eigen3::Eigen)