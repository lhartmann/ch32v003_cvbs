#include "ch32v003_cvbs_text_32x24.h"
#include "container_of.h"
#include <string.h>

static void on_vblank(cvbs_context_t *cvbs) {
	cvbs_text_32x24_context_t *cvbs_text = container_of(cvbs, cvbs_text_32x24_context_t, cvbs);
	if (!cvbs->line) cvbs_text->frame_counter++;
}

//
static void on_scanline(cvbs_context_t *cvbs, cvbs_scanline_t *scanline) {
	cvbs_text_32x24_context_t *cvbs_text = container_of(cvbs, cvbs_text_32x24_context_t, cvbs);
	uint8_t *img  = cvbs->line&1 ? cvbs_text->VRAM1 : cvbs_text->VRAM0;
	const uint8_t *font = cvbs_text->active_font+1 + ((cvbs->line%8) << *cvbs_text->active_font);/////// ASCII
	const uint8_t *src  = cvbs_text->VRAM + cvbs->line/8*32;

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

	const cvbs_pulse_properties_t *pp = cvbs->pulse_properties;
	scanline->horizontal_start = (int)(5.7e-6*48e6) + pp->sync_normal;
	scanline->data_length = 33;
	scanline->data = img;
}

#if !FUNCONF_USE_DEBUGPRINTF
int putchar(int c) {
	cvbs_context_t *cvbs = cvbs_get_active_context();
	cvbs_text_32x24_context_t *cvbs_text = container_of(cvbs, cvbs_text_32x24_context_t, cvbs);
	uint32_t *pos = &cvbs_text->cursor_position;

	switch(c) {
		case '\f':
			memset(cvbs_text->VRAM, ' ', sizeof(cvbs_text->VRAM));
			*pos = 0;
			break;

		case '\r':
			*pos = *pos/32*32;
			break;

		case '\n':
			*pos = *pos/32*32+32;
			break;

		case '\b':
			if (*pos & 31) (*pos)--;
			break;

		case '\t':
			while(*pos % 3) putchar(' ');
			break;

		default:
			while (*pos >= sizeof(cvbs_text->VRAM)) {
				memmove(cvbs_text->VRAM, cvbs_text->VRAM+32, sizeof(cvbs_text->VRAM)-32);
				memset(cvbs_text->VRAM + sizeof(cvbs_text->VRAM) - 32, ' ', 32);
				*pos -= 32;
			}
			cvbs_text->VRAM[(*pos)++] = c;
	}
	return 0;
}

int _write(int fd, const char *buf, int size) {
	while (size--) putchar(*buf++);
	return 0;
}
#endif // !FUNCONF_USE_DEBUGPRINTF

void cvbs_text_32x24_context_init(cvbs_text_32x24_context_t *cvbs_text) {
	memset(cvbs_text, 0, sizeof(*cvbs_text));
	cvbs_context_init(&cvbs_text->cvbs, CVBS_STD_ZX81_NTSC);
	cvbs_text->cvbs.on_scanline = on_scanline;
	cvbs_text->cvbs.on_vblank = on_vblank;
}
