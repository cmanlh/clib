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

#include "soundfont2.h"

int main() {
    FILE *file = fopen("C:\\Users\\luhong\\Downloads\\FSS-SteelStringGuitar-SF2-20200521.tar\\FSS-SteelStringGuitar-SF2-20200521\\FSS-SteelStringGuitar-20200521.sf2", "rb");
    if (file == NULL) {
        perror("Can't open the file.");
        return EXIT_FAILURE;
    }

    char fourcc[5];
    if (soundfont_read_fourcc(fourcc, file)) {
        fourcc[4] = '\0';
        printf("FourCC : %s\n", fourcc);
    }

    printf("%d\n", soundfont_read_size(file));

    if (soundfont_read_fourcc(fourcc, file)) {
        fourcc[4] = '\0';
        printf("FourCC : %s\n", fourcc);
    }

    if (soundfont_read_fourcc(fourcc, file)) {
        fourcc[4] = '\0';
        printf("FourCC : %s\n", fourcc);
    }
    uint32_t subChunkSize = soundfont_read_size(file);
    printf("%d\n", subChunkSize);

    if (soundfont_read_fourcc(fourcc, file)) {
        fourcc[4] = '\0';
        printf("FourCC : %s\n", fourcc);
    }

    SoundFontChunk chunk;
    static char *SOUNDFONT_TYPE_LIST = "LIST";
    SoundFontInfo info;
    info.size = subChunkSize - 4;
    soundfont_init_info(&info);
    soundfont_read_info(&info, file);
    soundfont_print_info(&info);

    if (soundfont_read_fourcc(fourcc, file)) {
        fourcc[4] = '\0';
        printf("FourCC : %s\n", fourcc);
    }
    subChunkSize = soundfont_read_size(file);
    printf("%d\n", subChunkSize);

    if (soundfont_read_fourcc(fourcc, file)) {
        fourcc[4] = '\0';
        printf("FourCC : %s\n", fourcc);
    }

    SoundFontSdtaData sdta;
    soundfont_read_sdta(&sdta, file);

    if (soundfont_read_fourcc(fourcc, file)) {
        fourcc[4] = '\0';
        printf("FourCC : %s\n", fourcc);
    }
    subChunkSize = soundfont_read_size(file);
    printf("%d\n", subChunkSize);

    if (soundfont_read_fourcc(fourcc, file)) {
        fourcc[4] = '\0';
        printf("FourCC : %s\n", fourcc);
    }

    soundfont_release_info(&info);
    soundfont_release_sdta(&sdta);
    fclose(file);

    return EXIT_SUCCESS;
}
