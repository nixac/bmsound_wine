#ifndef _BMSWPW_DEFINES_H
#define _BMSWPW_DEFINES_H
#include <spa/param/audio/format-utils.h>
#include <pipewire/stream.h>
#include <pthread.h>


typedef struct pw_stream_events pw_stream_events_t;
typedef struct pw_thread_loop pw_thread_loop_t;
typedef struct pw_main_loop pw_main_loop_t;
typedef struct pw_stream pw_stream_t;
typedef struct pw_properties pw_properties_t;
typedef struct pw_buffer pw_buffer_t;
typedef struct pw_time pw_time_t;
typedef struct spa_buffer spa_buffer_t;
typedef struct spa_pod_builder spa_pod_builder_t;
typedef struct spa_pod spa_pod_t;
typedef struct spa_audio_info_raw spa_audio_info_raw_t;
typedef struct bmsw_audio_info_raw bmsw_audio_info_raw_t;
typedef struct bmsw_pwout bmsw_pwout_t;
typedef struct bmsw_pwdata bmsw_pwdata_t;
typedef void (*bmsw_audio_callback_t)(void *);

struct bmsw_pwdata /* callback only data */
{
    pw_stream_t *stream; // buffer stream *managed by pipewire
    pw_properties_t *properties; // buffer stream properties *managed by pipewire
    bmsw_audio_info_raw_t *format; // stream audio format(s) *managed by bmswpw

    // Specialized payload (placeholder)
    bmsw_pwout_t *node; // master recursion
};
struct bmsw_pwout /* output only pipewire node */
{
    struct
    {
        union
        {
            uint8_t *u8;
            int16_t *s16;
        }; // raw buffer *managed by bmswpw_buffer
        int cursor; // position in raw buffer
        uint32_t size; // raw buffer max size in bytes
    } in, out;//in: iidx incoming out: used by pipewire stream

    union
    {
        pw_thread_loop_t *concurrent;
        pw_main_loop_t *sequential;
    } event_loop; // stream event loop
    pw_stream_events_t event_handler; // events called on data (operates mostly on event_data)
    bmsw_pwdata_t event_data;
    bmsw_audio_callback_t event_cb; // optional function called each time buffer has been processed
    void *event_cb_arg;

    //_REM: experimental, possible target to removal
    pw_buffer_t *q_buf;
    pthread_mutex_t lock;
};

/* Extensions */
struct bmsw_audio_info_raw
{
    struct spa_audio_info_raw;
    int stride;
    int frames;
};

/* Common audio formats *///_REV: refactor as lookup table if necessary for multi-format support
#define BMSPWM_SPA_FORMAT  SPA_AUDIO_INFO_RAW_INIT(.format = SPA_AUDIO_FORMAT_S16, .channels = bmsw_config->audio_channels, .rate = bmsw_config->audio_rate)

#endif