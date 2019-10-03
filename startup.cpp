#include <iostream>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "sdlref.h"

int main(int argc, char *argv[]) {
    Screen screen;
    screen.listfile = "list.txt";

    if (argc > 2) {
        std::cerr << "USAGE: sdlref [list file name]\n";
        return 1;
    } else if (argc == 2) {
        screen.listfile = argv[1];
    }

    if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER) != 0){
        std::cout << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    screen.width = 1024;
    screen.height = 720;
    screen.window = SDL_CreateWindow("SDL Reference",
                                       SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                       screen.width, screen.height,
                                       SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_MAXIMIZED);
    if (screen.window == nullptr){
        std::cout << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    screen.renderer = SDL_CreateRenderer(screen.window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (screen.renderer == nullptr){
        std::cout << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(screen.window);
        SDL_Quit();
        return 1;
    }

    int result = IMG_Init(IMG_INIT_PNG);
    if (!result) {
        std::cerr << "Failed to initialize SDL_image: " << IMG_GetError() << "\n";
        SDL_DestroyRenderer(screen.renderer);
        SDL_DestroyWindow(screen.window);
        SDL_Quit();
        return 1;
    }

    screen.font = IMG_LoadTexture(screen.renderer, "font_20x15.png");
    if (!screen.font) {
        std::cerr << "Failed to load font: " << IMG_GetError() << "\n";
        IMG_Quit();
        SDL_DestroyRenderer(screen.renderer);
        SDL_DestroyWindow(screen.window);
        SDL_Quit();
        return 1;
    }

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "best");  // make the scaled rendering look smoother.
    SDL_SetRenderDrawBlendMode(screen.renderer, SDL_BLENDMODE_BLEND);

    try {
        mainloop(screen);
    } catch (std::runtime_error &e) {
        std::cerr << "Fatal Error: " << e.what() << "\n";
    }

    SDL_DestroyTexture(screen.font);
    SDL_DestroyRenderer(screen.renderer);
    SDL_DestroyWindow(screen.window);
    return 0;
}
