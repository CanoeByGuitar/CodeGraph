file(GLOB SOURCE_FILES
        *.cpp
        *.h)


add_executable(main ${SOURCE_FILES})
target_include_directories(main PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(main PUBLIC glfw imgui glad glm)