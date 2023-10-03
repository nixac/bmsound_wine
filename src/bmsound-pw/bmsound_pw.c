#include "bmsound_pw.h"
#include "bmswpw_defines.h"
#include "bmswpw_callbacks.h"
#include <spa/param/audio/format-utils.h>
#include <pipewire/pipewire.h>
#include <unistd.h>


/*    bmswpw_buffer_t    *///_REV: probably separate later
void bmswpw_init_buffer(bmswpw_buffer_t *this, const spa_audio_info_raw_t format)
{
    this->size = 1024; //_REV: this should be deduced from format etc.
    this->u8 = malloc(this->size);
    this->event_data.format = malloc(sizeof(spa_audio_info_raw_t));
    *this->event_data.format = format;
};
void bmswpw_init_events(bmswpw_buffer_t *this, const pw_stream_events_t handler)
{
    this->event_handler = handler;
    this->event_handler.version = PW_VERSION_STREAM_EVENTS;
}
void bmswpw_init_stream(bmswpw_buffer_t *this, pw_properties_t *prop)
{
    if (!this->event_handler.version || !this->u8) return;
    this->event_data.properties = prop;

    // Create event loop
    this->event_loop.concurrent = pw_thread_loop_new("bmsw", NULL);

    // Create buffer stream
    this->event_data.stream = pw_stream_new_simple(
            pw_thread_loop_get_loop(this->event_loop.concurrent),
            "bmsw-stream",
            prop,
            &this->event_handler,
            &this->event_data
    );

    // Configure format of buffer stream (e.g. 2ch@44100hz:16bit depth stream)
    spa_pod_builder_t pod = SPA_POD_BUILDER_INIT(this->u8, this->size);
    const spa_pod_t *params[1]; //_INFO: allowed to go out of scope
    params[0] = spa_format_audio_raw_build(&pod, SPA_PARAM_EnumFormat, (spa_audio_info_raw_t *) this->event_data.format);

    // Connect buffer stream with event loop
    pw_stream_connect(this->event_data.stream,
                      PW_DIRECTION_OUTPUT, // specifies buffer stream as output only
                      PW_ID_ANY,
                      PW_STREAM_FLAG_AUTOCONNECT | PW_STREAM_FLAG_MAP_BUFFERS | PW_STREAM_FLAG_RT_PROCESS, // buffer stream flags, see limitations arising with PW_STREAM_FLAG_RT_PROCESS
                      params, 1
    );
}
void bmswpw_poll_start(bmswpw_buffer_t *this)
{
    pw_thread_loop_start(this->event_loop.concurrent);
}
void bmswpw_poll_stop(bmswpw_buffer_t *this)
{
    pw_thread_loop_stop(this->event_loop.concurrent);
}
void bmswpw_destroy_buffer(bmswpw_buffer_t *this)
{
    // Cleanup event loop
    if (this->event_loop.concurrent)
    {
        bmswpw_poll_stop(this);
        pw_thread_loop_destroy(this->event_loop.concurrent);
    }

    // Cleanup managed memory
    if (this->event_data.format) free(this->event_data.format);
    if (this->u8) free(this->u8);


}

/*    Public API    */
int bmswpw_start()
{
    return 0;
}
int bmswpw_deinit(const char *title)
{
    return 0;
}

/*    Experimental    *///_REV: move to experimental
int bmswpw_exp_test_concurrent(const char *title)
{
    pw_init(NULL, NULL); //_REV: move to on library load if thread safe

    bmswpw_buffer_t client;
    bmswpw_init_buffer(&client, BMSPWM_PCM_STEREO);

    bmswpw_init_events(&client, (const pw_stream_events_t) {.process = sine_process});

    bmswpw_init_stream(&client, pw_properties_new(
            PW_KEY_MEDIA_TYPE, "Audio",
            PW_KEY_MEDIA_CATEGORY, "Playback",
            PW_KEY_MEDIA_ROLE, "Game",
            PW_KEY_MEDIA_NAME, "待機",
            PW_KEY_APP_NAME, title,
            NULL
    ));

    bmswpw_poll_start(&client);

    // Run program for 10sec
    sleep(10);

    bmswpw_poll_stop(&client);

    bmswpw_destroy_buffer(&client);


    return 0;
}
int bmswpw_exp_test_sequential(const char *title)
{
    // pw basic init code
    bmswpw_buffer_t client = {0,};
    pw_init(NULL, NULL);

    // create a loop object
    client.event_loop.sequential = pw_main_loop_new(NULL);

    // create a stream object is this on_get_buffer??
    //_INFO: pw_stream_new() is alternative with more control, but harder to setup
    client.event_handler = (pw_stream_events_t) {
            PW_VERSION_STREAM_EVENTS,
            .process = sine_process //_REM: event is called whenever we need to produce more data (is this each period, so sample size based)
    };
    client.event_data.stream = pw_stream_new_simple(
            pw_main_loop_get_loop(client.event_loop.sequential),
            "bmsw-stream",
            pw_properties_new(PW_KEY_MEDIA_TYPE, "Audio",
                              PW_KEY_MEDIA_CATEGORY, "Playback",
                              PW_KEY_MEDIA_ROLE, "Game",
                              PW_KEY_MEDIA_NAME, "待機",
                              PW_KEY_APP_NAME, title,
                              NULL
            ),
            &client.event_handler,
            &client.event_data
    );

    // This is only format of stream (so in sync with on_is_format_supported()?), example for 2ch@44100hz:16bit depth stream
    const spa_pod_t *params[1];
    uint8_t buffer[1024]; //_REM: can this internal buffer be used directly to bind at on_get_buffer() ?
    spa_pod_builder_t b = SPA_POD_BUILDER_INIT(buffer, sizeof(buffer)); //_BUG: does scope matter or can this be inside spa_format_audio_raw_build()
    params[0] = spa_format_audio_raw_build(&b, SPA_PARAM_EnumFormat,
                                           &SPA_AUDIO_INFO_RAW_INIT(
                                                   .format = SPA_AUDIO_FORMAT_S16,
                                                   .channels = 2,
                                                   .rate = 44100));

    // Connects stream to main loop (data.main_loop)
    pw_stream_connect(client.event_data.stream,
                      PW_DIRECTION_OUTPUT, // specifies data.stream should be run in output mode
                      PW_ID_ANY,
                      PW_STREAM_FLAG_AUTOCONNECT | PW_STREAM_FLAG_MAP_BUFFERS | PW_STREAM_FLAG_RT_PROCESS, // stream flags, see limitations on PW_STREAM_FLAG_RT_PROCESS
                      params, 1 // params are supported formats
    );

    // Runs polling loop on THIS THREAD
    pw_main_loop_run(client.event_loop.sequential); // results in stream_events.process polling

    return 0;
}
