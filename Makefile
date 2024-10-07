all : flash

TARGET:=main
LD_LIBS+=-lm

CH32V003FUN=support/ch32v003fun/ch32v003fun
MINICHLINK?=support/ch32v003fun/minichlink
ADDITIONAL_C_FILES=ch32v003_cvbs.c ch32v003_cvbs_text_32x24.c ch32v003_cvbs_graphics_128x96.c
EXTRA_ELF_DEPENDENCIES=fonts

include ${CH32V003FUN}/ch32v003fun.mk

.PHONY: fonts
fonts:
	make -C fonts

all: $(TARGET).bin
flash : cv_flash
clean : cv_clean
	make -C fonts clean
