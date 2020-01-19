#include <obs-module.h>

/* Defines common functions (required) */
OBS_DECLARE_MODULE()

/* Implements common ini-based locale (optional) */
OBS_MODULE_USE_DEFAULT_LOCALE("obs-streamlink", "en-US")

#define blog(log_level, format, ...)                    \
	blog(log_level, "[streamlink] " format, \
	      ##__VA_ARGS__)

#define debug(format, ...) blog(LOG_DEBUG, format, ##__VA_ARGS__)
#define info(format, ...) blog(LOG_INFO, format, ##__VA_ARGS__)
#define warn(format, ...) blog(LOG_WARNING, format, ##__VA_ARGS__)

bool obs_module_load(void)
{
	info("Initialized");
    return true;
}

MODULE_EXPORT const char* obs_module_description(void)
{
	return "OBS source plugin to receive stream using streamlink.";
}
