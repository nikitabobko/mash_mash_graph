cmake_minimum_required(VERSION 3.5)
project(task-02)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES
        common.h
        glad.c
        main.cpp
        ShaderProgram.h
        ShaderProgram.cpp
        core.cpp util.cpp util.h)

set(ADDITIONAL_INCLUDE_DIRS dependencies/include/GLAD dependencies/include/SOIL)
set(ADDITIONAL_LIBRARY_DIRS dependencies/lib)
set(ADDITIONAL_RUNTIME_LIBRARY_DIRS dependencies/bin)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/res DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

link_directories(dependencies/lib)

if (WIN32)

else ()
    list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules")
    find_package(glfw3 REQUIRED)
endif ()

include_directories(${ADDITIONAL_INCLUDE_DIRS})

find_package(OpenGL REQUIRED)

find_package(assimp REQUIRED)

#find_package(soil REQUIRED)

add_executable(main ${SOURCE_FILES})

target_include_directories(main PRIVATE ${OPENGL_INCLUDE_DIR})
add_custom_command(TARGET main POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory "${PROJECT_SOURCE_DIR}/shaders" "${PROJECT_BINARY_DIR}")

if (WIN32)
    add_custom_command(TARGET main POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory "${PROJECT_SOURCE_DIR}/dependencies/bin" $<TARGET_FILE_DIR:main>)
    #set(CMAKE_MSVCIDE_RUN_PATH ${ADDITIONAL_RUNTIME_LIBRARY_DIRS})
    target_compile_options(main PRIVATE)
    target_link_libraries(main LINK_PUBLIC ${OPENGL_gl_LIBRARY} glfw3dll)
else ()
    target_compile_options(main PRIVATE -Wnarrowing)
    target_link_libraries(main LINK_PUBLIC ${OPENGL_gl_LIBRARY} glfw rt dl SOIL assimp)
endif ()

