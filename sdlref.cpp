#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include "sdlref.h"

struct RefImage {
    std::string filename;
    SDL_Texture *image;
    int rawWidth, rawHeight;
    double multiplier;
};

struct Button {
    int image;
    std::string text;
    int x, y, w, h;
};

bool getFileList(std::vector<std::string> &dest, const std::string &filename);
void shuffleFileList(std::vector<std::string> &dest);

static int timerEventId = -1;


void scaleImage(const Screen &screen, RefImage &ref, SDL_Rect &destRect) {
    destRect.w = ref.rawWidth;
    destRect.h = ref.rawHeight;

    if (destRect.h != screen.height) {
        ref.multiplier = static_cast<double>(screen.height) / static_cast<double>(destRect.h);
        destRect.h *= ref.multiplier;
        destRect.w *= ref.multiplier;
    }
    if (destRect.w > screen.width) {
        ref.multiplier = static_cast<double>(screen.width) / static_cast<double>(destRect.w);
        destRect.h *= ref.multiplier;
        destRect.w *= ref.multiplier;
    }

}

void getRef(Screen &screen, RefImage &ref, SDL_Rect &destRect) {
    destRect.x = destRect.y = 0;
    destRect.w = destRect.h = 0;
    ref.rawHeight = 0;
    ref.rawWidth = 0;

    if (ref.image) {
        SDL_DestroyTexture(ref.image);
    }

    ref.image = IMG_LoadTexture(screen.renderer, ref.filename.c_str());
    if (!ref.image) return;

    SDL_QueryTexture(ref.image, nullptr, nullptr, &ref.rawWidth, &ref.rawHeight);
    scaleImage(screen, ref, destRect);
}

void textout(Screen &screen, int x, int y, const std::string &text) {
    SDL_Rect src = { 0, 0, fontWidth, fontHeight };
    SDL_Rect dest = { x, y, fontWidth, fontHeight };

    for (char c : text) {
        src.x = (c - 32) * fontWidth;
        SDL_RenderCopy(screen.renderer, screen.font, &src, &dest);
        dest.x += fontWidth;
    }
}


Uint32 timerCallback(Uint32 interval, void *param) {
    SDL_Event event;
    event.type = timerEventId;
    SDL_PushEvent(&event);
    return MS_PER_SECOND;
}


void mainloop(Screen &screen) {
    std::vector<std::string> refImages;
    if (!getFileList(refImages, screen.listfile)) {
        std::cerr << "Failed to load image file list.\n";
        return;
    }
    if (refImages.empty()) {
        std::cerr << "Image file list is empty.\n";
        return;
    }

    unsigned imageNumber = 0;
    int interval = 30;
    int counter = 0;
    bool showHelp = true;

    RefImage ref{""};
    SDL_Rect destRect;

    timerEventId = SDL_RegisterEvents(1);
    SDL_AddTimer(MS_PER_SECOND, timerCallback, nullptr);

    while (1) {
        SDL_SetRenderDrawColor(screen.renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(screen.renderer);

        destRect.x = destRect.y = 0;
        if (ref.image) {
            SDL_RenderCopy(screen.renderer, ref.image, nullptr, &destRect);
            if (interval > 0 && counter >= interval) {
                SDL_Rect cover = { 0, 0, destRect.w, destRect.h };
                SDL_SetRenderDrawColor(screen.renderer, 0, 0, 0, 127);
                SDL_RenderFillRect(screen.renderer, &cover);
            }
        } else {
            if (ref.filename.empty())   textout(screen, 10, 10, "Press SPACE to load image.");
            else                        textout(screen, 10, 10, "Image not found.");
        }

        if (showHelp) {
            textout(screen, screen.width - HELP_WIDTH,  10, "    Q  Quit");
            textout(screen, screen.width - HELP_WIDTH,  35, "    R  Reset Timer");
            textout(screen, screen.width - HELP_WIDTH,  60, "    H  Toggle Help");
            textout(screen, screen.width - HELP_WIDTH,  85, "    S  Reshuffle");
            textout(screen, screen.width - HELP_WIDTH, 110, "SPACE  Next Image");
            textout(screen, screen.width - HELP_WIDTH, 160, "    1  30s timer");
            textout(screen, screen.width - HELP_WIDTH, 185, "    2  60s timer");
            textout(screen, screen.width - HELP_WIDTH, 210, "    3  2m timer");
            textout(screen, screen.width - HELP_WIDTH, 235, "    4  5m timer");
            textout(screen, screen.width - HELP_WIDTH, 260, "    5  30m timer");
            textout(screen, screen.width - HELP_WIDTH, 285, "    6  60m timer");
            textout(screen, screen.width - HELP_WIDTH, 310, "    7  2h timer");
            textout(screen, screen.width - HELP_WIDTH, 335, "    8  5h timer");
            textout(screen, screen.width - HELP_WIDTH, 360, "    9  No timer");

            if (interval > 0) {
                textout(screen, screen.width - HELP_WIDTH, 400, "Interval: " + std::to_string(interval) + "s");
                textout(screen, screen.width - HELP_WIDTH, 425, "Time: " + std::to_string(counter));
            }

            if (imageNumber != 0) {
                std::stringstream ss;
                ss << "Size: " << ref.rawWidth << "x" << ref.rawHeight;
                ss << "  Scale: " << std::fixed << std::setprecision(2) << ref.multiplier;
                ss << "   Number: " << (imageNumber - 1) << " of " << refImages.size();
                textout(screen, screen.width - ref.filename.size() * fontWidth, screen.height - fontHeight * 2, ref.filename);
                textout(screen, screen.width - ss.str().size() * fontWidth, screen.height - fontHeight, ss.str());
            }
        }

        SDL_RenderPresent(screen.renderer);



        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_USEREVENT) {
                if (interval > 0 && ref.image && counter < interval) ++counter;
            }
            if (event.type == SDL_QUIT) return;
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_RESIZED) {
                screen.width = event.window.data1;
                screen.height = event.window.data2;
                scaleImage(screen, ref, destRect);
            }
            if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                    case SDLK_SPACE:
                        ref.filename = refImages[imageNumber];
                        getRef(screen, ref, destRect);
                        counter = 0;

                        ++imageNumber;
                        if (imageNumber >= refImages.size()) {
                            imageNumber = 0;
                        }
                        break;

                    case SDLK_1:
                        interval = 30;
                        break;
                    case SDLK_2:
                        interval = 60;
                        break;
                    case SDLK_3:
                        interval = SECONDS_PER_MINUTE * 2;
                        break;
                    case SDLK_4:
                        interval = SECONDS_PER_MINUTE * 5;
                        break;
                    case SDLK_5:
                        interval = SECONDS_PER_MINUTE * 30;
                        break;
                    case SDLK_6:
                        interval = SECONDS_PER_MINUTE * 60;
                        break;
                    case SDLK_7:
                        interval = SECONDS_PER_HOUR * 2;
                        break;
                    case SDLK_8:
                        interval = SECONDS_PER_HOUR * 5;
                        break;
                    case SDLK_9:
                        interval = -1;
                        break;
                    case SDLK_0:
                        interval = 1;
                        break;

                    case SDLK_s:
                        shuffleFileList(refImages);
                        imageNumber = 0;
                        ref.filename = "";
                        ref.image = nullptr;
                        counter = 0;
                        break;
                    case SDLK_h:
                        showHelp = !showHelp;
                        break;
                    case SDLK_r:
                        counter = 0;
                        break;
                    case SDLK_q:
                        return;
                }
            }
        }
    }
}
