all : flash

TARGET:=main
LD_LIBS+=-lm

CH32FUN=support/ch32fun/ch32fun
MINICHLINK?=support/ch32fun/minichlink
ADDITIONAL_C_FILES=ch32v003_cvbs.c ch32v003_cvbs_text_32x24.c ch32v003_cvbs_graphics_128x96.c
EXTRA_ELF_DEPENDENCIES=fonts

TARGET_MCU?=CH32V003
include ${CH32FUN}/ch32fun.mk

.PHONY: fonts
fonts:
	make -C fonts

all: $(TARGET).bin
flash : cv_flash
clean : cv_clean
	make -C fonts clean
