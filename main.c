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

volatile unsigned frame = 0;
void on_vblank(cvbs_context_t *ctx) {
	if (!ctx->line) frame++;
}
void wait_for_vsync() {
	unsigned was = frame;
	while (was == frame);
}

#include "hanoi.h"

cvbs_context_t cvbs_context;
const uint8_t *active_font = zx81_ascii_font;

uint8_t VRAM0[36];
uint8_t VRAM1[36];
uint8_t VRAM[32*24];

#include "uart_dma.h"

//
void on_scanline(cvbs_context_t *ctx, cvbs_scanline_t *scanline) {
	if (cvbs_is_active_line(&cvbs_context)) {
		uint8_t *img  = ctx->line&1 ? VRAM1 : VRAM0;
		const uint8_t *font = active_font+1 + ((ctx->line%8) << *active_font);/////// ASCII
		const uint8_t *src  = VRAM + ctx->line/8*32;

		#if 0 // Loopy code, ISR takes ~1175 cycles.
		for (int i=0; i<32; i++) {
			img[i] = font[src[i] & 0x7F] ^ (src[i]&0x80 ? 0xFF : 0);
		}
		#elif 1 // Partially unrolled, ISR takes ~800 cycles, costs +76 .text bytes.
		for (int i=0; i<32; i+=4) {
			img[i+0] = font[src[i+0] & 0x7F] ^ (src[i+0]&0x80 ? 0xFF : 0);
			img[i+1] = font[src[i+1] & 0x7F] ^ (src[i+1]&0x80 ? 0xFF : 0);
			img[i+2] = font[src[i+2] & 0x7F] ^ (src[i+2]&0x80 ? 0xFF : 0);
			img[i+3] = font[src[i+3] & 0x7F] ^ (src[i+3]&0x80 ? 0xFF : 0);
		}
		#else // Fully unrolled, ISR takes ~722 cycles, costs +836 .text bytes.
		int i=0;
		img[i] = font[src[i] & 0x7F] ^ (src[i]&0x80 ? 0xFF : 0); i++;
		img[i] = font[src[i] & 0x7F] ^ (src[i]&0x80 ? 0xFF : 0); i++;
		img[i] = font[src[i] & 0x7F] ^ (src[i]&0x80 ? 0xFF : 0); i++;
		img[i] = font[src[i] & 0x7F] ^ (src[i]&0x80 ? 0xFF : 0); i++;
		img[i] = font[src[i] & 0x7F] ^ (src[i]&0x80 ? 0xFF : 0); i++;
		img[i] = font[src[i] & 0x7F] ^ (src[i]&0x80 ? 0xFF : 0); i++;
		img[i] = font[src[i] & 0x7F] ^ (src[i]&0x80 ? 0xFF : 0); i++;
		img[i] = font[src[i] & 0x7F] ^ (src[i]&0x80 ? 0xFF : 0); i++;
		img[i] = font[src[i] & 0x7F] ^ (src[i]&0x80 ? 0xFF : 0); i++;
		img[i] = font[src[i] & 0x7F] ^ (src[i]&0x80 ? 0xFF : 0); i++;
		img[i] = font[src[i] & 0x7F] ^ (src[i]&0x80 ? 0xFF : 0); i++;
		img[i] = font[src[i] & 0x7F] ^ (src[i]&0x80 ? 0xFF : 0); i++;
		img[i] = font[src[i] & 0x7F] ^ (src[i]&0x80 ? 0xFF : 0); i++;
		img[i] = font[src[i] & 0x7F] ^ (src[i]&0x80 ? 0xFF : 0); i++;
		img[i] = font[src[i] & 0x7F] ^ (src[i]&0x80 ? 0xFF : 0); i++;
		img[i] = font[src[i] & 0x7F] ^ (src[i]&0x80 ? 0xFF : 0); i++;
		img[i] = font[src[i] & 0x7F] ^ (src[i]&0x80 ? 0xFF : 0); i++;
		img[i] = font[src[i] & 0x7F] ^ (src[i]&0x80 ? 0xFF : 0); i++;
		img[i] = font[src[i] & 0x7F] ^ (src[i]&0x80 ? 0xFF : 0); i++;
		img[i] = font[src[i] & 0x7F] ^ (src[i]&0x80 ? 0xFF : 0); i++;
		img[i] = font[src[i] & 0x7F] ^ (src[i]&0x80 ? 0xFF : 0); i++;
		img[i] = font[src[i] & 0x7F] ^ (src[i]&0x80 ? 0xFF : 0); i++;
		img[i] = font[src[i] & 0x7F] ^ (src[i]&0x80 ? 0xFF : 0); i++;
		img[i] = font[src[i] & 0x7F] ^ (src[i]&0x80 ? 0xFF : 0); i++;
		img[i] = font[src[i] & 0x7F] ^ (src[i]&0x80 ? 0xFF : 0); i++;
		img[i] = font[src[i] & 0x7F] ^ (src[i]&0x80 ? 0xFF : 0); i++;
		img[i] = font[src[i] & 0x7F] ^ (src[i]&0x80 ? 0xFF : 0); i++;
		img[i] = font[src[i] & 0x7F] ^ (src[i]&0x80 ? 0xFF : 0); i++;
		img[i] = font[src[i] & 0x7F] ^ (src[i]&0x80 ? 0xFF : 0); i++;
		img[i] = font[src[i] & 0x7F] ^ (src[i]&0x80 ? 0xFF : 0); i++;
		img[i] = font[src[i] & 0x7F] ^ (src[i]&0x80 ? 0xFF : 0); i++;
		img[i] = font[src[i] & 0x7F] ^ (src[i]&0x80 ? 0xFF : 0); i++;
		#endif
		img[32] = 0;

		const cvbs_pulse_properties_t *pp = ctx->pulse_properties;
		scanline->horizontal_start = (int)(5.7e-6*48e6) + pp->sync_normal;
		scanline->data_length = 33;
		scanline->data = img;
	}
}

