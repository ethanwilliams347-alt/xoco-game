cmake_minimum_required(VERSION 3.14)
project(SlopPhysics VERSION 1.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Use FetchContent to automatically download and build SDL2
# This ensures it compiles on any machine without needing manual library installation
include(FetchContent)
FetchContent_Declare(
    SDL2
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG release-2.30.0
)

# Build SDL as a static library to avoid needing to copy DLLs around
set(SDL_SHARED OFF CACHE BOOL "Build SDL shared library" FORCE)
set(SDL_STATIC ON CACHE BOOL "Build SDL static library" FORCE)
FetchContent_MakeAvailable(SDL2)

# Source files
set(SOURCES
    src/main.cpp
    src/physics/grid.cpp
)

add_executable(SlopPhysics ${SOURCES})

# Include local src directory
target_include_directories(SlopPhysics PRIVATE src)

# Link SDL2 (SDL2main is required on Windows to abstract WinMain)
target_link_libraries(SlopPhysics PRIVATE SDL2main SDL2-static)
