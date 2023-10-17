#include "bmsound_config.h"
#include <stdio.h>


// Subunit's private init function listing
void bmswexp_callbacks();
void bmswexp_pw();
static void init() __attribute__((constructor));

void init()
{
    printf("Initializing bmsound::pipewire\n");

#ifndef BMSW_NOEXPERIMENTAL
    // Experimental APIs
    bmswexp_pw();
    bmswexp_callbacks();
#endif

    // Built-in config
    bmsw_config_init(NULL);
}
