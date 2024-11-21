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

RiffFourCC riff_read_fourcc(FILE *file) {
    static char *STR_RIFF = "RIFF";
    static char *STR_LIST = "LIST";

    char fourcc[5];
    size_t bytesRead = fread(&fourcc, 1, 4, file);
    if (bytesRead != 4) {
        if (feof(file)) {
            printf("Reached the end of the file.");
        } else if (ferror(file)) {
            perror("Failed to read the file.");
        }
    } else {
        fourcc[4] = '\0';
        if (strcmp(STR_RIFF, fourcc) == 0) {
            return RIFF_FOURCC_RIFF;
        } else if (strcmp(STR_LIST, fourcc) == 0) {
            return RIFF_FOURCC_LIST;
        }
    }
    return RIFF_FOURCC_UNKNOW;
}