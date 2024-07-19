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

# Basic Usage

For use in text mode, just do `printf(...)` on `main()`, or directly modify the `VRAM[]` array. The `VRAM[]` layout is pretty simple, 32 bytes per text row, 24 rows. Font defaults to ZX81-styled ASCII so you can just-use strings, and a few extra symbols are provided for low-res graphics. Check `fonts/zx81_ascii_font.png` for the mapping.

# Demo-Scene Style Usage

If you want to show off then you probably want to modify `on_scanline(...)`. This function is called once per active display line, and should fill the `cvbs_scanline_t` structure as needed. So far, the structure has only 3 fields:
* `data` must point to an array of pixel data, bytes, MSB is left-most. LSB of last byte must be zero to keep horizontal blank (byte must be even, or simply zero).
* `data_length` is kind of self-explanatory.
* `horizontal_start` controls when to start outputting pixels. It is a count of SYSCLK cycles from the falling edge of horizontal sync.

Beware that `on_scanline(...)` runs on interrupt context, and simultaneous to SPI DMA. This means that the code should be fast, and handle potential race-conditons with foreground code on `main()`. Additionally, the pixel `data` array of the previous scanline is still in use for the DMA and must be preserved, potentially requiring double-buffering.

Effects such as smooth horizontal scrolling, italic text, perspective graphics, or wobbly images can be achieved by setting `horizontal_start` approprieately. Smooth vertical scrolling or stretching can be achieved by setting `data` with some line offset. Also, high-res images can be displayed by pointing `data` to flash.

P.S.: Horizontal stretch should be possible by changing the SPI clock on the fly, but this is not yet implemented. Needs a new field on `cvbs_scanline_t`, and changes on `TIM1_UP_IRQHandler()`.

# Some insights
* SPI hardware is used for pixel data output, 6Mb/s.
* Timer 1 is used for sync:
    * Period equals 64us of a PAL scanline (most of the time).
    * TIM1 CH1 is set as PWM, active low, for sync pulses.
    * TIM1 CH3 is used to start SPI DMA at the right time.
* 3 fonts are available:zx81_ascii_font.png
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
