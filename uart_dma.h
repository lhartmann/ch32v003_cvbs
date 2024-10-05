#include "ch32v003fun.h"

void uart_init(
	uint32_t baud
) {
	// Enable GPIOD and UART.
	RCC->APB2PCENR |= RCC_APB2Periph_GPIOD | RCC_APB2Periph_USART1;

	// Push-Pull, 10MHz Output, GPIO D5, with AutoFunction
	GPIOD->CFGLR &= ~(0xf<<(4*5));
	GPIOD->CFGLR |= (GPIO_CNF_IN_FLOATING)<<(4*6);
	GPIOD->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP_AF)<<(4*5);

	USART1->BRR = 48000000 / baud;
	USART1->CTLR1 = 0b10000000001100;
	USART1->CTLR2 = 0;
	USART1->CTLR3 = 0;//0b00011000000;
	USART1->GPR = 0;
}

void uart_dma_rx_start(
	uint8_t *rxbuff,
	size_t rxlen
) {
	// Uses DMA1 Channel 5
	DMA1_Channel5->PADDR = (uint32_t)&USART1->DATAR;
	DMA1_Channel5->MADDR = (uint32_t)rxbuff;
	DMA1_Channel5->CNTR  = rxlen;
	DMA1_Channel5->CFGR  =
		DMA_M2M_Disable |
		DMA_DIR_PeripheralSRC |
		DMA_Priority_Low |
		DMA_MemoryInc_Enable |
		DMA_PeripheralInc_Disable |
		DMA_PeripheralDataSize_Byte |
		DMA_MemoryDataSize_Byte |
		DMA_Mode_Normal |
		DMA_CFGR1_EN;

	USART1->CTLR3 |= USART_CTLR3_DMAR;
}

void uart_dma_rx_wait() {
	while (~DMA1->INTFR & DMA1_IT_TC5);
	DMA1->INTFCR = DMA1_IT_TC5;
}

void uart_vram_demo() {
	uart_init(921600);
	while(true) {
		wait_for_vsync();
		USART1->DATAR = '.';
		uart_dma_rx_start(VRAM, sizeof(VRAM));
//		uart_dma_rx_wait();
	}
}
