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

# The simulation is deliberately free of any SDL dependency so it can be built
# and tested headlessly. Only the shell (window, input, blitting) touches SDL.
set(ENGINE_SOURCES
    src/physics/grid.cpp
    src/physics/player.cpp
    src/physics/tool.cpp
    src/game/run.cpp
)

add_executable(SlopPhysics src/main.cpp ${ENGINE_SOURCES})

# Include local src directory
target_include_directories(SlopPhysics PRIVATE src)

# Link SDL2 (SDL2main is required on Windows to abstract WinMain)
target_link_libraries(SlopPhysics PRIVATE SDL2main SDL2-static)

# Physics tests. No test framework by design - the harness is plain C++ and
# returns a non-zero exit code on failure, which is all CTest needs.
option(BUILD_TESTING "Build the physics test harness" ON)
if(BUILD_TESTING)
    enable_testing()
    add_executable(grid_test tests/test_grid.cpp ${ENGINE_SOURCES})
    target_include_directories(grid_test PRIVATE src tests)
    add_test(NAME grid_test COMMAND grid_test)

    add_executable(player_test tests/test_player.cpp ${ENGINE_SOURCES})
    target_include_directories(player_test PRIVATE src tests)
    add_test(NAME player_test COMMAND player_test)

    add_executable(tool_test tests/test_tool.cpp ${ENGINE_SOURCES})
    target_include_directories(tool_test PRIVATE src tests)
    add_test(NAME tool_test COMMAND tool_test)

    add_executable(collapse_test tests/test_collapse.cpp ${ENGINE_SOURCES})
    target_include_directories(collapse_test PRIVATE src tests)
    add_test(NAME collapse_test COMMAND collapse_test)

    add_executable(run_test tests/test_run.cpp ${ENGINE_SOURCES})
    target_include_directories(run_test PRIVATE src tests)
    add_test(NAME run_test COMMAND run_test)

    # Benchmark. Deliberately not an add_test() - timings are for reading, and a
    # slow machine should not fail the build.
    add_executable(grid_bench tests/bench_grid.cpp ${ENGINE_SOURCES})
    target_include_directories(grid_bench PRIVATE src tests)
endif()
