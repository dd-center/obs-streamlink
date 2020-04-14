cmake_minimum_required(VERSION 3.5)
set(CMAKE_CXX_STANDARD 17)
project(obs-streamlink)

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}")

find_package(FFmpeg REQUIRED
	COMPONENTS avcodec avfilter avdevice avutil swscale avformat swresample)
find_package(Python3 COMPONENTS Interpreter Development)
find_library(Python3Gen NAME "python38.lib" PATHS "${Python3_LIBRARY_DIRS}" NO_DEFAULT_PATH)
# find_library(Python3Gen_Debug NAME "python38_d.lib" PATHS "${Python3_LIBRARY_DIRS}" NO_DEFAULT_PATH)

set(CMAKE_PREFIX_PATH "${QTDIR}")
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(OBS_ARCH_NAME "obs-streamlink.dll") # hardcode warning

find_package(Qt5Core REQUIRED)
find_package(Qt5Widgets REQUIRED)

add_library(obs-streamlink MODULE
	obs-streamlink.cpp
	streamlink-source.cpp
	media-playback/decode.cpp
	media-playback/media.cpp
	python-streamlink.cpp
	)
target_link_libraries(obs-streamlink PRIVATE
	"${LIBOBS_LIB}"
	"${W32_PTHREADS_LIB}"
	"${OBS_FRONTEND_LIB}"
	Qt5::Core
	Qt5::Widgets
	${FFMPEG_LIBRARIES}
	delayimp)
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
	target_link_libraries(obs-streamlink PRIVATE "${Python3Gen}")
else()
	target_link_libraries(obs-streamlink PRIVATE "${Python3Gen}")
endif()
target_include_directories(obs-streamlink PRIVATE
	${Qt5Core_INCLUDES}
	${Qt5Widgets_INCLUDES}
	${FFMPEG_INCLUDE_DIRS}
	"${LIBOBS_INCLUDE_DIR}"
	"${OBS_FRONTEND_INCLUDE_DIR}"
	"${Python3_INCLUDE_DIRS}"
)
set(RELEASE_DIR "${PROJECT_SOURCE_DIR}/release")
target_link_options(obs-streamlink PUBLIC
        #"/NODEFAULTLIB:python38_d.lib"
        #"/NODEFAULTLIB:python3_d.lib"
        "/NODEFAULTLIB:python38.lib"
        "/NODEFAULTLIB:python3.lib"
		"/delayload:python38.dll")

add_custom_command(TARGET obs-streamlink POST_BUILD
		# Copy to obs-studio dev environment for immediate testing
				# »¹ÊÇÅ×ÍßÐ»¶û¶¥
		COMMAND powershell -Command "if ($<CONFIG:Release> -eq 1 ) {robocopy ${PROJECT_SOURCE_DIR}/out/build/x64-Release ${RELEASE_DIR}/obs-plugins ${OBS_ARCH_NAME};robocopy ${PROJECT_SOURCE_DIR}/data ${RELEASE_DIR}/data/obs-plugins/obs-streamlink /MIR;}"

		COMMAND powershell -Command "if ($<CONFIG:Debug> -eq 1 ) { robocopy ${PROJECT_SOURCE_DIR}/out/build/x64-Debug D:/obs-studio-master/out/build/x64-Debug/rundir/Debug/obs-plugins/64bit ${OBS_ARCH_NAME};robocopy ${PROJECT_SOURCE_DIR}/data D:/obs-studio-master/out/build/x64-Debug/rundir/Debug/data/obs-plugins/obs-streamlink /MIR;}"
)