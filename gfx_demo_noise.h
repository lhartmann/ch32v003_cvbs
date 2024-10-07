#include "ch32v003_cvbs_graphics_128x96.h"

void gfx_demo_noise(cvbs_graphics_128x96_context_t *gfx) {
	unsigned k = 12345678;
	for (int j=0; j<60*15; j++) {
		for (int i=0; i<sizeof(gfx->VRAM); i++) {
			*(volatile uint8_t *)&gfx->VRAM[i] = k;

			if (k&1)
				k = (k>>1) ^ 0xA0000001UL;
			else
				k = k>>1;
		}
		cvbs_graphics_128x96_wait_for_vsync(gfx);
	}
}
