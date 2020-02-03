#pragma once
#include <obs-source.h>

struct obs_source_info_old {
	/* ----------------------------------------------------------------- */
	/* Required implementation*/

	/** Unique string identifier for the source */
	const char* id;

	/**
	 * Type of source.
	 *
	 * OBS_SOURCE_TYPE_INPUT for input sources,
	 * OBS_SOURCE_TYPE_FILTER for filter sources, and
	 * OBS_SOURCE_TYPE_TRANSITION for transition sources.
	 */
	enum obs_source_type type;

	/** Source output flags */
	uint32_t output_flags;

	/**
	 * Get the translated name of the source type
	 *
	 * @param  type_data  The type_data variable of this structure
	 * @return               The translated name of the source type
	 */
	const char* (*get_name)(void* type_data);

	/**
	 * Creates the source data for the source
	 *
	 * @param  settings  Settings to initialize the source with
	 * @param  source    Source that this data is associated with
	 * @return           The data associated with this source
	 */
	void* (*create)(obs_data_t* settings, obs_source_t* source);

	/**
	 * Destroys the private data for the source
	 *
	 * Async sources must not call obs_source_output_video after returning
	 * from destroy
	 */
	void (*destroy)(void* data);

	/** Returns the width of the source.  Required if this is an input
	 * source and has non-async video */
	uint32_t(*get_width)(void* data);

	/** Returns the height of the source.  Required if this is an input
	 * source and has non-async video */
	uint32_t(*get_height)(void* data);

	/* ----------------------------------------------------------------- */
	/* Optional implementation */

	/**
	 * Gets the default settings for this source
	 *
	 * @param[out]  settings  Data to assign default settings to
	 * @deprecated            Use get_defaults2 if type_data is needed
	 */
	void (*get_defaults)(obs_data_t* settings);

	/**
	 * Gets the property information of this source
	 *
	 * @return         The properties data
	 * @deprecated     Use get_properties2 if type_data is needed
	 */
	obs_properties_t* (*get_properties)(void* data);

	/**
	 * Updates the settings for this source
	 *
	 * @param data      Source data
	 * @param settings  New settings for this source
	 */
	void (*update)(void* data, obs_data_t* settings);

	/** Called when the source has been activated in the main view */
	void (*activate)(void* data);

	/**
	 * Called when the source has been deactivated from the main view
	 * (no longer being played/displayed)
	 */
	void (*deactivate)(void* data);

	/** Called when the source is visible */
	void (*show)(void* data);

	/** Called when the source is no longer visible */
	void (*hide)(void* data);

	/**
	 * Called each video frame with the time elapsed
	 *
	 * @param  data     Source data
	 * @param  seconds  Seconds elapsed since the last frame
	 */
	void (*video_tick)(void* data, float seconds);

	/**
	 * Called when rendering the source with the graphics subsystem.
	 *
	 * If this is an input/transition source, this is called to draw the
	 * source texture with the graphics subsystem using the specified
	 * effect.
	 *
	 * If this is a filter source, it wraps source draw calls (for
	 * example applying a custom effect with custom parameters to a
	 * source).  In this case, it's highly recommended to use the
	 * obs_source_process_filter function to automatically handle
	 * effect-based filter processing.  However, you can implement custom
	 * draw handling as desired as well.
	 *
	 * If the source output flags do not include SOURCE_CUSTOM_DRAW, all
	 * a source needs to do is set the "image" parameter of the effect to
	 * the desired texture, and then draw.  If the output flags include
	 * SOURCE_COLOR_MATRIX, you may optionally set the "color_matrix"
	 * parameter of the effect to a custom 4x4 conversion matrix (by
	 * default it will be set to an YUV->RGB conversion matrix)
	 *
	 * @param data    Source data
	 * @param effect  Effect to be used with this source.  If the source
	 *                output flags include SOURCE_CUSTOM_DRAW, this will
	 *                be NULL, and the source is expected to process with
	 *                an effect manually.
	 */
	void (*video_render)(void* data, gs_effect_t* effect);

	/**
	 * Called to filter raw async video data.
	 *
	 * @note          This function is only used with filter sources.
	 *
	 * @param  data   Filter data
	 * @param  frame  Video frame to filter
	 * @return        New video frame data.  This can defer video data to
	 *                be drawn later if time is needed for processing
	 */
	struct obs_source_frame* (*filter_video)(
		void* data, struct obs_source_frame* frame);

	/**
	 * Called to filter raw audio data.
	 *
	 * @note          This function is only used with filter sources.
	 *
	 * @param  data   Filter data
	 * @param  audio  Audio data to filter.
	 * @return        Modified or new audio data.  You can directly modify
	 *                the data passed and return it, or you can defer audio
	 *                data for later if time is needed for processing.  If
	 *                you are returning new data, that data must exist
	 *                until the next call to the filter_audio callback or
	 *                until the filter is removed/destroyed.
	 */
	struct obs_audio_data* (*filter_audio)(void* data,
		struct obs_audio_data* audio);

