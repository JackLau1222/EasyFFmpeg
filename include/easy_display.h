#ifndef __EASY_DISPLAY_H__
#define __EASY_DISPLAY_H__

#include "easy_common.h"

#include <SDL2/SDL.h>
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>

#define ONESECOND 1000

/**
 * Initialize SDL and create a window.
 *
 * @param window A pointer to a pointer to an SDL_Window, which will be allocated and initialized.
 * @param renderer A pointer to a pointer to an SDL_Renderer, which will be allocated and initialized.
 * @param width The width of the window.
 * @param height The height of the window.
 */
static inline int easy_init_sdl_for_render(SDL_Window **window, SDL_Renderer **renderer, int width, int height)
{
    if (SDL_Init(SDL_INIT_VIDEO)){
        fprintf(stderr, "Couldn't initialize SDL - %s\n", SDL_GetError());
        return -1;
    }
    *window = SDL_CreateWindow("EasyFFmpeg", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_OPENGL| SDL_WINDOW_RESIZABLE);
    if(!window){
        fprintf(stderr, "Failed to create window, %s\n", SDL_GetError());
        return -1;
    }   
    *renderer = SDL_CreateRenderer(*window, -1, 0);
    return 0;
}

/**
 * Render a YUV420P frame using SDL.
 * 
 * @param renderer The SDL renderer to use for rendering.
 * @param texture The SDL texture to update with the YUV data.
 * @param frame The AVFrame containing the YUV420P data.
 * @param delay The delay in milliseconds to wait after rendering the frame.
 */
static inline void easy_render_yuv420p(SDL_Renderer *renderer, SDL_Texture *texture, AVFrame *frame, int delay)
{
    SDL_UpdateYUVTexture(texture, NULL,
                        frame->data[0], frame->linesize[0] ,
                        frame->data[1], frame->linesize[1],
                        frame->data[2], frame->linesize[2]);
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
    SDL_Delay((Uint32)delay);
}

#endif // __EASY_DISPLAY_H__