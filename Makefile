ARM_LINUX_PREFIX := arm-linux-musleabi
ARM_NONE_PREFIX := arm-none-eabi

SDL_CFLAGS := $(shell sdl2-config --cflags --libs)
CFLAGS := -Iinclude

CANON_CFLAGS := -Os -T./specs/canon_link.ld -fno-builtin -fPIC -pie -ffreestanding -fno-exceptions -march=armv7-a -DPLATFORM_CANON
LEXMARK_CFLAGS := -DPLATFORM_LEXMARK

all: data canon_pc lexmark_pc canon_wasm lexmark_wasm canon lexmark

canon_wasm:
	@echo "[BUILD] canon WASM"
	@emcc $(CFLAGS) -DWASM=1 -DPLATFORM_CANON_SDL -o build/canon_wasm.js src/*.c -g -lm -s USE_SDL=2
	@cat data/web.html | sed -e 's/FILENAME/canon_wasm/g' > build/canon_wasm.html

lexmark_wasm:
	@echo "[BUILD] lexmark WASM"
	@emcc $(CFLAGS) -DWASM=1 -DPLATFORM_LEXMARK_SDL -o build/lexmark_wasm.js src/*.c -g -lm -s USE_SDL=2
	@cat data/web.html | sed -e 's/FILENAME/lexmark_wasm/g' > build/lexmark_wasm.html

canon_pc:
	@echo "[BUILD] canon SDL"
	@gcc $(SDL_CFLAGS) -DPLATFORM_CANON_SDL $(CFLAGS) -o build/canon_sdl src/*.c -lSDL2

lexmark_pc:
	@echo "[BUILD] lexmark SDL"
	@gcc $(SDL_CFLAGS) -DPLATFORM_LEXMARK_SDL $(CFLAGS) -o build/lexmark_sdl src/*.c -lSDL2

canon:
	@echo "[BUILD] canon"
	@$(ARM_NONE_PREFIX)-gcc $(CANON_CFLAGS) $(CFLAGS) -o build/canon.elf src/crt0.s src/*.c -nostartfiles -nodefaultlibs -static
	@$(ARM_NONE_PREFIX)-objcopy -O binary build/canon.elf build/canon.bin

lexmark:
	@echo "[BUILD] lexmark"
	@$(ARM_LINUX_PREFIX)-gcc $(LEXMARK_CFLAGS) $(CFLAGS) -o build/lexmark.elf src/*.c -static

.PHONY: data
data:
	@echo "[BUILD] data"
	@xxd -i ./data/font_big.bin | sed -e 's/__data_//g' | sed -e 's/^unsigned/static unsigned/g' > ./include/font_big.h
	@xxd -i ./data/font_small.bin | sed -e 's/__data_//g' | sed -e 's/^unsigned/static unsigned/g' > ./include/font_small.h

.PHONY: clean
clean:
	@echo "[CLEAN]"
	@rm -f build/*
