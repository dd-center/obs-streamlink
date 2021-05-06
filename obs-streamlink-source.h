#pragma once

#include <string>
#include <obs-module.h>

inline const char *source_name = "streamlink_source";

struct streamlink_source {
	obs_source_t *source;
};

using streamlink_source_t = struct streamlink_source;
