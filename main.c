//
//  main.c
//  first c++ project
//
//  Created by arham on 14/06/24.
//

#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdint.h>

#define SCREEN_WIDTH 64
#define SCREEN_HEIGHT 32
#define SCREEN_SCALE 10

struct Chip8
{
    uint8_t opcode;
    uint8_t memory[0xFFF];
    uint8_t v_register[16];
    uint16_t I; // special register to store memory addresses
    uint16_t pc;
    uint8_t gfx[64 * 32];
    uint8_t delay_timer;
    uint8_t sound_timer;
    uint16_t stack[16];
    uint16_t sp;
    uint8_t key[16];
};

int app_keymap[16] = {
    SDLK_1,
    SDLK_2,
    SDLK_3,
    SDLK_4,
    SDLK_q,
    SDLK_w,
    SDLK_e,
    SDLK_r,
    SDLK_a,
    SDLK_s,
    SDLK_d,
    SDLK_f,
    SDLK_z,
    SDLK_x,
    SDLK_c,
    SDLK_v,
};

void sdl_error(const char msg[])
{
    printf("%s: %s\n", msg, SDL_GetError());
    SDL_Quit();
    exit(1);
}

void cleanup_sdl(SDL_Window *win)
{
    SDL_DestroyWindow(win);
    SDL_Quit();
}

struct InitChip8
{
    SDL_Window *window;
    SDL_Renderer *renderer;
    struct Chip8 chip8;
    SDL_AudioDeviceID audio_device;
};

void audio_callback(void *userdata, Uint8 *stream, int len)
{
    // Example: generate a simple square wave
    static unsigned int audio_pos = 0;
    for (int i = 0; i < len; i++)
    {
        stream[i] = (audio_pos++ & 0x40) ? 255 : 0;
    }
}

void initialize_chip8(struct Chip8 *chip8)
{
    chip8->pc = 0x200;
    chip8->opcode = 0;
    chip8->I = 0;
    chip8->sp = 0;
    chip8->delay_timer = 0;
    chip8->sound_timer = 0;

    for (int i = 0; i < 16; i++)
    {
        chip8->v_register[i] = 0;
        chip8->stack[i] = 0;
        chip8->key[i] = 0;
    }

    for (int i = 0; i < 64 * 32; i++)
    {
        chip8->gfx[i] = 0;
    }

    for (int i = 0; i < 0xFFF; i++)
    {
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

    for (int i = 0; i < 80; i++)
    {
        // fontset starts at 0x50
        chip8->memory[0x50 + i] = chip8_fontset[i];
    }
}

struct InitChip8 app_init()
{
    struct InitChip8 initChip8;
    initialize_chip8(&initChip8.chip8);

    const char title[] = "CHIP8 Emulator";
    const int init_components = SDL_INIT_VIDEO | SDL_INIT_AUDIO;

    if (SDL_Init(init_components) != 0)
    {
        sdl_error("SDL_Init Error");
    }

    initChip8.window = SDL_CreateWindow(title,
                                        SDL_WINDOWPOS_CENTERED,
                                        SDL_WINDOWPOS_CENTERED,
                                        SCREEN_WIDTH * SCREEN_SCALE,
                                        SCREEN_HEIGHT * SCREEN_SCALE,
                                        0);

    if (initChip8.window == NULL)
    {
        sdl_error("SDL_CreateWindow Error : ");
    }

    initChip8.renderer = SDL_CreateRenderer(initChip8.window, -1, SDL_RENDERER_SOFTWARE | SDL_RENDERER_PRESENTVSYNC);

    if (initChip8.renderer == NULL)
    {
        SDL_DestroyWindow(initChip8.window);
        sdl_error("SDL_CreateRenderer Error: ");
    }

    // init audio
    // audio beep
    SDL_AudioSpec want, have;
    SDL_memset(&want, 0, sizeof(want));
    want.freq = 48000;
    want.format = AUDIO_U8;
    want.channels = 1;
    want.samples = 2048;
    want.callback = audio_callback;

    SDL_AudioDeviceID audio_device = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);
    if (audio_device == 0)
    {
        printf("Failed to open audio: %s\n", SDL_GetError());
    }

    initChip8.audio_device = audio_device;

    return initChip8;
}

