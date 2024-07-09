all: ascii.h zx81_ascii_font.h zx81_font.h

ascii.h: makefont_ascii.py
	./makefont_ascii.py

zx81_ascii_font.h: makefont_zx81_ascii.py zx81_ascii_font.png
	./makefont_zx81_ascii.py

zx81_font.h: makefont_zx81.py zx81_font.png
	./makefont_zx81.py

clean:
	rm -f ascii.h zx81_ascii.h zx81.h || true
