#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <SDL2/SDL.h>

struct Chip8
{
    unsigned short opcode;
    unsigned char memory[0xFFF];
    unsigned char v_register[16];
    unsigned short I; // special register to store memory addresses
    unsigned short pc;
    unsigned char gfx[64 * 32];
    unsigned char delay_timer;
    unsigned char sound_timer;
    unsigned short stack[16];
    unsigned short sp;
    unsigned char key[16];
};
