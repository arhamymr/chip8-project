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

SDL_Window *sdl_init() {
    const char title[] = "CHIP8 Emulator";
    const int init_components = SDL_INIT_VIDEO | SDL_INIT_AUDIO;

    if (SDL_Init(init_components) != 0) {
        sdl_error("SDL_Init Error");
    }

    SDL_Window *win = SDL_CreateWindow(title, 
                                        SDL_WINDOWPOS_CENTERED, 
                                        SDL_WINDOWPOS_CENTERED, 
                                        SCREEN_WIDTH * SCREEN_SCALE, 
                                        SCREEN_SCALE * SCREEN_SCALE, 
                                        0);

    if (win == NULL) {
        sdl_error("SDL_CreateWindow Error : ");
    }

    return win;
}


SDL_Renderer *sdl_renderer(SDL_Window *win) {
    SDL_Renderer *renderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_SOFTWARE | SDL_RENDERER_PRESENTVSYNC);
    
    if (renderer == NULL) {
        SDL_DestroyWindow(win);
        sdl_error("SDL_CreateRenderer Error: ");
    }

   

    return renderer;
}

int main(void) {

    SDL_Window *win = sdl_init();
    SDL_Renderer *renderer = sdl_renderer(win);

    SDL_Surface* surface = SDL_GetWindowSurface(win);
    
    if (surface == NULL) {
        cleanup_sdl(win);
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

        

        SDL_UpdateWindowSurface(win);
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
                SDL_RenderPresent(renderer);
            }
        
        }
        SDL_RenderPresent(renderer);
    }

    cleanup_sdl(win);
    return 0;
}
