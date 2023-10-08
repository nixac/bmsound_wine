#ifndef _BMSOUND_EXPERIMENTAL_H
#define _BMSOUND_EXPERIMENTAL_H
#include <stdint.h>


#define FPTR_MAX 16
/*    Experimental function forward declarations    */
// Client scope
enum bmswpw_exp_i
{
    bmswpw_get_sbuf_i,
    bmswpw_send_sbuf_i,
    bmswpw_process_i,
    bmswpw_callback_i,
    bmswpw_exp_i_LAST
};
typedef unsigned char *(*bmswpw_get_sbuf_t)(void *, uint32_t);
typedef int (*bmswpw_send_sbuf_t)(void *, uint32_t);
typedef void (*bmswpw_process_t)(void *);
typedef void (*bmswpw_callback_t)(void *);
// Common/shared scope
enum none_exp_i
{
    bmswpw_update_process_i,
    bmswpw_replace_buffer_i,
    none_exp_i_LAST
};
typedef int (*bmswpw_update_process_t)(void *, bmswpw_process_t);
typedef void *(*bmswpw_replace_buffer_t)(void *, void *, int);

/*    Experimental API wrappers    */
enum profile_exp
{
    T_NONE = 0, // doubles as profile independent storage

    /* audio backend tests */
    T_AUDIO_FORMAT,
    T_NOTIF_CALLBACK,
    T_NOTIF_SPICE, // utilizes notif_callback implementation, but syncs server to spice(client) instead of client to server _INFO: working, stable candidate
    T_TWIN_CURSOR,
    T_SPOOFED_LOOP,
    T_STATIC_SINE,

    T_LAST
};
typedef enum profile_exp profile_exp_t;
extern void *bmsw_experiment_dispatcher[T_LAST][FPTR_MAX];
extern profile_exp_t bmsexp_profile;
#define EXPERIMENTAL(__expid, __func) (((__func##_t)bmsw_experiment_dispatcher[__expid][__func##_i]))
#ifndef BMSW_NODBGEXEC
#define DBGEXEC(__body) __body
#else
#define DBGEXEC(__body)
#endif

#endif