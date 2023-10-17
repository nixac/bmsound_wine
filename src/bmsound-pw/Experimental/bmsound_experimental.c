#include <string.h>
#include <stdio.h>
#include "bmsound_experimental.h"


void *bmsw_experiment_dispatcher[T_LAST][FPTR_MAX];
profile_exp_t bmswexp_profile = T_NONE;
const char *const bmswexp_profile_name[T_LAST] = {
        [T_NONE] = "stable",
        [T_AUDIO_FORMAT] = "audio_format",
        [T_NOTIF_CALLBACK]  = "notif_callback",
        [T_NOTIF_SPICE]  = "notif_spice",
        [T_TWIN_CURSOR]  = "twin_cursor"
};

profile_exp_t bmswexp_profile_by_name(const char *name)
{
    for (profile_exp_t profile = T_NONE; profile < T_LAST; profile++)
    {
        if (bmswexp_profile_name[profile] && !strcmp(name, bmswexp_profile_name[profile]))
            return profile;
    }
    DBGEXEC(printf("Invalid profile '%s'\n", name));
    return bmswexp_profile;
}
const char *bmswexp_name_by_profile(profile_exp_t profile)
{
    static const char *profile_invalid = "invalid";
    if (profile >= T_NONE && profile < T_LAST && bmswexp_profile_name[profile]) return bmswexp_profile_name[profile];
    return profile_invalid;
}
