CC=gcc
CFLAGS=-I include -L lib -l SDL2-2.0.0
SRC= main.c

run : main
	./main
	
main: main.c
	$(CC) $(SRC) -o main $(CFLAGS)

# experimental target x86_64
x86_64:
	$(CC) $(SRC) -o mainX86 $(CFLAGS) -march=x86-64

# experimental target for arm
arm64:
	$(CC) $(SRC) -o mainarm64 $(CFLAGS) -march=armv8-a

clean:
	rm -f main mainX86 mainarm64