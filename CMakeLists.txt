cmake_minimum_required(VERSION 3.20)
project(Frost LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ======================================================
# Only build vendor if not already defined
# Prevents duplicate target errors when multiple
# projects include Frost as a submodule
# ======================================================
if(NOT TARGET glad)
    add_library(glad STATIC
        ${CMAKE_CURRENT_SOURCE_DIR}/vendor/glad/src/glad.c
     "src/Core/ThreadPool.h")
    target_include_directories(glad PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/vendor/glad/include>
    )
endif()

if(NOT TARGET glfw)
    set(GLFW_BUILD_DOCS     OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(GLFW_INSTALL        OFF CACHE BOOL "" FORCE)
    add_subdirectory(
        ${CMAKE_CURRENT_SOURCE_DIR}/vendor/glfw
        ${CMAKE_CURRENT_BINARY_DIR}/vendor/glfw
    )
endif()

if(NOT TARGET freetype)
    set(FT_DISABLE_ZLIB     ON CACHE BOOL "" FORCE)
    set(FT_DISABLE_BZIP2    ON CACHE BOOL "" FORCE)
    set(FT_DISABLE_PNG      ON CACHE BOOL "" FORCE)
    set(FT_DISABLE_HARFBUZZ ON CACHE BOOL "" FORCE)
    set(FT_DISABLE_BROTLI   ON CACHE BOOL "" FORCE)
    add_subdirectory(
        ${CMAKE_CURRENT_SOURCE_DIR}/vendor/freetype
        ${CMAKE_CURRENT_BINARY_DIR}/vendor/freetype
    )
endif()

# ======================================================
# FrostEngine library
# ======================================================
file(GLOB_RECURSE FROST_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
)

add_library(Frost STATIC ${FROST_SOURCES} "src/Core/ThreadPool.h")

# ======================================================
# Include paths — PUBLIC so consumers inherit them
# ======================================================
target_include_directories(Frost
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/vendor/stb_image>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/vendor/glad/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/vendor/glfw/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/vendor/freetype/include>
)

# ======================================================
# Platform linking
# ======================================================
if(WIN32)
    target_link_libraries(Frost
        PUBLIC
            glad
            glfw
            freetype
            opengl32
    )
elseif(UNIX)
    target_link_libraries(Frost
        PUBLIC
            glad
            glfw
            freetype
            GL
            dl
            pthread
            X11
    )
endif()

# ======================================================
# Compiler definitions
# ======================================================
target_compile_definitions(Frost
    PUBLIC
        $<$<CONFIG:Debug>:FROST_DEBUG>
        $<$<CONFIG:Release>:FROST_RELEASE>
)

# ======================================================
# Compiler warnings — PRIVATE so they don't leak
# to consumers
# ======================================================
if(MSVC)
    target_compile_options(Frost PRIVATE /W4)
else()
    target_compile_options(Frost PRIVATE
        -Wall
        -Wextra
        -Wno-unused-parameter
    )
endif()