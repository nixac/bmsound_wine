#ifndef _BMSWPW_DEFINES_H
#define _BMSWPW_DEFINES_H
#include <spa/param/audio/format-utils.h>
#include <pipewire/stream.h>


typedef struct pw_stream_events pw_stream_events_t;
typedef struct pw_thread_loop pw_thread_loop_t;
typedef struct pw_main_loop pw_main_loop_t;
typedef struct pw_stream pw_stream_t;
typedef struct pw_properties pw_properties_t;
typedef struct pw_buffer pw_buffer_t;
typedef struct spa_buffer spa_buffer_t;
typedef struct spa_pod_builder spa_pod_builder_t;
typedef struct spa_pod spa_pod_t;
typedef struct spa_audio_info_raw spa_audio_info_raw_t;
typedef struct bmswpw_buffer bmswpw_buffer_t;
typedef struct bmswpw_data bmswpw_data_t;


struct bmswpw_data /* callback only data, the smaller, the better */
{
    pw_stream_t *stream; // buffer stream *managed by pipewire
    pw_properties_t *properties; // buffer stream properties *managed by pipewire
    spa_audio_info_raw_t *format; // stream audio format(s) *managed by bmswpw

    // Specialized payload (placeholder)
    double accumulator;
};
struct bmswpw_buffer
{
    uint8_t *u8; // raw buffer *managed by bmswpw_buffer
    uint32_t size; // raw buffer max size

    pw_stream_events_t event_handler; // events called on data (uses mostly *bmswpw_data, *bmswpw_data.stream)
    union
    {
        pw_thread_loop_t *concurrent;//concurrent/parallel
        pw_main_loop_t *sequential;
    }event_loop; // buffer event loop
    bmswpw_data_t event_data;
};

/* Common audio formats *///_REV: refactor as lookup key(val) if necessary for multi-format support
#define BMSPWM_PCM_STEREO  SPA_AUDIO_INFO_RAW_INIT(.format = SPA_AUDIO_FORMAT_S16,.channels = 2,.rate = 44100)

#endif