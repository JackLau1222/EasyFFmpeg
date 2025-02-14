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
#ifndef __EASY_COMMON_H__
#define __EASY_COMMON_H__

#define CHECK_ERROR(err) \
    if ((err) < 0) { \
        char errbuf[128]; \
        av_strerror((err), errbuf, sizeof(errbuf)); \
        fprintf(stderr, "Error: %s\n", errbuf); \
    }

#endif // __EASY_COMMON_H__