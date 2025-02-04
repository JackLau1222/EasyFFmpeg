/*
 * Copyright 2025 Jack Lau
 * Email: jacklau1222gm@gmail.com
 *
 * This file is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This file is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 */
#ifndef __EASY_MEDIA_H__
#define __EASY_MEDIA_H__

#include "easy_common.h"

#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>

/**
 * Open an input file and prepare it for decoding.
 * 
 * @param filename The name of the input file.
 * @param fmt_ctx A pointer to a pointer to an AVFormatContext, which will be allocated and initialized.
 * @param dec_ctx A pointer to a pointer to an AVCodecContext, which will be allocated and initialized.
 * @param video_stream_index A pointer to an integer that will store the index of the video stream.
 * 
 * @return 0 on success, a negative AVERROR code on failure.
 */
static inline int easy_open_video(const char *filename, AVFormatContext **fmt_ctx, AVCodecContext **dec_ctx, int *video_stream_index)
{
    const AVCodec *dec;
    int ret;

    if ((ret = avformat_open_input(fmt_ctx, filename, NULL, NULL)) < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot open input file\n");
        return ret;
    }

    if ((ret = avformat_find_stream_info(*fmt_ctx, NULL)) < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot find stream information\n");
        return ret;
    }

    /* select the video stream */
    ret = av_find_best_stream(*fmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, &dec, 0);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot find a video stream in the input file\n");
        return ret;
    }
    *video_stream_index = ret;

    /* create decoding context */
    *dec_ctx = avcodec_alloc_context3(dec);
    if (!*dec_ctx)
        return AVERROR(ENOMEM);
    avcodec_parameters_to_context(*dec_ctx, (*fmt_ctx)->streams[*video_stream_index]->codecpar);

    /* init the video decoder */
    if ((ret = avcodec_open2(*dec_ctx, dec, NULL)) < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot open video decoder\n");
        return ret;
    }

    return 0;
}

/**
 * Open an input file and prepare it for decoding audio.
 * 
 * @param filename The name of the input file.
 * @param fmt_ctx A pointer to a pointer to an AVFormatContext, which will be allocated and initialized.
 * @param dec_ctx A pointer to a pointer to an AVCodecContext, which will be allocated and initialized.
 * @param audio_stream_index A pointer to an integer that will store the index of the audio stream.
 * 
 * @return 0 on success, a negative AVERROR code on failure.
 */
static inline int easy_open_audio(const char *filename, AVFormatContext **fmt_ctx, AVCodecContext **dec_ctx, int *audio_stream_index)
{
    const AVCodec *dec;
    int ret;

    if ((ret = avformat_open_input(fmt_ctx, filename, NULL, NULL)) < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot open input file\n");
        return ret;
    }

    if ((ret = avformat_find_stream_info(*fmt_ctx, NULL)) < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot find stream information\n");
        return ret;
    }

    /* select the audio stream */
    ret = av_find_best_stream(*fmt_ctx, AVMEDIA_TYPE_AUDIO, -1, -1, &dec, 0);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot find a audio stream in the input file\n");
        return ret;
    }
    *audio_stream_index = ret;

    /* create decoding context */
    *dec_ctx = avcodec_alloc_context3(dec);
    if (!*dec_ctx)
        return AVERROR(ENOMEM);
    avcodec_parameters_to_context(*dec_ctx, (*fmt_ctx)->streams[*audio_stream_index]->codecpar);

    /* init the audio decoder */
    if ((ret = avcodec_open2(*dec_ctx, dec, NULL)) < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot open audio decoder\n");
        return ret;
    }

    return 0;
}

/**
 * Open an input file and prepare it for decoding both video and audio.
 * 
 * @param filename The name of the input file.
 * @param fmt_ctx A pointer to a pointer to an AVFormatContext, which will be allocated and initialized.
 * @param dec_video_ctx A pointer to a pointer to an AVCodecContext for video, which will be allocated and initialized.
 * @param video_stream_index A pointer to an integer that will store the index of the video stream.
 * @param dec_audio_ctx A pointer to a pointer to an AVCodecContext for audio, which will be allocated and initialized.
 * @param audio_stream_index A pointer to an integer that will store the index of the audio stream.
 * 
 * @return 0 on success, a negative AVERROR code on failure.
 */
static inline int easy_open_av(const char *filename, AVFormatContext **fmt_ctx, 
                               AVCodecContext **dec_video_ctx, int *video_stream_index,
                               AVCodecContext **dec_audio_ctx, int *audio_stream_index)
{
    const AVCodec *dec;
    int ret;

    if ((ret = avformat_open_input(fmt_ctx, filename, NULL, NULL)) < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot open input file\n");
        return ret;
    }

    if ((ret = avformat_find_stream_info(*fmt_ctx, NULL)) < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot find stream information\n");
        return ret;
    }

    /* select the video stream */
    ret = av_find_best_stream(*fmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, &dec, 0);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot find a video stream in the input file\n");
        return ret;
    }
    *video_stream_index = ret;

    /* select the audio stream */
    ret = av_find_best_stream(*fmt_ctx, AVMEDIA_TYPE_AUDIO, -1, -1, &dec, 0);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot find a audio stream in the input file\n");
        return ret;
    }
    *audio_stream_index = ret;

    /* create video decoding context */
    *dec_video_ctx = avcodec_alloc_context3(dec);
    if (!*dec_video_ctx)
        return AVERROR(ENOMEM);
    avcodec_parameters_to_context(*dec_video_ctx, (*fmt_ctx)->streams[*video_stream_index]->codecpar);

    /* create audio decoding context */
    *dec_audio_ctx = avcodec_alloc_context3(dec);
    if (!*dec_audio_ctx)
        return AVERROR(ENOMEM);
    avcodec_parameters_to_context(*dec_audio_ctx, (*fmt_ctx)->streams[*audio_stream_index]->codecpar);

    /* init the video decoder */
    if ((ret = avcodec_open2(*dec_video_ctx, dec, NULL)) < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot open video decoder\n");
        return ret;
    }

    /* init the audio decoder */
    if ((ret = avcodec_open2(*dec_audio_ctx, dec, NULL)) < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot open audio decoder\n");
        return ret;
    }

    return 0;
}

static inline void easy_alloc_frame(AVFrame **frame, int width, int height, enum AVPixelFormat pixel_format);


#endif