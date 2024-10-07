// ch32v003_ntsc.h
#ifndef CH32V003_CVBS_H
#define CH32V003_CVBS_H
#include <stdint.h>
#include <stdbool.h>

typedef struct cvbs_context_s cvbs_context_t;
typedef struct cvbs_pulse_s cvbs_pulse_t;
typedef struct cvbs_pulse_properties_s cvbs_pulse_properties_t;

typedef struct cvbs_scanline_s {
    uint16_t horizontal_start;
    uint16_t data_length;
    const uint8_t *data; // Last byte must be zero (HBLANK requirement)
    struct {
        // Default is 6MHz
        unsigned pixel_clock_12M : 1;
        unsigned pixel_clock_3M  : 1;
    } flags;
} cvbs_scanline_t;

typedef struct cvbs_pulse_s {
    unsigned half_period : 1;
    unsigned short_sync  : 1;
    unsigned long_sync   : 1;
    unsigned active      : 1;
    unsigned duration    : 8; // index based on pulses
} cvbs_pulse_t;

struct cvbs_pulse_properties_s {
    uint16_t horizontal_period;
    uint16_t sync_short;
    uint16_t sync_normal;
    uint16_t sync_long;
    cvbs_pulse_t pulse_sequence[];
};

struct cvbs_context_s {
    uint8_t pulse_index;
    uint8_t pulse_counter;
    cvbs_pulse_t current_pulse;
    int line;

    const cvbs_pulse_properties_t *pulse_properties;
    void (*on_vblank)(cvbs_context_t *ctx);
    void (*on_scanline)(cvbs_context_t *ctx, cvbs_scanline_t *scanline);
};

typedef enum cvbs_standard_e {
    CVBS_STD_PAL,
    CVBS_STD_ZX81_PAL,
    CVBS_STD_ZX81_NTSC,
} cvbs_standard_t;

void cvbs_context_init(cvbs_context_t *ctx, cvbs_standard_t cvbs_standard);
void cvbs_init(cvbs_context_t *ctx);
cvbs_context_t *cvbs_get_active_context();

static inline int cvbs_is_active_line(cvbs_context_t *ctx) {
    return ctx->current_pulse.active;
}

static inline uint16_t cvbs_horizontal_period(cvbs_context_t *ctx) {
    return ctx->pulse_properties->horizontal_period >> ctx->current_pulse.half_period;
}

static inline uint16_t cvbs_sync(cvbs_context_t *ctx) {
    if (ctx->current_pulse.short_sync)
        return ctx->pulse_properties->sync_short;

    if (ctx->current_pulse.long_sync)
        return ctx->pulse_properties->sync_long;

    return ctx->pulse_properties->sync_normal;
}

static inline void cvbs_step(cvbs_context_t *ctx) {
    ctx->line++;

    if (--ctx->pulse_counter)
        return;

    bool was_active = cvbs_is_active_line(ctx);

    ctx->current_pulse = ctx->pulse_properties->pulse_sequence[++ctx->pulse_index];

    if (!ctx->current_pulse.duration) {
        ctx->pulse_index = 0;
        ctx->current_pulse = ctx->pulse_properties->pulse_sequence[0];
    }

    ctx->pulse_counter = ctx->current_pulse.duration;

    bool is_active = cvbs_is_active_line(ctx);

    if (was_active != is_active)
        ctx->line = 0;
}

extern int32_t TIM1_UP_IRQHandler_active_duration;
extern int32_t TIM1_UP_IRQHandler_blank_duration;

#endif // CH32V003_CVBS_H