	/**
	 * Called to enumerate all active sources being used within this
	 * source.  If the source has children that render audio/video it must
	 * implement this callback.
	 *
	 * @param  data           Filter data
	 * @param  enum_callback  Enumeration callback
	 * @param  param          User data to pass to callback
	 */
	void (*enum_active_sources)(void* data,
		obs_source_enum_proc_t enum_callback,
		void* param);

	/**
	 * Called when saving a source.  This is a separate function because
	 * sometimes a source needs to know when it is being saved so it
	 * doesn't always have to update the current settings until a certain
	 * point.
	 *
	 * @param  data      Source data
	 * @param  settings  Settings
	 */
	void (*save)(void* data, obs_data_t* settings);

	/**
	 * Called when loading a source from saved data.  This should be called
	 * after all the loading sources have actually been created because
	 * sometimes there are sources that depend on each other.
	 *
	 * @param  data      Source data
	 * @param  settings  Settings
	 */
	void (*load)(void* data, obs_data_t* settings);

	/**
	 * Called when interacting with a source and a mouse-down or mouse-up
	 * occurs.
	 *
	 * @param data         Source data
	 * @param event        Mouse event properties
	 * @param type         Mouse button pushed
	 * @param mouse_up     Mouse event type (true if mouse-up)
	 * @param click_count  Mouse click count (1 for single click, etc.)
	 */
	void (*mouse_click)(void* data, const struct obs_mouse_event* event,
		int32_t type, bool mouse_up, uint32_t click_count);
	/**
	 * Called when interacting with a source and a mouse-move occurs.
	 *
	 * @param data         Source data
	 * @param event        Mouse event properties
	 * @param mouse_leave  Mouse leave state (true if mouse left source)
	 */
	void (*mouse_move)(void* data, const struct obs_mouse_event* event,
		bool mouse_leave);

	/**
	 * Called when interacting with a source and a mouse-wheel occurs.
	 *
	 * @param data         Source data
	 * @param event        Mouse event properties
	 * @param x_delta      Movement delta in the horizontal direction
	 * @param y_delta      Movement delta in the vertical direction
	 */
	void (*mouse_wheel)(void* data, const struct obs_mouse_event* event,
		int x_delta, int y_delta);
	/**
	 * Called when interacting with a source and gain focus/lost focus event
	 * occurs.
	 *
	 * @param data         Source data
	 * @param focus        Focus state (true if focus gained)
	 */
	void (*focus)(void* data, bool focus);

	/**
	 * Called when interacting with a source and a key-up or key-down
	 * occurs.
	 *
	 * @param data         Source data
	 * @param event        Key event properties
	 * @param focus        Key event type (true if mouse-up)
	 */
	void (*key_click)(void* data, const struct obs_key_event* event,
		bool key_up);

	/**
	 * Called when the filter is removed from a source
	 *
	 * @param  data    Filter data
	 * @param  source  Source that the filter being removed from
	 */
	void (*filter_remove)(void* data, obs_source_t* source);

	/**
	 * Private data associated with this entry
	 */
	void* type_data;

	/**
	 * If defined, called to free private data on shutdown
	 */
	void (*free_type_data)(void* type_data);

	bool (*audio_render)(void* data, uint64_t* ts_out,
		struct obs_source_audio_mix* audio_output,
		uint32_t mixers, size_t channels,
		size_t sample_rate);

	/**
	 * Called to enumerate all active and inactive sources being used
	 * within this source.  If this callback isn't implemented,
	 * enum_active_sources will be called instead.
	 *
	 * This is typically used if a source can have inactive child sources.
	 *
	 * @param  data           Filter data
	 * @param  enum_callback  Enumeration callback
	 * @param  param          User data to pass to callback
	 */
	void (*enum_all_sources)(void* data,
		obs_source_enum_proc_t enum_callback,
		void* param);

	void (*transition_start)(void* data);
	void (*transition_stop)(void* data);

	/**
	 * Gets the default settings for this source
	 *
	 * If get_defaults is also defined both will be called, and the first
	 * call will be to get_defaults, then to get_defaults2.
	 *
	 * @param       type_data The type_data variable of this structure
	 * @param[out]  settings  Data to assign default settings to
	 */
	void (*get_defaults2)(void* type_data, obs_data_t* settings);

	/**
	 * Gets the property information of this source
	 *
	 * @param data      Source data
	 * @param type_data The type_data variable of this structure
	 * @return          The properties data
	 */
	obs_properties_t* (*get_properties2)(void* data, void* type_data);

	bool (*audio_mix)(void* data, uint64_t* ts_out,
		struct audio_output_data* audio_output,
		size_t channels, size_t sample_rate);

	/** Icon type for the source */
	// NOT SUPPORTED IN VERSION EARLIER THAN 24.0.6
	//enum obs_icon_type icon_type;
};