#include "ch32v003_cvbs_graphics_128x96.h"

void v81_mandelbrot_128x96(cvbs_graphics_128x96_context_t *gfx) {
	// Start screen with checkerboard
	for (unsigned line=0; line < 96; line+=2) {
		memset(gfx->VRAM + 16*line +  0, 0x55, 16);
		memset(gfx->VRAM + 16*line + 16, 0xAA, 16);
	}

	mandelbrot_context_t ctx = {
		96, 48,
		1./64, 1./64,
		0
	};

	const unsigned HEIGHT = 96;
	const unsigned WIDTH = 128;

	for (int y=0; y<HEIGHT; y++) {
		for (int x=0; x<WIDTH; x+=8) {
			volatile uint8_t *vram = &gfx->VRAM[y*16 + x/8];
			*vram = 0;

			for (int dx=0; dx<8; dx++) {
				int m = 0x80 >> dx;
				if (mandlebrot_pixel(x+dx, y, &ctx))
					*vram |= m;
				else
					*vram &= ~m;
			}
		}
	}
}

