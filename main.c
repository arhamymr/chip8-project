//
//  main.c
//  first c++ project
//
//  Created by arham on 14/06/24.
//

#include <SDL2/SDL.h>
#include <stdbool.h>
#include <SDL2/SDL_image.h>
#include "image_loader.h"
#include "error.h"
#include "chip8.h"


#define SCREEN_WIDTH 64
#define SCREEN_HEIGHT 32 
#define SCREEN_SCALE 10

void cleanup_sdl(SDL_Window *win) {
    SDL_DestroyWindow(win);
    SDL_Quit();
}

void cleanup_image(SDL_Surface *image, SDL_Texture *texture) {
    SDL_FreeSurface(image);
    SDL_DestroyTexture(texture);
    SDL_FreeSurface(image);
    IMG_Quit();
}


struct InitChip8{
    SDL_Window *window;
    SDL_Renderer *renderer;
    struct Chip8 chip8;
};

struct InitChip8 app_init() {
    struct InitChip8 initChip8;
    initialize_chip8(&initChip8.chip8);

    const char title[] = "CHIP8 Emulator";
    const int init_components = SDL_INIT_VIDEO | SDL_INIT_AUDIO;

    if (SDL_Init(init_components) != 0) {
        sdl_error("SDL_Init Error");
    }

    initChip8.window = SDL_CreateWindow(title, 
                                        SDL_WINDOWPOS_CENTERED, 
                                        SDL_WINDOWPOS_CENTERED, 
                                        SCREEN_WIDTH * SCREEN_SCALE, 
                                        SCREEN_HEIGHT * SCREEN_SCALE, 
                                        0);

    if (initChip8.window == NULL) {
        sdl_error("SDL_CreateWindow Error : ");
    }

    initChip8.renderer = SDL_CreateRenderer(initChip8.window, -1, SDL_RENDERER_SOFTWARE | SDL_RENDERER_PRESENTVSYNC);
    
    if (initChip8.renderer == NULL) {
        SDL_DestroyWindow(initChip8.window);
        sdl_error("SDL_CreateRenderer Error: ");
    }

    return initChip8;
}

int main(void) {

    struct InitChip8 initChip8 = app_init();

    SDL_Window *window = initChip8.window;
    SDL_Renderer *renderer = initChip8.renderer;
    struct Chip8 chip8 = initChip8.chip8;


    int n = sizeof(chip8.memory) / sizeof(chip8.memory[0]);
    printf("data on n: %d\n", n);
    printf("rom loaded:");
    for (int i = 0; i < n; i++) {
        printf("%02X ", chip8.memory[i]);
        load_program_to_memory("roms/Pong.ch8", &chip8);
    }

    SDL_Surface* surface = SDL_GetWindowSurface(window);
    
    if (surface == NULL) {
        cleanup_sdl(window);
        sdl_error("SDL_GetWindowSurface Error: ");
        return 1;
    }


    SDL_Event e;
    bool quit = false;

    int posx = 0;
    int posy = 0;

    while(!quit){
        while (SDL_PollEvent(&e)){
            if (e.type == SDL_QUIT){
                quit = true;
            }
            switch (e.type){
                case SDL_KEYDOWN:
                    switch (e.key.keysym.sym){
                        case SDLK_LEFT:
                            posx -= 10;
                            break;
                        case SDLK_RIGHT:
                            posx += 10;
                            break;
                        case SDLK_UP:
                            posy -= 10;
                            break;
                        case SDLK_DOWN:
                            posy += 10;
                            break;
                    }
                    break;
            }
        }

        

        SDL_UpdateWindowSurface(window);
          // SDL_FreeSurface(image);
        // SDL_DestroyTexture(texture);
        SDL_RenderClear(renderer);
        render_image(renderer, "assets/wa-icon.png", posx, posy, 40,40);
        render_image(renderer, "assets/wa-icon.png", 40,40, 40,40);

        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 0);

        for (int i=0; i < SCREEN_WIDTH; i++) {
            for (int j=0; j < SCREEN_HEIGHT; j++) {
                SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255); // Set the color to green
                SDL_Rect pixel = { i * SCREEN_SCALE, j * SCREEN_SCALE, SCREEN_SCALE, SCREEN_SCALE };
                SDL_RenderFillRect(renderer, &pixel);
            }
        
        }
        SDL_RenderPresent(renderer);
    }

    cleanup_sdl(window);
    return 0;
}
