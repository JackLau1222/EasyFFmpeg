/*
 * copyright (c) 2025 Jack Lau
 * 
 * This file is a example about decoding and saving video frames through EasyFFmpeg API
 * 
 * FFmpeg version 5.1.4
 */
#include <stdio.h>

#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include "../include/easy_utils.h"
#include "../include/easy_media.h"


void decode(AVCodecContext *dec_ctx, AVFrame *frame, AVPacket *pkt,	FILE *f, char *fileName)
{
	char buf[1024];
	int ret;

	//send packet to decoder
	ret = avcodec_send_packet(dec_ctx, pkt);
	if (ret < 0) {
        CHECK_ERROR(ret);
        return;
	}
	while (ret >= 0) {
		// receive frame from decoder
		// we may receive multiple frames or we may consume all data from decoder, then return to main loop
		ret = avcodec_receive_frame(dec_ctx, frame);
		if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
			return;
		else if (ret < 0) {
			// something wrong, quit program
			CHECK_ERROR(ret);
            return;
		}
		//printf("saving frame %3d\n", dec_ctx->pkt_serial);
		printf("saving frame %lld\n", frame->pts);
        fflush(stdout);
		// send frame info to writing function
        
        /* save ppm */
        // easy_save_ppm(frame->data[0], frame->linesize[0],
        //              frame->data[1], frame->linesize[1],
        //              frame->data[2], frame->linesize[2],
        //              frame->width, frame->height, f);
        /* save yuv video */
        // easy_save_yuv420(frame->data[0], frame->linesize[0], 
        //                  frame->data[1], frame->linesize[1],
        //                  frame->data[2], frame->linesize[2],
        //                  frame->width, frame->height, f);
        
        /* save yuv data into ppm using easy ffmpeg */
        // easy_save_yuv_to_ppm(frame->data[0],
        //                      frame->data[1],
        //                      frame->data[2],
        //                      frame->width, frame->height, fileName);
        
        /* save yuv data into ppm using swscale */
        unsigned char *rgb_buffer = (unsigned char *)malloc(3 * frame->width * frame->height);
        easy_reformat_to_rgb24(frame, rgb_buffer, frame->width, frame->height, frame->format);
        easy_save_ppm(rgb_buffer, 3 * frame->width, frame->width, frame->height, fileName);
	}
}

int main(int argc, char *argv[])
{
	// declare format and codec contexts, also codec for decoding
	AVFormatContext *fmt_ctx = NULL;
	AVCodecContext *codec_ctx = NULL;
	const AVCodec *Codec = NULL;
	int ret;
    if (argc != 3) {
        fprintf(stderr, "Usage: %s input output\n", argv[0]);
        exit(1);
    }
	const char *infilename = argv[1];
	char *outfilename = argv[2];
	int VideoStreamIndex = -1;

	FILE *fout = NULL;

	AVFrame *frame = NULL;
	AVPacket *pkt = NULL;

	easy_open_video(infilename, &fmt_ctx, &codec_ctx, &VideoStreamIndex);

	// dump video stream info
	av_dump_format(fmt_ctx, VideoStreamIndex, infilename, 0);

	//init packet
	pkt = av_packet_alloc();
	if (!pkt)
	{
		av_log(NULL, AV_LOG_ERROR, "Cannot init packet\n");
		goto end;
	}

	// init frame
	frame = av_frame_alloc();
	if (!frame)
	{
		av_log(NULL, AV_LOG_ERROR, "Cannot init frame\n");
		goto end;
	}

	// open output file
	fout = fopen(outfilename, "w");
	if (!fout)
	{
		av_log(NULL, AV_LOG_ERROR, "Cannot open output file\n");
		goto end;
	}

	// main loop
	while (1)
	{
		// read an encoded packet from file
		if ((ret = av_read_frame(fmt_ctx, pkt)) < 0)
		{
			av_log(NULL, AV_LOG_ERROR, "cannot read frame");
			break;
		}
		// if packet data is video data then send it to decoder
		if (pkt->stream_index == VideoStreamIndex)
		{
			decode(codec_ctx, frame, pkt, fout, outfilename);
		}

		// release packet buffers to be allocated again
		av_packet_unref(pkt);
	}

	//flush decoder
	decode(codec_ctx, frame, NULL, fout, outfilename);

	// clear and out
end:
	if (fout)
		fclose(fout);
	if (codec_ctx)
		avcodec_close(codec_ctx);
	if (fmt_ctx)
		avformat_close_input(&fmt_ctx);
	if (frame)
		av_frame_free(&frame);
	if (pkt)
		av_packet_free(&pkt);

	return 0;
}