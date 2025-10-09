#include "ch32v003_cvbs.h"
#include "ch32fun.h"
#include <string.h>

static cvbs_context_t *cvbs_context;
cvbs_context_t *cvbs_get_active_context() {
	return cvbs_context;
}

/*
 * initialize SPI and DMA
 */
static void spi_init( void )
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
int32_t TIM1_UP_IRQHandler_active_duration;
int32_t TIM1_UP_IRQHandler_blank_duration;
void TIM1_UP_IRQHandler( void ) __attribute__((interrupt));
void TIM1_UP_IRQHandler() {
	// Profiling interrupt duration
	int32_t start_of_interrupt = SysTick->CNT;

	//
	TIM1->INTFR &= ~TIM_UIF;

	static cvbs_scanline_t scanline;

	// Based on the current line
	if (cvbs_is_active_line(cvbs_context)) {
		static uint32_t data_length;
		data_length = scanline.data_length;

		DMA1_Channel3->MADDR = (uint32_t)scanline.data;
		DMA1_Channel6->MADDR = (uint32_t)&data_length;

		// Enable DMA trigger
		TIM1->DMAINTENR |= TIM_CC3DE;

		if (scanline.flags.pixel_clock_3M)
			SPI1->CTLR1 = (SPI1->CTLR1 & ~SPI_CTLR1_BR) | SPI_BaudRatePrescaler_16;
		else if (scanline.flags.pixel_clock_12M)
			SPI1->CTLR1 = (SPI1->CTLR1 & ~SPI_CTLR1_BR) | SPI_BaudRatePrescaler_4;
		else // pixel clock 6M
			SPI1->CTLR1 = (SPI1->CTLR1 & ~SPI_CTLR1_BR) | SPI_BaudRatePrescaler_8;
	} else {
		// Stop DMA trigger
		TIM1->DMAINTENR &= ~TIM_CC3DE;
	}

	// Think about the next line
	cvbs_step(cvbs_context);
	if (cvbs_is_active_line(cvbs_context)) {
		if (cvbs_context->on_scanline)
			cvbs_context->on_scanline(cvbs_context, &scanline);
	} else {
		if (cvbs_context->on_vblank)
			cvbs_context->on_vblank(cvbs_context);
	}

	// Prepare next sync pulse, and horizontal_start
	TIM1->ATRLR = cvbs_horizontal_period(cvbs_context);
	TIM1->CH1CVR = cvbs_sync(cvbs_context);
	TIM1->CH3CVR = scanline.horizontal_start + cvbs_context->pulse_properties->sync_normal;

	// Profiling interrupt duration
	start_of_interrupt = SysTick->CNT - start_of_interrupt;
	if (start_of_interrupt < 0) start_of_interrupt += SysTick->CMP+1;

	if (cvbs_is_active_line(cvbs_context))
		TIM1_UP_IRQHandler_active_duration = start_of_interrupt;
	else
		TIM1_UP_IRQHandler_blank_duration = start_of_interrupt;
}

static void timer_init() {
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

static void dma_init() {
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

void cvbs_context_init(cvbs_context_t *ctx) {
    memset(ctx, 0, sizeof(cvbs_context_t));
}

void cvbs_init(cvbs_context_t *ctx) {
    cvbs_context = ctx;
	RCC->APB2PRSTR &= ~(RCC_TIM1RST | RCC_SPI1RST);
    spi_init();
    timer_init();
    dma_init();
}

void cvbs_finish(cvbs_context_t *ctx) {
	// Disable timer interrupts
	NVIC_DisableIRQ( TIM1_UP_IRQn );

	// Disable DMA
	DMA1_Channel3->CFGR = 0;
	DMA1_Channel6->CFGR = 0;

	// Reset Timer1 and SPI
	RCC->APB2PRSTR |= RCC_TIM1RST | RCC_SPI1RST;

	cvbs_context = 0;
}
