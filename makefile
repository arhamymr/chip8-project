CC=gcc 
CFLAGS=-I/opt/homebrew/include -L/opt/homebrew/lib  -l SDL2-2.0.0 -l SDL2_image -Wall -g
SRC=main.c


run : main
	./main

main: clean
	$(CC) -g $(SRC) -o main $(CFLAGS)

wasm-build:
	emcc main-web.c -s USE_SDL=2 -o web/index.html --preload-file roms/

wasm-run:
	http-server web/

clean:
	rm -f main