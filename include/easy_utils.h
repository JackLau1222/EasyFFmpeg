#ifndef __EASY_UTILS_H__
#define __EASY_UTILS_H__

#include "easy_common.h"

#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>

#include <stdio.h>

/**
 * Save a grayscale image (PGM format).
 * 
 * @param buffer The pointer to the image data (Y channel).
 * @param linesize The number of bytes in a row of the image (width, could be larger than actual width).
 * @param width The width of the image in pixels.
 * @param height The height of the image in pixels.
 * @param name The file name to save the PGM image to.
 * 
 * @return 0 on success, -1 on failure.
 */
static inline int easy_save_pgm(unsigned char* buffer, int linesize, int width, int height, char *name)
{
    FILE *f;
    f = fopen(name, "wb");
    fprintf(f, "P5\n%d %d\n%d\n", width, height, 255);
    for (int i = 0; i < height; i++){
        fwrite(buffer + i * linesize, 1, width, f);
    }
    fclose(f);
    return 0;
}

/**
 * Save a color image (PPM format).
 * 
 * @param buffer The pointer to the image data (RGB format).
 * @param linesize The number of bytes in a row of the image (width, could be larger than actual width).
 * @param width The width of the image in pixels.
 * @param height The height of the image in pixels.
 * @param name The file name to save the PPM image to.
 * 
 * @return 0 on success, -1 on failure.
 */
static inline int easy_save_ppm(unsigned char* buffer, int linesize, int width, int height, char *name) {
    FILE *f = fopen(name, "wb");
    if (!f) return -1;
    
    fprintf(f, "P6\n%d %d\n%d\n", width, height, 255);
    for (int i = 0; i < height; i++) {
        fwrite(buffer + i * linesize, 1, width * 3, f); // PPM: Each pixel is 3 bytes (RGB)
    }

    fclose(f);
    return 0;
}

/**
 * Save a YUV image as a PPM file.
 * 
 * @param y The pointer to the Y plane data.
 * @param u The pointer to the U plane data.
 * @param v The pointer to the V plane data.
 * @param width The width of the image in pixels.
 * @param height The height of the image in pixels.
 * @param filename The file name to save the PPM image to.
 * 
 * @note This function may take a long time to execute as it converts the color format of every pixel.
 * 
 * @return 0 on success, -1 on failure.
 */
int easy_save_yuv_to_ppm(unsigned char* y, unsigned char* u, unsigned char* v, int width, int height, const char* filename) {
    FILE *f = fopen(filename, "wb");
    if (!f) return -1;  // Error opening file

    // Write the PPM header
    fprintf(f, "P6\n%d %d\n255\n", width, height);  // P6 format, width, height, max color value (255)

    // Allocate buffer for RGB data
    unsigned char *rgb_buffer = (unsigned char *)malloc(3 * width * height);
    if (!rgb_buffer) {
        fclose(f);
        return -1;  // Memory allocation failure
    }

    int buffer_index = 0;  // To track position in the buffer

    // Convert YUV to RGB and store in the buffer
    for (int j = 0; j < height; j++) {
        for (int i = 0; i < width; i++) {
            // Y, U, V values for the pixel
            unsigned char Y = y[j * width + i];
            unsigned char U = u[(j / 2) * (width / 2) + (i / 2)];
            unsigned char V = v[(j / 2) * (width / 2) + (i / 2)];

            // Convert YUV to RGB
            int R = Y + 1.402 * (V - 128);
            int G = Y - 0.344136 * (U - 128) - 0.714136 * (V - 128);
            int B = Y + 1.772 * (U - 128);

            // Clamp RGB values to [0, 255]
            if (R < 0) R = 0; if (R > 255) R = 255;
            if (G < 0) G = 0; if (G > 255) G = 255;
            if (B < 0) B = 0; if (B > 255) B = 255;

            // Store RGB values in the buffer
            rgb_buffer[buffer_index++] = R;
            rgb_buffer[buffer_index++] = G;
            rgb_buffer[buffer_index++] = B;
        }
    }

    // Write the entire buffer to the file at once
    fwrite(rgb_buffer, 1, 3 * width * height, f);

    // Clean up and close the file
    free(rgb_buffer);
    fclose(f);  // Close the file after writing

    return 0;   // Success
}

