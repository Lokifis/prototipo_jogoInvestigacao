cmake_minimum_required(VERSION 3.16)
project(JogoInvestigacao VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Tipo de build" FORCE)
endif()

# Gera compile_commands.json (demais IDEs)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ---------------------------------------------------------------
# Dependências
# GLFW e GLM
# GLAD
# ---------------------------------------------------------------
find_package(OpenGL REQUIRED)
find_package(glfw3 3.3 QUIET)

include(FetchContent)

if(NOT glfw3_FOUND)
    message(STATUS "GLFW não encontrada no sistema, baixando...")
    set(GLFW_BUILD_DOCS     OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG        3.4
    )
    FetchContent_MakeAvailable(glfw)
endif()

find_package(glm QUIET)
if(NOT glm_FOUND)
    message(STATUS "GLM não encontrada no sistema, baixando...")
    FetchContent_Declare(
        glm
        GIT_REPOSITORY https://github.com/g-truc/glm.git
        GIT_TAG        1.0.1
    )
    FetchContent_MakeAvailable(glm)
endif()

# ---------------------------------------------------------------
# Fontes
#   main.cpp
#   include/   (glad/, KHR/, graphics/, world/)
#   src/       (glad.c, graphics/*.cpp, world/*.cpp)
#   shaders/   (scene.*, post.*, ui.*)
#   assets/    (data/clues.txt, data/rooms.txt)
#   docs/      (UML.md)
# ---------------------------------------------------------------
file(GLOB_RECURSE PROJECT_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/*.c"
)

add_executable(${PROJECT_NAME} main.cpp ${PROJECT_SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}          # permite #include "include/graphics/Shader.h"
    ${CMAKE_SOURCE_DIR}/include  # permite o glad.h achar <KHR/khrplatform.h>
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    ASSET_DIR="${CMAKE_SOURCE_DIR}/"
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    OpenGL::GL
    glfw
    glm::glm
)

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_SOURCE_DIR}/shaders"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/shaders"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_SOURCE_DIR}/assets"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/assets"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>/assets"
)
