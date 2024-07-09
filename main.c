/*
 * Example for SPI with circular DMA for audio output
 * 04-10-2023 E. Brombaugh
 */

#include "ch32v003fun.h"
#include <stdio.h>
#include <string.h>
#include "fonts/zx81_ascii.h"
#include "mandlebrot.h"
#include "ch32v003_cvbs.h"

// PAL timings from https://martin.hinner.info/vga/pal.html

// Horizontal:         us  pixel(@6MHz)
//   Sync pulse...:  4.66  28
//   Back porch...:  5.66  34
//   Active.......: 52.00  312 (only 256 will be used)
//   Front porch..:  1.66  10
//   Total........: 64.00  384
#define H_SYNC    28
#define H_BACK    34
#define H_ACTIVE 312
#define H_FRONT   10
#define H_TOTAL  384

#define H_DMA_START ((H_ACTIVE-256)/2 + H_SYNC + H_BACK - 8) // Where to start pixel output

// Vertical:        Lines (pulses)
//   Sync pulse...: 3   (  6)
//   Back porch...: 2.5 (  6)
//   Active.......: 253 (253)
//   Front porch..: 2.5 (  6)
//   Total........: 262 (269)
#define V_SYNC     5
#define V_BACK     5
#define V_ACTIVE 253
#define V_FRONT    6
#define V_TOTAL  (V_SYNC+V_BACK+V_ACTIVE+V_FRONT)

#define V_DMA_START ((V_ACTIVE-192)/2 + 6)
#define V_DMA_END   (V_DMA_START + 192)

#define PAL_EQUALIZE ((int)(6e6 * 2.35e-6 + 0.5))
#define PAL_SYNC     ((int)(6e6 * 27.3e-6 + 0.5))

uint8_t VRAM0[36];
uint8_t VRAM1[36];
uint8_t VRAM[32*24];

void DMA_queue_odd_line() {
	VRAM1[32] = 0;
	DMA1_Channel3->MADDR = (uint32_t)VRAM1;
}
void DMA_queue_even_line() {
	VRAM0[32] = 0;
	DMA1_Channel3->MADDR = (uint32_t)VRAM0;
}

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

