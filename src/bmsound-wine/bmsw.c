// Unix includes
#include "bmsound_pw.h"
#include "bmsound_config.h"
#include <unistd.h>
//#include <pthread.h>
#include <stdio.h>
// Wine includes
#include <windef.h>
#include <windows.h>


typedef void (*BmswCallback_t)(void *);

void WINAPI BmswConfigInit(const char *path)
{
    bmsw_config_init(path);
}
void WINAPI BmswExperimentalProfile(const char *name)
{
    bmswexp_profile = bmswexp_profile_by_name(name);
}
void WINAPI BmswClientUpdateCallback(void *client, BmswCallback_t cb, void *arg)
{
    bmswpw_update_callback(client, cb, arg);
}
void* WINAPI BmswClientCreate(const char *title, BmswCallback_t cb, void *arg)
{
    return bmswpw_create(title, cb, arg);
}
int WINAPI BmswClientStart(void *client)
{
    return bmswpw_start(client);
}
int WINAPI BmswClientStop(void *client)
{
    return bmswpw_stop(client);
}
int WINAPI BmswClientDestroy(void *client)
{
    return bmswpw_destroy(client);
}
unsigned char* WINAPI BmswClientGetBuffer(void *client, unsigned int n)
{
    return bmswpw_get_buffer(client, n);
}
int WINAPI BmswClientReleaseBuffer(void *client, unsigned int n)
{
    return bmswpw_release_buffer(client, n);
}
int WINAPI BmswClientFormatSupported(int rate, int channel, int depth)
{
    return bmswpw_is_format_supported(rate, channel, depth);
}
LONGLONG WINAPI BmswClientWasapiPeriod(void *client)
{
    return bmswpw_wasapi_period(client);
}
