#include "obs-streamlink.h"
#include <obs-module.h>
#include "obs-source-old.h"

#include <filesystem>
#include <Windows.h>

#include "python-streamlink.h"

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("obs-streamlink", "en-US")
MODULE_EXPORT const char *obs_module_description(void)
{
	return "Streamlink Source";
}

extern "C" obs_source_info_old streamlink_source_info;
std::filesystem::path obs_streamlink_data_path;

bool obs_module_load(void)
{
	std::string data_path = obs_get_module_data_path(obs_current_module());
	obs_streamlink_data_path = data_path;
	if (!std::filesystem::exists(obs_streamlink_data_path / obs_streamlink_python_ver))
	{
		blog(LOG_ERROR, "[Streamlink Source] Failed to initialize streamlink source!! Python38 not found in plugin data path.");
		return false;
	}

	streamlink::Initialize();
	obs_register_source_s(reinterpret_cast<const obs_source_info*>(&streamlink_source_info), sizeof(obs_source_info_old));
	return true;
}

