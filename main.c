//
//  main.c
//  first c++ project
//
//  Created by arham on 14/06/24.
//

#include "SDL2/SDL.h"

int main(void) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        printf("SDL_Init Error: %s\n", SDL_GetError());
        return 1;
    }
    return 0;
}
