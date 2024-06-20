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

struct AppConfig {
    unsigned short screen_width;
    unsigned short screen_height;
    unsigned short screen_scale;
};


struct InitChip8 {
    SDL_Window *window;
    SDL_Renderer *renderer;
    struct Chip8 chip8;
    struct AppConfig appConfig;
};



struct InitChip8 app_init() {
    struct InitChip8 initChip8;
    struct AppConfig appConfig;

    appConfig.screen_width = 64;
    appConfig.screen_height = 32;
    appConfig.screen_scale = 10;

    initialize_chip8(&initChip8.chip8);
    initChip8.appConfig = appConfig;

    const char title[] = "CHIP8 Emulator";
    const int init_components = SDL_INIT_VIDEO | SDL_INIT_AUDIO;

    if (SDL_Init(init_components) != 0) {
        sdl_error("SDL_Init Error");
    }

    initChip8.window = SDL_CreateWindow(title, 
                                        SDL_WINDOWPOS_CENTERED, 
                                        SDL_WINDOWPOS_CENTERED, 
                                        appConfig.screen_width * appConfig.screen_scale, 
                                        appConfig.screen_height * appConfig.screen_scale, 
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
    struct AppConfig appConfig = initChip8.appConfig;

    load_program_to_memory("roms/ibm-logo.ch8", &chip8);
    
    int n = sizeof(chip8.memory) / sizeof(chip8.memory[0]);
    printf("data on n: %d\n", n);
    printf("rom loaded:");
    for (int i = 0; i < n; i++) {
        printf("%02X ", chip8.memory[i]);        
    }

    SDL_Surface* surface = SDL_GetWindowSurface(window);
    
    if (surface == NULL) {
        cleanup_sdl(window);
        sdl_error("SDL_GetWindowSurface Error: ");
        return 1;
    }


    SDL_Event e;
    bool quit = false;
    int opcodes_per_frame = 10;
    while(!quit){
        while (SDL_PollEvent(&e)){
            if (e.type == SDL_QUIT){
                quit = true;
            }
       }

       execute_opcode(&chip8);
       render_display(renderer, &chip8);
       // Update the screen
       SDL_RenderPresent(renderer);
    }

    cleanup_sdl(window);
    return 0;
}
