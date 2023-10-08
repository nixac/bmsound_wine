#include "bmswpw_callbacks.h"
#include "bmswpw_defines.h"
#include "bmswpw_util.h"
#include "bmsound_experimental.h"
#include <spa/param/audio/format-utils.h>
#include <pipewire/pipewire.h>
#include <math.h>


/*    Internally reused    */
#define buffer_set(__b, __val, __size) memset((__b)->buffer->datas[0].data, (__val), (__size))
#define buffer_cpy(__b, __val, __size) memcpy((__b)->buffer->datas[0].data, (__val), (__size));
static inline int process_beg(bmsw_pwdata_t *data, pw_buffer_t **b)
{
    //pw_time_t time;
    pw_buffer_t *buffer = pw_stream_dequeue_buffer(data->stream);
    if (!buffer)
    {
        DBGEXEC(printf("out of buffers: %s\n", strerror(errno));)
        return 0;
    }

    int nframes = buffer->buffer->datas[0].maxsize / data->format->stride;
#if PW_CHECK_VERSION(0, 3, 49)
    if (buffer->requested != 0)
        nframes = SPA_MIN(buffer->requested, nframes);
#endif
    //if (!(*b)->buffer->datas[0].data) return;
    *b = buffer;
    return nframes;
}
static inline void process_end(bmsw_pwdata_t *data, pw_buffer_t *b, int nframes)
{
    b->buffer->datas[0].chunk->size = nframes * data->format->stride;
    b->buffer->datas[0].chunk->offset = 0;
    b->buffer->datas[0].chunk->stride = data->format->stride;
    pw_stream_queue_buffer(data->stream, b);
}

/*    Handlers    */
// Communicates with caller through ambiguous notification callback _INFO: in chunk size may have to be bigger/smaller than pipewire internal
void process_notif_callback(void *rdata)
{
    // Init
    pw_buffer_t *buffer;
    bmsw_pwdata_t *data = rdata;
    int nframes = process_beg(data, &buffer);
    if (!nframes) return;

    // Process
    pthread_mutex_lock(&data->node->lock);
    DBGEXEC(if (data->node->in.cursor > 5000) printf("BUFFER UNDERRUN\n");)
    if (data->node->in.cursor)
    {
        nframes = data->node->in.cursor / data->format->stride;
        buffer_cpy(buffer, data->node->in.u8, data->node->in.cursor);
        data->node->in.cursor = 0;
        pthread_mutex_unlock(&data->node->lock);
    }
    else
    {
        pthread_mutex_unlock(&data->node->lock);
        DBGEXEC(printf("BUFFER OVERRUN\n");)
        buffer_set(buffer, 0, nframes * data->format->stride);
    }
    data->node->event_cb(data->node->event_cb_arg);

    // Finish
    process_end(data, buffer, nframes);
}
// Playbacks buffer according to internal timer without communicating with caller, assuming buffer is always available _INFO: critical syncing part code is missing _BUG: may need mutex at least for syncing
void process_twin_cursor(void *rdata)
{
    // Init
    pw_buffer_t *buffer;
    bmsw_pwdata_t *data = rdata;
    int nframes = process_beg(data, &buffer);
    if (!nframes) return;

    // Process
    //int framemax=data->node->in.size/data->format->stride;
    int nbeg = data->node->out.cursor;
    int nend = nbeg + nframes * data->format->stride;
    if (nend > data->node->in.size)
    {
        buffer_cpy(buffer, data->node->in.u8 + nbeg, data->node->in.size - nbeg);
        nbeg = 0;
        nend -= data->node->in.size;
    }
    buffer_cpy(buffer, data->node->in.u8 + nbeg, nend - nbeg);
    data->node->out.cursor = nend;

    // Finish
    process_end(data, buffer, nframes);//_BUG: should set amount of copied frames (so in case of insufficient node.in less than requested)
}

/*    Experimental    */
#include "bmswexp_callbacks.frag.c"
