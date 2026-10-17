cmake_minimum_required(VERSION 3.16)
project(WhorldLinux LANGUAGES CXX)

if(NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message(FATAL_ERROR "This target is the native Linux frontend; the original Windows project is in trunk/")
endif()
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED IMPORTED_TARGET sdl2)
pkg_check_modules(CAIRO REQUIRED IMPORTED_TARGET cairo)
pkg_check_modules(ALSA REQUIRED IMPORTED_TARGET alsa)
pkg_check_modules(GL REQUIRED IMPORTED_TARGET gl)

add_library(whorld_core
    linux/whorld_core.cpp
    linux/playlist.cpp
    linux/gamepad.cpp
    linux/midi_decode.cpp
    linux/snapshot.cpp
    linux/movie.cpp
    linux/midi_actions.cpp
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

    # CPU-only cairo post-processing unit test (no SDL/OpenGL, so never skipped).
    add_executable(whorld_post_test linux/test_post.cpp linux/cairo_canvas.cpp)
    target_link_libraries(whorld_post_test PRIVATE whorld_core PkgConfig::CAIRO)
    target_compile_options(whorld_post_test PRIVATE -Wall -Wextra)
    add_test(NAME whorld_post COMMAND whorld_post_test)

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
endif()

install(TARGETS whorld RUNTIME DESTINATION bin)
install(FILES linux/whorld.desktop DESTINATION share/applications)
install(FILES linux/whorld.svg DESTINATION share/icons/hicolor/scalable/apps)
