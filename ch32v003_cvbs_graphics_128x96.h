#pragma once
#include <ch32v003_cvbs.h>

typedef struct cvbs_graphics_128x96_context_s {
	cvbs_context_t cvbs;
	uint32_t frame_counter;

	uint8_t VRAM0[128/8+4];
	uint8_t VRAM1[128/8+4];
	uint8_t VRAM[128*96/8];
} cvbs_graphics_128x96_context_t;

static inline void cvbs_graphics_128x96_wait_for_vsync(cvbs_graphics_128x96_context_t *ctx) {
	volatile uint32_t *is = &ctx->frame_counter;
	unsigned was = *is;
	while (was == *is);
}

void cvbs_graphics_128x96_context_init(cvbs_graphics_128x96_context_t *cvbs_text);
