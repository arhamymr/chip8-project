#include "chip8.h"

void initialize_chip8(struct Chip8 *chip8) {
    chip8->pc = 0x200;
    chip8->opcode = 0;
    chip8->I = 0;
    chip8->sp = 0;
    chip8->delay_timer = 0;
    chip8->sound_timer = 0;

    for (int i = 0; i < 16; i++) {
        chip8->v_register[i] = 0;
        chip8->stack[i] = 0;
        chip8->key[i] = 0;
    }

    for (int i = 0; i < 64 * 32; i++) {
        chip8->gfx[i] = 0;
    }

    for (int i = 0; i < 0xFFF; i++) {
        chip8->memory[i] = 0;
    }

    unsigned char chip8_fontset[80] = {
        0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
        0x20, 0x60, 0x20, 0x20, 0x70, // 1
        0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
        0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
        0x90, 0x90, 0xF0, 0x10, 0x10, // 4
        0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
        0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
        0xF0, 0x10, 0x20, 0x40, 0x40, // 7
        0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
        0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
        0xF0, 0x90, 0xF0, 0x90, 0x90, // A
        0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
        0xF0, 0x80, 0x80, 0x80, 0xF0, // C
        0xE0, 0x90, 0x90, 0x90, 0xE0, // D
        0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
        0xF0, 0x80, 0xF0, 0x80, 0x80  // F
    };

    for (int i = 0; i < 80; i++) {
        // fontset starts at 0x50
        chip8->memory[0x50 + i] = chip8_fontset[i];
    }
}


