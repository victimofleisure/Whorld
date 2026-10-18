cmake_minimum_required(VERSION 3.16)
project(WhorldLinux LANGUAGES CXX)

if(NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message(FATAL_ERROR "This target is the native Linux frontend; the original Windows project is in trunk/")
endif()
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Opt-in hardening build: AddressSanitizer + UndefinedBehaviorSanitizer for every
# target, so `ctest` can run the whole suite against instrumented code. Off by
# default, so the normal build (and the release binaries) are untouched. The
# third-party GPU/SDL stack (Mesa, X11, ALSA) can leak on teardown; run the
# sanitized suite with `ASAN_OPTIONS=detect_leaks=0` (or an LSan suppression
# file) to ignore those and still catch memory errors in Whorld itself:
#     cmake -S . -B build-asan -DWHORLD_SANITIZE=ON
#     cmake --build build-asan -j4
#     ASAN_OPTIONS=detect_leaks=0 ctest --test-dir build-asan --output-on-failure
option(WHORLD_SANITIZE "Build with AddressSanitizer and UndefinedBehaviorSanitizer" OFF)
if(WHORLD_SANITIZE)
    add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer -g)
    add_link_options(-fsanitize=address,undefined)
endif()

find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED IMPORTED_TARGET sdl2)
pkg_check_modules(CAIRO REQUIRED IMPORTED_TARGET cairo)
pkg_check_modules(ALSA REQUIRED IMPORTED_TARGET alsa)
pkg_check_modules(GL REQUIRED IMPORTED_TARGET gl)

add_library(whorld_core
    linux/whorld_core.cpp
    linux/playlist.cpp
    linux/osc.cpp
    linux/gamepad.cpp
    linux/midi_decode.cpp
    linux/snapshot.cpp
    linux/movie.cpp
    linux/midi_actions.cpp
    linux/projection.cpp
    linux/config.cpp
)

target_include_directories(whorld_core PUBLIC linux)
target_compile_options(whorld_core PRIVATE -Wall -Wextra)

add_library(whorld_midi STATIC linux/alsa_midi.cpp)
target_include_directories(whorld_midi PUBLIC linux)
target_link_libraries(whorld_midi PRIVATE PkgConfig::ALSA)
target_compile_options(whorld_midi PRIVATE -Wall -Wextra)

add_executable(whorld linux/main.cpp linux/gl_renderer.cpp linux/cairo_canvas.cpp linux/video_streamer.cpp
    linux/audio_input.cpp linux/audio_analysis.cpp)
target_link_libraries(whorld PRIVATE whorld_core whorld_midi PkgConfig::SDL2 PkgConfig::CAIRO PkgConfig::GL
    PkgConfig::ALSA)
target_compile_options(whorld PRIVATE -Wall -Wextra)

