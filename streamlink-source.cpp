// ReSharper disable CppParameterMayBeConstPtrOrRef
// ReSharper disable CppClangTidyClangDiagnosticGnuZeroVariadicMacroArguments

#include <obs-module.h>
#include <util/platform.h>
#include <util/dstr.h>
#include <nlohmann/json.hpp>
#include "python-streamlink.h"
#include "media-playback/media.h"

using namespace streamlinkish_mp;

#define FF_LOG(level, format, ...) \
	blog(level, "[Streamlink Source]: " format, ##__VA_ARGS__)
#define FF_LOG_S(source, level, format, ...)        \
	blog(level, "[Streamlink Source '%s']: " format, \
	     obs_source_get_name(source), ##__VA_ARGS__)
#define FF_BLOG(level, format, ...) \
	FF_LOG_S(s->source, level, format, ##__VA_ARGS__)

const char* URL = "url";
const char* DEFINITIONS = "definitions";
const char* REFRESH_DEFINITIONS = "refresh_definitions";
const char* HW_DECODE = "hw_decode";
const char* IS_ADVANCED_SETTINGS_SHOW = "is_advanced_settings_show";
const char* ADVANCED_SETTINGS = "advanced_settings";
const char* STREAMLINK_OPTIONS = "streamlink_options";
const char* HTTP_PROXY = "http_proxy";
const char* HTTPS_PROXY = "https_proxy";
const char* RING_BUFFER_SIZE = "ringbuffer_size";
const char* HLS_LIVE_EDGE = "hls_live_edge";
const char* HLS_SEGMENT_THREADS = "hls_segment_threads";
const char* STREAMLINK_CUSTOM_OPTIONS = "streamlink_custom_options";
struct streamlink_source {
	mp_media_t media{};
	bool media_valid{};
	bool destroy_media{};

	SwsContext *sws_ctx{};
	int sws_width{};
	int sws_height{};
	AVPixelFormat sws_format = AV_PIX_FMT_NONE;
	uint8_t *sws_data{};
	int sws_linesize{};
	obs_source_t *source{};
	obs_hotkey_id hotkey{};

	char* live_room_url{};
	char* definitions{};
	std::vector<std::string>* available_definitions{};

	bool is_hw_decoding{};
	bool is_setting_frame_open=false;

	streamlink::Stream* stream{};
	streamlink::Session* streamlink_session{};
};
using streamlink_source_t = struct streamlink_source;

void set_streamlink_custom_options(const char* custom_options_s, streamlink_source_t* s)
{
	if (strlen(custom_options_s) == 0)
		return;
	using namespace nlohmann;
	auto session = s->streamlink_session;
	try
	{
		auto custom_options = json::parse(custom_options_s);
		if (!custom_options.is_object())
			return FF_BLOG(LOG_WARNING, "Failed to set streamlink custom options, given json is not an object.");
		for (auto& [key, value] : custom_options.items())
		{
			if (key.empty())
				continue;
			if (value.is_boolean())
				session->SetOptionBool(key, value.get<bool>());
			else if (value.is_number_integer())
				session->SetOptionInt(key, value.get<int>());
			else if (value.is_number())
				session->SetOptionDouble(key, value.get<double>());
			else if (value.is_string())
				session->SetOptionString(key, value.get<std::string>());
			else
				FF_BLOG(LOG_WARNING, "Failed to set streamlink custom options %s, value type not recognized.", key.c_str());
		}
	}
	catch (json::exception& ex)
	{
		FF_BLOG(LOG_WARNING, "Failed to set streamlink custom options, bad JSON string: %s", ex.what());
	}
}

bool update_streamlink_session(void* data, obs_data_t* settings) {
    auto* s = static_cast<streamlink_source_t*>(data);

	const char* http_proxy_s = obs_data_get_string(settings, HTTP_PROXY);
	const char* https_proxy_s = obs_data_get_string(settings, HTTPS_PROXY);
	const long long ringbuffer_size = obs_data_get_int(settings, RING_BUFFER_SIZE);
	const long long hls_live_edge = obs_data_get_int(settings, HLS_LIVE_EDGE);
	const long long hls_segment_threads = obs_data_get_int(settings, HLS_SEGMENT_THREADS);
	const char* custom_options_s = obs_data_get_string(settings, STREAMLINK_CUSTOM_OPTIONS);
	//const char* streamlink_options_s = obs_data_get_string(settings, STREAMLINK_OPTIONS);
	streamlink::ThreadGIL state = streamlink::ThreadGIL();
	try {
        delete s->streamlink_session;

		s->streamlink_session = new streamlink::Session();
		auto session = s->streamlink_session;

		if(strlen(http_proxy_s)>1)
			session->SetOptionString("http-proxy", http_proxy_s);
		if (strlen(https_proxy_s) > 1)
			session->SetOptionString("https-proxy", https_proxy_s);
		if(ringbuffer_size>0)
			session->SetOptionInt("ringbuffer-size", static_cast<long long>(ringbuffer_size) * 1024 * 1024);
		session->SetOptionInt("hls-live-edge", hls_live_edge);
		session->SetOptionInt("hls-segment-threads", hls_segment_threads);
		session->SetOptionDouble("http-timeout", 5.0);
		session->SetOptionString("ffmpeg-ffmpeg", "DISABLED");
		set_streamlink_custom_options(custom_options_s, s);
		return true;
	}
	catch (std::exception & ex) {
		FF_BLOG(LOG_WARNING, "Error initializing streamlink session: %s", ex.what());
		return false;
	}
}
static void streamlink_source_defaults(obs_data_t *settings)
{
	obs_data_set_default_string(settings, DEFINITIONS, "best");
	obs_data_set_default_int(settings, RING_BUFFER_SIZE, 16);
	obs_data_set_default_int(settings, HLS_LIVE_EDGE, 8);
	obs_data_set_default_int(settings, HLS_SEGMENT_THREADS, 3);
	obs_data_set_default_string(settings, STREAMLINK_CUSTOM_OPTIONS, "{}");
}

static void streamlink_source_start(struct streamlink_source* s);
bool refresh_definitions(obs_properties_t* props,obs_property_t* prop,void* data) {
    auto s = static_cast<streamlink_source_t*>(data);

	obs_data_t* settings = obs_source_get_settings(s->source);
	const char* url = obs_data_get_string(settings, URL);
	update_streamlink_session(s, settings);
	obs_data_release(settings);
	auto state = streamlink::ThreadGIL();
	try {
		obs_property_t* list = obs_properties_get(props,DEFINITIONS);
		obs_property_list_clear(list);
		s->available_definitions->clear();
		auto streams = s->streamlink_session->GetStreamsFromUrl(url);
		for (auto& stream : streams) {
			const char* definition = stream.first.c_str();
			obs_property_list_add_string(list, definition, definition);
			s->available_definitions->push_back(definition);
		}
		return true;
	}
	catch (std::exception & ex) {
		FF_BLOG(LOG_WARNING, "Error fetching stream definitions for URL %s: %s", url, ex.what());
		return false;
	}
}

static obs_properties_t *streamlink_source_getproperties(void *data)
{
	// ReSharper disable CppAssignedValueIsNeverUsed
	// ReSharper disable CppJoinDeclarationAndAssignment
    auto s = static_cast<streamlink_source_t*>(data);
	UNUSED_PARAMETER(data);
	obs_properties_t *props = obs_properties_create();
	obs_properties_set_flags(props, OBS_PROPERTIES_DEFER_UPDATE);
    obs_property_t* prop;
    prop = obs_properties_add_text(props, URL, obs_module_text(URL), OBS_TEXT_DEFAULT);
	prop = obs_properties_add_list(props, DEFINITIONS, obs_module_text(DEFINITIONS), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_STRING);
	obs_property_set_modified_callback2(prop, [](void* priv, obs_properties_t*, obs_property_t* prop, obs_data_t* data) -> bool {
        auto s = static_cast<streamlink_source_t*>(priv);
		auto propName = obs_property_name(prop);
		auto shouldPropName = DEFINITIONS;
		if (astrcmp_n(propName, shouldPropName, strlen(shouldPropName)) != 0) return false;
		auto def = obs_data_get_string(data, shouldPropName);
		if (s->definitions) bfree(s->definitions);
		s->definitions = bstrdup(def);
		return false; // TODO find out WHY?
	}, s);
	for (auto& def : *s->available_definitions)
		obs_property_list_add_string(prop, def.c_str(), def.c_str());
	prop = obs_properties_add_button2(props,REFRESH_DEFINITIONS, obs_module_text(REFRESH_DEFINITIONS), refresh_definitions, s);
#ifndef __APPLE__
	obs_properties_add_bool(props, HW_DECODE,
				obs_module_text(HW_DECODE));
#endif
	obs_property_t* is_advanced_settings_show = obs_properties_add_bool(props, IS_ADVANCED_SETTINGS_SHOW, obs_module_text(IS_ADVANCED_SETTINGS_SHOW));
	obs_property_set_modified_callback(is_advanced_settings_show, [](obs_properties_t* props, obs_property_t* prop, obs_data_t* settings)->bool{
		UNUSED_PARAMETER(prop);
		bool show = obs_data_get_bool(settings, IS_ADVANCED_SETTINGS_SHOW);
		obs_property_t* advanced_settings = obs_properties_get(props, ADVANCED_SETTINGS);
		obs_property_set_visible(advanced_settings, show);
		return true;
		});
	obs_properties_t* advanced_settings = obs_properties_create();
	prop = obs_properties_add_text(advanced_settings, HTTP_PROXY, obs_module_text(HTTP_PROXY), OBS_TEXT_DEFAULT);
    prop = obs_properties_add_text(advanced_settings, HTTPS_PROXY, obs_module_text(HTTPS_PROXY), OBS_TEXT_DEFAULT);
    prop = obs_properties_add_int(advanced_settings, RING_BUFFER_SIZE, obs_module_text(RING_BUFFER_SIZE), 0, 256, 1);
	prop = obs_properties_add_int(advanced_settings, HLS_LIVE_EDGE, obs_module_text(HLS_LIVE_EDGE), 1, 20, 1);
	prop = obs_properties_add_int(advanced_settings, HLS_SEGMENT_THREADS, obs_module_text(HLS_SEGMENT_THREADS), 1, 10, 1);
	prop = obs_properties_add_text(advanced_settings, STREAMLINK_CUSTOM_OPTIONS, obs_module_text(STREAMLINK_CUSTOM_OPTIONS), OBS_TEXT_MULTILINE);
	//prop = obs_properties_add_text(advanced_settings, STREAMLINK_OPTIONS, obs_module_text(STREAMLINK_OPTIONS), OBS_TEXT_MULTILINE);
	obs_properties_add_group(props, ADVANCED_SETTINGS, obs_module_text(ADVANCED_SETTINGS),OBS_GROUP_NORMAL,advanced_settings);
	return props;
	// ReSharper restore CppAssignedValueIsNeverUsed
	// ReSharper restore CppJoinDeclarationAndAssignment
}

static void get_frame(void *opaque, struct obs_source_frame *f)
{
	auto *s = static_cast<streamlink_source_t*>(opaque);
	obs_source_output_video(s->source, f);
}

static void preload_frame(void *opaque, struct obs_source_frame *f)
{
	auto *s = static_cast<streamlink_source_t*>(opaque);
	obs_source_preload_video(s->source, f);
}

static void seek_frame(void* opaque, struct obs_source_frame* f)
{
	auto* s = static_cast<streamlink_source_t*>(opaque);
	obs_source_set_video_frame(s->source, f);
}

static void get_audio(void *opaque, struct obs_source_audio *a)
{
    auto s = static_cast<streamlink_source_t*>(opaque);
	obs_source_output_audio(s->source, a);
}

static void media_stopped(void *opaque)
{
    auto s = static_cast<streamlink_source_t*>(opaque);
	{
		obs_source_output_video(s->source, NULL);
		if (s->media_valid)
			s->destroy_media = true;
	}
}

static int read_packet(void* opaque, uint8_t* buf, int buf_size) {
    auto c = static_cast<streamlink_source_t*>(opaque);
	streamlink::ThreadGIL state = streamlink::ThreadGIL();
	try {
		return c->stream->Read(buf, buf_size); // Impossible to read > 4G at one packet..
	}
	catch (std::exception & ex) {
		FF_LOG(LOG_WARNING, "Failed read from video stream for livestream %s! %s", c->live_room_url, ex.what());
		return -1;
	}
}

int streamlink_open(streamlink_source_t* c) {
	auto state = streamlink::ThreadGIL();
	try {
		auto streams = c->streamlink_session->GetStreamsFromUrl(c->live_room_url);
        delete c->stream;
		auto pref = streams.find(c->definitions);
		if (pref == streams.end())
			pref = streams.find("best");
		if (pref == streams.end())
			pref = streams.begin();
		if (pref == streams.end()) {
			FF_LOG(LOG_WARNING, "No streams found for live url %s", c->live_room_url);
			return -1;
		}
		auto udly = pref->second.Open();
		c->stream = new streamlink::Stream(udly);
	}catch (std::exception & ex) {
		FF_LOG(LOG_WARNING, "Failed to open streamlink stream for URL %s! %s", c->live_room_url, ex.what());
		return -1;
	}
	return 0;
}

void streamlink_close(void* opaque) {
	// TODO error caching
    auto c = static_cast<streamlink_source_t*>(opaque);
	streamlink::ThreadGIL state = streamlink::ThreadGIL();
	if (c->stream) {
		c->stream->Close();
		delete c->stream;
		c->stream = nullptr;
	}
}

static void streamlink_source_open(struct streamlink_source *s)
{
	if (s->live_room_url && *s->live_room_url) {
        mp_media_info info = {
			s,
			get_frame,
			preload_frame,
			seek_frame,
			get_audio,
			media_stopped,
			read_packet,
			s->live_room_url,
			nullptr,
			nullptr, // TODO props for ffmpeg-param
			0,
			100,
			VIDEO_RANGE_DEFAULT,
			false,
			s->is_hw_decoding,
			false,
		    false
		};
		if (streamlink_open(s) == 0)
			s->media_valid = mp_media_init(&s->media, &info);
		else s->media_valid = false; // streamlink FAILED
	}
}

static void streamlink_source_tick(void *data, float seconds)
{
	UNUSED_PARAMETER(seconds);

    auto s = static_cast<streamlink_source_t*>(data);
	if (s->destroy_media) {
		if (s->media_valid) {
			mp_media_free(&s->media);
			streamlink_close(s);
			s->media_valid = false;
		}
		s->destroy_media = false;
	}
}

static void streamlink_source_start(struct streamlink_source *s)
{
	if (!s->media_valid)
		streamlink_source_open(s);

	if (s->media_valid) {
		mp_media_play(&s->media, false, false);
	}
}

static void streamlink_source_update(void *data, obs_data_t *settings)
{
    auto s = static_cast<streamlink_source_t*>(data);

	update_streamlink_session(s, settings);

    if (s->live_room_url)
		bfree(s->live_room_url);
	const char* live_room_url = obs_data_get_string(settings, URL);
	s->live_room_url = live_room_url ? bstrdup(live_room_url) : NULL;
#ifndef __APPLE__
	s->is_hw_decoding = obs_data_get_bool(settings, HW_DECODE);
#endif
	if (s->media_valid) {
		mp_media_free(&s->media);
		streamlink_close(s);
		s->media_valid = false;
	}
	bool active = obs_source_active(s->source);

	if (active)
		streamlink_source_start(s);
}

static const char *streamlink_source_getname(void *unused)
{
	UNUSED_PARAMETER(unused);
	return obs_module_text("StreamlinkSource");
}

static void restart_hotkey(void *data, obs_hotkey_id id, obs_hotkey_t *hotkey,
			   bool pressed)
{
	UNUSED_PARAMETER(id);
	UNUSED_PARAMETER(hotkey);
	UNUSED_PARAMETER(pressed);

    auto s = static_cast<streamlink_source_t*>(data);
	if (obs_source_active(s->source))
		streamlink_source_start(s);
}

static void restart_proc(void *data, calldata_t *cd)
{
	restart_hotkey(data, 0, NULL, true);
	UNUSED_PARAMETER(cd);
}

static void get_duration(void *data, calldata_t *cd)
{
    auto s = static_cast<streamlink_source_t*>(data);
	int64_t dur = 0;
	if (s->media.fmt)
		dur = s->media.fmt->duration;

	calldata_set_int(cd, "duration", dur * 1000);
}

static void get_nb_frames(void *data, calldata_t *cd)
{
    auto s = static_cast<streamlink_source_t*>(data);
	int64_t frames = 0;

	if (!s->media.fmt) {
		calldata_set_int(cd, "num_frames", frames);
		return;
	}

	int video_stream_index = av_find_best_stream(
		s->media.fmt, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);

	if (video_stream_index < 0) {
		FF_BLOG(LOG_WARNING, "Getting number of frames failed: No "
				     "video stream in media file!");
		calldata_set_int(cd, "num_frames", frames);
		return;
	}

	AVStream *stream = s->media.fmt->streams[video_stream_index];

	if (stream->nb_frames > 0) {
		frames = stream->nb_frames;
	} else {
		FF_BLOG(LOG_DEBUG, "nb_frames not set, estimating using frame "
				   "rate and duration");
		const AVRational avg_frame_rate = stream->avg_frame_rate;
		frames = static_cast<int64_t>(ceil(static_cast<double>(s->media.fmt->duration) /
			static_cast<double>(AV_TIME_BASE) *
			static_cast<double>(avg_frame_rate.num) /
			static_cast<double>(avg_frame_rate.den)));
	}

	calldata_set_int(cd, "num_frames", frames);
}

static void streamlink_source_destroy(void* data);

static void *streamlink_source_create(obs_data_t *settings, obs_source_t *source)
{
    auto s = static_cast<streamlink_source_t*>(bzalloc(sizeof(struct streamlink_source)));
	s->source = source;
	s->available_definitions = new std::vector<std::string>;

	s->hotkey = obs_hotkey_register_source(source, "MediaSource.Restart",
					       obs_module_text("RestartMedia"),
					       restart_hotkey, s);
	s->definitions = bstrdup("best");

	proc_handler_t *ph = obs_source_get_proc_handler(source);
	proc_handler_add(ph, "void restart()", restart_proc, s);
	proc_handler_add(ph, "void get_duration(out int duration)",
			 get_duration, s);
	proc_handler_add(ph, "void get_nb_frames(out int num_frames)",
			 get_nb_frames, s);

	if (!update_streamlink_session(s, settings)) {
		streamlink_source_destroy(s);
		return nullptr;
	}
	streamlink_source_update(s, settings);
	return s;
}

static void streamlink_source_destroy(void *data)
{
    auto s = static_cast<streamlink_source_t*>(data);

	if (s->hotkey)
		obs_hotkey_unregister(s->hotkey);
	if (s->media_valid)
		mp_media_free(&s->media);

	if (s->sws_ctx != NULL)
		sws_freeContext(s->sws_ctx);
	streamlink_close(s);
	bfree(s->sws_data);
	bfree(s->live_room_url);
	bfree(s->definitions);
	delete s->available_definitions;
	delete s->streamlink_session;
	bfree(s);
}

static void streamlink_source_show(void *data)
{
    auto s = static_cast<streamlink_source_t*>(data);
	streamlink_source_start(s);
}

static void streamlink_source_hide(void *data)
{
    auto s = static_cast<streamlink_source_t*>(data);

	if (s->media_valid)
		mp_media_stop(&s->media);
	obs_source_output_video(s->source, NULL);
	streamlink_close(s);
}

static void streamlink_source_restart(void* data)
{
	streamlink_source_hide(data);
	streamlink_source_show(data);
}

extern "C" obs_source_info streamlink_source_info = {
	"streamlink_source",                                  // id
	OBS_SOURCE_TYPE_INPUT,                                // type
	OBS_SOURCE_ASYNC_VIDEO                                // output_flags
    | OBS_SOURCE_AUDIO
    | OBS_SOURCE_DO_NOT_DUPLICATE
    | OBS_SOURCE_CONTROLLABLE_MEDIA,
	streamlink_source_getname,                            // get_name
	streamlink_source_create,                             // create
	streamlink_source_destroy,                            // destroy
	nullptr, nullptr,                                     // get_width, get_height
	streamlink_source_defaults,                           // get_defaults
	streamlink_source_getproperties,                      // get_properties
	streamlink_source_update,                             // update
	nullptr, nullptr,                                     // activate, deactivate: Not called when shown in preview
	streamlink_source_show, streamlink_source_hide,       // show, hide
	streamlink_source_tick,                               // video_tick
	nullptr,                                              // video_render
	nullptr, nullptr,                                     // filter_video, filter_audio
	nullptr,                                              // enum_active_sources
	nullptr, nullptr,                                     // save, load
	nullptr, nullptr, nullptr, nullptr,                   // mouse_move, mouse_wheel, focus, key_click
	nullptr, nullptr, nullptr, nullptr, nullptr,
    nullptr, nullptr, nullptr, nullptr, nullptr,nullptr,
	OBS_ICON_TYPE_MEDIA,                                  // icon_type
	nullptr,                                              // media_play_pause
    streamlink_source_restart,                            // media_restart
	streamlink_source_hide                                // stop
	// other initializer omitted
};  // NOLINT(clang-diagnostic-missing-field-initializers)