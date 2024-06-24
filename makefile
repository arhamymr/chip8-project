CC=gcc 
CFLAGS=-I/opt/homebrew/include -L/opt/homebrew/lib  -l SDL2-2.0.0 -l SDL2_image -Wall -g
SRC=main.c

run : main
	./main

main: clean
	$(CC) $(SRC) -o main $(CFLAGS)

clean:
	rm -f *.o main