void load_program_to_memory(const char *filename, struct Chip8 *chip8)
{
    FILE *file = fopen(filename, "rb");

    if (file == NULL)
    {
        printf("Error: Couldn't open file %s\n", filename);
        exit(1);
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    rewind(file);

    if (file_size > 0xFFF - 0x200)
    {
        printf("Error: File size too large\n");
        exit(1);
    }

    unsigned char *buffer = (unsigned char *)malloc(file_size);

    if (buffer == NULL)
    {
        printf("Error: Couldn't allocate memory for buffer\n");
        exit(1);
    }

    int result = fread(buffer, 1, file_size, file);

    if (result != file_size)
    {
        printf("Error: Couldn't read file\n");
        exit(1);
    }

    for (int i = 0; i < file_size; i++)
    {
        chip8->memory[i + 0x200] = buffer[i];
        if (i < 10 || i > file_size - 10)
        { // Log the first and last 10 bytes loaded
            printf("Memory[0x%X] = 0x%X\n", i + 0x200, buffer[i]);
        }
    }

    fclose(file);
    free(buffer);
}

void execute_opcode(struct Chip8 *chip8)
{
    unsigned short opcode = chip8->memory[chip8->pc] << 8 | chip8->memory[chip8->pc + 1];

    printf("Execute opcode %02X \n", opcode);
    switch (opcode & 0xF000)
    {
    // opcode 6a02;
    case 0x0000:
        switch (opcode & 0x00FF)
        {
        case 0x00E0:
            for (int i = 0; i < 64 * 32; i++)
            {
                chip8->gfx[i] = 0;
            }
            break;
        case 0x00EE:
            chip8->sp--;
            chip8->pc = chip8->stack[chip8->sp];

            break;
        default:
            printf("Unknown sub opcode 0x0 : 0x%X\n", opcode);
            break;
        }

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
        if (chip8->v_register[(opcode >> 8) & 0x000F] == (opcode & 0x00FF))
        {
            chip8->pc += 4;
        }
        else
        {
            chip8->pc += 2;
        }
        break;
    case 0x4000:
        if (chip8->v_register[(opcode >> 8) & 0x000F] != (opcode & 0x00FF))
        {
            chip8->pc += 4;
        }
        else
        {
            chip8->pc += 2;
        }
        break;
    case 0x5000:
        if (chip8->v_register[(opcode >> 8) & 0x000F] == chip8->v_register[(opcode >> 4) & 0x000F])
        {
            chip8->pc += 4;
        }
        else
        {
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
        switch (opcode & 0x000F)
        {
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

            if (sum > 0xFF)
            {
                chip8->v_register[0xF] = 1;
            }
            else
            {
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

            if (xGreaterThanY)
            {
                chip8->v_register[0xF] = 1;
            }
            else
            {
                chip8->v_register[0xF] = 0;
            }

            chip8->v_register[x] -= chip8->v_register[y];
            break;
        }

        case 0x0006:
        {
            unsigned short x = (opcode >> 8) & 0x000F;

            if (chip8->v_register[x] & 0x0001)
            {
                chip8->v_register[0xF] = 1;
            }
            else
            {
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

            if (yGreaterThanX)
            {
                chip8->v_register[0xF] = 1;
            }
            else
            {
                chip8->v_register[0xF] = 0;
            }

            chip8->v_register[x] = chip8->v_register[y] - chip8->v_register[x];

            break;
        }

        case 0x000E:
        {
            unsigned short x = (opcode >> 8) & 0x000F;

            if (chip8->v_register[x] & 0x8000)
            {
                chip8->v_register[0xF] = 1;
            }
            else
            {
                chip8->v_register[0xF] = 0;
            }

            // shift left 1 bit equivalent to multiply by 2
            // you can shift or multiply
            chip8->v_register[x] <<= 1;
            break;
        }

        default:
            printf("Unknown sub opcode 0x8 : 0x%X\n", opcode);
            break;
        }
        chip8->pc += 2;
        break;
    case 0x9000:
    {
        unsigned short x = (opcode >> 8) & 0x000F;
        unsigned short y = (opcode >> 4) & 0x000F;

        if (chip8->v_register[x] != chip8->v_register[y])
        {
            chip8->pc += 4;
        }
        else
        {
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

        for (int row = 0; row < n; row++)
        {
            pixel = chip8->memory[chip8->I + row];
            for (int col = 0; col < 8; col++)
            {
                if ((pixel & (0x80 >> col)) != 0)
                {
                    if (chip8->gfx[(x + col + ((y + row) * 64))] == 1)
                    {
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
        switch (opcode & 0x00FF)
        {
        case 0x009E:
            if (chip8->key[chip8->v_register[(opcode >> 8) & 0x000F]] != 0)
            {
                chip8->pc += 4;
            }
            else
            {
                chip8->pc += 2;
            }
            break;
        case 0x00A1:
            if (chip8->key[chip8->v_register[(opcode >> 8) & 0x000F]] == 0)
            {
                chip8->pc += 4;
            }
            else
            {
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
        switch (opcode & 0x00FF)
        {
        case 0x0007:
            chip8->v_register[(opcode >> 8) & 0x000F] = chip8->delay_timer;
            chip8->pc += 2;
            break;
        case 0x000A:
        {
            printf("it reached the instruction set ?");

            unsigned short x = (opcode >> 8) & 0x000F;

            bool key_pressed = false;

            for (int i = 0; i < 16; i++)
            {
                if (chip8->key[i] != 0)
                {
                    chip8->v_register[x] = i;
                    key_pressed = true;
                }
            }

            if (!key_pressed)
            {
                return;
            }

            chip8->pc += 2;
            break;
        }

        case 0x0015:
        {
            unsigned short x = (opcode >> 8) & 0x000F;
            chip8->delay_timer = chip8->v_register[x];
            chip8->pc += 2;
            break;
        }
        case 0x0018:
        {
            unsigned short x = (opcode >> 8) & 0x000F;
            chip8->sound_timer = chip8->v_register[x];
            chip8->pc += 2;
            break;
        }
        case 0x001E:
        {
            unsigned short x = (opcode >> 8) & 0x000F;
            chip8->I += chip8->v_register[x];
            chip8->pc += 2;
            break;
        }
        case 0x0029:
        {
            unsigned short x = (opcode >> 8) & 0x000F;
            chip8->I = chip8->v_register[x] * 0x5;
            chip8->pc += 2;
            break;
        }
        case 0x0033:
        {
            unsigned short x = (opcode >> 8) & 0x000F;
            chip8->memory[chip8->I] = chip8->v_register[x] / 100;
            chip8->memory[chip8->I + 1] = (chip8->v_register[x] / 10) % 10;
            chip8->memory[chip8->I + 2] = chip8->v_register[x] % 10;
            chip8->pc += 2;
            break;
        }
        case 0x0055:
        {
            unsigned short x = (opcode >> 8) & 0x000F;
            for (int i = 0; i <= x; i++)
            {
                chip8->memory[chip8->I + i] = chip8->v_register[i];
            }
            chip8->I += x + 1;
            chip8->pc += 2;
            break;
        }
        case 0x0065:
        {
            unsigned short x = (opcode >> 8) & 0x000F;
            for (int i = 0; i <= x; i++)
            {
                chip8->v_register[i] = chip8->memory[chip8->I + i];
            }
            chip8->I += x + 1;
            chip8->pc += 2;
            break;
        }
        default:
            printf("Unknown sub opcode 0xF: 0x%X\n", opcode);
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

void render_display(SDL_Renderer *renderer, struct Chip8 *chip8)
{
    SDL_RenderClear(renderer);
    for (int i = 0; i < sizeof(chip8->gfx); i++)
    {
        int x = i % 64;
        int y = i / 64;
        SDL_Rect fillRect = {x * 10, y * 10, 10, 10};
        SDL_SetRenderDrawColor(renderer, chip8->gfx[i] == 1 ? 0xFF : 0x00, 0x00, 0x00, 0x00);
        SDL_RenderFillRect(renderer, &fillRect);
    }
}

// run this handle timer in 60HZ
void handle_timer(struct Chip8 *chip8, SDL_AudioDeviceID audio_device)
{

    // if non zero
    if (chip8->delay_timer > 0)
    {
        chip8->delay_timer--;
    }

    if (chip8->sound_timer > 0)
    {
        if (chip8->sound_timer == 1)
        {
            SDL_PauseAudioDevice(audio_device, 0); // Resume playing the beep sound
        }
        chip8->sound_timer--;
    }
    else
    {
        SDL_PauseAudioDevice(audio_device, 1); // Resume playing the beep sound
    }
}

void handle_keypres(struct Chip8 *chip8, int index, bool pressed)
{
    for (int i = 0; i < 16; i++)
    {
        printf("%d", chip8->key[i]);
        if (i < 15)
        {
            printf(", ");
        }
    }

    chip8->key[index] = pressed ? 1 : 0;
}

int main(void)
{

    struct InitChip8 initChip8 = app_init();

    SDL_Window *window = initChip8.window;
    SDL_Renderer *renderer = initChip8.renderer;
    struct Chip8 chip8 = initChip8.chip8;
    SDL_AudioDeviceID audio_device = initChip8.audio_device;

    load_program_to_memory("roms/Maze.ch8", &chip8);

    SDL_Surface *surface = SDL_GetWindowSurface(window);

    if (surface == NULL)
    {
        cleanup_sdl(window);
        sdl_error("SDL_GetWindowSurface Error: ");
        return 1;
    }

    // timer
    // SDL_TimerID timerID = SDL_AddTimer(17, timer_callback, &chip8);

    SDL_Event e;
    bool quit = false;

    Uint32 start_tick;

    while (!quit)
    {
        start_tick = SDL_GetTicks();
        execute_opcode(&chip8);

        handle_timer(&chip8, audio_device);

        while (SDL_PollEvent(&e))
        {
            switch (e.type)
            {
            case SDL_QUIT:
                quit = true;
                break;
            case SDL_KEYDOWN:
                printf("keydown ");
                for (int i = 0; i < 16; i++)
                {
                    if (e.key.keysym.sym == app_keymap[i])
                    {
                        handle_keypres(&chip8, i, true);
                    }
                }
                break;
            case SDL_KEYUP:
                printf("keyup ");
                for (int i = 0; i < 16; i++)
                {
                    if (e.key.keysym.sym == app_keymap[i])
                    {
                        handle_keypres(&chip8, i, false);
                    }
                }
                break;
            }
        }

        render_display(renderer, &chip8);
        Uint32 frame_time = SDL_GetTicks() - start_tick;
        if (frame_time < 16)
        {
            SDL_Delay(16 - frame_time);
        }
        SDL_RenderPresent(renderer);
    }

    cleanup_sdl(window);
    // SDL_RemoveTimer(timerID);
    return 0;
}
