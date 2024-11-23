/*
    LICENSE (MIT)

    Copyright (c) 2024 cmanlh (https://gitee.com/lifeonwalden/clib)
                              (https://github.com/cmanlh/clib)

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
*/
#ifndef CMANLH_SOUNDFONT
#define CMANLH_SOUNDFONT

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct SoundFontInfo {
    uint32_t size;
    uint16_t major;  // major version of the sound font file
    uint16_t minor;  // minor version of the sound font file
    char *engine;
    char *name;
    char *romName;
    uint16_t romMajor;
    uint16_t romMinor;
    char *createDate;
    char *author;
    char *product;
    char *copyright;
    char *comments;
    char *tools;
} SoundFontInfo;

typedef struct SoundFontChunk {
    char fourcc[5];
    uint32_t size;
} SoundFontChunk;

bool soundfont_read_fourcc(char *fourcc, FILE *file);

uint32_t soundfont_read_size(FILE *file);

SoundFontChunk soundfont_read_chunk_info(FILE *file);

SoundFontChunk soundfont_read_chunk(FILE *file);

void soundfont_init_info(SoundFontInfo *info);
void soundfont_read_info(SoundFontInfo *info, FILE *file);
void soundfont_release_info(SoundFontInfo *info);

#endif