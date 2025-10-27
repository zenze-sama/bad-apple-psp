#include <SDL.h>
#include <stdio.h>

#define SCREEN_WIDTH 480
#define SCREEN_HEIGHT 272
#define FRAME_SIZE (SCREEN_WIDTH * SCREEN_HEIGHT / 8)
#define FRAME_COUNT 6572
#define FPS 30
#define FRAME_TIME (1000 / FPS)

int main(int argc, char *argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) < 0) {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return -1;
    }

    SDL_Window *window = SDL_CreateWindow(
        "Bad Apple PSP",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        SCREEN_WIDTH, SCREEN_HEIGHT,
        0
    );
    if (!window) {
        SDL_Log("SDL_CreateWindow failed: %s", SDL_GetError());
        SDL_Quit();
        return -1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        SDL_Log("SDL_CreateRenderer failed: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    SDL_Texture *texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        SCREEN_WIDTH,
        SCREEN_HEIGHT
    );
    if (!texture) {
        SDL_Log("SDL_CreateTexture failed: %s", SDL_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    FILE *file = fopen("frames.dat", "rb");
    if (!file) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "File Error", "Failed to open frames.dat", NULL);
        SDL_DestroyTexture(texture);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    Uint8 frameData[FRAME_SIZE];
    Uint32 *pixels;
    int pitch;

    int running = 1;
    SDL_Event event;
    Uint32 frameStart, frameTime;
    int currentFrame = 0;

    while (running && currentFrame < FRAME_COUNT) {
        frameStart = SDL_GetTicks();

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT)
                running = 0;
            else if (event.type == SDL_CONTROLLERBUTTONDOWN &&
                     event.cbutton.button == SDL_CONTROLLER_BUTTON_START)
                running = 0;
        }

        size_t bytesRead = fread(frameData, 1, FRAME_SIZE, file);
        if (bytesRead < FRAME_SIZE) {
            break;
        }

        if (SDL_LockTexture(texture, NULL, (void**)&pixels, &pitch) < 0) {
            SDL_Log("SDL_LockTexture failed: %s", SDL_GetError());
            break;
        }

        int pixelIndex = 0;
        for (int i = 0; i < FRAME_SIZE; ++i) {
            Uint8 byte = frameData[i];
            for (int bit = 7; bit >= 0; --bit) {
                Uint32 color = (byte & (1 << bit)) ? 0xFFFFFFFF : 0xFF000000;
                pixels[pixelIndex++] = color;
            }
        }

        SDL_UnlockTexture(texture);

        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);

        currentFrame++;

        frameTime = SDL_GetTicks() - frameStart;
        if (frameTime < FRAME_TIME) {
            SDL_Delay(FRAME_TIME - frameTime);
        }
    }

    fclose(file);
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}