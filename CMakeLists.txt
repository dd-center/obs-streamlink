project(obs-streamlink)

set(obs-streamlink_HEADERS
	obs-streamlink-source.h)

set(obs-streamlink_SOURCES
	obs-streamlink.cpp
	obs-streamlink-source.cpp)

if(WIN32)
	set(MODULE_DESCRIPTION "OBS Streamlink module")
endif()

add_library(obs-streamlink MODULE
	${obs-streamlink_HEADERS}
	${obs-streamlink_SOURCES})
target_link_libraries(obs-streamlink
	libobs)

set_target_properties(obs-streamlink PROPERTIES FOLDER "plugins/obs-streamlink")

install_obs_plugin_with_data(obs-streamlink data)
