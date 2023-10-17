#ifndef _BMSOUND_CONFIG_H
#define _BMSOUND_CONFIG_H
#include "bmsound_experimental.h"


typedef struct bmsw_config bmsw_config_t;
struct bmsw_config
{
    profile_exp_t *exp_profile;
    int audio_fpc; // frames requested per each chunk/process call
    int audio_channels;
    int audio_depth; // depth in bits
};

extern const bmsw_config_t *bmsw_config;

void bmsw_config_init(const char *path); // reinitializes bmsw_config, can be called multiple times, path=NULL for reset to default values

#endif