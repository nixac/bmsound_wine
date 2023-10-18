#ifndef _BMSOUND_PW_H
#define _BMSOUND_PW_H
#include <stdint.h>


/*    Utils (client=NULL for library-scoped queries)    */
int bmswpw_format_is_supported(int rate, int channel, int depth, void *client); // format availability for current endpoint
int bmswpw_format_period_fpc(void *client); // frames per chunk
long long bmswpw_format_period_wrt(void *client); // wrt per chunk _INFO: wasapi decided to invent a new time unit, so now we have to translate one more thing (WASAPI Reference Time)

/*    Core    */
void *bmswpw_create(const char *title, void *event_cb, void *event_cb_arg);
int bmswpw_start(void *client);
int bmswpw_stop(void *client);
int bmswpw_destroy(void *client);
unsigned char *bmswpw_get_buffer(void *client, unsigned int n);
int bmswpw_release_buffer(void *client, unsigned int n);

/*    Advanced    */
void bmswpw_update_callback(void *client, void *event_cb, void *event_cb_arg);

#endif