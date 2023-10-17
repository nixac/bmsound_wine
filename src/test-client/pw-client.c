#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sndfile.h>
#include <signal.h>
#ifdef __cplusplus
extern "C" {
#endif
#include "bmsound_pw.h"
#include "bmsound_config.h"
#include "bmsound_test.h"
#ifdef __cplusplus
}
#endif


static const profile_exp_t profile = T_NOTIF_CALLBACK;
static void *(*test[T_LAST])(void *);
volatile sig_atomic_t sig_ = 0;

/*    Utils    */
void sig_handler(int sig)
{
    sig_ = sig;
}
int16_t *get_sndbuf(const char *fname)
{
    SF_INFO info;
    SNDFILE *file = sf_open(fname, SFM_READ, &info);
    if (file == NULL)
    {
        printf("File error: '%s'\n", fname ? fname : "");
        return NULL;
    }
    sf_command(file, SFC_GET_CURRENT_SF_INFO, &info, sizeof(info));

    // Load into the buffer
    int16_t *buffer = (signed short *) malloc(info.frames * info.channels * sizeof(int16_t));
    int numFramesRead = sf_readf_short(file, buffer, info.frames);

    sf_close(file);
    return buffer;
}

/*    Tests    */
// Just for testing pipewire server connection
void *test_static_sine(void *sndbuf_)
{
    bmswtest_client_concurrent("pw-client");
    bmswtest_client_sequential("pw-client");
    return NULL;
}
// Just for testing audio buffers for format misconfigurations
void *test_audio_format(void *sndbuf_)
{
    char *sndbuf = (char *) sndbuf_;
    void *client = bmswpw_create("pw-client", NULL, NULL);
    if(!client) return NULL;
    EXPERIMENTAL(T_NONE, bmswpw_update_process)(client, EXPERIMENTAL(T_AUDIO_FORMAT, bmswpw_process));
    free(EXPERIMENTAL(T_NONE, bmswpw_replace_buffer)(client, sndbuf, 0));

    bmswpw_start(client);

    while (!sig_) sleep(1);
    EXPERIMENTAL(T_NONE, bmswpw_replace_buffer)(client, malloc(50000), 0);// just a workaround for double free

    return client;
}
// Unsafe buffer mirroring with latency defined by cursors distance delta (proof-of-concept)
void *test_twin_cursor(void *sndbuf_)
{
    char *sndbuf = (char *) sndbuf_;
    void *client = bmswpw_create("pw-client", NULL, NULL);
    if(!client) return NULL;
    EXPERIMENTAL(T_NONE, bmswpw_update_process)(client, EXPERIMENTAL(T_TWIN_CURSOR, bmswpw_process));

    bmswpw_start(client);
    //_INFO should be 441 for 10msec, since usleep is not exactly known for its interruption precision, we use slightly more than backend to prevent buffer underruns during test as single underrun and audio desync increases to whole buffer size (loops back to be precise)
    while (!sig_)
    {
        memcpy(EXPERIMENTAL(T_TWIN_CURSOR, bmswpw_get_sbuf)(client, 446), sndbuf, 446 * 4);
        EXPERIMENTAL(T_TWIN_CURSOR, bmswpw_send_sbuf)(client, 446);
        sndbuf += 446 * 4;
        usleep(10000);
    }
    //_REV: could probably measure latency here after sndbuf flush (by comparing in/out cursors)

    return client;
}
// Event driven audio buffer fetching from ambiguous audio interface
struct tntcb_data
{
    void *client;
    char *sndbuf;
};
typedef struct tntcb_data tntcb_data_t;
void tntcb_cb(tntcb_data_t *ntcb_data)
{
    //_INFO: notify event handler from spicetools would be here (if callbacks to executable worked..), first process will be silent if get/release part wasn't called beforehand
    memcpy(EXPERIMENTAL(T_NOTIF_CALLBACK, bmswpw_get_sbuf)(ntcb_data->client, bmsw_config->audio_fpc), ntcb_data->sndbuf, bmsw_config->audio_fpc * 4);// amount here defines audio latency, seems to scale fine so far
    EXPERIMENTAL(T_NOTIF_CALLBACK, bmswpw_send_sbuf)(ntcb_data->client, bmsw_config->audio_fpc);
    ntcb_data->sndbuf += bmsw_config->audio_fpc * 4;
}
void *test_notify_cb(void *sndbuf)
{
    static tntcb_data_t forward; // must be in scope for client duration, managed by caller
    void *client = bmswpw_create("pw-client", (void *) tntcb_cb, &forward);
    if(!client) return NULL;
    EXPERIMENTAL(T_NONE, bmswpw_update_process)(client, EXPERIMENTAL(T_NOTIF_CALLBACK, bmswpw_process));
    forward.client = client;
    forward.sndbuf = (char *) sndbuf;

    bmswpw_start(client);
    return client;
}

int main(void)
{
    test[T_AUDIO_FORMAT] = test_audio_format;
    test[T_TWIN_CURSOR] = test_twin_cursor;
    test[T_NOTIF_CALLBACK] = test_notify_cb;
    test[T_STATIC_SINE] = test_static_sine;
    signal(SIGTERM, sig_handler);
    signal(SIGINT, sig_handler);
    bmsw_config_init("src/test-client/client-config.json");
    char *sndbuf = (char *) get_sndbuf("local/test/sousoushi.wav");

    /*    simulated loop on iidx side    */
    void *client = NULL;
    if (test[profile]) client = test[profile](sndbuf);

    while (!sig_) sleep(1);
    if (client) bmswpw_destroy(client);
    free(sndbuf);
    return 0;
}
