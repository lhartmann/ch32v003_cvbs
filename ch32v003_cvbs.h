// ch32v003_ntsc.h
#pragma once
#include <stdint.h>

typedef struct cvbs_context_s cvbs_context_t;
typedef struct cvbs_pulse_s cvbs_pulse_t;
typedef struct cvbs_pulse_properties_s cvbs_pulse_properties_t;

typedef enum cvbs_standard_e {
    CVBS_STD_PAL,
} cvbs_standard_t;

void cvbs_context_init(cvbs_context_t *ctx, cvbs_standard_t cvbs_standard);
void cvbs_step(cvbs_context_t *ctx);
uint16_t cvbs_horizontal_period(cvbs_context_t *ctx);
uint16_t cvbs_sync(cvbs_context_t *ctx);
