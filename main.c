#include <SDL.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define SCREEN_WIDTH 480
#define SCREEN_HEIGHT 272

#define VIDEO_WIDTH 320
#define VIDEO_HEIGHT 180

#define FRAME_SIZE (VIDEO_WIDTH * VIDEO_HEIGHT / 8)
#define FRAME_COUNT 6572
#define FPS 30
#define FRAME_TIME (1000 / FPS)

static Uint32 LUT[256][8]; // LookUp table optimization

void init_lut(void) {
    for (int b = 0; b < 256; ++b) {
        for (int bit = 7; bit >= 0; --bit) {
            LUT[b][7 - bit] = (b & (1 << bit)) ? 0xFFFFFFFF : 0xFF000000;
        }
    }
}

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
        VIDEO_WIDTH,
        VIDEO_HEIGHT
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

    init_lut();

    Uint8 *frameData = malloc(FRAME_SIZE);
    if (!frameData) {
        SDL_Log("Memory allocation failed!");
        fclose(file);
        SDL_DestroyTexture(texture);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    Uint32 *pixels = malloc(VIDEO_WIDTH * VIDEO_HEIGHT * sizeof(Uint32));
    if (!pixels) {
        SDL_Log("Pixel buffer allocation failed!");
        free(frameData);
        fclose(file);
        SDL_DestroyTexture(texture);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    SDL_Rect dstRect;
    dstRect.w = VIDEO_WIDTH;
    dstRect.h = VIDEO_HEIGHT;
    dstRect.x = (SCREEN_WIDTH - VIDEO_WIDTH) / 2;
    dstRect.y = (SCREEN_HEIGHT - VIDEO_HEIGHT) / 2;

    int running = 1;
    SDL_Event event;
    Uint32 frameStart, frameTime;
    int currentFrame = 0;
    Uint32 lastFpsTime = SDL_GetTicks();
    int framesRendered = 0;

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
        if (bytesRead < FRAME_SIZE) break;

        Uint32 *dst = pixels;
        for (int i = 0; i < FRAME_SIZE; ++i) {
            // apparently this is faster 
            Uint32 *src = LUT[frameData[i]];
            dst[0] = src[0]; dst[1] = src[1]; dst[2] = src[2]; dst[3] = src[3];
            dst[4] = src[4]; dst[5] = src[5]; dst[6] = src[6]; dst[7] = src[7];
            dst += 8;
        }

        SDL_UpdateTexture(texture, NULL, pixels, VIDEO_WIDTH * sizeof(Uint32));

        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, &dstRect);
        SDL_RenderPresent(renderer);

        currentFrame++;
        framesRendered++;

        frameTime = SDL_GetTicks() - frameStart;
        if (frameTime < FRAME_TIME) SDL_Delay(FRAME_TIME - frameTime);

        Uint32 now = SDL_GetTicks();
        if (now - lastFpsTime >= 1000) {
            framesRendered = 0;
            lastFpsTime = now;
        }
    }

    free(pixels);
    free(frameData);
    fclose(file);
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
