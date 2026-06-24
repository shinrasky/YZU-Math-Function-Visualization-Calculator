cmake_minimum_required(VERSION 3.15)
project(math_calculator C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(RAYLIB_ROOT "${PROJECT_SOURCE_DIR}/third_party/raylib" CACHE PATH "Path to raylib package root")

include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${RAYLIB_ROOT}/include
)

link_directories(${RAYLIB_ROOT}/lib)

file(GLOB_RECURSE SOURCES "${PROJECT_SOURCE_DIR}/src/*.c")

add_executable(${PROJECT_NAME} ${SOURCES})

set(RAYLIB_LIB "${RAYLIB_ROOT}/lib/libraylib.a")
set(GLFW_IMPORT_LIB "${RAYLIB_ROOT}/lib/libglfw3.dll.a")

target_link_libraries(${PROJECT_NAME}
    ${RAYLIB_LIB}
    ${GLFW_IMPORT_LIB}
    winmm
    gdi32
    opengl32
)

if(MINGW)
    target_link_libraries(${PROJECT_NAME} m)
endif()

if(EXISTS "${RAYLIB_ROOT}/lib/libraylib.dll")
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${RAYLIB_ROOT}/lib/libraylib.dll"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>/libraylib.dll")
endif()

if(EXISTS "${RAYLIB_ROOT}/lib/glfw3.dll")
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${RAYLIB_ROOT}/lib/glfw3.dll"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>/glfw3.dll")
endif()
