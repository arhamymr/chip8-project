CC=gcc
CFLAGS=-I.
FILES = main.c lib/cpu.c lib/print.c

main: main.c
	$(CC) $(FILES) -o main $(CFLAGS)


# experimental target x86_64
x86_64:
	$(CC) $(FILES) -o mainX86 $(CFLAGS) -march=x86-64

# experimental target for arm
arm64:
	$(CC) $(FILES) -o mainarm64 $(CFLAGS) -march=armv8-a

clean:
	rm -f main mainX86 mainarm64