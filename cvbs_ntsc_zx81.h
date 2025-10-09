// Hardware requirements:
//   HCLK must be 48MHz.
//   Timer1 runs at HCLK, 48MHz;
//   Timer1 is used for horizontal frequency.
//   Timer1 Channel1 is PWM for SYNC pulses
//   Timer1 Channel3 is DMA trigger for first SPI transfer
//   SPI runs at pixel clock, 6MHz.
//   SPI DMA does 33 transfers to SPI TX buffer.

// Based on the urls, based on 240p NES, then centered.
// https://www.batsocks.co.uk/readme/video_timing.htm
// https://www.nesdev.org/wiki/NTSC_video
static const cvbs_pulse_properties_t cvbs_NTSC_ZX81 = {
    .horizontal_period = 48e6 * 63.55e-6 + 0.5,
    .sync_short        = 48e6 * 4.7e-6/2 + 0.5,
    .sync_normal       = 48e6 * 4.7e-6 + 0.5,
    .sync_long         = 48e6 * (63.55e-6 - 4.7e-6) + 0.5,

    // 262 lines
    .pulse_sequence = {
    //   H  S  L  A    N
        {0, 0, 0, 0,  48}, // Pre-render blanking
        {0, 0, 0, 1, 192}, // 192 active lines centered over 240p.
        {0, 0, 0, 0,  19}, // Post-render blanking
        {0, 0, 1, 0,   3}, // Vsync
        {0, 0, 0, 0,   0}, // END
    }
};

