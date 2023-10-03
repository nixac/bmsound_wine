#ifndef _BMSOUND_PW_H
#define _BMSOUND_PW_H
/*_TODO:
 * * format passed in data since process should be rate/channel info aware
 * */


/* User api*/
int bmswpw_start();
int bmswpw_deinit(const char *title);

/* Tests */
int bmswpw_exp_test_concurrent(const char *title); // threaded sine
int bmswpw_exp_test_sequential(const char *title); // blocking sine
int pw_client_init(const char *title);

#endif