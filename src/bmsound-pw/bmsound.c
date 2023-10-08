#include "bmsound_pw.h"
#include <stdio.h>


// Subunits private init function listing
void bmswexp_callbacks();
void bmswexp_pw();
static void init() __attribute__((constructor));

void init()
{
    printf("Initializing bmsound::pipewire\n");
#ifndef BMSW_NOEXPERIMENTAL
    bmswexp_pw();
    bmswexp_callbacks();
#endif
}
