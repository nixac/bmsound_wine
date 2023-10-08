#ifndef BMSW_NOEXPERIMENTAL


/*    Tests    */
// Playback static sine sound
void process_sine(void *userdata)
{
#define process_sine_CHANNELS 2
#define process_sine_RATE 44100
#define process_sine_VOLUME 0.7
    static double accumulator;
    bmsw_pwdata_t *data = userdata;
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

    stride = sizeof(int16_t) * process_sine_CHANNELS;
    n_frames = buf->datas[0].maxsize / stride;

    for (i = 0; i < n_frames; i++)
    {
        accumulator += (M_PI + M_PI) * 440 / process_sine_RATE;
        if (accumulator >= (M_PI + M_PI))
            accumulator -= (M_PI + M_PI);

        val = sin(accumulator) * process_sine_VOLUME * 16767.f;
        for (c = 0; c < process_sine_CHANNELS; c++)
            *dst++ = val;
    }

    buf->datas[0].chunk->offset = 0;
    buf->datas[0].chunk->stride = stride;
    buf->datas[0].chunk->size = n_frames * stride;

    pw_stream_queue_buffer(data->stream, b);
}
// Playback buffer directly as it is, for testing stream format only
void process_audio_format(void *rdata)
{
    // Init
    pw_buffer_t *buffer;
    bmsw_pwdata_t *data = rdata;
    int nframes = process_beg(data, &buffer);
    if (!nframes) return;

    // Process
    int nbeg = data->node->out.cursor;
    int nend = nbeg + nframes * data->format->stride;
    buffer_cpy(buffer, data->node->in.u8 + nbeg, nend - nbeg);
    data->node->out.cursor = nend;

    // Finish
    process_end(data, buffer, nframes);//_BUG: should set amount of copied frames (so in case of insufficient node.in less than required)
}

/* Init */
void bmswexp_callbacks()
{
    bmsw_experiment_dispatcher[T_NOTIF_CALLBACK][bmswpw_process_i] = process_notif_callback;
    bmsw_experiment_dispatcher[T_TWIN_CURSOR][bmswpw_process_i] = process_twin_cursor;
    bmsw_experiment_dispatcher[T_AUDIO_FORMAT][bmswpw_process_i] = process_audio_format;
    bmsw_experiment_dispatcher[T_STATIC_SINE][bmswpw_process_i] = process_sine;
}

#endif