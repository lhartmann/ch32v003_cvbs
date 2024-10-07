#pragma once
#include <ch32v003_cvbs.h>

typedef struct cvbs_text_32x24_context_s {
    cvbs_context_t cvbs;
    uint32_t frame_counter;
    uint32_t cursor_position;

    const uint8_t *active_font;

    uint8_t VRAM0[36];
    uint8_t VRAM1[36];
    uint8_t VRAM[32*24];
} cvbs_text_32x24_context_t;

static inline void cvbs_text_32x24_wait_for_vsync(cvbs_text_32x24_context_t *ctx) {
    volatile uint32_t *is = &ctx->frame_counter;
	unsigned was = *is;
	while (was == *is);
}

void cvbs_text_32x24_context_init(cvbs_text_32x24_context_t *cvbs_text);
