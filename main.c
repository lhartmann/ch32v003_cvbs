/*
 * CH32V003 CVBS output test.
 * 2024-07-17 Lucas V. Hartmann
 */

#include "ch32v003fun.h"
#include <stdio.h>
#include <string.h>
#include "fonts/zx81_ascii.h"
#include "mandlebrot.h"
#include "ch32v003_cvbs.h"
#include "ch32v003_cvbs_text_32x24.h"
#include "ch32v003_cvbs_graphics_128x96.h"
#include "hanoi.h"
#include "uart_dma.h"
#include "gfx_demo_noise.h"
#include "gfx_demo_mandelbrot.h"

static void graphics_demos() {
	cvbs_graphics_128x96_context_t cvbs_gfx;
	cvbs_graphics_128x96_context_init(&cvbs_gfx);
	cvbs_init(&cvbs_gfx.cvbs);
	printf("GFX init ok.\n");

	v81_mandelbrot_128x96(&cvbs_gfx);
	gfx_demo_noise(&cvbs_gfx);

	cvbs_finish(&cvbs_gfx.cvbs);
}

static void text_demos() {
	cvbs_text_32x24_context_t cvbs_text;

	cvbs_text_32x24_context_init(&cvbs_text);
	cvbs_text.active_font = zx81_ascii_font;
	cvbs_init(&cvbs_text.cvbs);

//	uart_vram_demo(&cvbs_text);
	hanoi_main(&cvbs_text);

	v81_mandelbrot(&cvbs_text);

	for (int i=0; i<60; i++) {
		Delay_Ms( 1000 );
		printf("%d, AD=%ld, BD=%ld, T=%d.\n",
			i++,
			TIM1_UP_IRQHandler_active_duration,
			TIM1_UP_IRQHandler_blank_duration,
			cvbs_horizontal_period(&cvbs_text.cvbs)
		);
	}

	cvbs_finish(&cvbs_text.cvbs);
}

int main() {
	SystemInit();

	while (true) {
		graphics_demos();
		text_demos();
	}
}
