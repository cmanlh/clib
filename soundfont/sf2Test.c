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
    FILE *file = fopen("resources/ProtoSquare.sf2", "rb");
    // FILE *file = fopen("C:\\Users\\luhong\\Downloads\\FluidR3_GM.sf2", "rb");
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

    uint32_t listSubChunkSize;
    SoundFontInfo info;

    SoundFontSdtaData sdta;
    SoundFontPdtaData pdta;
    for (int i = 0; i < 3; i++) {
        listSubChunkSize = 0;

        switch (soundfont_fetch_list_type(file, &listSubChunkSize)) {
            case SOUNDFONT_TYPE_INFO: {
                printf("Start to process INFO chunk.\n");
                info.size = listSubChunkSize - 4;
                soundfont_init_info(&info);
                soundfont_read_info(&info, file);
                soundfont_print_info(&info);
                break;
            }
            case SOUNDFONT_TYPE_SDTA: {
                printf("Start to process sdta chunk.\n");
                soundfont_read_sdta(&sdta, file);
                break;
            }
            case SOUNDFONT_TYPE_PDTA: {
                printf("Start to process pdta chunk.\n");
                soundfont_init_pdta(&pdta);
                soundfont_read_pdta(&pdta, listSubChunkSize - 4, file);
                soundfont_print_pdta(&pdta);
                break;
            }
            default:
                break;
        }
    }

    soundfont_release_info(&info);
    soundfont_release_sdta(&sdta);
    soundfont_release_pdta(&pdta);
    fclose(file);

    return EXIT_SUCCESS;
}