// Timer Init
int timered = 0;
void TIM1_UP_IRQHandler( void ) __attribute__((interrupt));
int32_t TIM1_UP_IRQHandler_duration;
const uint8_t *active_font = zx81_ascii_font;
void TIM1_UP_IRQHandler() {
	TIM1_UP_IRQHandler_duration = SysTick->CNT;
	timered++;
	TIM1->INTFR &= ~TIM_UIF;

	static uint16_t line = 0;

	// Based on the current line
	if (line >= V_DMA_START && line < V_DMA_END) {
		if (line & 1) DMA_queue_odd_line();
		else          DMA_queue_even_line();
		// Enable DMA trigger
		TIM1->DMAINTENR |= TIM_CC3DE;
	} else {
		// Stop DMA trigger
		TIM1->DMAINTENR &= ~TIM_CC3DE;
	}

	// Think about the next line
	line++;

	//
	if (line == V_TOTAL) {
		line = 0;
		TIM1->ATRLR  = H_TOTAL-1;
		TIM1->CH1CVR = H_SYNC;
	} else if (line == V_ACTIVE) {
		// Start front porch
		TIM1->ATRLR  = H_TOTAL/2-1;
		TIM1->CH1CVR = PAL_EQUALIZE;
	} else if (line == V_ACTIVE + V_FRONT) {
		TIM1->CH1CVR = PAL_SYNC;
	} else if (line == V_ACTIVE + V_FRONT + V_SYNC) {
		TIM1->CH1CVR = PAL_EQUALIZE;
	}

	if (line >= V_DMA_START && line < V_DMA_END) {
		uint32_t active_line = line-V_DMA_START;
		uint8_t *img  = line&1 ? VRAM1 : VRAM0;
		const uint8_t *font = active_font+1 + ((active_line%8) << *active_font);/////// ASCII
		const uint8_t *src  = VRAM + active_line/8*32;

		for (int i=0; i<32; i++) {
			img[i]= font[src[i] & 0x7F] ^ (src[i]&0x80 ? 0xFF : 0); // Negateable, loopy: ~1040 cycles
//			img[i] = font[src[i]];                              // Non Negateable, loopy: ~780 cycles
		}

#if 0 // Non negateable, Unrolled loop: ~420 cycles
		int i=0;
		img[i] = font[src[i]]; i++;
		img[i] = font[src[i]]; i++;
		img[i] = font[src[i]]; i++;
		img[i] = font[src[i]]; i++;
		img[i] = font[src[i]]; i++;
		img[i] = font[src[i]]; i++;
		img[i] = font[src[i]]; i++;
		img[i] = font[src[i]]; i++;
		img[i] = font[src[i]]; i++;
		img[i] = font[src[i]]; i++;
		img[i] = font[src[i]]; i++;
		img[i] = font[src[i]]; i++;
		img[i] = font[src[i]]; i++;
		img[i] = font[src[i]]; i++;
		img[i] = font[src[i]]; i++;
		img[i] = font[src[i]]; i++;
		img[i] = font[src[i]]; i++;
		img[i] = font[src[i]]; i++;
		img[i] = font[src[i]]; i++;
		img[i] = font[src[i]]; i++;
		img[i] = font[src[i]]; i++;
		img[i] = font[src[i]]; i++;
		img[i] = font[src[i]]; i++;
		img[i] = font[src[i]]; i++;
		img[i] = font[src[i]]; i++;
		img[i] = font[src[i]]; i++;
		img[i] = font[src[i]]; i++;
		img[i] = font[src[i]]; i++;
		img[i] = font[src[i]]; i++;
		img[i] = font[src[i]]; i++;
		img[i] = font[src[i]]; i++;
		img[i] = font[src[i]]; i++;
#endif

#if 0 // Negateable, unrolled: ~670 cycles
		int i=0;
		img[i]= (font[src[i] & 0x3F]) ^ (src[i]&0x80 ? 0xFF : 0); i++;
		img[i]= (font[src[i] & 0x3F]) ^ (src[i]&0x80 ? 0xFF : 0); i++;
		img[i]= (font[src[i] & 0x3F]) ^ (src[i]&0x80 ? 0xFF : 0); i++;
		img[i]= (font[src[i] & 0x3F]) ^ (src[i]&0x80 ? 0xFF : 0); i++;
		img[i]= (font[src[i] & 0x3F]) ^ (src[i]&0x80 ? 0xFF : 0); i++;
		img[i]= (font[src[i] & 0x3F]) ^ (src[i]&0x80 ? 0xFF : 0); i++;
		img[i]= (font[src[i] & 0x3F]) ^ (src[i]&0x80 ? 0xFF : 0); i++;
		img[i]= (font[src[i] & 0x3F]) ^ (src[i]&0x80 ? 0xFF : 0); i++;
		img[i]= (font[src[i] & 0x3F]) ^ (src[i]&0x80 ? 0xFF : 0); i++;
		img[i]= (font[src[i] & 0x3F]) ^ (src[i]&0x80 ? 0xFF : 0); i++;
		img[i]= (font[src[i] & 0x3F]) ^ (src[i]&0x80 ? 0xFF : 0); i++;
		img[i]= (font[src[i] & 0x3F]) ^ (src[i]&0x80 ? 0xFF : 0); i++;
		img[i]= (font[src[i] & 0x3F]) ^ (src[i]&0x80 ? 0xFF : 0); i++;
		img[i]= (font[src[i] & 0x3F]) ^ (src[i]&0x80 ? 0xFF : 0); i++;
		img[i]= (font[src[i] & 0x3F]) ^ (src[i]&0x80 ? 0xFF : 0); i++;
		img[i]= (font[src[i] & 0x3F]) ^ (src[i]&0x80 ? 0xFF : 0); i++;
		img[i]= (font[src[i] & 0x3F]) ^ (src[i]&0x80 ? 0xFF : 0); i++;
		img[i]= (font[src[i] & 0x3F]) ^ (src[i]&0x80 ? 0xFF : 0); i++;
		img[i]= (font[src[i] & 0x3F]) ^ (src[i]&0x80 ? 0xFF : 0); i++;
		img[i]= (font[src[i] & 0x3F]) ^ (src[i]&0x80 ? 0xFF : 0); i++;
		img[i]= (font[src[i] & 0x3F]) ^ (src[i]&0x80 ? 0xFF : 0); i++;
		img[i]= (font[src[i] & 0x3F]) ^ (src[i]&0x80 ? 0xFF : 0); i++;
		img[i]= (font[src[i] & 0x3F]) ^ (src[i]&0x80 ? 0xFF : 0); i++;
		img[i]= (font[src[i] & 0x3F]) ^ (src[i]&0x80 ? 0xFF : 0); i++;
		img[i]= (font[src[i] & 0x3F]) ^ (src[i]&0x80 ? 0xFF : 0); i++;
		img[i]= (font[src[i] & 0x3F]) ^ (src[i]&0x80 ? 0xFF : 0); i++;
		img[i]= (font[src[i] & 0x3F]) ^ (src[i]&0x80 ? 0xFF : 0); i++;
		img[i]= (font[src[i] & 0x3F]) ^ (src[i]&0x80 ? 0xFF : 0); i++;
		img[i]= (font[src[i] & 0x3F]) ^ (src[i]&0x80 ? 0xFF : 0); i++;
		img[i]= (font[src[i] & 0x3F]) ^ (src[i]&0x80 ? 0xFF : 0); i++;
		img[i]= (font[src[i] & 0x3F]) ^ (src[i]&0x80 ? 0xFF : 0); i++;
		img[i]= (font[src[i] & 0x3F]) ^ (src[i]&0x80 ? 0xFF : 0); i++;
#endif
	}


	TIM1_UP_IRQHandler_duration = SysTick->CNT - TIM1_UP_IRQHandler_duration;
	if (TIM1_UP_IRQHandler_duration < 0) TIM1_UP_IRQHandler_duration += SysTick->CMP+1;
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
	TIM1->PSC = FUNCONF_SYSTEM_CORE_CLOCK/6e6-1; // 6MHz pixel clock
	TIM1->CTLR1 = TIM_ARPE | TIM_URS | TIM_CEN; // Auto-reload, use shadow, enable

	// CH1, PWM with shadow register, starts low.
	TIM1->CCER |= TIM_CC1E | TIM_CC1P;
	TIM1->CHCTLR1 |= 6*TIM_OC1M_0 + TIM_OC1PE;
	TIM1->BDTR |= TIM_MOE;

	// CH3 is used for DMA6 triggering
	TIM1->CHCTLR2 |= 6*TIM_OC3M_0 + TIM_OC3PE;
	TIM1->CH3CVR = H_DMA_START;

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

	for (int i=32; i<sizeof(VRAM); i++) {
//		VRAM[i] = i%64 + (i&0x40 ? 0x80 : 0);
		VRAM[i] = 0;
	}

	sprintf(VRAM, "Lucas Vinicius Hartmann  ");
	// v81_mandelbrot(VRAM);

	int i=0;
	while(1) {
		Delay_Ms( 1000 );
		printf("%d\n", i++);
//		printf("TIM1_UP_IRQHandler_duration = %ld\n", TIM1_UP_IRQHandler_duration);
	}
}

void VRAM_scroll() {
	memmove(VRAM, VRAM+32, sizeof(VRAM)-32);
	memset(VRAM + sizeof(VRAM) - 32,  ' ', 32);
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
