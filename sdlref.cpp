#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "sdlref.h"

enum class ErrorType {
    None,
    BadImageList,
    EmptyImageList,
};

struct RefImage {
    std::string filename;
    SDL_Texture *image;
    int rawWidth, rawHeight;
    double multiplier;
};

bool getFileList(std::vector<std::string> &dest, const std::string &filename);
void shuffleFileList(std::vector<std::string> &dest);

ReturnType showHelp(Screen &screen);

static int timerEventId = -1;


void scaleImage(const Screen &screen, RefImage &ref, SDL_Rect &destRect) {
    destRect.x = 0;
    destRect.y = 0;
    destRect.w = ref.rawWidth;
    destRect.h = ref.rawHeight;

    if (destRect.h != screen.height) {
        ref.multiplier = static_cast<double>(screen.height) / static_cast<double>(destRect.h);
        destRect.h *= ref.multiplier;
        destRect.w *= ref.multiplier;
        destRect.x = (screen.width - destRect.w) / 2;
    }
    if (destRect.w > screen.width) {
        ref.multiplier = static_cast<double>(screen.width) / static_cast<double>(destRect.w);
        destRect.h *= ref.multiplier;
        destRect.w *= ref.multiplier;
        destRect.x = 0;
        destRect.y = (screen.height - destRect.h) / 2;
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

std::string intervalToString(int interval) {
    if (interval == 0) return "0 s";
    std::stringstream ss;
    int hours = 0, minutes = 0;
    while (interval >= SECONDS_PER_HOUR)     { interval -= SECONDS_PER_HOUR;   ++hours;   }
    if (hours > 0)      ss << hours << " h ";
    while (interval >= SECONDS_PER_MINUTE)   { interval -= SECONDS_PER_MINUTE; ++minutes; }
    if (minutes > 0)    ss << minutes << " m ";
    if (interval > 0)   ss << interval << " s";
    return ss.str();
}


void mainloop(Screen &screen) {
    ErrorType error = ErrorType::None;

    std::vector<std::string> refImages;
    if (!getFileList(refImages, screen.listfile))   error = ErrorType::BadImageList;
    else if (refImages.empty())                     error = ErrorType::EmptyImageList;

    unsigned imageNumber = 0;
    int interval = 30;
    int counter = 0;
    bool showImageInformation = false;
    int fullscreenMode = 0;

    RefImage ref{""};
    SDL_Rect destRect;

    timerEventId = SDL_RegisterEvents(1);
    SDL_AddTimer(MS_PER_SECOND, timerCallback, nullptr);

    while (1) {
        SDL_SetRenderDrawColor(screen.renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(screen.renderer);

        if (error == ErrorType::None) {
            if (ref.image) {
                SDL_RenderCopy(screen.renderer, ref.image, nullptr, &destRect);
                if (interval > 0 && counter >= interval) {
                    SDL_SetRenderDrawColor(screen.renderer, 0, 0, 0, 127);
                    SDL_RenderFillRect(screen.renderer, nullptr);
                }
            } else {
                if (ref.filename.empty()) {
                    textout(screen, 10, 10, "Press SPACE to load image.");
                    textout(screen, 10, 35, "Press H or F1 to display help.");
                } else {
                    textout(screen, 10, 10, "Image not found.");
                }
            }

            if (interval > 0) {
                textout(screen, screen.width - HELP_WIDTH, 10, "Interval: " + intervalToString(interval));
                textout(screen, screen.width - HELP_WIDTH, 35, "Time: " + intervalToString(counter));
            }

            if (imageNumber < refImages.size() && showImageInformation) {
                std::stringstream ss;
                ss << "Size: " << ref.rawWidth << "x" << ref.rawHeight;
                ss << "  Scale: " << std::fixed << std::setprecision(2) << ref.multiplier;
                if (imageNumber != 0)   ss << "   Number: " << imageNumber;
                else                    ss << "   Number: " << refImages.size();
                ss << " of " << refImages.size();
                textout(screen, screen.width - ref.filename.size() * fontWidth - 10, screen.height - fontHeight * 2 - 15, ref.filename);
                textout(screen, screen.width - ss.str().size() * fontWidth - 10, screen.height - fontHeight - 10, ss.str());
            }

        } else if (error == ErrorType::BadImageList) {
            std::stringstream ss;
            ss << "Failed to load image file list \"" << screen.listfile << "\".";
            textout(screen, 10, 10, ss.str());
            textout(screen, 10, 35, "Press Q or ESCAPE to quit.");
        } else if (error == ErrorType::EmptyImageList) {
            textout(screen, 10, 10, "Image file list is empty.");
            textout(screen, 10, 35, "Press Q or ESCAPE to quit.");
        } else {
            std::stringstream ss;
            ss << "Unhandled error type " << static_cast<int>(error) << '.';
            textout(screen, 10, 10, ss.str());
            textout(screen, 10, 35, "Press Q or ESCAPE to quit.");
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
                    case SDLK_RIGHT:
                    case SDLK_SPACE:
                        if (error != ErrorType::None) break;
                        ref.filename = refImages[imageNumber];
                        getRef(screen, ref, destRect);
                        counter = 0;

                        ++imageNumber;
                        if (imageNumber >= refImages.size()) {
                            imageNumber = 0;
                        }
                        break;
                    case SDLK_LEFT:
                        if (error != ErrorType::None) break;
                        if (imageNumber > 0)    --imageNumber;
                        else                    imageNumber = refImages.size() - 1;

                        if (imageNumber == 0)   ref.filename = refImages[refImages.size() - 1];
                        else                    ref.filename = refImages[imageNumber - 1];
                        getRef(screen, ref, destRect);
                        counter = 0;
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

                    case SDLK_f:
                        if (fullscreenMode) fullscreenMode = 0;
                        else                fullscreenMode = SDL_WINDOW_FULLSCREEN_DESKTOP;
                        SDL_SetWindowFullscreen(screen.window, fullscreenMode);
                        break;
                    case SDLK_s:
                        shuffleFileList(refImages);
                        imageNumber = 0;
                        ref.filename = "";
                        ref.image = nullptr;
                        counter = 0;
                        break;
                    case SDLK_i:
                        showImageInformation = !showImageInformation;
                        break;
                    case SDLK_F1:
                    case SDLK_h: {
                        ReturnType rt = showHelp(screen);
                        if (rt == ReturnType::Quit) {
                            return;
                        } else if (rt == ReturnType::Rescale) {
                            scaleImage(screen, ref, destRect);
                        }
                        break; }
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


ReturnType showHelp(Screen &screen) {
    int column2 = screen.width / 2;
    bool rescale = false;
    while (1) {
        SDL_SetRenderDrawColor(screen.renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(screen.renderer);

        textout(screen, 10,  10, "    Q  Quit");
        textout(screen, 10,  35, "    R  Reset Timer");
        textout(screen, 10,  60, "    H  Show help (this screen)");
        textout(screen, 10,  85, "    I  Toggle image info");
        textout(screen, 10, 110, "    S  Reshuffle");
        textout(screen, 10, 135, " LEFT  Previous Image");
        textout(screen, 10, 160, "RIGHT  Next Image");
        textout(screen, 10, 185, "SPACE  Next Image");

        textout(screen, column2,  10, "Timer Length");
        textout(screen, column2,  35, "1  30s");
        textout(screen, column2,  60, "2  60s");
        textout(screen, column2,  85, "3  2m");
        textout(screen, column2, 110, "4  5m");
        textout(screen, column2, 135, "5  30m");
        textout(screen, column2, 160, "6  60m");
        textout(screen, column2, 185, "7  2h");
        textout(screen, column2, 210, "8  5h");
        textout(screen, column2, 235, "9  Off");

        textout(screen, 10, screen.height - 25, "Press a key to continue");
        SDL_RenderPresent(screen.renderer);


        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_USEREVENT) {
                // do nothing
            }
            if (event.type == SDL_QUIT) return ReturnType::Quit;
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_RESIZED) {
                screen.width = event.window.data1;
                screen.height = event.window.data2;
                column2 = screen.width / 2;
                rescale = true;
            }
            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_q) {
                    return ReturnType::Quit;
                } else {
                    return rescale ? ReturnType::Rescale : ReturnType::Normal;
                }
            }
        }
    }
}
