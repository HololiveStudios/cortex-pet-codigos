cmake_minimum_required(VERSION 3.20)
project(cortex_pet C)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

option(CORTEX_WAYLAND "Force Wayland on Linux" OFF)

include(FetchContent)
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG        3.4
    GIT_SHALLOW    TRUE
)
set(GLFW_BUILD_DOCS     OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL        OFF CACHE BOOL "" FORCE)
if(CORTEX_WAYLAND)
    set(GLFW_USE_WAYLAND ON CACHE BOOL "" FORCE)
endif()
FetchContent_MakeAvailable(glfw)

# GLAD2: gere em https://gen.glad.sh  (API: OpenGL 3.3 Core, C, gerar loader)
add_library(glad STATIC external/glad/src/gl.c)
target_include_directories(glad PUBLIC external/glad/include)

set(SRCS
    src/main.c
    src/platform/window.c
    src/renderer/pixel_draw.c
    src/renderer/face.c
    src/renderer/framebuffer.c
    src/animation/animation.c
    # Motor SNN — arquivos raiz, sem modificação
    neuro.c
    behavior.c
    homeostasis.c
    conditioning.c
    memory.c
    sensors.c
)

add_executable(cortex_pet ${SRCS})

target_include_directories(cortex_pet PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/platform
    ${CMAKE_CURRENT_SOURCE_DIR}/src/renderer
    ${CMAKE_CURRENT_SOURCE_DIR}/src/animation
)

target_link_libraries(cortex_pet PRIVATE glfw glad m)

if(UNIX AND NOT APPLE)
    target_link_libraries(cortex_pet PRIVATE dl)
endif()
if(APPLE)
    target_link_libraries(cortex_pet PRIVATE
        "-framework Cocoa" "-framework OpenGL" "-framework IOKit")
endif()
