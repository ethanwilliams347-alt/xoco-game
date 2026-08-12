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
    src/scene/scene.cpp
)

# Rendering that is not SDL-side. V7's light field reads the grid and produces a
# plain ARGB buffer, so it is testable headlessly like the simulation is - but it
# is emphatically not part of ENGINE_SOURCES, because that variable's meaning is
# "what the simulation is", and the one rule V7 must not break is that light
# never becomes a simulation input. Kept separate so the day something under
# src/physics/ needs this list, the mistake is visible in the build file.
set(RENDER_SOURCES
    src/render/light.cpp
    src/render/player_anim.cpp
)

# V4's prop list parser. Its own variable rather than a line in either of the
# two above, because it belongs to neither and the distinction is the point.
# It is not ENGINE_SOURCES: a prop exercises no system, is never dug, ignited or
# displaced, and nothing under src/physics/ has any business reading this. It is
# not RENDER_SOURCES either, since it draws nothing and knows no SDL - it turns
# a text file into a list of records. Kept separate so that if something in the
# simulation ever reaches for a prop, the mistake has to be written into this
# file to compile, which is the same guard V7's light field is given above.
set(SCENE_PROP_SOURCES
    src/scene/props.cpp
    src/scene/sprites.cpp
)

# src/ui is SDL-side, like main.cpp - built only into the game executable,
# not ENGINE_SOURCES, which stays what the headless test suites link against.
add_executable(SlopPhysics src/main.cpp src/ui/text.cpp src/ui/hotbar.cpp ${ENGINE_SOURCES} ${RENDER_SOURCES} ${SCENE_PROP_SOURCES})

# Include local src directory
target_include_directories(SlopPhysics PRIVATE src)

# Link SDL2 (SDL2main is required on Windows to abstract WinMain)
target_link_libraries(SlopPhysics PRIVATE SDL2main SDL2-static)

# main.cpp loads the F4 scene by a path relative to the working directory
# ("assets/..."), not relative to the source tree, so the assets have to sit
# next to the built executable or a normal run from build/Debug/ can't find
# them. Copied post-build rather than referenced in place so the exe stays
# runnable from a copied build directory too.
#
# `copy_directory` is recursive, so `assets/wip/` - drafts, candidates and
# superseded art - would be copied next to the exe and eventually shipped. It is
# removed immediately after rather than by excluding it from the copy, because
# CMake's copy_directory has no exclude and the alternatives (globbing every
# file, or a second source directory) both break the moment someone adds an
# asset without reading this. tools/load_sprite.py's staging skips directories
# and therefore already agrees with this; the two paths must keep agreeing or
# `wip/` is only out of the build until the next full rebuild.
add_custom_command(TARGET SlopPhysics POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:SlopPhysics>/assets
    COMMAND ${CMAKE_COMMAND} -E rm -rf
        $<TARGET_FILE_DIR:SlopPhysics>/assets/wip
)

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

    add_executable(scene_test tests/test_scene.cpp ${ENGINE_SOURCES})
    target_include_directories(scene_test PRIVATE src tests)
    add_test(NAME scene_test COMMAND scene_test)

    add_executable(light_test tests/test_light.cpp ${ENGINE_SOURCES} ${RENDER_SOURCES})
    target_include_directories(light_test PRIVATE src tests)
    add_test(NAME light_test COMMAND light_test)

    add_executable(anim_test tests/test_anim.cpp ${ENGINE_SOURCES} ${RENDER_SOURCES})
    target_include_directories(anim_test PRIVATE src tests)
    add_test(NAME anim_test COMMAND anim_test)

    # V4's prop list. Links only the parser - no engine, no SDL - which is the
    # build file agreeing with what props.h claims about where this sits.
    add_executable(props_test tests/test_props.cpp ${SCENE_PROP_SOURCES})
    target_include_directories(props_test PRIVATE src tests)
    # Run from the source tree so the suite can parse the shipped
    # assets/test_props.txt, which is the fixture main.cpp actually loads.
    add_test(NAME props_test COMMAND props_test WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

    # The sprite manifest. Links the same parser-only set for the same reason:
    # it turns a text file into a list of records, knows no SDL, and nothing in
    # the simulation may read it.
    add_executable(sprites_test tests/test_sprites.cpp ${SCENE_PROP_SOURCES})
    target_include_directories(sprites_test PRIVATE src tests)
    add_test(NAME sprites_test COMMAND sprites_test)

    # Benchmark. Deliberately not an add_test() - timings are for reading, and a
    # slow machine should not fail the build.
    add_executable(grid_bench tests/bench_grid.cpp ${ENGINE_SOURCES} ${RENDER_SOURCES})
    target_include_directories(grid_bench PRIVATE src tests)

    # Headless frame preview. Not an add_test() either, and for a stronger reason
    # than the benchmark's: it asserts nothing at all. It composites backdrop,
    # cells and V7's light exactly as main.cpp does and dumps the result, so a
    # defect that is purely a *look* can be examined without a window.
    #
    # It exists because session 2's lighting blowout was invisible to all seven
    # suites - every one of them passed on a frame nobody could see through - and
    # the numbers it prints (peak channel, share of the view brightly lit, share
    # of pixels clipped to white) are what turned "looks overexposed" into
    # something with a before and an after. Pipe it through tools/rawpng.py.
    add_executable(preview_light tests/preview_light.cpp ${ENGINE_SOURCES} ${RENDER_SOURCES})
    target_include_directories(preview_light PRIVATE src tests)

    # Burn timing and burn shape, in numbers. Not an add_test() for the same
    # reason preview_light is not: what it reports is a matter of taste, and a
    # test pinning today's taste would be a copy of the constants rather than a
    # check on them. See the header comment for why it exists.
    add_executable(burn_probe tests/burn_probe.cpp ${ENGINE_SOURCES})
    target_include_directories(burn_probe PRIVATE src tests)

    # Where water ends up when a powder is poured into it, in numbers and in an
    # ASCII slice. Not an add_test() because the thing it reports is a shape.
    add_executable(water_probe tests/water_probe.cpp ${ENGINE_SOURCES})
    target_include_directories(water_probe PRIVATE src tests)

    # How much of the authored rim highlight is still on a surface once the
    # world has settled. Not an add_test() for the same reason as the two
    # above: it measures an input to an art decision, not a correctness
    # property. See the header comment for the question it answers.
    add_executable(rim_probe tests/rim_probe.cpp ${ENGINE_SOURCES})
    target_include_directories(rim_probe PRIVATE src tests)
endif()
