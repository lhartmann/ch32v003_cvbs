#include "ch32v003_cvbs_graphics_128x96.h"
#include "container_of.h"
#include <string.h>

static void on_vblank(cvbs_context_t *cvbs) {
	cvbs_graphics_128x96_context_t *cvbs_gfx = container_of(cvbs, cvbs_graphics_128x96_context_t, cvbs);
	if (!cvbs->line) cvbs_gfx->frame_counter++;
}

//
static void on_scanline(cvbs_context_t *cvbs, cvbs_scanline_t *scanline) {
	cvbs_graphics_128x96_context_t *cvbs_gfx = container_of(cvbs, cvbs_graphics_128x96_context_t, cvbs);
	int line = cvbs->line / 2;
	uint8_t *img  = line&1 ? cvbs_gfx->VRAM1 : cvbs_gfx->VRAM0;

	const uint8_t *src  = cvbs_gfx->VRAM + line*(128/8);
	memcpy(img, src, 128/8);
	img[128/8] = 0;

	const cvbs_pulse_properties_t *pp = cvbs->pulse_properties;
	memset(scanline, 0, sizeof(*scanline));
	scanline->horizontal_start = (int)(5.7e-6*48e6) + pp->sync_normal;
	scanline->data_length = 17;
	scanline->data = img;
	scanline->flags.pixel_clock_3M = 1;
}

void cvbs_graphics_128x96_context_init(cvbs_graphics_128x96_context_t *cvbs_gfx) {
	memset(cvbs_gfx, 0, sizeof(*cvbs_gfx));
	cvbs_context_init(&cvbs_gfx->cvbs, CVBS_STD_ZX81_NTSC);
	cvbs_gfx->cvbs.on_scanline = on_scanline;
	cvbs_gfx->cvbs.on_vblank = on_vblank;
}
