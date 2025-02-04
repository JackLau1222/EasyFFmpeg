/*
 * copyright (c) 2025 Jack Lau
 * 
 * This file is a example about filtering video through EasyFFmpeg API
 * 
 * FFmpeg version 5.1.4
 */
#include "../include/easy_api.h"
#include <stdio.h>
#include <stdlib.h>

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavutil/opt.h>


const char *filter_descr = 
    "[in0]pad=iw*2:ih[int];[int][in1]overlay=w[out]";

static AVFormatContext *fmt_ctx1;
static AVFormatContext *fmt_ctx2;
static AVCodecContext *dec_ctx1;
static AVCodecContext *dec_ctx2;

AVFilterContext *buffersink_ctx;
AVFilterContext *buffersrc_ctx1;
AVFilterContext *buffersrc_ctx2;
AVFilterGraph *filter_graph;

static int video_stream_index1 = -1;
static int video_stream_index2 = -1;

int width      = 640;
int height     = 480;
enum AVPixelFormat pix_fmt = AV_PIX_FMT_YUV420P;
AVRational sample_aspect_ratio = {1, 1};

static int init_filters(const char *filters_descr, AVRational time_base, int width, int height, enum AVPixelFormat pix_fmt, AVRational sample_aspect_ratio)
{
    char args[512];
    int ret = 0;
    const AVFilter *buffersrc  = avfilter_get_by_name("buffer");
    const AVFilter *buffersink = avfilter_get_by_name("buffersink");
    enum AVPixelFormat pix_fmts[] = { AV_PIX_FMT_YUV420P, AV_PIX_FMT_GRAY8, AV_PIX_FMT_NONE };

    filter_graph = avfilter_graph_alloc();
    if (!filter_graph) {
        ret = AVERROR(ENOMEM);
        goto end;
    }

    /* buffer video source: the decoded frames from the decoder will be inserted here. */
    snprintf(args, sizeof(args),
            "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
            width, height, pix_fmt,
            time_base.num, time_base.den,
            sample_aspect_ratio.num, sample_aspect_ratio.den);

    ret = avfilter_graph_create_filter(&buffersrc_ctx1, buffersrc, "in0",
                                       args, NULL, filter_graph);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot create buffer source 1\n");
        goto end;
    }
    ret = avfilter_graph_create_filter(&buffersrc_ctx2, buffersrc, "in1",
                                       args, NULL, filter_graph);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot create buffer source 2\n");
        goto end;
    }

    /* buffer video sink: to terminate the filter chain. */
    ret = avfilter_graph_create_filter(&buffersink_ctx, buffersink, "out",
                                       NULL, NULL, filter_graph);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot create buffer sink\n");
        goto end;
    }

    ret = av_opt_set_int_list(buffersink_ctx, "pix_fmts", pix_fmts,
                              AV_PIX_FMT_NONE, AV_OPT_SEARCH_CHILDREN);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot set output pixel format\n");
        goto end;
    }

    /*
     * Set the endpoints for the filter graph. The filter_graph will
     * be linked to the graph described by filters_descr.
     */

    AVFilterInOut *inputs  = NULL;
    AVFilterInOut *outputs = NULL;


    inputs = avfilter_inout_alloc();
    if (!inputs) {
        ret = AVERROR(ENOMEM);
        goto end;
    }
    inputs->name       = av_strdup("out");
    inputs->filter_ctx = buffersink_ctx;
    inputs->pad_idx    = 0;
    inputs->next = NULL;

    outputs = avfilter_inout_alloc();
    if (!outputs) {
        ret = AVERROR(ENOMEM);
        goto end;
    }
    outputs->name       = av_strdup("in0");
    outputs->filter_ctx = buffersrc_ctx1;
    outputs->pad_idx    = 0;
    outputs->next       = avfilter_inout_alloc();
    if (!outputs->next) {
        ret = AVERROR(ENOMEM);
        goto end;
    }
    outputs->next->name = av_strdup("in1");
    outputs->next->filter_ctx = buffersrc_ctx2;
    outputs->next->pad_idx    = 0;
    outputs->next->next = NULL;

    // if ((ret = avfilter_graph_parse2(filter_graph, filter_descr, inputs, outputs)) < 0)
    //     goto end;
    if ((ret = avfilter_graph_parse_ptr(filter_graph, filters_descr, &inputs, &outputs, NULL)) < 0)
        goto end;
    if ((ret = avfilter_graph_config(filter_graph, NULL)) < 0)
        goto end;

end:
    avfilter_inout_free(&inputs);
    avfilter_inout_free(&outputs);
    return ret;
}

