# ch32v003_cvbs
Proof of concept CVBS / TV Out for CH32V003 microcontroller, supporting PAL and NTSC modes.

The old-school ZX81 (and the clone by Microdigial, TK85), used a shift register for TV out, with a bit of logic for sync. SPI is a shift register, so I found it can be used for this purpose.

See it in action: [Short video on X](https://x.com/lcsvh/status/1805445163596058799/video/1), [A few comments on youtube](https://www.youtube.com/watch?v=9GBjHImoiPg).

![vlcsnap-2024-09-11-23h45m57s923c](https://github.com/user-attachments/assets/9c153eb1-9620-4f3e-b59b-5af53daf31c4)

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

1. Create a context, either `cvbs_text_32x24_context_t` for text mode, or `cvbs_graphics_128x96_context_t` for graphics mode.
2. Initialize context.
3. Update `context->VRAM`.
4. Finish the video once not needed.

## Text Mode (32x24)

Create the context (allocate VRAM), initialize, and start video. There are 3 fonts under `fonts/`, with ZX81-styled ASCII being preferred. It uses the same graphics as ZX-81, but remapped to ASCII, and extended with bold symbols for uppercase and a few extra symbols low-res graphics. Check `fonts/zx81_ascii_font.png` for the mapping.
```C
cvbs_text_32x24_context_t cvbs_text;      // Create a context
cvbs_text_32x24_context_init(&cvbs_text); // Initialize
cvbs_text.active_font = zx81_ascii_font;  // Select font
cvbs_init(&cvbs_text.cvbs);               // Enable video
```

Basic printf is supported.
```C
printf("Hello world!\n");
```

Or you can access VRAM directly:
```C
memset(cvbs_text.VRAM, '.', sizeof(cvbs_text.VRAM));
cvbs_text.VRAM[row*32 + col] = "X";
```

When you wish to stop video or change mode, disable it.
```C
cvbs_finish(&cvbs_text.cvbs);             // optionally, disable video.
```

## Graphics Mode (128x96)

Create the context (allocate VRAM), initialize, and start video.
```C
cvbs_graphics_128x96_context_t cvbs_gfx;
cvbs_graphics_128x96_context_init(&cvbs_gfx);
cvbs_init(&cvbs_gfx.cvbs);
```

Basic printf is *NOT* supported (yet). You have to access VRAM directly. Layout is 16 bytes per row, 1 bit per pixel, MSB at the left.
```C
memset(cvbs_gfx.VRAM, 0x00, sizeof(cvbs_gfx.VRAM)); // Clear screen

uint32_t offset = y*16 + x/8; // Sets a single pixel.
uint8_t mask = 0x80 >> (x % 8);
cvbs_gfx.VRAM[offset] |= mask;
```

When you wish to stop video or change mode, disable it.
```C
cvbs_finish(&cvbs_gfx.cvbs);
```

# Advanced Usage

For demo-style usage you can create new contexts. The base CVBS code will handle timing and DMA, and provides a pair of callbacks for you.

`on_scanline(...)` is called once per active display line, and should fill the `cvbs_scanline_t` structure as needed. So far, the structure has 4 fields:
* `data` must point to an array of pixel data, bytes, MSB is left-most. LSB of last byte must be zero to keep horizontal blank (byte must be even, or simply zero).
* `data_length` is kind of self-explanatory.
* `horizontal_start` controls when to start outputting pixels. It is a count of SYSCLK cycles from the falling edge of horizontal sync.
* `flags` can be used to change the pixel clock. The default 6MHz yields 256-320 pixlels per scanline. Lower clocks make wider pixels, and vice-versa.

Beware that `on_scanline(...)` runs on interrupt context, and simultaneous to SPI DMA. This means that the code should be fast, and handle potential race-conditons with foreground code on `main()`. Additionally, the pixel `data` array of the previous scanline is still in use for the DMA and must be preserved, potentially requiring double-buffering.

`on_vblank(...)` is called once per blanking scanline. Can be used for code vsyncing, or game logic updates.

Effects such as smooth horizontal scrolling, italic text, perspective graphics, or wobbly images can be achieved by setting `horizontal_start` approprieately. Smooth vertical scrolling or stretching can be achieved by setting `data` with some line offset. Also, high-res images can be displayed by pointing `data` to FLASH.

# Some insights
* SPI hardware is used for pixel data output, 3, 6 or 12Mb/s.
* Timer 1 is used for sync:
    * Period equals 64us of a scanline (most of the time).
    * TIM1 CH1 is set as PWM, active low, for sync pulses.
    * TIM1 CH3 is used to start SPI DMA at the right time.
* 3 fonts are available:zx81_ascii_font.png
    * fonts/zx81.h is the original font and character coding.
    * fonts/zx81_ascii.h is based on the original, but extended to ascii, and uppercase symbols made bold. Check fonts/zx81_ascii_font.png, where red pixel are used to mark differences.
    * fonts/ascii.h is borrowed from [dhepper](https://github.com/dhepper/font8x8/blob/master/font8x8_basic.h).
* On HSYNC interrupt (Timer1 CH1) code the SPI DMA is prepared for the current pixel buffer, then `on_scanline(...)` or `on_vblank(...)` will be called accordingly.
* The `ch32v003_cvbs.*` files are supposed to implement most of the scanning logic.
* `ch32v003fun` is included as a submodule so:
    * `git clone --recursive` this repo, or
    * `git submodule update --init --recursive` after cloning.
* Not all depeendencies are properly set, always use `make clean all` to build.

License: MIT

©2024 Lucas. V. Hartmann
