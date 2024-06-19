#include "image_loader.h"

void render_image(SDL_Renderer *renderer, char path[], int x, int y, int w, int h) {
    SDL_Surface* image = IMG_Load(path);

    if (image == NULL) {
        sdl_error("IMG_Load Error: ");
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, image);

    if (texture == NULL) {
        sdl_error("SDL_CreateTextureFromSurface Error: ");
    }

    
    SDL_Rect destRect;
    destRect.x = x;
    destRect.y = y;
    destRect.w = w;
    destRect.h = h;

    SDL_RenderCopy(renderer, texture, NULL, &destRect);
}