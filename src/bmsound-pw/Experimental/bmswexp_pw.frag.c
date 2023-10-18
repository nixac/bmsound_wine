#ifndef BMSW_NOEXPERIMENTAL


/*    Wrappers ()    *///_REV: This is pretty ugly structure-wise right now and probably should be moved elsewhere (separate unit)
volatile int buffer_ready = 0;
void callback_notif_spice(void *arg)
{
    /*_INFO: Block until spicetools is done
     * stream: process()->buffer_ready:->buffer_ready?->process()
     * client: get_framesize()->get_buffer()->buffer_ready?:->release_buffer()->buffer_ready->SetEvent()->get_framesize()
     * _TODO: alternative, try moving callback to before buffer_cpy() not after, this would require different pipe than above and use nanotime on spice side
     * */
    buffer_ready = 0;
    while (!buffer_ready);
}

/*    Experimental::bmswpw_buffer_t    */
unsigned char *bmswpw_get_sbuf_notif_spice(bmsw_pwout_t *this, uint32_t n)
{
    while (buffer_ready);
    pthread_mutex_lock(&this->lock);
    return this->in.u8 + this->in.cursor;
}
int bmswpw_send_sbuf_notif_spice(bmsw_pwout_t *this, uint32_t n)
{
    this->in.cursor += n * this->event_data.format->stride;
    if (this->in.cursor > 100000)
    {
        DBGEXEC(printf("EXTREME BUFFER UNDERRUN DETECTED\n"));
    }
    buffer_ready = 1;
    pthread_mutex_unlock(&this->lock);
    return 0;
}
unsigned char *bmswpw_get_sbuf_twin_cursor(bmsw_pwout_t *this, uint32_t n)
{
    //pw_thread_loop_lock(this->event_loop.concurrent);
    return this->in.u8 + this->in.cursor;
}
int bmswpw_send_sbuf_twin_cursor(bmsw_pwout_t *this, uint32_t n)
{
    this->in.cursor += n * this->event_data.format->stride;
    if (this->in.cursor > this->in.size)
    {
        int overbuf = this->in.cursor - this->in.size;
        memcpy(this->in.u8, this->in.u8 + this->in.size, overbuf); // let's hope out.cursor is not too ahead/behind and do this
        this->in.cursor = overbuf;
    }
    //pw_thread_loop_unlock(this->event_loop.concurrent);

    return 0;
}
unsigned char *bmswpw_get_sbuf_notif_callback(bmsw_pwout_t *this, uint32_t n)
{
    pthread_mutex_lock(&this->lock);
    return this->in.u8 + this->in.cursor;
}
int bmswpw_send_sbuf_notif_callback(bmsw_pwout_t *this, uint32_t n)
{
    this->in.cursor += n * this->event_data.format->stride;
    if (this->in.cursor > 100000)
    {
        DBGEXEC(printf("EXTREME BUFFER UNDERRUN DETECTED\n"));
    }
    pthread_mutex_unlock(&this->lock);
    return 0;
}
unsigned char *bmswpw_get_sbuf_spoofed_loop(bmsw_pwout_t *this, uint32_t n)
{
    if ((this->q_buf = pw_stream_dequeue_buffer(this->event_data.stream)) == NULL || this->q_buf->buffer->datas[0].data == NULL)
    {
        return NULL;
    }

    uint32_t n_max = this->q_buf->buffer->datas[0].maxsize / this->event_data.format->stride;
    if (!n || n > n_max)
    {
        n = n_max;
        DBGEXEC(printf("BUFFER UNDERRUN WAS ABOUT TO RESULT IN HEAP CORRUPTION\n"));
        return NULL;
    }
    return (unsigned char *) this->q_buf->buffer->datas[0].data;
}
int bmswpw_send_sbuf_spoofed_loop(bmsw_pwout_t *this, uint32_t n)
{
    this->q_buf->buffer->datas[0].chunk->offset = 0;
    this->q_buf->buffer->datas[0].chunk->stride = this->event_data.format->stride;
    this->q_buf->buffer->datas[0].chunk->size = n * this->event_data.format->stride;
    pw_stream_queue_buffer(this->event_data.stream, this->q_buf);
    return 0;
}
int bmswpw_update_process(bmsw_pwout_t *this, bmswpw_process_t func)
{
    this->event_handler.process = func;
}
void *bmswpw_replace_buffer(void *client, void *newbuf, int newsize)
{
    bmsw_pwout_t *client_ = (bmsw_pwout_t *) client;
    void *rv = client_->in.u8;
    client_->in.u8 = newbuf;
    client_->in.size = newsize;
    return rv;
}

