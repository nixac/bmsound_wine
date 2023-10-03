#include "bmswpw_callbacks.h"
#include "bmswpw_defines.h"
#include <spa/param/audio/format-utils.h>
#include <pipewire/pipewire.h>
#include <math.h>


/*    Handlers    */
void on_process(void *userdata)
{


    // This part would be audio buffer updating, which would be done game-side
    /*
    for (i = 0; i < n_frames; i++)
    {
        data->accumulator += M_PI_M2 * 440 / DEFAULT_RATE;
        if (data->accumulator >= M_PI_M2)
            data->accumulator -= M_PI_M2;

        val = sin(data->accumulator) * DEFAULT_VOLUME * 16767.f;
        for (c = 0; c < DEFAULT_CHANNELS; c++)
            *dst++ = val;
    }
    */

}

/*    Experimental    *///_REV: move to experimental
#define sine_process_CHANNELS 2
#define sine_process_RATE 44100
#define sine_process_VOLUME 0.7
void sine_process(void *userdata)
{
    bmswpw_data_t *data = userdata;
    pw_buffer_t *b;
    spa_buffer_t *buf;
    int i, c, n_frames, stride;
    int16_t *dst, val;

    if ((b = pw_stream_dequeue_buffer(data->stream)) == NULL)
    {
        pw_log_warn("out of buffers: %m");
        return;
    }

    buf = b->buffer;
    if ((dst = buf->datas[0].data) == NULL)
        return;

    stride = sizeof(int16_t) * sine_process_CHANNELS;
    n_frames = buf->datas[0].maxsize / stride;

    for (i = 0; i < n_frames; i++)
    {
        data->accumulator += (M_PI + M_PI) * 440 / sine_process_RATE;
        if (data->accumulator >= (M_PI + M_PI))
            data->accumulator -= (M_PI + M_PI);

        val = sin(data->accumulator) * sine_process_VOLUME * 16767.f;
        for (c = 0; c < sine_process_CHANNELS; c++)
            *dst++ = val;
    }

    buf->datas[0].chunk->offset = 0;
    buf->datas[0].chunk->stride = stride;
    buf->datas[0].chunk->size = n_frames * stride;

    pw_stream_queue_buffer(data->stream, b);
}