/*
 * entry
 */
int main()
{
	memset(VRAM0, 0xE7, sizeof(VRAM0));
	memset(VRAM1, 0x81, sizeof(VRAM1));
	SystemInit();

	cvbs_context_init(&cvbs_context, CVBS_STD_ZX81_NTSC);
	cvbs_context.on_scanline = on_scanline;
	cvbs_context.on_vblank = on_vblank;

	cvbs_init(&cvbs_context);

	for (int i=32; i<sizeof(VRAM); i++) {
//		VRAM[i] = i%64 + (i&0x40 ? 0x80 : 0);
//		VRAM[i] = i-32;
		VRAM[i] = ' ';
	}

//	uart_vram_demo();
	hanoi_main(VRAM);

//	memset(VRAM, 0x1F, sizeof(VRAM));
	if (1) {
		const char *name = "Lucas Vinicius Hartmann";
		int n = strlen(name);
		memcpy(VRAM+sizeof(VRAM)-n, name, n);
	}

	v81_mandelbrot(VRAM);

	int i=0;
	while(1) {
		Delay_Ms( 1000 );
		printf("%d, AD=%ld, BD=%ld, T=%d.\n",
			i++,
			TIM1_UP_IRQHandler_active_duration,
			TIM1_UP_IRQHandler_blank_duration,
			cvbs_horizontal_period(&cvbs_context)
		);
	}
}

void VRAM_scroll() {
	memmove(VRAM, VRAM+32, sizeof(VRAM)-32);
	memset(VRAM + sizeof(VRAM) - 32, ' ', 32);
}

#if !FUNCONF_USE_DEBUGPRINTF
int putchar(int c) {
	static int pos = 0;

	switch(c) {
		case '\f':
			memset(VRAM, ' ', sizeof(VRAM));
			pos = 0;
			break;

		case '\r':
			pos = pos/32*32;
			break;

		case '\n':
			pos = pos/32*32+32;
			break;

		case '\b':
			if (pos & 31) pos--;
			break;

		case '\t':
			while(pos % 3) putchar(' ');
			break;

		default:
			while (pos >= sizeof(VRAM)) {
				VRAM_scroll();
				pos -= 32;
			}
			VRAM[pos++] = c;
	}
	return 0;
}

int _write(int fd, const char *buf, int size) {
	while (size--) putchar(*buf++);
	return 0;
}
#endif // !FUNCONF_USE_DEBUGPRINTF
