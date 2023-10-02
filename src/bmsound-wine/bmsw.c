// Unix includes
#include "bmsound_pw.h"
// Wine includes
#include <windef.h>


int WINAPI bmsw_client_init(const char *title)
{
    return pw_client_init(title);
}
