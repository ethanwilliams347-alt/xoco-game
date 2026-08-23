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
    src/game/input_log.cpp
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

# V4's prop list parser, and P4's BMP reader. Its own variable rather than a
# line in either of the two above, because it belongs to neither and the
# distinction is the point.
#
# `bmp.cpp` joined it rather than ENGINE_SOURCES on the same test: it turns an
# authored file into a record (a `Scene`), knows no SDL, and nothing under
# src/physics/ may read it. It is here rather than in main.cpp - where the same
# code used to live - because P4's replayed benchmark row has to stamp the world
# the game actually plays in, and a loader inside the SDL shell is a loader no
# headless instrument can reach.
# It is not ENGINE_SOURCES: a prop exercises no system, is never dug, ignited or
# displaced, and nothing under src/physics/ has any business reading this. It is
# not RENDER_SOURCES either, since it draws nothing and knows no SDL - it turns
# a text file into a list of records. Kept separate so that if something in the
# simulation ever reaches for a prop, the mistake has to be written into this
# file to compile, which is the same guard V7's light field is given above.
set(SCENE_PROP_SOURCES
    src/scene/props.cpp
    src/scene/sprites.cpp
    src/scene/bmp.cpp
)

# V17's frame composition. **A fifth variable rather than a line in
# RENDER_SOURCES, and the difference is SDL.** Everything in RENDER_SOURCES is
# SDL-free by rule, and frame.cpp is nothing but SDL draw calls. Folding it in
# there would quietly make everything that links RENDER_SOURCES link SDL2 -
# today light_test, anim_test, grid_bench and preview_light, all four of which
# are headless and must stay so - and the day one of them wants to be run
# somewhere without SDL, nobody would be able to say when that started. **This
# comment said "nine headless suites" until 2026-08-18 and that was never a
# number anything counted**; it is written as the set now, because a set is what
# a grep can check and a count is what goes stale.
#
# It is not ENGINE_SOURCES for the reason light.cpp is not: this is rendering,
# and rendering must never become a simulation input. The guard the other four
# variables enforce is exactly this one - keep it separate, and the day
# something under src/physics/ reaches for the frame, the mistake has to be
# written into this file to compile.
#
# **src/ui joined this variable at W5 part 3 (2026-08-18), and so did
# render/overlay.cpp.** They were built only into the game executable, which
# meant the ~250 lines of HUD, reticle, wash and settings drawing were the
# largest block in the project that no test linked. They belong here on this
# variable's own test - SDL draw calls and nothing else - and putting them here
# is what puts them inside golden_frame_test. The name is now slightly narrower
# than the contents; the test the contents pass is the one in the paragraph
# above, not the word "frame".
set(FRAME_SOURCES
    src/render/frame.cpp
    src/render/overlay.cpp
    src/ui/text.cpp
    src/ui/hotbar.cpp
)