include(CTest)
if(BUILD_TESTING)
    add_executable(whorld_core_test linux/test_core.cpp)
    target_link_libraries(whorld_core_test PRIVATE whorld_core)
    add_test(NAME whorld_core COMMAND whorld_core_test)

    add_executable(whorld_media_test linux/test_media.cpp)
    target_link_libraries(whorld_media_test PRIVATE whorld_core)
    add_test(NAME whorld_media COMMAND whorld_media_test)

    # Discovery degrades gracefully when /dev/snd/seq is missing (prints a clear
    # message and still exits 0), so this smoke test is reliable without hardware.
    add_test(NAME whorld_list_midi COMMAND whorld --list-midi)

    add_executable(whorld_audio_test linux/test_audio.cpp linux/audio_analysis.cpp)
    target_link_libraries(whorld_audio_test PRIVATE whorld_core)
    add_test(NAME whorld_audio COMMAND whorld_audio_test)

    add_test(NAME whorld_image COMMAND whorld --patch ${CMAKE_SOURCE_DIR}/linux/demo.whp
        --render ${CMAKE_CURRENT_BINARY_DIR}/test-render.png --frames 120 --size 480x360)

    add_test(NAME whorld_playlist_image COMMAND whorld --playlist ${CMAKE_SOURCE_DIR}/linux/demo.whl
        --render ${CMAKE_CURRENT_BINARY_DIR}/test-playlist.png --frames 60 --size 480x360)

    # --render must honour post-processing instead of silently ignoring it.
    add_test(NAME whorld_post_image COMMAND whorld --patch ${CMAKE_SOURCE_DIR}/linux/demo.whp
        --mirror quad --render ${CMAKE_CURRENT_BINARY_DIR}/test-mirror.png --frames 30 --size 320x240)

    # Projection mapping (--warp) must work in the offscreen --render path too.
    add_test(NAME whorld_warp_image COMMAND whorld --patch ${CMAKE_SOURCE_DIR}/linux/demo.whp
        --warp "0.06,0.04,0.95,0.10,0.90,0.96,0.10,0.88"
        --render ${CMAKE_CURRENT_BINARY_DIR}/test-warp.png --frames 30 --size 320x240)

    # Display-less GPU rendering: --render --renderer gpu with the offscreen SDL
    # video driver must produce a PNG with no X server (the kiosk case). If this
    # SDL build has no usable offscreen driver or GL, the run exits 77 and the
    # test is skipped rather than failing an otherwise healthy machine.
    add_test(NAME whorld_headless_gpu COMMAND sh -c
        "$<TARGET_FILE:whorld> --patch '${CMAKE_SOURCE_DIR}/linux/demo.whp' --render '${CMAKE_CURRENT_BINARY_DIR}/test-headless.png' --frames 5 --size 160x120 --renderer gpu || exit 77")
    set_tests_properties(whorld_headless_gpu PROPERTIES
        ENVIRONMENT "SDL_VIDEODRIVER=offscreen"
        SKIP_RETURN_CODE 77)

    # CPU-only cairo post-processing unit test (no SDL/OpenGL, so never skipped).
    add_executable(whorld_post_test linux/test_post.cpp linux/cairo_canvas.cpp)
    target_link_libraries(whorld_post_test PRIVATE whorld_core PkgConfig::CAIRO)
    target_compile_options(whorld_post_test PRIVATE -Wall -Wextra)
    add_test(NAME whorld_post COMMAND whorld_post_test)

    # Pure-math projection mapping (quad -> homography) unit test; no GL needed.
    add_executable(whorld_projection_test linux/test_projection.cpp)
    target_link_libraries(whorld_projection_test PRIVATE whorld_core)
    target_compile_options(whorld_projection_test PRIVATE -Wall -Wextra)
    add_test(NAME whorld_projection COMMAND whorld_projection_test)

    # Show-config INI parser and ShowConfig (de)serialization; pure, no GL/SDL.
    add_executable(whorld_config_test linux/test_config.cpp)
    target_link_libraries(whorld_config_test PRIVATE whorld_core)
    target_compile_options(whorld_config_test PRIVATE -Wall -Wextra)
    add_test(NAME whorld_config COMMAND whorld_config_test)

    # --config-save captures the effective settings, and --config reads them back
    # as defaults. The capture needs no display; the reload renders one PNG on
    # the CPU path, so both are deterministic on any machine.
    add_test(NAME whorld_config_save COMMAND whorld --patch ${CMAKE_SOURCE_DIR}/linux/demo.whp
        --mirror quad --blend screen --morph 1.5
        --config-save ${CMAKE_CURRENT_BINARY_DIR}/test-show.conf)
    set_tests_properties(whorld_config_save PROPERTIES FIXTURES_SETUP whorld_config)
    add_test(NAME whorld_config_image COMMAND whorld --config ${CMAKE_CURRENT_BINARY_DIR}/test-show.conf
        --renderer cairo --render ${CMAKE_CURRENT_BINARY_DIR}/test-config.png
        --frames 30 --size 320x240)
    set_tests_properties(whorld_config_image PROPERTIES FIXTURES_REQUIRED whorld_config)

    # OSC parsing, convention addressing and a localhost UDP loopback.
    add_executable(whorld_osc_test linux/test_osc.cpp)
    target_link_libraries(whorld_osc_test PRIVATE whorld_core)
    target_compile_options(whorld_osc_test PRIVATE -Wall -Wextra)
    add_test(NAME whorld_osc COMMAND whorld_osc_test)

    add_test(NAME whorld_snapshot_fixture COMMAND whorld_media_test --write-snapshot
        ${CMAKE_CURRENT_BINARY_DIR}/fixture.whs)
    set_tests_properties(whorld_snapshot_fixture PROPERTIES FIXTURES_SETUP whorld_snapshot)
    add_test(NAME whorld_snapshot_image COMMAND whorld --snapshot ${CMAKE_CURRENT_BINARY_DIR}/fixture.whs
        --render ${CMAKE_CURRENT_BINARY_DIR}/test-snapshot.png --frames 60 --size 480x360)
    set_tests_properties(whorld_snapshot_image PROPERTIES FIXTURES_REQUIRED whorld_snapshot)

    # Record a WHM2 movie, then render one of its frames back to an image.
    add_test(NAME whorld_movie COMMAND whorld --patch ${CMAKE_SOURCE_DIR}/linux/demo.whp
        --movie-out ${CMAKE_CURRENT_BINARY_DIR}/test.whm --frames 30 --size 320x240 --movie-rate 30)
    set_tests_properties(whorld_movie PROPERTIES FIXTURES_SETUP whorld_movie)
    add_test(NAME whorld_movie_frame COMMAND whorld --movie ${CMAKE_CURRENT_BINARY_DIR}/test.whm
        --movie-frame 10 --render ${CMAKE_CURRENT_BINARY_DIR}/test-movie-frame.png --size 480x360)
    set_tests_properties(whorld_movie_frame PROPERTIES FIXTURES_REQUIRED whorld_movie)

    # The GPU canvas must match the cairo reference; skipped without OpenGL.
    add_executable(whorld_gpu_test linux/test_gpu.cpp linux/gl_renderer.cpp linux/cairo_canvas.cpp linux/video_streamer.cpp)
    target_link_libraries(whorld_gpu_test PRIVATE whorld_core
        PkgConfig::SDL2 PkgConfig::CAIRO PkgConfig::GL)
    target_compile_options(whorld_gpu_test PRIVATE -Wall -Wextra)
    add_test(NAME whorld_gpu COMMAND whorld_gpu_test ${CMAKE_SOURCE_DIR}/linux/demo.whp)
    set_tests_properties(whorld_gpu PROPERTIES SKIP_RETURN_CODE 77)

    # Command-line smoke/error-path matrix: --help, bad --warp/--config, unknown
    # flags, a --config-save/--config round trip rendered deterministically with
    # the cairo backend under the offscreen SDL driver, and --list-midi. Passes
    # the built binary, a private temp working directory and the source linux
    # directory (for demo.whp). Needs no display and no GPU.
    add_test(NAME whorld_cli_smoke COMMAND bash
        ${CMAKE_CURRENT_SOURCE_DIR}/linux/cli_smoke.sh
        $<TARGET_FILE:whorld> ${CMAKE_CURRENT_BINARY_DIR}/cli-smoke
        ${CMAKE_CURRENT_SOURCE_DIR}/linux)
    set_tests_properties(whorld_cli_smoke PROPERTIES
        ENVIRONMENT "SDL_VIDEODRIVER=offscreen" TIMEOUT 120)

    # Under the sanitizer build the SDL/GL/ALSA/X11 teardown paths of the
    # third-party stack (Mesa, X11, ALSA) can report leaks that Whorld cannot
    # fix. Ignore leaks for the tests that pull those libraries in so the
    # sanitized suite stays green; the pure CPU tests keep full leak detection.
    if(WHORLD_SANITIZE)
        set_tests_properties(
            whorld_list_midi whorld_image whorld_playlist_image whorld_post_image
            whorld_warp_image whorld_config_save whorld_config_image
            whorld_snapshot_fixture whorld_snapshot_image whorld_movie whorld_movie_frame
            whorld_gpu
            PROPERTIES ENVIRONMENT "ASAN_OPTIONS=detect_leaks=0")
        set_tests_properties(whorld_headless_gpu PROPERTIES
            ENVIRONMENT "SDL_VIDEODRIVER=offscreen;ASAN_OPTIONS=detect_leaks=0")
        set_tests_properties(whorld_cli_smoke PROPERTIES
            ENVIRONMENT "SDL_VIDEODRIVER=offscreen;ASAN_OPTIONS=detect_leaks=0")
    endif()
endif()

install(TARGETS whorld RUNTIME DESTINATION bin)
install(FILES linux/whorld.desktop DESTINATION share/applications)
install(FILES linux/whorld.svg DESTINATION share/icons/hicolor/scalable/apps)
