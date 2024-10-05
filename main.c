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
#include "hanoi.h"
#include "uart_dma.h"

static cvbs_text_32x24_context_t cvbs_text;

/*
 * entry
 */
int main()
{
	SystemInit();

	cvbs_text_32x24_context_init(&cvbs_text, CVBS_STD_ZX81_NTSC);
	cvbs_text.active_font = zx81_ascii_font;
	cvbs_init(&cvbs_text.cvbs);

//	uart_vram_demo(&cvbs_text);
	hanoi_main(&cvbs_text);

	v81_mandelbrot(&cvbs_text);

	int i=0;
	while(1) {
		Delay_Ms( 1000 );
		printf("%d, AD=%ld, BD=%ld, T=%d.\n",
			i++,
			TIM1_UP_IRQHandler_active_duration,
			TIM1_UP_IRQHandler_blank_duration,
			cvbs_horizontal_period(&cvbs_text.cvbs)
		);
	}
}
