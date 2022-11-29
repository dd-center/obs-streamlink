cmake_minimum_required(VERSION 3.20)
set(CMAKE_CXX_STANDARD 20)
project(obs-streamlink)

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}")

find_package(FFmpeg REQUIRED
    COMPONENTS avcodec avfilter avdevice avutil swscale avformat swresample)
find_package(Python3 3.8 EXACT REQUIRED COMPONENTS Interpreter Development)
find_library(Python3Gen NAME "python38.lib" PATHS "${Python3_LIBRARY_DIRS}" NO_DEFAULT_PATH)
#find_library(Python3Gen_Debug NAME "python38_d.lib" PATHS "${Python3_LIBRARY_DIRS}" NO_DEFAULT_PATH)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_library(obs-streamlink SHARED
    obs-streamlink.cpp
    streamlink-delayload.cpp
    streamlink-source.cpp
    media-playback/decode.cpp
    media-playback/media.cpp
    python-streamlink.cpp
)
target_include_directories(obs-streamlink PRIVATE
    "${FFMPEG_INCLUDE_DIRS}"
    "${W32_PTHREADS_INCLUDES}"
    "${LIBOBS_INCLUDE_DIR}"
    "${OBS_FRONTEND_INCLUDE_DIR}"
    "${Python3_INCLUDE_DIRS}"
    "deps/"
)

target_compile_options(obs-streamlink PUBLIC "/utf-8")
target_link_options(obs-streamlink PUBLIC
        "/NODEFAULTLIB:python38_d.lib"
        "/NODEFAULTLIB:python3_d.lib"
        "/NODEFAULTLIB:python38.lib"
        "/NODEFAULTLIB:python3.lib"

        "/NODEFAULTLIB:avcodec.lib"
        "/NODEFAULTLIB:avdevice.lib"
        "/NODEFAULTLIB:avfilter.lib"
        "/NODEFAULTLIB:avformat.lib"
        "/NODEFAULTLIB:avutil.lib"
        "/NODEFAULTLIB:swresample.lib"
        "/NODEFAULTLIB:swscale.lib"

        "/DELAYLOAD:python38.dll"
        "/DELAYLOAD:avcodec-59.dll"
        "/DELAYLOAD:avdevice-59.dll"
        "/DELAYLOAD:avfilter-8.dll"
        "/DELAYLOAD:avformat-59.dll"
        "/DELAYLOAD:avutil-57.dll"
        "/DELAYLOAD:swresample-4.dll"
        "/DELAYLOAD:swscale-6.dll"  
)

target_link_libraries(obs-streamlink PRIVATE
    "${LIBOBS_LIB}"
    "${W32_PTHREADS_LIB}"
    "${OBS_FRONTEND_LIB}"
    "${FFMPEG_LIBRARIES}"
    delayimp)
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_link_libraries(obs-streamlink PRIVATE "${Python3Gen}")
else()
    target_link_libraries(obs-streamlink PRIVATE "${Python3Gen}")
endif()

add_custom_command(TARGET obs-streamlink POST_BUILD
        # Copy to obs-studio dev environment for immediate testing
        COMMAND powershell -Command "if ($<CONFIG:Debug> -eq 1 ) { robocopy ${PROJECT_SOURCE_DIR}/out/build/x64-Debug ${OBS_RUNDIR}/obs-plugins/64bit ${OBS_ARCH_NAME}; robocopy ${PROJECT_SOURCE_DIR}/data ${OBS_RUNDIR}/data/obs-plugins/obs-streamlink /E;}"
)