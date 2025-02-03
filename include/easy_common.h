#ifndef __EASY_COMMON_H__
#define __EASY_COMMON_H__

#define CHECK_ERROR(err) \
    if ((err) < 0) { \
        char errbuf[128]; \
        av_strerror((err), errbuf, sizeof(errbuf)); \
        fprintf(stderr, "Error: %s\n", errbuf); \
    }

#endif // __EASY_COMMON_H__