#include "error.h"

void sdl_error(const char msg[]) {
    printf("%s: %s\n", msg, SDL_GetError());
    SDL_Quit();
    exit(1);
}