void load_program_to_memory(const char *filename, struct Chip8 *chip8) {
    FILE *file = fopen(filename, "rb");

    if (file == NULL) {
        printf("Error: Couldn't open file %s\n", filename);
        exit(1);
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    rewind(file);

    if (file_size > 0xFFF - 0x200) {
        printf("Error: File size too large\n");
        exit(1);
    }

    unsigned char *buffer = (unsigned char *)malloc(file_size);

    if (buffer == NULL) {
        printf("Error: Couldn't allocate memory for buffer\n");
        exit(1);
    }

    int result = fread(buffer, 1, file_size, file);

    if (result != file_size) {
        printf("Error: Couldn't read file\n");
        exit(1);
    }

    for (int i = 0; i < file_size; i++) {
        chip8->memory[i + 0x200] = buffer[i];
    }

    fclose(file);
    free(buffer);
}

void execute_opcode(struct Chip8* chip8) {
    unsigned short opcode = chip8->memory[chip8->pc] << 8 | chip8->memory[chip8->pc + 1];

    switch (opcode & 0xF000) {
        // opcode 6a02;
        case 0x0000:
            chip8->pc += 2;
            break;
        case 0x1000:
            chip8->pc = opcode & 0x0FFF;
            break;
        case 0x2000:
            chip8->stack[chip8->sp] = chip8->pc;
            chip8->sp++;
            chip8->pc = opcode & 0x0FFF;
            break;
        case 0x3000:
            if (chip8->v_register[(opcode >> 8) & 0x000F] == (opcode & 0x00FF)) {
                chip8->pc += 4;
            } else {
                chip8->pc += 2;
            }
            break;
        case 0x4000:
            if (chip8->v_register[(opcode >> 8) & 0x000F] != (opcode & 0x00FF)) {
                chip8->pc += 4;
            } else {
                chip8->pc += 2;
            }
            break;
        case 0x5000:
            if (chip8->v_register[(opcode >> 8) & 0x000F] == chip8->v_register[(opcode >> 4) & 0x000F]) {
                chip8->pc += 4;
            } else {
                chip8->pc += 2;
            }
            break;
        case 0x6000:
            chip8->v_register[(opcode >> 8) & 0x000F] = opcode & 0x00FF;
            chip8->pc += 2;
            break;
        case 0x7000:
            chip8->v_register[(opcode >> 8) & 0x000F] += opcode & 0x00FF;
            chip8->pc += 2;
            break;
        case 0x8000:
            switch (opcode & 0x000F) {
                case 0x0000:
                    chip8->v_register[(opcode >> 8) & 0x000F] = chip8->v_register[(opcode >> 4) & 0x000F];
                    
                    break;
                case 0x0001:
                    chip8->v_register[(opcode >> 8) & 0x000F] |= chip8->v_register[(opcode >> 4) & 0x000F];
                   
                    break;
                case 0x0002:
                    chip8->v_register[(opcode >> 8) & 0x000F] &= chip8->v_register[(opcode >> 4) & 0x000F];
                   
                    break;
                case 0x0003:
                    chip8->v_register[(opcode >> 8) & 0x000F] ^= chip8->v_register[(opcode >> 4) & 0x000F];
                  
                    break;
                case 0x0004:
                {
                    unsigned short x = (opcode >> 8) & 0x000F;
                    unsigned short y = (opcode >> 4) & 0x000F;

                    unsigned short sum = chip8->v_register[x] + chip8->v_register[y];
                    
                    if (sum > 0xFF) {
                        chip8->v_register[0xF] = 1;
                    } else {
                        chip8->v_register[0xF] = 0;
                    }

                    chip8->v_register[x] = sum & 0x00FF;
              
                    break;
                }
                   
                case 0x0005:
                {
                    unsigned short x = (opcode >> 8) & 0x000F;
                    unsigned short y = (opcode >> 4) & 0x000F;
                    
                    unsigned short xGreaterThanY = chip8->v_register[x] > chip8->v_register[y];

                    if (xGreaterThanY) {
                        chip8->v_register[0xF] = 1;
                    } else {
                        chip8->v_register[0xF] = 0;
                    }

                    chip8->v_register[x] -= chip8->v_register[y];
                    
                    break;
                }
                    

                case 0x0006:
                {
                    unsigned short x = (opcode >> 8) & 0x000F;

                    if (chip8->v_register[x] & 0x0001) {
                        chip8->v_register[0xF] = 1;
                    } else {
                        chip8->v_register[0xF] = 0;
                    }

                    // shift right 1 bit equavalent to divide by 2
                    // you can shift or divide
                    chip8->v_register[x] >>= 1;

                    break;
                }
                    
                case 0x0007:
                {
                    unsigned short x = (opcode >> 8) & 0x000F;
                    unsigned short y = (opcode >> 4) & 0x000F;

                    unsigned short yGreaterThanX = chip8->v_register[y] > chip8->v_register[x];

                    if (yGreaterThanX) {
                        chip8->v_register[0xF] = 1;
                    } else {
                        chip8->v_register[0xF] = 0;
                    }

                    chip8->v_register[x] = chip8->v_register[y] - chip8->v_register[x];
                   
                    break; 
                }
                   
                case 0x000E:
                {
                    unsigned short x = (opcode >> 8) & 0x000F;

                    if (chip8->v_register[x] & 0x8000) {
                        chip8->v_register[0xF] = 1;
                    } else {
                        chip8->v_register[0xF] = 0;
                    }

                    // shift left 1 bit equivalent to multiply by 2
                    // you can shift or multiply
                    chip8->v_register[x] <<= 1;
                    break;
                }
                   
                default:
                    printf("Unknown sub opcode 0x8 : 0x%X\n", opcode);
                    chip8->pc += 2;
                    break;
            }
            chip8->pc += 2;
            break;
        case 0x9000:
        {
            unsigned short x = (opcode >> 8) & 0x000F;
            unsigned short y = (opcode >> 4) & 0x000F;
            
            if (chip8->v_register[x] != chip8->v_register[y]) {
                chip8->pc += 4;
            } else {
                chip8->pc += 2;
            }

            break;
        }
           
        case 0xA000:
            chip8->I = opcode & 0x0FFF;
            chip8->pc += 2;
            break;
        case 0xB000:
            chip8->pc = (opcode & 0x0FFF) + chip8->v_register[0];
            break;
        case 0xC000:
        {
            unsigned short x = (opcode >> 8) & 0x000F;
            unsigned short random = rand() % 0xFF;

            chip8->v_register[x] = random & (opcode & 0x00FF);
            chip8->pc += 2;
            break;
        }
            
        case 0xD000:
        {
            unsigned short x = chip8->v_register[(opcode >> 8) & 0x000F];
            unsigned short y = chip8->v_register[(opcode >> 4) & 0x000F];
            unsigned short n = opcode & 0x000F;
            unsigned short pixel;
            chip8->v_register[0xF] = 0;
            
            for (int row =0; row < n; row++) {
                pixel = chip8->memory[chip8->I + row];
                for (int col = 0; col < 8; col++) {
                    if ((pixel & (0x80 >> col)) != 0) {
                        if (chip8->gfx[(x + col + ((y + row) * 64))] == 1) {
                            chip8->v_register[0xF] = 1;
                        }
                        chip8->gfx[x + col + ((y + row) * 64)] ^= 1;
                    }
                }
            }

            chip8->pc += 2;
            break;
        }
            
        case 0xE000:
            switch (opcode & 0x00FF) {
                case 0x009E:
                    if (chip8->key[chip8->v_register[(opcode >> 8) & 0x000F]] != 0) {
                        chip8->pc += 4;
                    } else {
                        chip8->pc += 2;
                    }
                    break;
                case 0x00A1:
                    if (chip8->key[chip8->v_register[(opcode >> 8) & 0x000F]] == 0) {
                        chip8->pc += 4;
                    } else {
                        chip8->pc += 2;
                    }
                    break;
                default:
                    printf("Unknown sub opcode 0xE : 0x%X\n", opcode);
                    chip8->pc += 2;
                    break;
            }
            break;
        case 0xF000:
            switch (opcode & 0x00FF) {
                case 0x07:
                    chip8->v_register[(opcode >> 8) & 0x000F] = chip8->delay_timer;
                    chip8->pc += 2;
                    break;
                case 0x0A:
                {
                    unsigned short x = (opcode >> 8) & 0x000F;
                    
                    bool key_pressed = false;
                    
                    for (int i = 0; i < 16; i++) {
                        if (chip8->key[i] != 0) {
                            chip8->v_register[x] = i;
                            key_pressed = true;
                        }
                    }

                    if (!key_pressed) {
                        return;
                    }

                    chip8->pc += 2;
                    break;
                }
                    
                case 0x15:
                {
                    unsigned short x = (opcode >> 8) & 0x000F;
                    chip8->delay_timer = chip8->v_register[x];
                    chip8->pc += 2;
                    break;
                }
                case 0x18:
                {
                    unsigned short x = (opcode >> 8) & 0x000F;
                    chip8->sound_timer = chip8->v_register[x];
                    chip8->pc += 2;
                    break;
                }
                case 0x1E:
                {
                    unsigned short x = (opcode >> 8) & 0x000F;
                    chip8->I += chip8->v_register[x];
                    chip8->pc += 2;
                    break;
                }
                case 0x29:
                {
                    unsigned short x = (opcode >> 8) & 0x000F;
                    chip8->I = chip8->v_register[x] * 0x5;
                    chip8->pc += 2;
                    break;
                }
                case 0x33:
                {
                    unsigned short x = (opcode >> 8) & 0x000F;
                    chip8->memory[chip8->I] = chip8->v_register[x] / 100;
                    chip8->memory[chip8->I + 1] = (chip8->v_register[x] / 10) % 10;
                    chip8->memory[chip8->I + 2] = chip8->v_register[x] % 10;
                    chip8->pc += 2;
                    break;
                }
                case 0x55:
                {
                    unsigned short x = (opcode >> 8) & 0x000F;
                    for (int i = 0; i <= x; i++) {
                        chip8->memory[chip8->I + i] = chip8->v_register[i];
                    }
                    chip8->I += x + 1;
                    chip8->pc += 2;
                    break;
                }
                case 0x65:
                {
                    unsigned short x = (opcode >> 8) & 0x000F;
                    for (int i = 0; i <= x; i++) {
                        chip8->v_register[i] = chip8->memory[chip8->I + i];
                    }
                    chip8->I += x + 1;
                    chip8->pc += 2;
                    break;
                }
                default: 
                    printf("Unknown sub opcode: 0x%X\n", opcode & 0x00FF);
                    chip8->pc += 2;
                    break;
            }
            break;
        default: 
            printf("Unknown opcode: 0x%X\n", opcode);
            chip8->pc += 2;
            break;
    }
}

void render_display(SDL_Renderer *renderer, struct Chip8* chip8) {
    for (int i=0;i < sizeof(chip8->gfx); i++) {
        int x = i % 64;
        int y = i / 64;
        SDL_Rect fillRect = { x * 10, y * 10, 10, 10 };
        SDL_SetRenderDrawColor(renderer, chip8->gfx[i] == 1 ? 0xFF: 0x00, 0x00, 0x00, 0x00);
        SDL_RenderFillRect(renderer, &fillRect);
    }
}