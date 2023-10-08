#ifndef _BMSOUND_TEST_H
#define _BMSOUND_TEST_H
/*_INFO:
 * Forwards declaration of experimental functions that can be used as self-contained test units
 * Exists mainly to ensure API consistency
 *
 * */


/*    Simple audio output    */
int bmswtest_client_concurrent(const char *title); // threaded sine
int bmswtest_client_sequential(const char *title); // blocking sine

#endif