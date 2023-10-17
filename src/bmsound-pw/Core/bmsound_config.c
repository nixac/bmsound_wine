#include "bmsound_config.h"
#include "bmsound_experimental.h"
#include <yyjson.h>
#include <stddef.h>


const bmsw_config_t *bmsw_config = NULL;
static bmsw_config_t bmsw_config_;

int parse_json(const char *path)
{
    union
    {
        int64_t sint;
        uint64_t uint;
        double num;
        const char *str;
    } yy;
    yyjson_doc *cfgjson = yyjson_read_file(path, 0, NULL, NULL);
    yyjson_val *cfgroot = yyjson_doc_get_root(cfgjson);
    if (!cfgroot) return 0;

    //_INFO: *get_sint() fails for valid positive sint values (so it's get negative, more than get signed long)
    if (yyjson_ptr_get_str(cfgroot, "/audio/profile", &yy.str))
    {
        DBGEXEC(printf("Updating profile: '%s' -> '%s'\n", bmswexp_name_by_profile(*bmsw_config_.exp_profile), yy.str));
        *bmsw_config_.exp_profile = bmswexp_profile_by_name(yy.str);
    }
    if (yyjson_ptr_get_uint(cfgroot, "/audio/fpc", &yy.uint))
    {
        DBGEXEC(printf("Updating audio fpc: '%d' -> '%ld'\n", bmsw_config_.audio_fpc, yy.uint));
        bmsw_config_.audio_fpc = yy.sint;
    }
    if (yyjson_ptr_get_uint(cfgroot, "/audio/channels", &yy.uint))
    {
        DBGEXEC(printf("Updating audio channels: '%d' -> '%ld'\n", bmsw_config_.audio_channels, yy.uint));
        bmsw_config_.audio_channels = yy.sint;
    }
    if (yyjson_ptr_get_uint(cfgroot, "/audio/depth", &yy.uint))
    {
        DBGEXEC(printf("Updating audio depth: '%d' -> '%ld'\n", bmsw_config_.audio_depth, yy.uint));
        bmsw_config_.audio_depth = yy.sint;
    }

    yyjson_doc_free(cfgjson);
    return 1;
}

void bmsw_config_init(const char *path)
{
    bmsw_config = &bmsw_config_;
    bmsw_config_.exp_profile = &bmswexp_profile;

    // Defaults as per config specification in vault
    *bmsw_config_.exp_profile = T_NOTIF_SPICE;//_REV: T_NONE <- stable(should be default)
    bmsw_config_.audio_fpc = 64;
    bmsw_config_.audio_channels = 2;
    bmsw_config_.audio_depth = 16;

    // User config
    if (!parse_json(path))
    {
        if (path) DBGEXEC(printf("Invalid config: '%s'\n", path));
    }
}
