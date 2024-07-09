#define WIDTH  (32*2)
#define HEIGHT (24*2)

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

// int Mandlebrot_Pixel(int x, int y) {
// 	x -= WIDTH/2;
// 	y -= HEIGHT/2;
//
// 	double cr = (double)x/WIDTH*2.5 - 0.33333;
// 	double ci = (double)y/WIDTH*2.5;
// 	return is_mandlebrot(cr,ci);
// }

int mandlebrot_pixel(int x, int y, double x0, double x1, double y0, double y1) {
	double cr = x * (x1-x0) / WIDTH  + x0;
	double ci = y * (y1-y0) / HEIGHT + y0;
	return is_mandlebrot(cr,ci);
}

void v81_mandelbrot_screen(uint8_t *vram, double x0, double x1, double y0, double y1) {
	for (int y=0; y<HEIGHT; y+=2) {
		for (int x=0; x<WIDTH; x+=2) {
			*(volatile uint8_t *)&vram[y/2*32 + x/2] = 0x1f;

			int tl = mandlebrot_pixel(x+0, y+0, x0, x1, y0, y1);
			int tr = mandlebrot_pixel(x+1, y+0, x0, x1, y0, y1);
			int bl = mandlebrot_pixel(x+0, y+1, x0, x1, y0, y1);
			int br = mandlebrot_pixel(x+1, y+1, x0, x1, y0, y1);

			vram[y/2*32 + x/2] = (tl + tr*2 + bl*4) ^ (br ? 128^7 : 0);
		}
	}
}

void v81_mandelbrot(uint8_t *vram) {
	// double X0 = 0.750222;
	// double X1 = 0.749191;
	// double Y0 = 0.031161;
	// double Y1 = 0.031752;

	double Xc = -1.4142;
	double Yc =  0;
	double dx = 0.5/WIDTH;
	double dy = 0.5/WIDTH;

	// My original coordinates
	// double X0=-1.583;
	// double X1=+0.917;
	// double Y0=-937./1000;
	// double Y1=+937./1000;

	while(1) {
		double x0 = Xc - 32*dx;
		double x1 = Xc + 31*dx;
		double y0 = Yc - 24*dy;
		double y1 = Yc + 23*dy;
		v81_mandelbrot_screen(vram, x0, x1, y0, y1);

		dx *= 0.707;
		dy *= 0.707;
	}
}