int main(int argc, char **argv)
{
    int ret;
    AVPacket packet1, packet2;
    AVFrame *frame1, *frame2;
    AVFrame *filt_frame;

    if (argc != 4) {
        fprintf(stderr, "Usage: %s input1 input2 output.yuv\n", argv[0]);
        exit(1);
    }

    frame1 = av_frame_alloc();
    frame2 = av_frame_alloc();
    filt_frame = av_frame_alloc();
    av_init_packet(&packet1);
    av_init_packet(&packet2);
    if (!frame1 || !frame2 || !filt_frame || !&packet1 || !&packet2) {
        fprintf(stderr, "Could not allocate frame or packet\n");
        exit(1);
    }

    if ((ret = easy_open_video(argv[1], &fmt_ctx1, &dec_ctx1, &video_stream_index1)) < 0)
        goto end;
    if ((ret = easy_open_video(argv[2], &fmt_ctx2, &dec_ctx2, &video_stream_index2)) < 0)
        goto end;
    
    AVRational time_base = fmt_ctx1->streams[video_stream_index1]->time_base;
    width = fmt_ctx1->streams[video_stream_index1]->codecpar->width;
    height = fmt_ctx1->streams[video_stream_index1]->codecpar->height;
    if ((ret = init_filters(filter_descr, time_base, width, height, pix_fmt, sample_aspect_ratio)) < 0)
        goto end;

    char *fileName = argv[3];
    AVFrame *video_dst = av_frame_alloc();
    int frameNumber = 0;
    FILE *f = fopen(fileName, "wb");

    SDL_Window *win = NULL;
    SDL_Renderer *renderer = NULL;
    easy_init_sdl_for_render(&win, &renderer, 640, 480);
    SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, width*2, height);;
    SDL_Event event;


    int finished1 = 0, finished2 = 0;
    /* read all packets */
    while (!finished1 || !finished2) {
        // Process video1
        if (!finished1 && av_read_frame(fmt_ctx1, &packet1) >= 0) {
            if (packet1.stream_index == video_stream_index1) {
                // Send the packet to the decoder.
                if (avcodec_send_packet(dec_ctx1, &packet1) == 0) {
                    // Receive all available frames.
                    while (avcodec_receive_frame(dec_ctx1, frame1) == 0) {
                        // Feed the frame into the filter graph for input 1.
                        if (av_buffersrc_add_frame_flags(buffersrc_ctx1, frame1, AV_BUFFERSRC_FLAG_KEEP_REF) < 0) {
                            fprintf(stderr, "Error while feeding frame to filter graph (video1)\n");
                        }
                        av_frame_unref(frame1);
                    }
                }
            }
            av_packet_unref(&packet1);
        } else {
            finished1 = 1;
            // Optionally send a flush signal to the decoder for video1.
        }

        // Process video2
        if (!finished2 && av_read_frame(fmt_ctx2, &packet2) >= 0) {
            if (packet2.stream_index == video_stream_index2) {
                if (avcodec_send_packet(dec_ctx2, &packet2) == 0) {
                    while (avcodec_receive_frame(dec_ctx2, frame2) == 0) {
                        // Feed the frame into the filter graph for input 2.
                        if (av_buffersrc_add_frame_flags(buffersrc_ctx2, frame2, AV_BUFFERSRC_FLAG_KEEP_REF) < 0) {
                            fprintf(stderr, "Error while feeding frame to filter graph (video2)\n");
                        }
                        av_frame_unref(frame2);
                    }
                }
            }
            av_packet_unref(&packet2);
        } else {
            finished2 = 1;
            // Optionally flush decoder for video2.
        }

        // Try to pull a filtered frame from the sink.
        int ret = av_buffersink_get_frame(buffersink_ctx, filt_frame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            // av_frame_free(&filt_frame);
            continue;
        } else if (ret < 0) {
            fprintf(stderr, "Error retrieving frame from filter graph\n");
            av_frame_free(&filt_frame);
            break;
        }
        printf("frameNumber: %d\n", frameNumber++);
        char buffer[1024];
        snprintf(buffer, sizeof(buffer), "%s-%d.ppm", fileName, frameNumber);
        // easy_save_pgm_video(filt_frame->data[0], filt_frame->linesize[0], 
        //                     filt_frame->width, filt_frame->height, f);
        // easy_save_pgm(filt_frame->data[0], filt_frame->linesize[0], filt_frame->width, filt_frame->height, buffer);
        // easy_save_yuv_to_ppm(filt_frame->data[0], 
        //                     filt_frame->data[1], 
        //                     filt_frame->data[2], 
        //                     filt_frame->width, filt_frame->height, buffer);
        easy_save_yuv420(filt_frame->data[0], filt_frame->linesize[0],
                        filt_frame->data[1], filt_frame->linesize[1],
                        filt_frame->data[2], filt_frame->linesize[2],
                        filt_frame->width, filt_frame->height, f);
        easy_render_yuv420p(&renderer, &texture, filt_frame, 25);
        if ((ret = easy_sdl_event_in_loop(&event)) < 0) goto end;

    }
end:
    avfilter_graph_free(&filter_graph);
    avcodec_free_context(&dec_ctx1);
    avcodec_free_context(&dec_ctx2);
    avformat_close_input(&fmt_ctx1);
    avformat_close_input(&fmt_ctx2);
    av_frame_free(&frame1);
    av_frame_free(&frame2);
    av_frame_free(&filt_frame);
    av_packet_free(&packet1);
    av_packet_free(&packet2);

    if (ret < 0 && ret != AVERROR_EOF) {
        fprintf(stderr, "Error occurred: %s\n", av_err2str(ret));
        exit(1);
    }

    exit(0);
}