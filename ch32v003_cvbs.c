#include "ch32v003_cvbs.h"
#include "ch32v003fun.h"
#include <string.h>

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
    uint16_t dma_start;
    cvbs_pulse_t pulse_sequence[];
};

struct cvbs_context_s {
    uint8_t pulse_index;
    uint8_t pulse_counter;
    cvbs_pulse_t current_pulse;
    int line;

    const cvbs_pulse_properties_t *pulse_properties;
    void (*on_vblank)(cvbs_context_t *ctx, int line);
    void (*on_line)(cvbs_context_t *ctx, int line);
};

// Hardware requirements:
//   HCLK must be 48MHz.
//   Timer1 runs at HCLK, 48MHz;
//   Timer1 is used for horizontal frequency.
//   Timer1 Channel1 is PWM for SYNC pulses
//   Timer1 Channel3 is DMA trigger for first SPI transfer
//   SPI runs at pixel clock, 6MHz.
//   SPI DMA does 33 transfers to SPI TX buffer.

// https://www.batsocks.co.uk/readme/video_timing.htm

static const cvbs_pulse_properties_t PAL_pulse_properties = {
    // PAL 50Hz, 64us per scanline
    .horizontal_period = 3072, // 48MHz * 64us
    .sync_short        = 113,  // 48MHz * 2.35us
    .sync_normal       = 226,  // 48MHz * 4.7us
    .sync_long         = 1310, // 48MHz * (64us/2 - 4.7us)
    .dma_start         = 750,  // TBD

    // 625 lines = 640 pulses = 30 halfs + 610 periods
    .pulse_sequence = {
    //   H  S  L  A    N
        {1, 0, 1, 0,   5}, // Sync long/2
        {1, 1, 0, 0,   5}, // Sync short/2
        {0, 0, 0, 0,  46}, // Top blank
        {0, 0, 0, 1, 230}, // Active
        {0, 0, 0, 0,  29}, // Bottom blank
        {1, 1, 0, 0,   5}, // Sync short/2
        {1, 0, 1, 0,   5}, // Sync long/2
        {1, 1, 0, 0,   4}, // Sync short/2
        {0, 1, 0, 0,   1}, // Sync short
        {0, 0, 0, 0,  46}, // Top blank
        {0, 0, 0, 1, 230}, // Active
        {0, 0, 0, 0,  28}, // Bottom blank
        {1, 0, 0, 0,   1}, // Bottom blank/2
        {1, 1, 0, 0,   5}, // Sync short/2
        {0, 0, 0, 0,   0}, // END
    }
};

static const cvbs_pulse_properties_t *cvbs_pulse_properties[] = {
    [CVBS_STD_PAL] = &PAL_pulse_properties,
};

void cvbs_context_init(cvbs_context_t *ctx, cvbs_standard_t cvbs_standard) {
    memset(ctx, 0, sizeof(cvbs_context_t));
    ctx->pulse_properties = cvbs_pulse_properties[cvbs_standard];
}

void cvbs_step(cvbs_context_t *ctx) {
    if (--ctx->pulse_counter)
        return;

    ctx->current_pulse = ctx->pulse_properties->pulse_sequence[++ctx->pulse_index];

    if (!ctx->current_pulse.duration) {
        ctx->pulse_index = 0;
        ctx->current_pulse = ctx->pulse_properties->pulse_sequence[0];
    }

    ctx->pulse_counter = ctx->current_pulse.duration;
}

uint16_t cvbs_horizontal_period(cvbs_context_t *ctx) {
    return ctx->pulse_properties->horizontal_period >> ctx->current_pulse.half_period;
}

uint16_t cvbs_sync(cvbs_context_t *ctx) {
    if (ctx->current_pulse.short_sync)
        return ctx->pulse_properties->sync_short;

    if (ctx->current_pulse.long_sync)
        return ctx->pulse_properties->sync_long;

    return ctx->pulse_properties->sync_normal;
}