/**
 * Save a grayscale image (PGM format).
 * 
 * @param y The pointer to the Y plane data.
 * @param y_linesize The number of bytes in a row of the Y plane.
 * @param width The width of the image in pixels.
 * @param height The height of the image in pixels.
 * @param f The file pointer to save the YUV image to.
 * 
 * @note The caller is responsible for creating and destroying the file pointer.
 * 
 * @return 0 on success, -1 on failure.
 */
static inline int easy_save_pgm_video(unsigned char* y, int y_linesize, int width, int height, FILE *f) {
    if (!f) return -1;
    
    fprintf(f, "P5\n%d %d\n%d\n", width, height, 255);
    for (int i = 0; i < height; i++) {
        fwrite(y + i * y_linesize, 1, width, f);
    }
    return 0;
}

/**
 * Save a YUV420P video (Y, U, V planes).
 * 
 * @param y The pointer to the Y plane data.
 * @param y_linesize The number of bytes in a row of the Y plane.
 * @param u The pointer to the U plane data.
 * @param u_linesize The number of bytes in a row of the U plane.
 * @param v The pointer to the V plane data.
 * @param v_linesize The number of bytes in a row of the V plane.
 * @param width The width of the image in pixels.
 * @param height The height of the image in pixels.
 * @param f The file pointer to save the YUV image to.
 * 
 * @note The caller is responsible for creating and destroying the file pointer.
 * @note you can play it use `ffplay -f rawvideo -video_size 1920x1080 easy.yuv`.
 * 
 * @return 0 on success, -1 on failure.
 */
static inline int easy_save_yuv420(unsigned char* y, int y_linesize, 
                                   unsigned char* u, int u_linesize, 
                                   unsigned char* v, int v_linesize, 
                                   int width, int height, FILE *f) {
    if (!f) return -1;

    for (int i = 0; i < height; i++) {
        fwrite(y + i * y_linesize, 1, width, f);
    }

    for (int i = 0; i < height / 2; i++) {
        fwrite(u + i * u_linesize, 1, width / 2, f);
    }

    for (int i = 0; i < height / 2; i++) {
        fwrite(v + i * v_linesize, 1, width / 2, f);
    }
    return 0;
}

/**
 * Save raw PCM audio data.
 * 
 * @param buffer The pointer to the PCM audio data.
 * @param size The size of the audio data in bytes.
 * @param name The file name to save the PCM audio to.
 * 
 * @return 0 on success, -1 on failure.
 */
static inline int easy_save_pcm(unsigned char* buffer, int size, char *name) {
    FILE *f = fopen(name, "wb");
    if (!f) return -1;

    fwrite(buffer, 1, size, f);
    
    fclose(f);
    return 0;
}

/**
 * Convert an AVFrame from its original pixel format to RGB24 format.
 * 
 * @param frame The AVFrame to be converted.
 * @param rgb_buffer The buffer to store the converted RGB24 data.
 * @param width The width of the frame in pixels.
 * @param height The height of the frame in pixels.
 * @param pixel_format The original pixel format of the frame.
 */
static inline void easy_reformat_to_rgb24(AVFrame *frame, unsigned char *rgb_buffer, int width, int height, enum AVPixelFormat pixel_format) 
{
    struct SwsContext *sws_ctx = NULL;

    // Initialize the conversion context
    sws_ctx = sws_getContext(width, height, pixel_format,
                             width, height, AV_PIX_FMT_RGB24,
                             SWS_BILINEAR, NULL, NULL, NULL);

    if (!sws_ctx) {
        fprintf(stderr, "Error creating SwsContext\n");
        return;
    }

    // Convert the YUV frame to RGB
    sws_scale(sws_ctx, (const uint8_t *const *)frame->data, frame->linesize, 0, height, &rgb_buffer, &width);

    // Free the conversion context
    sws_freeContext(sws_ctx);
}


#endif // __EASY_UTILS_H__