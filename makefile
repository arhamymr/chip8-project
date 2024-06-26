CC=gcc 
CFLAGS=-I/opt/homebrew/include -L/opt/homebrew/lib  -l SDL2-2.0.0 -l SDL2_image -Wall -g
SRC=main.c


ifeq ($(USE_EMSCRIPTEN),1)
CFLAGS += -DEMS
endif


run : main
	./main

main: clean
	$(CC) $(SRC) -o main $(CFLAGS)

wasm-build:
	emcc main.c -s USE_SDL=2 -o web/index.html --preload-file roms/tetris.ch8

wasm-run:
	http-server .

clean:
	rm -f *.o web/**