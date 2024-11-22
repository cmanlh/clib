/*
    RIFF file process library

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
#include "riff.h"

#include <string.h>

static char *RIFF_TYPE_LIST = "LIST";

uint32_t riff_read_size(FILE *file) {
    uint8_t chunkSize[4];
    size_t bytesRead = fread(&chunkSize, 1, 4, file);
    if (bytesRead != 4) {
        if (feof(file)) {
            printf("Reached the end of the file.");
        } else if (ferror(file)) {
            perror("Failed to read the file.");
        }
    } else {
        return (chunkSize[3] << 24) | (chunkSize[2] << 16) | (chunkSize[1] << 8) | chunkSize[0];
    }
}

bool riff_read_fourcc(char *fourcc, FILE *file) {
    size_t bytesRead = fread(fourcc, 1, 4, file);
    if (bytesRead != 4) {
        if (feof(file)) {
            printf("Reached the end of the file.");
        } else if (ferror(file)) {
            perror("Failed to read the file.");
        }
        return false;
    } else {
        return true;
    }
}

RiffSubChunk riff_read_chunk_info(FILE *file) {
    RiffSubChunk chunk;

    if (riff_read_fourcc(chunk.fourcc, file)) {
        chunk.fourcc[4] = '\0';
        chunk.size = riff_read_size(file);

        if (0 == strcmp(RIFF_TYPE_LIST, chunk.fourcc)) {
            printf("FourCC : %s Size : %d\n", chunk.fourcc, chunk.size);

            riff_read_fourcc(chunk.fourcc, file);
            chunk.fourcc[4] = '\0';
            chunk.size -= 4;
        } else {
            fseek(file, chunk.size, SEEK_CUR);
        }
    } else {
        chunk.size = 0;
    }

    return chunk;
}

RiffSubChunk riff_read_chunk(FILE *file);