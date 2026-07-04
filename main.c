#include <stdlib.h>
#include <stdbool.h>
#include <SDL.h>

#include "la.h"
#define STB_IMAGE_IMPLEMENTATION
#include "./stb_image.h"

#define FONT_WIDTH 128
#define FONT_HEIGHT 64
#define FONT_COLS 18
#define FONT_ROWS 7
#define FONT_CHAR_WIDTH  (FONT_WIDTH  / FONT_COLS)
#define FONT_CHAR_HEIGHT (FONT_HEIGHT / FONT_ROWS)

void scc(int code)
{
    if (code < 0){
        fprintf(stderr, "SDL ERROR: %s\n", SDL_GetError());
        exit(1);
    }
}

void *scp(void *ptr)

{
    if (ptr == NULL){
        fprintf(stderr, "SDL ERROR: %s\n", SDL_GetError());
        exit(1);
    }

    return ptr;
}

SDL_Surface *surface_from_file(const char *file_path)
{
    int width, height, n;
    unsigned char *pixels =
        stbi_load(file_path, &width, &height, &n, STBI_rgb_alpha);

    if (pixels == NULL){
        fprintf(
            stderr,
            "ERROR: Could not load file %s: %s\n",
            file_path,
            stbi_failure_reason()
        );
        exit(1);
    }

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    const Uint32 rmask = 0xff000000;
    const Uint32 gmask = 0x00ff0000;
    const Uint32 bmask = 0x0000ff00;
    const Uint32 amask = 0x000000ff;
#else
    const Uint32 rmask = 0x000000ff;
    const Uint32 gmask = 0x0000ff00;
    const Uint32 bmask = 0x00ff0000;
    const Uint32 amask = 0xff000000;
#endif
    const int depth = 32;
    const int pitch = 4 * width;


    return scp(SDL_CreateRGBSurfaceFrom(
           (void *)pixels, width, height, depth, pitch,
           rmask, gmask, bmask, amask));
}

#define ASCII_DISPLAY_LOW 32
#define ASCII_DISPLAY_HIGH 126

typedef struct {
    SDL_Texture *spritesheet;
    SDL_Rect glyph_table[ASCII_DISPLAY_HIGH - ASCII_DISPLAY_LOW + 1];
} Font;

Font font_load_from_file(SDL_Renderer *renderer, const char *file_path){
    Font font = {0};
    SDL_Surface *font_surface =  surface_from_file(file_path);
    font.spritesheet = scp(
         SDL_CreateTextureFromSurface(renderer, font_surface));
    SDL_FreeSurface(font_surface);

    for (size_t ascii = ASCII_DISPLAY_LOW; ascii <= ASCII_DISPLAY_HIGH; ++ascii)
    {
        const size_t index = ascii - ASCII_DISPLAY_LOW;
        const size_t col = index % FONT_COLS;
        const size_t row = index / FONT_COLS;

        font.glyph_table[index] = (SDL_Rect){
            .x = col * FONT_CHAR_WIDTH,
            .y = row * FONT_CHAR_HEIGHT,
            .w = FONT_CHAR_WIDTH,
            .h = FONT_CHAR_HEIGHT,
        };
    }

    return font;
}

void render_char(SDL_Renderer *renderer, Font *font,
                 char c, Vec2f pos, float scale)
{

    const SDL_Rect dst = {
        .x = (int)floorf(pos.x),
        .y = (int)floorf(pos.y),
        .w = (int)floorf(FONT_CHAR_WIDTH * scale),
        .h = (int)floorf(FONT_CHAR_HEIGHT * scale),
    };

    assert(c >= ASCII_DISPLAY_LOW);
    assert(c <= ASCII_DISPLAY_HIGH);
    const size_t index = c - ASCII_DISPLAY_LOW;
    scc(SDL_RenderCopy(renderer, font->spritesheet,
                       &font->glyph_table[index], &dst));
}

void render_text(SDL_Renderer *renderer, Font *font,
    const char *text, Vec2f pos, Uint32 color, float scale)
{
    scc(SDL_SetTextureColorMod(
        font->spritesheet,
        (color >> (8* 0)) & 0xff,
        (color >> (8* 1)) & 0xff,
        (color >> (8* 2)) & 0xff
    ));

    scc(SDL_SetTextureAlphaMod(font->spritesheet, (color >> (8* 3)) & 0xff));

    const size_t n = strlen(text);
    Vec2f pen = pos;
    for (size_t i = 0; i < n; i++){
        render_char(renderer, font, text[i], pen, scale);
        pen.x += FONT_CHAR_WIDTH * scale;
    }
}

int main(void) {
    scc(SDL_Init(SDL_INIT_VIDEO));

    SDL_Window *window =
      scp(SDL_CreateWindow("Text Editor", 0, 0, 800, 600,SDL_WINDOW_RESIZABLE));
    SDL_Renderer *renderer =
        scp(SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED));

    Font font = font_load_from_file(renderer, "./charmap-oldschool_white.png");

    bool quit = false;

    while (!quit){
        SDL_Event event = {0};
        while (SDL_PollEvent(&event)){
            switch (event.type){
                case SDL_QUIT:{
                    quit = true;
                } break;
            }
        }

        scc(SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0));
        scc(SDL_RenderClear(renderer));
        render_text(renderer, &font, "Hello, World",
                    vec2f(0.0, 0.0), 0xff0000ff, 5.0f );
        SDL_RenderPresent(renderer);
    }

    SDL_Quit();
    return 0;
}