# src/ui is SDL-side, like main.cpp - built only into the game executable,
# not ENGINE_SOURCES, which stays what the headless test suites link against.
add_executable(SlopPhysics src/main.cpp ${ENGINE_SOURCES} ${RENDER_SOURCES} ${SCENE_PROP_SOURCES} ${FRAME_SOURCES})

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

    # T1's debug tooling. Links only ENGINE_SOURCES because src/game/debug_view.h
    # is a header - it deliberately does not become a fourth source variable, so
    # the guard the three above enforce is untouched. What it needs from the
    # engine is a Grid to inspect.
    add_executable(debug_test tests/test_debug.cpp ${ENGINE_SOURCES})
    target_include_directories(debug_test PRIVATE src tests)
    add_test(NAME debug_test COMMAND debug_test)

    # Links the BMP reader too, and runs from the source tree, because part of
    # what this suite now checks is that the *shipped* fixture scene still loads
    # to the cell count P4's session log was recorded against.
    add_executable(scene_test tests/test_scene.cpp ${ENGINE_SOURCES} ${SCENE_PROP_SOURCES})
    target_include_directories(scene_test PRIVATE src tests)
    add_test(NAME scene_test COMMAND scene_test WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

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

    # V19's wrapping backdrop arithmetic. **Links no source set at all** - the
    # narrowest link line in the project, because src/render/backdrop_wrap.h is
    # a header of pure arithmetic and nothing else. That is not an accident of
    # how it was written: the size relationship between a backdrop layer and the
    # pan range is what V16 retires, and the replacement's one hard part is
    # where the copies go, which needs no window to check. It sits next to
    # golden_frame_test and links a fraction of what it does, which is the split
    # worth preserving if this file grows a wrapping *draw* test later - that one
    # would belong in the golden frame, not here.
    add_executable(backdrop_test tests/test_backdrop.cpp)
    target_include_directories(backdrop_test PRIVATE src tests)
    add_test(NAME backdrop_test COMMAND backdrop_test)

    # The camera's view arithmetic: the centred framing, the world-edge clamps,
    # and the anchor mechanism the clamps have to survive. Links no sources -
    # `camera.h` is an SDL-free header. Was `camera_bias_test` until V23b
    # retired the moving anchor; the Camera half of that suite is what this is.
    add_executable(camera_test tests/test_camera.cpp)
    target_include_directories(camera_test PRIVATE src tests)
    add_test(NAME camera_test COMMAND camera_test)

    # V17's golden frame. **The first test target in the project that links
    # SDL**, and that is worth saying out loud rather than leaving to be noticed
    # from the link line: every suite above it is headless by construction, and
    # this one is not - it composes a real frame with the real draw calls, into
    # a software renderer over an SDL_Surface. It needs no display and no video
    # subsystem (see the header comment), so it still runs on a build machine,
    # but it is no longer true that "the test suites do not link SDL" and the
    # next person to write that sentence should find this comment first.
    #
    # SDL2main is deliberately *not* linked. It exists to hand Windows' WinMain
    # to a game; this has a plain main() and a console, and linking it would
    # replace that entry point.
    add_executable(golden_frame_test tests/test_golden_frame.cpp
                   ${ENGINE_SOURCES} ${RENDER_SOURCES} ${FRAME_SOURCES})
    target_include_directories(golden_frame_test PRIVATE src tests)
    target_link_libraries(golden_frame_test PRIVATE SDL2-static)
    add_test(NAME golden_frame_test COMMAND golden_frame_test)

    # W5's startup decisions - the terrain scans that plant the objective and
    # the props, and which display mode to open at. **This is the suite the item
    # was admitted on**: every check in it used to run inside `main()`, print a
    # line to stdout, and be verifiable only by a person reading that line.
    #
    # `game/boot.h` is a header for the same reason `game/debug_view.h` is - a
    # sixth source-set variable is a sixth chance to blur the guard the other
    # five enforce - so this links the two existing sets it needs and adds
    # nothing to the build graph. SCENE_PROP_SOURCES is here, and
    # WORKING_DIRECTORY with it, because half the suite runs the *shipped*
    # fixture: the scene BMPs, the prop list and the prop sprites, which is what
    # turns the launch check into an assertion rather than a re-statement of it.
    #
    # One suite rather than two, with `choose_display_mode` in it, because the
    # two are one extraction: they are the decisions startup takes before the
    # first frame, split from the SDL that answers questions for them.
    add_executable(boot_test tests/test_boot.cpp ${ENGINE_SOURCES} ${SCENE_PROP_SOURCES})
    target_include_directories(boot_test PRIVATE src tests)
    add_test(NAME boot_test COMMAND boot_test WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

    # W5 part 2's shell decisions - the frame pacer's freeze rule, its step
    # count and interpolation alpha, and the settings menu's navigation and
    # selection. All of it used to live inside `main()`'s frame loop or inside
    # an SDL event switch, which is the definition of unreachable here.
    #
    # `game/pacer.h` and `game/settings_menu.h` are headers for `debug_view.h`'s
    # reason, so again nothing is added to the build graph. No WORKING_DIRECTORY
    # and no SCENE_PROP_SOURCES: unlike boot_test this suite opens no files, and
    # ENGINE_SOURCES is here only because pacer.h states its step against
    # `Run::FIXED_DT` - which is the point, the pacer and the physics must not be
    # able to disagree about how long a step is.
    add_executable(shell_test tests/test_shell.cpp ${ENGINE_SOURCES})
    target_include_directories(shell_test PRIVATE src tests)
    add_test(NAME shell_test COMMAND shell_test)

    # W3's doc-truth suite. Links no sources - it includes `element.h` for a
    # `sizeof` and otherwise reads files - and it is registered *last* on
    # purpose, so that the suite count it asserts is the one a reader sees at
    # the bottom of `ctest`'s output. WORKING_DIRECTORY, like scene_test,
    # because every path it opens is repo-relative.
    add_executable(docs_test tests/test_docs.cpp)
    target_include_directories(docs_test PRIVATE src tests)
    add_test(NAME docs_test COMMAND docs_test WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

    # Benchmark. Deliberately not an add_test() - timings are for reading, and a
    # slow machine should not fail the build.
    # Links SCENE_PROP_SOURCES as of P4: the replayed row rebuilds the fixture
    # scene through the same loader the game uses.
    add_executable(grid_bench tests/bench_grid.cpp ${ENGINE_SOURCES} ${RENDER_SOURCES} ${SCENE_PROP_SOURCES})
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

    # The instrument for the `Element::ticks` representation question - what a
    # second byte costs, and whether a packed integer velocity can carry a
    # gravity term. Not an add_test() for a stronger reason than the probes
    # above: every representation it flies is one the engine does not have, so a
    # test over it would pin a prototype rather than check the product.
    add_executable(velocity_probe tests/velocity_probe.cpp ${ENGINE_SOURCES})
    target_include_directories(velocity_probe PRIVATE src tests)

    # Where the receding ground plane's band lands on screen, and how much of it
    # the fixture's own terrain covers up. Not an add_test() for the same reason
    # the four above are not: it reports an input to a composition decision.
    #
    # **It is the one probe that reaches into src/render/, and it does so only
    # through the two headers that are arithmetic** - backdrop_layers.h and
    # backdrop_wrap.h, both of which are header-only and SDL-free by design.
    # RENDER_SOURCES is deliberately not linked: the moment this needs a .cpp
    # from the renderer it has stopped being a headless instrument, and that
    # should be a link error rather than a habit.
    add_executable(plane_probe tests/plane_probe.cpp ${ENGINE_SOURCES})
    target_include_directories(plane_probe PRIVATE src tests)
endif()
