#include "bmsound_pw.h"
#include "bmswpw_defines.h"
#include "bmswpw_callbacks.h"
#include "bmswpw_util.h"
#include "bmsound_experimental.h"
#include "bmsound_config.h"
#include <spa/param/audio/format-utils.h>
#include <pipewire/pipewire.h>
#include <unistd.h>
#include <math.h>
#include <stdint.h>


/*    bmswpw_buffer_t    */
void bmswpw_init_buffer(bmsw_pwout_t *this, const spa_audio_info_raw_t format)
{
    this->event_data.format = malloc(sizeof(bmsw_audio_info_raw_t));
    *(spa_audio_info_raw_t *) this->event_data.format = format;
    this->event_data.format->stride = sizeof(int16_t) * this->event_data.format->channels;
    this->event_data.format->frames = bmsw_config->audio_fpc;

    //_BUG: size of this buffer matters a lot depending on chosen callback, 10sec may seem overkill, but for twin cursor -O3 makes no difference, size of this does (the bigger, the higher possible in<->out cursor latency distance)
    this->in.size = 1764000;
    this->in.u8 = calloc(this->in.size + this->event_data.format->rate * this->event_data.format->stride, 1);//+1s out-of-boundaries
    this->in.cursor = 0;

    //_REV: out.size should be calculated from format values
    this->out.size = 4096;
    this->out.u8 = calloc(this->out.size, 1);
    this->out.cursor = this->in.size - (this->event_data.format->frames * this->event_data.format->stride);//single chunk latency for twin cursor

    this->event_data.node = this;
    pthread_mutex_init(&this->lock, NULL);
}
void bmswpw_init_events(bmsw_pwout_t *this, const pw_stream_events_t handler, void *event_cb, void *event_cb_arg)
{
    this->event_handler = handler;
    this->event_handler.version = PW_VERSION_STREAM_EVENTS;
    if (!this->event_cb) this->event_cb = cb_empty;
    if (event_cb)
    {
        this->event_cb = event_cb;
        this->event_cb_arg = event_cb_arg;
    }
}
void bmswpw_init_stream(bmsw_pwout_t *this, pw_properties_t *prop)
{
    if (!this->event_handler.version || !this->event_data.node) return;
    this->event_data.properties = prop;

    // Create event loop
    this->event_loop.concurrent = pw_thread_loop_new("bmsw", NULL);

    // Create buffer stream
    this->event_data.stream = pw_stream_new_simple(
            pw_thread_loop_get_loop(this->event_loop.concurrent),
            "bmsw-stream",
            this->event_data.properties,
            &this->event_handler,
            &this->event_data
    );

    // Configure format of buffer stream (e.g. 2ch@44100hz:16bit depth stream)
    spa_pod_builder_t pod = SPA_POD_BUILDER_INIT(this->out.u8, this->out.size);
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
void bmswpw_poll_start(bmsw_pwout_t *this)
{
    this->in.cursor = 0;
    this->out.cursor = this->in.size - (this->event_data.format->frames * this->event_data.format->stride);
    pw_thread_loop_start(this->event_loop.concurrent);
}
void bmswpw_poll_stop(bmsw_pwout_t *this)
{
    pw_thread_loop_stop(this->event_loop.concurrent);
}
void bmswpw_destroy_buffer(bmsw_pwout_t *this)
{
    // Cleanup event loop
    if (this->event_loop.concurrent)
    {
        bmswpw_poll_stop(this);
        pw_thread_loop_destroy(this->event_loop.concurrent);
    }

    // Cleanup managed memory
    free(this->event_data.format);
    free(this->out.u8);
    free(this->in.u8);
    this->event_data.format = NULL;
    this->out.u8 = NULL;
    this->in.u8 = NULL;
    pthread_mutex_destroy(&this->lock);
}

/*    Public API    */
void *bmswpw_create(const char *title, void *event_cb, void *event_cb_arg)
{
    pw_init(NULL, NULL); //_REV: move to on library load if thread safe

    static bmsw_pwout_t client;
    bmswpw_init_buffer(&client, BMSPWM_SPA_FORMAT);

#ifndef BMSW_NOEXPERIMENTAL
    if (bmswexp_profile)
    {
        DBGEXEC(printf("Experimental API enabled. Initializing profile '%d' (%s)\n", bmswexp_profile, bmswexp_name_by_profile(bmswexp_profile)));
    }
    else
    {
        DBGEXEC(printf("Stable API unimplemented\n"));
        return NULL;
        //_TODO: Default API (so API for T_NONE, most likely Stable API in experimental builds)
    }
    if (EXPERIMENTAL(bmswexp_profile, bmswpw_callback) && !event_cb) bmswpw_update_callback(&client, EXPERIMENTAL(bmswexp_profile, bmswpw_callback), &client);
    bmswpw_init_events(&client, (const pw_stream_events_t) {.process = EXPERIMENTAL(bmswexp_profile, bmswpw_process)}, event_cb, event_cb_arg);
#else
    //_TODO: Stable API
#endif

    char latency[32];
    snprintf(latency, 32, "%d/%d", client.event_data.format->frames, client.event_data.format->rate);
    bmswpw_init_stream(&client, pw_properties_new(
            PW_KEY_MEDIA_TYPE, "Audio",
            PW_KEY_MEDIA_CATEGORY, "Playback",
            PW_KEY_MEDIA_ROLE, "Game",
            PW_KEY_MEDIA_NAME, "待機",
            PW_KEY_APP_NAME, title,
            PW_KEY_NODE_LATENCY, latency,
            NULL
    ));

    return &client;
}
int bmswpw_start(void *client)
{
    bmswpw_poll_start(client);
    return 0;
}
int bmswpw_stop(void *client)
{
    bmswpw_poll_stop(client);
    return 0;
}
int bmswpw_destroy(void *client)
{
    bmswpw_destroy_buffer(client);
    return 0;
}
unsigned char *bmswpw_get_buffer(void *client, uint32_t n)
{
#ifndef BMSW_NOEXPERIMENTAL
    return EXPERIMENTAL(bmswexp_profile, bmswpw_get_sbuf)(client, n);
#endif
    //_TODO: Stable API
}
int bmswpw_release_buffer(void *client, uint32_t n)
{
#ifndef BMSW_NOEXPERIMENTAL
    return EXPERIMENTAL(bmswexp_profile, bmswpw_send_sbuf)(client, n);
#endif
    //_TODO: Stable API
}
void bmswpw_update_callback(void *client, void *event_cb, void *event_cb_arg)
{
    bmsw_pwout_t *client_ = (bmsw_pwout_t *) client;
    client_->event_cb = event_cb;
    client_->event_cb_arg = event_cb_arg;
}
int bmswpw_format_is_supported(int rate, int channel, int depth, void *client)
{

    int rate_ = bmsw_config->audio_rate;
    int channel_ = bmsw_config->audio_channels;
    int depth_ = bmsw_config->audio_depth;
    if (client)
    {
        bmsw_pwout_t *client_ = client;
        rate_ = client_->event_data.format->rate;
        channel_ = client_->event_data.format->channels;
        //depth_ = client_->event_data.format->format; //_BUG: spa_format to bit value lookup table
    }

    if (channel == channel_ && rate == rate_ && depth == depth_) return 0;
    return -1;
}
int bmswpw_format_period_fpc(void *client)
{
    if (client)
        return ((bmsw_pwout_t *) client)->event_data.format->frames;
    return bmsw_config->audio_fpc;
}
long long bmswpw_format_period_wrt(void *client)
{
    int frames = bmsw_config->audio_fpc;
    int rate = bmsw_config->audio_rate;
    if (client)
    {
        bmsw_pwout_t *client_ = client;
        frames = client_->event_data.format->frames;
        rate = client_->event_data.format->rate;
    }

    return (long long) ceil(1e7 * frames / rate);
}

/*    Experimental    */
#include "bmswexp_pw.frag.c"
