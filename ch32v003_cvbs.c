#include "ch32v003_cvbs.h"
#include "ch32v003fun.h"
#include <string.h>

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
