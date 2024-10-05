#include "ch32v003_cvbs_text_32x24.h"

#define WIDTH  (32*2)
#define HEIGHT (24*2)

typedef struct mandelbrot_context_s {
	// Where on screen is (0,0).
	// Counted in low-res pixels from top left.
	// Each VRAM character symbol is 2x2.
	int x0, y0;

	// Zoom control, distance between pixels
	double dx, dy;

	cvbs_text_32x24_context_t *cvbs_text;
} mandelbrot_context_t;

int is_mandlebrot(double re, double im) {
	double tr = 0;
	double ti = 0;
	double kr,ki;

	for (int i=0; i<100; i++) {
		// k = t*t+c;
		kr = tr*tr - ti*ti + re;
		ki = tr*ti + tr*ti + im;
		// t = k*k+c;
		double krkr = kr*kr;
		double kiki = ki*ki;
		if (krkr+kiki > 4) return 0;

		tr = kr*kr - ki*ki + re;
		ti = kr*ki + kr*ki + im;
	}
	return tr*tr + ti*ti < 1;
}

int mandlebrot_pixel(int x, int y, const mandelbrot_context_t *ctx) {
	double cr = (x - ctx->x0) * ctx->dx;
	double ci = (y - ctx->y0) * ctx->dy;
	return is_mandlebrot(cr,ci);
}

void v81_mandelbrot_screen(const mandelbrot_context_t *ctx) {
	for (int y=0; y<HEIGHT; y+=2) {
		for (int x=0; x<WIDTH; x+=2) {
			volatile uint8_t *vram = &ctx->cvbs_text->VRAM[y/2*32 + x/2];
			*vram = 0x1f;

			int tl = mandlebrot_pixel(x+0, y+0, ctx);
			int tr = mandlebrot_pixel(x+1, y+0, ctx);
			int bl = mandlebrot_pixel(x+0, y+1, ctx);
			int br = mandlebrot_pixel(x+1, y+1, ctx);

			*vram = (tl + tr*2 + bl*4) ^ (br ? 128^7 : 0);
		}
	}
}

void v81_mandelbrot(cvbs_text_32x24_context_t *cvbs_text) {
	mandelbrot_context_t ctx = {
		48, 24,
		1./32, 1./32,
		cvbs_text
	};

	v81_mandelbrot_screen(&ctx);
}

