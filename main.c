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

cvbs_context_t cvbs_context;
const uint8_t *active_font = zx81_ascii_font;

uint8_t VRAM0[36];
uint8_t VRAM1[36];
uint8_t VRAM[32*24];

/*
 * initialize SPI and DMA
 */
void spi_init( void )
{
	// Enable DMA + Peripherals
	RCC->AHBPCENR |= RCC_AHBPeriph_DMA1;
	RCC->APB2PCENR |= RCC_APB2Periph_GPIOC | RCC_APB2Periph_SPI1;

	// MOSI on PC6, 10MHz Output, alt func, push-pull
	GPIOC->CFGLR &= ~(0xf<<(4*6));
	GPIOC->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP_AF)<<(4*6);
	
	// SCK on PC5, 10MHz Output, alt func, push-pull
	GPIOC->CFGLR &= ~(0xf<<(4*5));
	GPIOC->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP_AF)<<(4*5);

	// Configure SPI 
	SPI1->CTLR1 = 
		SPI_NSS_Soft | SPI_CPHA_1Edge | SPI_CPOL_Low | SPI_DataSize_8b |
		SPI_Mode_Master | SPI_Direction_1Line_Tx |
		SPI_BaudRatePrescaler_8;

	// enable SPI port
	SPI1->CTLR1 |= SPI_CTLR1_SPE;
	SPI1->CTLR2 |= SPI_CTLR2_TXDMAEN;
}

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

// Timer Init
void TIM1_UP_IRQHandler( void ) __attribute__((interrupt));
int32_t TIM1_UP_IRQHandler_active_duration;
int32_t TIM1_UP_IRQHandler_blank_duration;
void TIM1_UP_IRQHandler() {
	// Profiling interrupt duration
	int32_t start_of_interrupt = SysTick->CNT;

	//
	TIM1->INTFR &= ~TIM_UIF;

	cvbs_context_t *ctx = &cvbs_context;
	static cvbs_scanline_t scanline;

	// Based on the current line
	if (cvbs_is_active_line(&cvbs_context)) {
		static uint32_t data_length;
		data_length = scanline.data_length;

		DMA1_Channel3->MADDR = (uint32_t)scanline.data;
		DMA1_Channel6->MADDR = (uint32_t)&data_length;

		// Enable DMA trigger
		TIM1->DMAINTENR |= TIM_CC3DE;
	} else {
		// Stop DMA trigger
		TIM1->DMAINTENR &= ~TIM_CC3DE;
	}

	// Think about the next line
	cvbs_step(&cvbs_context);
	if (cvbs_is_active_line(ctx)) {
		if (ctx->on_scanline)
			ctx->on_scanline(ctx, &scanline);
	} else {
		if (ctx->on_vblank)
			ctx->on_vblank(ctx);
	}

	// Prepare next sync pulse, and horizontal_start
	TIM1->ATRLR = cvbs_horizontal_period(&cvbs_context);
	TIM1->CH1CVR = cvbs_sync(&cvbs_context);
	TIM1->CH3CVR = scanline.horizontal_start + ctx->pulse_properties->sync_normal;

	// Profiling interrupt duration
	start_of_interrupt = SysTick->CNT - start_of_interrupt;
	if (start_of_interrupt < 0) start_of_interrupt += SysTick->CMP+1;

	if (cvbs_is_active_line(ctx))
		TIM1_UP_IRQHandler_active_duration = start_of_interrupt;
	else
		TIM1_UP_IRQHandler_blank_duration = start_of_interrupt;
}

void timer_init() {
	RCC->APB2PCENR |= RCC_APB2Periph_TIM1 | RCC_APB2Periph_GPIOD;
	// Reset TIM1 to init all regs
	RCC->APB2PRSTR |= RCC_APB2Periph_TIM1;
	RCC->APB2PRSTR &= ~RCC_APB2Periph_TIM1;

	// CH1 on PD2, used as SYNC
	GPIOD->CFGLR &= ~(0xf<<(4*2));
	GPIOD->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP_AF)<<(4*2);

	// Timebase Setup
	TIM1->PSC = 0;
	TIM1->CTLR1 = TIM_ARPE | TIM_URS | TIM_CEN; // Auto-reload, use shadow, enable

	// CH1, PWM with shadow register, starts low.
	TIM1->CCER |= TIM_CC1E | TIM_CC1P;
	TIM1->CHCTLR1 |= 6*TIM_OC1M_0 + TIM_OC1PE;
	TIM1->BDTR |= TIM_MOE;

	// CH3 is used for DMA6 triggering
	TIM1->CHCTLR2 |= 6*TIM_OC3M_0 + TIM_OC3PE;

	TIM1->DMAINTENR = TIM_UIE;
	NVIC_EnableIRQ( TIM1_UP_IRQn );
}

void dma_init() {
	RCC->AHBPCENR |= RCC_AHBPeriph_DMA1;

	// SPI DMA is partially configured, completed later via DMA6
	DMA1_Channel3->CFGR =
		DMA_M2M_Disable |
		DMA_DIR_PeripheralDST |
		DMA_Priority_VeryHigh |
		DMA_MemoryInc_Enable |
		DMA_PeripheralInc_Disable |
		DMA_PeripheralDataSize_Byte |
		DMA_MemoryDataSize_Byte |
		DMA_Mode_Normal |
		DMA_CFGR1_EN;
	DMA1_Channel3->PADDR = (uint32_t)&SPI1->DATAR;
	static const uint32_t DMA1_Channel3_CNTR = 33;

	// DMA1_Channel6 triggered by TIM1_CH3, used solely to start SPI DMA at the right time
	DMA1_Channel6->PADDR = (uint32_t)&DMA1_Channel3->CNTR;
	DMA1_Channel6->MADDR = (uint32_t)&DMA1_Channel3_CNTR;
	DMA1_Channel6->CNTR  = 1;
	DMA1_Channel6->CFGR  =
		DMA_M2M_Disable |
		DMA_DIR_PeripheralDST |
		DMA_Priority_VeryHigh |
		DMA_MemoryInc_Disable |
		DMA_PeripheralInc_Disable |
		DMA_PeripheralDataSize_Word |
		DMA_MemoryDataSize_Word |
		DMA_Mode_Circular |
		DMA_CFGR1_EN;
}

/*
 * entry
 */
int main()
{
	memset(VRAM0, 0xE7, sizeof(VRAM0));
	memset(VRAM1, 0x81, sizeof(VRAM1));
	SystemInit();
	spi_init();
	timer_init();
	dma_init();

	cvbs_context_init(&cvbs_context, CVBS_STD_ZX81_PAL);
	cvbs_context.on_scanline = on_scanline;
	cvbs_context.on_vblank = on_vblank;

	for (int i=32; i<sizeof(VRAM); i++) {
//		VRAM[i] = i%64 + (i&0x40 ? 0x80 : 0);
//		VRAM[i] = i-32;
		VRAM[i] = ' ';
	}

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
