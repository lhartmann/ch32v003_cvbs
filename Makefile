all : flash

TARGET:=main
LD_LIBS+=-lm

CH32V003FUN=support/ch32v003fun/ch32v003fun
MINICHLINK?=support/ch32v003fun/minichlink
ADDITIONAL_C_FILES=ch32v003_cvbs.c

include ${CH32V003FUN}/ch32v003fun.mk

fonts:
	make -C fonts

all: fonts $(TARGET).bin
flash : cv_flash
clean : cv_clean
	make -C fonts clean
