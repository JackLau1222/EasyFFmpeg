/*
 * copyright (c) 2024 Jack Lau
 * 
 * This file is a tutorial about palying(decoding and rendering) video through ffmpeg and SDL API 
 * 
 * FFmpeg version 6.0.1
 * SDL2 version 2.30.3
 *
 * Tested on MacOS 14.1.2, compiled with clang 14.0.3
 */

#include <SDL2/SDL.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavcodec/avcodec.h>

#include "../include/easy_display.h"
#include "../include/easy_media.h"

typedef struct VideoState{
    AVCodecContext *avctx;
    AVPacket       *pkt;
    AVFrame        *frame;
    AVStream       *stream;

    SDL_Texture    *texture;
}VideoState;


static int w_width = 1920;
static int w_height = 1080;

static SDL_Window *win = NULL;
static SDL_Renderer *renderer = NULL;


static int decode(VideoState *is)
{
    int ret = -1;

    char buffer[1024];
    //send packet to decoder
    ret = avcodec_send_packet(is->avctx, is->pkt);
    if(ret < 0){
        av_log(NULL, AV_LOG_ERROR, "Failed to send frame to decoder!\n");
        goto end;
    }

    while (ret >= 0)
    {
        ret = avcodec_receive_frame(is->avctx, is->frame);
        if(ret == AVERROR(EAGAIN) || ret == AVERROR_EOF){
            ret = 0;
            goto end;
        }else if(ret < 0){
            ret = -1;
            goto end;
        }

        int frameRate = is->stream->r_frame_rate.num/is->stream->r_frame_rate.den;
        easy_render_yuv420p(renderer, is->texture, is->frame, frameRate);
    }
    

end:
    return ret;
}


int main(int argc, char *argv[])
{

    int ret = -1;
    int idx = -1;
    AVFormatContext *fmtCtx = NULL;
    AVStream *inStream = NULL;
    const AVCodec *decodec = NULL;
    AVCodecContext *ctx = NULL;

    SDL_Texture *texture = NULL;
    SDL_Event event;

    Uint32 pixformat = 0;

    int video_height = 0;
    int video_width = 0;

    AVPacket *pkt = NULL;
    AVFrame *frame = NULL;

    VideoState *is = NULL; 
    
    //deal with arguments
    char *src;

    av_log_set_level(AV_LOG_DEBUG);

    if(argc < 2){
        av_log(NULL, AV_LOG_ERROR, "the arguments must be more than 2!\n");
        exit(-1);
    }

    src = argv[1];
    
    is = av_mallocz(sizeof(VideoState));
    if(!is){
        av_log(NULL, AV_LOG_ERROR, "No Memory!\n");
        goto end;
    }

    //init SDL
    easy_init_sdl_for_render(&win, &renderer, w_width, w_height);

    easy_open_video(src, &fmtCtx, &ctx, &idx);

    //create texture for render 
    video_width = ctx->width;
    video_height = ctx->height;
    pixformat = SDL_PIXELFORMAT_IYUV;
    texture = SDL_CreateTexture(renderer, pixformat, SDL_TEXTUREACCESS_STREAMING, video_width, video_height);

    pkt = av_packet_alloc();
    frame = av_frame_alloc();

    is->stream = fmtCtx->streams[idx];
    is->texture = texture;
    is->avctx = ctx;
    is->pkt = pkt;
    is->frame = frame;
    
    //decode
    while(av_read_frame(fmtCtx, pkt) >= 0){
        if(pkt->stream_index == idx ){
            //render
            decode(is);
        }
        //deal with SDL event
        
        SDL_PollEvent(&event);
        switch (event.type)
        {
        case SDL_QUIT:
            goto quit;
            break;
        
        default:
            break;
        }

        av_packet_unref(pkt);
    
    }
    is->pkt = NULL;
    decode(is);

quit:
    ret = 0;
end:
    if(frame){
        av_frame_free(&frame);
    }
    if (pkt){
        av_packet_free(&pkt);
    }
    if(ctx){
        avcodec_free_context(&ctx);
    }
    if(fmtCtx){
        avformat_close_input(&fmtCtx);
    }
    if(win){
        SDL_DestroyWindow(win);
    }
    if(renderer){
        SDL_DestroyRenderer(renderer);
    }
    if(texture){
        SDL_DestroyTexture(texture);
    }
    SDL_Quit();    
    return ret;
}