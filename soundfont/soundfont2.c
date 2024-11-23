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
#include "soundfont2.h"

static char *SOUNDFONT_TYPE_LIST = "LIST";

uint32_t soundfont_read_size(FILE *file) {
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

bool soundfont_read_fourcc(char *fourcc, FILE *file) {
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

SoundFontChunk soundfont_read_chunk_info(FILE *file) {
    SoundFontChunk chunk;

    if (soundfont_read_fourcc(chunk.fourcc, file)) {
        chunk.fourcc[4] = '\0';
        chunk.size = soundfont_read_size(file);

        if (0 == strcmp(SOUNDFONT_TYPE_LIST, chunk.fourcc)) {
            printf("FourCC : %s Size : %d\n", chunk.fourcc, chunk.size);

            soundfont_read_fourcc(chunk.fourcc, file);
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

SoundFontChunk soundfont_read_chunk(FILE *file);

bool soundfont_read_sdta(SoundFontSdtaData *sdta, FILE *file) {
    char fourcc[5];
    fourcc[4] = '\0';
    if (soundfont_read_fourcc(fourcc, file)) {
        uint32_t chunkSize = soundfont_read_size(file);

        if (0 == strcmp(fourcc, "smpl")) {
            sdta->size = chunkSize;
            sdta->data = (uint8_t *)malloc(chunkSize);
            if (NULL != sdta->data) {
                fread(sdta->data, 1, chunkSize, file);
            } else {
                perror("Not enough memory for sdta.");

                return false;
            }
        } else {
            perror("Invalid chunk type for sdta.");

            return false;
        }
    } else {
        perror("Failed to read stda chunk.");

        return false;
    }

    return true;
}

void soundfont_release_sdta(SoundFontSdtaData *sdta) {
    free(sdta->data);
}

void soundfont_init_info(SoundFontInfo *info) {
    info->engine = NULL;
    info->name = NULL;
    info->romName = NULL;
    info->createDate = NULL;
    info->author = NULL;
    info->product = NULL;
    info->copyright = NULL;
    info->comments = NULL;
    info->tools = NULL;
}

void soundfont_read_info(SoundFontInfo *info, FILE *file) {
    uint32_t infoSize = info->size;

    char fourcc[5];
    fourcc[4] = '\0';
    while (infoSize > 0) {
        if (soundfont_read_fourcc(fourcc, file)) {
            uint32_t chunkSize = soundfont_read_size(file);
            infoSize -= 8;

            if (0 == strcmp(fourcc, "ifil")) {
                uint8_t version[4];
                fread(version, 1, 4, file);
                info->major = version[1] << 8 | version[0];
                info->minor = version[3] << 8 | version[2];
            } else if (0 == strcmp(fourcc, "isng")) {
                info->engine = (char *)malloc(chunkSize);
                fread(info->engine, 1, chunkSize, file);
            } else if (0 == strcmp(fourcc, "INAM")) {
                info->name = (char *)malloc(chunkSize);
                fread(info->name, 1, chunkSize, file);
            } else if (0 == strcmp(fourcc, "irom")) {
                info->romName = (char *)malloc(chunkSize);
                fread(info->romName, 1, chunkSize, file);
            } else if (0 == strcmp(fourcc, "iver")) {
                uint8_t version[4];
                fread(version, 1, 4, file);
                info->romMajor = version[1] << 8 | version[0];
                info->romMinor = version[3] << 8 | version[2];
            } else if (0 == strcmp(fourcc, "ICRD")) {
                info->createDate = (char *)malloc(chunkSize);
                fread(info->createDate, 1, chunkSize, file);
            } else if (0 == strcmp(fourcc, "IENG")) {
                info->author = (char *)malloc(chunkSize);
                fread(info->author, 1, chunkSize, file);
            } else if (0 == strcmp(fourcc, "IPRD")) {
                info->product = (char *)malloc(chunkSize);
                fread(info->product, 1, chunkSize, file);
            } else if (0 == strcmp(fourcc, "ICOP")) {
                info->copyright = (char *)malloc(chunkSize);
                fread(info->copyright, 1, chunkSize, file);
            } else if (0 == strcmp(fourcc, "ICMT")) {
                info->comments = (char *)malloc(chunkSize);
                fread(info->comments, 1, chunkSize, file);
            } else if (0 == strcmp(fourcc, "ISFT")) {
                info->tools = (char *)malloc(chunkSize);
                fread(info->tools, 1, chunkSize, file);
            }

            infoSize -= chunkSize;
        } else {
            break;
        }
    }
}
void soundfont_release_info(SoundFontInfo *info) {
    if (NULL != info->engine) {
        free(info->engine);
    }
    if (NULL != info->author) {
        free(info->author);
    }
    if (NULL != info->romName) {
        free(info->romName);
    }
    if (NULL != info->createDate) {
        free(info->createDate);
    }
    if (NULL != info->product) {
        free(info->product);
    }
    if (NULL != info->copyright) {
        free(info->copyright);
    }
    if (NULL != info->comments) {
        free(info->comments);
    }
    if (NULL != info->tools) {
        free(info->tools);
    }
}

void soundfont_print_info(SoundFontInfo *info) {
    printf("major : %d minor : %d\n", info->major, info->minor);
    printf("engine : %s\n", info->engine);
    printf("name : %s\n", info->name);
    printf("romName : %s\n", info->romName);
    printf("romMajor : %d romMinor : %d\n", info->romMajor, info->romMinor);
    printf("createDate : %s\n", info->createDate);
    printf("author : %s\n", info->author);
    printf("product : %s\n", info->product);
    printf("copyright : %s\n", info->copyright);
    printf("comments : %s\n", info->comments);
    printf("tools : %s\n", info->tools);
}