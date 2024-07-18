# ch32v003_cvbs
Proof of concept CVBS / TV Out / PAL video output for CH32V003 microcontroller.

The old-school ZX81 (and the clone by Microdigial, TK85), used a shift register for TV out, with a bit of logic for sync. SPI is a shift register, so I found it can be used for this purpose.

[See it in action](https://x.com/lcsvh/status/1805445163596058799/video/1)

# Required Hardware
```
         390R
PD2 ----/\/\/\----+---- Video output
         220R     |
PC6 ----/\/\/\----|
         180R     |
GND ----/\/\/\----'
```

# Some insights
* SPI hardware is used for pixel data output, 6Mb/s.
* Timer 1 is used for sync:
    * Period equals 64us of a PAL scanline (most of the time).
    * TIM1 CH1 is set as PWM, active low, for sync pulses.
    * TIM1 CH3 is used to start SPI DMA at the right time.
* 3 fonts are available:
    * fonts/zx81.h is the original font and character coding.
    * fonts/zx81_ascii.h is based on the original, but extended to ascii, and uppercase symbols made bold. Check fonts/zx81_ascii_font.png, where red pixel are used to mark differences.
    * fonts/ascii.h is borrowed from [dhepper](https://github.com/dhepper/font8x8/blob/master/font8x8_basic.h).
* A VRAM array holds 24 rows, 32 columns of data, character codes as defined by the selected font.
* VRAM0/VRAM1 are a double buffer for pixel data, one line each. Extra bytes at the end are used for padding and ensuring 0-filled blanking interval.
* On HSYNC interrupt (Timer1 CH1) code the SPI DMA is prepared for the current pixel buffer, then the next buffer is filled.
* The `ch32v003_cvbs.*` files are supposed to implement most of the scanning logic, and defer specifics to on_scanline and on_vblank callbacks. Implementation is partial, with most initializations still in `main.c`.
* `ch32v003fun` is included as a submodule so:
    * `git clone --recursive` this repo, or
    * `git submodule update --init --recursive` after cloning.
* Fonts are not built automatically. Run `make -C fonts` before continuing.
* Not all depeendencies are properly set, always use `make clean all` to build.


License: MIT

©2024 Lucas. V. Hartmann