/*    Tests    */
int bmswtest_client_concurrent(const char *title)
{
    pw_init(NULL, NULL);

    bmsw_pwout_t client;
    bmswpw_init_buffer(&client, BMSPWM_SPA_FORMAT);

    bmswpw_init_events(&client, (const pw_stream_events_t) {.process = EXPERIMENTAL(T_STATIC_SINE, bmswpw_process)}, NULL, NULL);

    bmswpw_init_stream(&client, pw_properties_new(
            PW_KEY_MEDIA_TYPE, "Audio",
            PW_KEY_MEDIA_CATEGORY, "Playback",
            PW_KEY_MEDIA_ROLE, "Game",
            PW_KEY_MEDIA_NAME, "待機",
            PW_KEY_APP_NAME, title,
            NULL
    ));

    bmswpw_poll_start(&client);

    // Stop after 10s
    sleep(10);

    bmswpw_poll_stop(&client);

    bmswpw_destroy_buffer(&client);

    return 0;
}
int bmswtest_client_sequential(const char *title)
{
    // pw basic init code
    bmsw_pwout_t client = {0,};
    pw_init(NULL, NULL);

    // create a loop object
    client.event_loop.sequential = pw_main_loop_new(NULL);

    // create a stream object is this on_get_buffer??
    //_INFO: pw_stream_new() is an alternative with more control, but harder setup
    client.event_handler = (pw_stream_events_t) {
            PW_VERSION_STREAM_EVENTS,
            .process =  EXPERIMENTAL(T_STATIC_SINE, bmswpw_process)
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

    // This is only format of stream (in sync with on_is_format_supported), example for 2ch@44100hz:16bit depth stream
    const spa_pod_t *params[1];
    uint8_t buffer[1024];
    spa_pod_builder_t b = SPA_POD_BUILDER_INIT(buffer, sizeof(buffer)); // builder can go out of scope
    params[0] = spa_format_audio_raw_build(&b, SPA_PARAM_EnumFormat,
                                           &SPA_AUDIO_INFO_RAW_INIT(
                                                   .format = SPA_AUDIO_FORMAT_S16,
                                                   .channels = 2,
                                                   .rate = 44100));

    // Connects stream to main loop
    pw_stream_connect(client.event_data.stream,
                      PW_DIRECTION_OUTPUT, // specifies data.stream should be run in output mode
                      PW_ID_ANY,
                      PW_STREAM_FLAG_AUTOCONNECT | PW_STREAM_FLAG_MAP_BUFFERS | PW_STREAM_FLAG_RT_PROCESS, // stream flags, see limitations on PW_STREAM_FLAG_RT_PROCESS
                      params, 1 // params are supported formats
    );

    // Runs polling loop in THIS THREAD
    pw_main_loop_run(client.event_loop.sequential); // results in stream_events.process polling and block

    return 0;
}

/* Init */
void bmswexp_pw()
{
    bmsw_experiment_dispatcher[T_NOTIF_CALLBACK][bmswpw_get_sbuf_i] = bmswpw_get_sbuf_notif_callback;
    bmsw_experiment_dispatcher[T_NOTIF_SPICE][bmswpw_get_sbuf_i] = bmswpw_get_sbuf_notif_spice;
    bmsw_experiment_dispatcher[T_TWIN_CURSOR][bmswpw_get_sbuf_i] = bmswpw_get_sbuf_twin_cursor;
    bmsw_experiment_dispatcher[T_NOTIF_CALLBACK][bmswpw_send_sbuf_i] = bmswpw_send_sbuf_notif_callback;
    bmsw_experiment_dispatcher[T_NOTIF_SPICE][bmswpw_send_sbuf_i] = bmswpw_send_sbuf_notif_spice;
    bmsw_experiment_dispatcher[T_TWIN_CURSOR][bmswpw_send_sbuf_i] = bmswpw_send_sbuf_twin_cursor;
    bmsw_experiment_dispatcher[T_NOTIF_SPICE][bmswpw_callback_i] = callback_notif_spice;

    bmsw_experiment_dispatcher[T_NONE][bmswpw_update_process_i] = bmswpw_update_process;
    bmsw_experiment_dispatcher[T_NONE][bmswpw_replace_buffer_i] = bmswpw_replace_buffer;
}

#endif