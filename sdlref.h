#ifndef SDLREF_H_384653425
#define SDLREF_H_384653425

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

const int MS_PER_SECOND = 1000;
const int SECONDS_PER_MINUTE = 60;
const int SECONDS_PER_HOUR = 3600;

const int HELP_WIDTH = 300;

const int fontWidth = 15;
const int fontHeight = 20;

struct Screen {
    int width, height;
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *font;
};

void mainloop(Screen &screen);

#endif
