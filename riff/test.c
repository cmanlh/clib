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

#include <stdio.h>
#include <stdlib.h>

#include "riff.h"

int main() {
    FILE *file = fopen("resources/ProtoSquare.sf2", "rb");
    if (file == NULL) {
        perror("Can't open the file.");
        return EXIT_FAILURE;
    }

    char fourcc[5];
    if (riff_read_fourcc(fourcc, file)) {
        fourcc[4] = '\0';
        printf("FourCC : %s\n", fourcc);
    }

    printf("%d\n", riff_read_size(file));

    if (riff_read_fourcc(fourcc, file)) {
        fourcc[4] = '\0';
        printf("FourCC : %s\n", fourcc);
    }

    if (riff_read_fourcc(fourcc, file)) {
        fourcc[4] = '\0';
        printf("FourCC : %s\n", fourcc);
    }

    printf("%d\n", riff_read_size(file));

    if (riff_read_fourcc(fourcc, file)) {
        fourcc[4] = '\0';
        printf("FourCC : %s\n", fourcc);
    }

    RiffSubChunk chunk;
    static char *RIFF_TYPE_LIST = "LIST";

    while (true) {
        chunk = riff_read_chunk_info(file);
        if (0 == chunk.size) {
            break;
        }
        printf("FourCC : %s Size : %d\n", chunk.fourcc, chunk.size);
    }

    fclose(file);

    return EXIT_SUCCESS;
}
