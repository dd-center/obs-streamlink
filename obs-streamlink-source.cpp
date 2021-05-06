#include "obs-streamlink-source.h"
  
static const char *streamlink_source_get_name(void *unused)
{
	UNUSED_PARAMETER(unused);
	return obs_module_text(source_name);
}

static void *streamlink_source_create(obs_data_t *settings,
				      obs_source_t *source)
{
	UNUSED_PARAMETER(settings);
	const auto new_source = static_cast<streamlink_source_t *>(
		bzalloc(sizeof(struct streamlink_source)));
	new_source->source = source;
	return new_source;
}

static void streamlink_source_destroy(void *data)
{
	UNUSED_PARAMETER(data);
}

static void streamlink_source_defaults(obs_data_t *settings)
{
	UNUSED_PARAMETER(settings);
}

static obs_properties_t *streamlink_source_get_properties(void *data)
{
	UNUSED_PARAMETER(data);
	obs_properties_t *props = obs_properties_create();
	return props;
}

static void streamlink_source_update(void *data, obs_data_t *settings)
{
	UNUSED_PARAMETER(data);
	UNUSED_PARAMETER(settings);
}

static void streamlink_source_activate(void *data)
{
	UNUSED_PARAMETER(data);
}

static void streamlink_source_deactivate(void *data)
{
	UNUSED_PARAMETER(data);
}

static void streamlink_source_tick(void *data, float seconds)
{
	UNUSED_PARAMETER(data);
	UNUSED_PARAMETER(seconds);
}

extern "C" struct obs_source_info streamlink_source_info = {
	"streamlink_source",
	OBS_SOURCE_TYPE_INPUT,
	OBS_SOURCE_ASYNC_VIDEO | OBS_SOURCE_AUDIO | OBS_SOURCE_DO_NOT_DUPLICATE,
	streamlink_source_get_name,
	streamlink_source_create,
	streamlink_source_destroy,
	nullptr,
	nullptr,
	streamlink_source_defaults,
	streamlink_source_get_properties,
	streamlink_source_update,
	streamlink_source_activate,
	streamlink_source_deactivate,
	nullptr,
	nullptr,
	streamlink_source_tick,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	OBS_ICON_TYPE_MEDIA,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	1,
	nullptr,
	nullptr};
