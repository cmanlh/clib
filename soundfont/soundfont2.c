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

static char *STR_FORMAT_LIST = "LIST";
static char *STR_TYPE_INFO = "INFO";
static char *STR_TYPE_SDTA = "sdta";
static char *STR_TYPE_PDTA = "pdta";
static char *STR_PDTA_TYPE_PHDR = "phdr";
static char *STR_PDTA_TYPE_PBAG = "pbag";
static char *STR_PDTA_TYPE_PMOD = "pmod";
static char *STR_PDTA_TYPE_PGEN = "pgen";
static char *STR_PDTA_TYPE_INST = "inst";
static char *STR_PDTA_TYPE_IBAG = "ibag";
static char *STR_PDTA_TYPE_IMOD = "imod";
static char *STR_PDTA_TYPE_IGEN = "igen";

static bool read_pdta_preset_header(SoundFontPdtaData *pdta, uint32_t *pdtaSize, FILE *file);
static bool read_pdta_preset_index(SoundFontPdtaData *pdta, uint32_t *pdtaSize, FILE *file);
static bool read_pdta_preset_mod(SoundFontPdtaData *pdta, uint32_t *pdtaSize, FILE *file);
static bool read_pdta_preset_gen(SoundFontPdtaData *pdta, uint32_t *pdtaSize, FILE *file);
static bool read_pdta_preset_inst(SoundFontPdtaData *pdta, uint32_t *pdtaSize, FILE *file);
static bool read_pdta_preset_ibag(SoundFontPdtaData *pdta, uint32_t *pdtaSize, FILE *file);
static bool read_pdta_inst_mod(SoundFontPdtaData *pdta, uint32_t *pdtaSize, FILE *file);
static bool read_pdta_inst_gen(SoundFontPdtaData *pdta, uint32_t *pdtaSize, FILE *file);

static void print_pdta_preset_header(SoundFontPdtaData *pdta);
static void print_pdta_preset_index(SoundFontPdtaData *pdta);
static void print_pdta_preset_mod(SoundFontPdtaData *pdta);
static void print_pdta_preset_gen(SoundFontPdtaData *pdta);
static void print_pdta_preset_inst(SoundFontPdtaData *pdta);
static void print_pdta_preset_ibag(SoundFontPdtaData *pdta);
static void print_pdta_inst_mod(SoundFontPdtaData *pdta);
static void print_pdta_inst_gen(SoundFontPdtaData *pdta);

uint32_t soundfont_read_size(FILE *file) {
    uint8_t chunkSize[4];
    size_t bytesRead = fread(&chunkSize, 1, 4, file);
    if (bytesRead != 4) {
        if (feof(file)) {
            printf("Reached the end of the file.");
        } else if (ferror(file)) {
            perror("Failed to read the file.");
        }
        return 0;
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

SoundFontListType soundfont_fetch_list_type(FILE *file, uint32_t *size) {
    char fourcc[5];
    fourcc[4] = '\0';
    if (soundfont_read_fourcc(fourcc, file)) {
        if (0 == strcmp(STR_FORMAT_LIST, fourcc)) {
            *size = soundfont_read_size(file);
            if (*size > 0) {
                if (soundfont_read_fourcc(fourcc, file)) {
                    if (0 == strcmp(STR_TYPE_INFO, fourcc)) {
                        return SOUNDFONT_TYPE_INFO;
                    } else if (0 == strcmp(STR_TYPE_SDTA, fourcc)) {
                        return SOUNDFONT_TYPE_SDTA;
                    } else if (0 == strcmp(STR_TYPE_PDTA, fourcc)) {
                        return SOUNDFONT_TYPE_PDTA;
                    }
                }
            }
        } else {
            printf("Unexpected sound font 2 format. Expected : %s Actual : %s", STR_FORMAT_LIST, fourcc);
        }
    }

    return SOUNDFONT_TYPE_UNKNOW;
}

SoundFontChunk soundfont_read_chunk_info(FILE *file) {
    SoundFontChunk chunk;

    if (soundfont_read_fourcc(chunk.fourcc, file)) {
        chunk.fourcc[4] = '\0';
        chunk.size = soundfont_read_size(file);

        if (0 == strcmp(STR_FORMAT_LIST, chunk.fourcc)) {
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

void soundfont_init_pdta(SoundFontPdtaData *pdta) {
    pdta->presetHeader = NULL;
    pdta->presetHeaderSize = 0;

    pdta->presetIndex = NULL;
    pdta->presetIndexSize = 0;

    pdta->presetMod = NULL;
    pdta->presetModSize = 0;

    pdta->presetGen = NULL;
    pdta->presetGenSize = 0;

    pdta->presetInst = NULL;
    pdta->presetInstSize = 0;

    pdta->presetIbag = NULL;
    pdta->presetIbagSize = 0;

    pdta->iMod = NULL;
    pdta->iModSize = 0;

    pdta->iGen = NULL;
    pdta->iGenSize = 0;
}

bool soundfont_read_pdta(SoundFontPdtaData *pdta, uint32_t size, FILE *file) {
    uint32_t pdtaSize = size;

    char fourcc[5];
    fourcc[4] = '\0';
    if (soundfont_read_fourcc(fourcc, file)) {
        pdtaSize -= 4;

        if (0 == strcmp(STR_PDTA_TYPE_PHDR, fourcc)) {
            read_pdta_preset_header(pdta, &pdtaSize, file);
        }
    }

    if (soundfont_read_fourcc(fourcc, file)) {
        pdtaSize -= 4;

        if (0 == strcmp(STR_PDTA_TYPE_PBAG, fourcc)) {
            read_pdta_preset_index(pdta, &pdtaSize, file);
        }
    }

    if (soundfont_read_fourcc(fourcc, file)) {
        pdtaSize -= 4;

        if (0 == strcmp(STR_PDTA_TYPE_PMOD, fourcc)) {
            read_pdta_preset_mod(pdta, &pdtaSize, file);
        }
    }

    if (soundfont_read_fourcc(fourcc, file)) {
        pdtaSize -= 4;

        if (0 == strcmp(STR_PDTA_TYPE_PGEN, fourcc)) {
            read_pdta_preset_gen(pdta, &pdtaSize, file);
        }
    }

    if (soundfont_read_fourcc(fourcc, file)) {
        pdtaSize -= 4;

        if (0 == strcmp(STR_PDTA_TYPE_INST, fourcc)) {
            read_pdta_preset_inst(pdta, &pdtaSize, file);
        }
    }

    if (soundfont_read_fourcc(fourcc, file)) {
        pdtaSize -= 4;

        if (0 == strcmp(STR_PDTA_TYPE_IBAG, fourcc)) {
            read_pdta_preset_ibag(pdta, &pdtaSize, file);
        }
    }

    if (soundfont_read_fourcc(fourcc, file)) {
        pdtaSize -= 4;

        if (0 == strcmp(STR_PDTA_TYPE_IMOD, fourcc)) {
            read_pdta_inst_mod(pdta, &pdtaSize, file);
        }
    }

    if (soundfont_read_fourcc(fourcc, file)) {
        pdtaSize -= 4;

        if (0 == strcmp(STR_PDTA_TYPE_IGEN, fourcc)) {
            read_pdta_inst_gen(pdta, &pdtaSize, file);
        }
    }

    return true;
}

void soundfont_release_pdta(SoundFontPdtaData *pdta) {
    if (NULL != pdta->presetHeader) {
        free(pdta->presetHeader);
    }
    if (NULL != pdta->presetIndex) {
        free(pdta->presetIndex);
    }
    if (NULL != pdta->presetMod) {
        free(pdta->presetMod);
    }
    if (NULL != pdta->presetGen) {
        free(pdta->presetGen);
    }
    if (NULL != pdta->presetInst) {
        free(pdta->presetInst);
    }
    if (NULL != pdta->presetIbag) {
        free(pdta->presetIbag);
    }
    if (NULL != pdta->iMod) {
        free(pdta->iMod);
    }
    if (NULL != pdta->iGen) {
        free(pdta->iGen);
    }
}

void soundfont_print_pdta(SoundFontPdtaData *pdta) {
    print_pdta_preset_header(pdta);
    print_pdta_preset_index(pdta);
    print_pdta_preset_mod(pdta);
    print_pdta_preset_gen(pdta);
    print_pdta_preset_inst(pdta);
    print_pdta_preset_ibag(pdta);
    print_pdta_inst_mod(pdta);
    print_pdta_inst_gen(pdta);
}

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
                printf("Not enough memory for reading sdta.\n");

                return false;
            }
        } else {
            printf("Invalid chunk type for sdta.\n");

            return false;
        }
    } else {
        printf("Failed to read stda chunk.\n");

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

static bool read_pdta_preset_header(SoundFontPdtaData *pdta, uint32_t *pdtaSize, FILE *file) {
    uint32_t chunkSize = soundfont_read_size(file);
    if (chunkSize <= 0) {
        printf("Failed to fetch the size of preset header.");
        return false;
    }
    *pdtaSize -= 4;

    uint16_t HEADER_SIZE = 38;
    if (chunkSize % HEADER_SIZE != 0) {
        printf("Broken data for preset header.");
        return false;
    }
    pdta->presetHeaderSize = chunkSize / HEADER_SIZE;
    pdta->presetHeader = (SoundFontPresetHeader *)malloc(sizeof(SoundFontPresetHeader) * pdta->presetHeaderSize);

    uint8_t buffer[4];
    for (int i = 0; i < pdta->presetHeaderSize; i++) {
        SoundFontPresetHeader *header = pdta->presetHeader + i;

        if (20 != fread(header->name, 1, 20, file)) {
            return false;
        }

        if (2 == fread(&buffer, 1, 2, file)) {
            header->preset = buffer[0] | buffer[1] << 8;
        } else {
            return false;
        }

        if (2 == fread(&buffer, 1, 2, file)) {
            header->bank = buffer[0] | buffer[1] << 8;
        } else {
            return false;
        }

        if (2 == fread(&buffer, 1, 2, file)) {
            header->presetBagNdx = buffer[0] | buffer[1] << 8;
        } else {
            return false;
        }

        if (4 == fread(&buffer, 1, 4, file)) {
            header->library = buffer[0] | buffer[1] << 8 | buffer[2] << 16 | buffer[3] << 24;
        } else {
            return false;
        }

        if (4 == fread(&buffer, 1, 4, file)) {
            header->genre = buffer[0] | buffer[1] << 8 | buffer[2] << 16 | buffer[3] << 24;
        } else {
            return false;
        }

        if (4 == fread(&buffer, 1, 4, file)) {
            header->morphology = buffer[0] | buffer[1] << 8 | buffer[2] << 16 | buffer[3] << 24;
        } else {
            return false;
        }
    }

    *pdtaSize -= chunkSize;

    return true;
}

static void print_pdta_preset_header(SoundFontPdtaData *pdta) {
    printf("\n=== PDTA PRESET HEADER START ===\n");
    for (int i = 0; i < pdta->presetHeaderSize; i++) {
        SoundFontPresetHeader *header = pdta->presetHeader + i;
        printf("name : %s ", header->name);
        printf("preset : %d ", header->preset);
        printf("bank : %d ", header->bank);
        printf("presetBagNdx : %d ", header->presetBagNdx);
        printf("library : %d ", header->library);
        printf("genre : %d ", header->genre);
        printf("morphology : %d ", header->morphology);
        printf("\n");
    }
    printf("=== PDTA PRESET HEADER END   ===\n");
}

static bool read_pdta_preset_index(SoundFontPdtaData *pdta, uint32_t *pdtaSize, FILE *file) {
    uint32_t chunkSize = soundfont_read_size(file);
    if (chunkSize <= 0) {
        printf("Failed to fetch the size of preset index.");
        return false;
    }
    *pdtaSize -= 4;

    uint16_t INDEX_SIZE = 4;
    if (chunkSize % INDEX_SIZE != 0) {
        printf("Broken data for preset index.");
        return false;
    }
    pdta->presetIndexSize = chunkSize / INDEX_SIZE;
    pdta->presetIndex = (SoundFontPresetIndex *)malloc(sizeof(SoundFontPresetIndex) * pdta->presetIndexSize);

    uint8_t buffer[2];
    for (int i = 0; i < pdta->presetIndexSize; i++) {
        SoundFontPresetIndex *index = pdta->presetIndex + i;

        fread(&buffer, 1, 2, file);
        index->genNdx = buffer[0] | buffer[1] << 8;

        fread(&buffer, 1, 2, file);
        index->modNdx = buffer[0] | buffer[1] << 8;
    }

    *pdtaSize -= chunkSize;

    return true;
}

static void print_pdta_preset_index(SoundFontPdtaData *pdta) {
    printf("\n=== PDTA PRESET INDEX START ===\n");
    for (int i = 0; i < pdta->presetIndexSize; i++) {
        SoundFontPresetIndex *index = pdta->presetIndex + i;
        printf("genNdx : %d modNdx : %d\n", index->genNdx, index->modNdx);
    }
    printf("=== PDTA PRESET INDEX END   ===\n");
}

static bool read_pdta_preset_mod(SoundFontPdtaData *pdta, uint32_t *pdtaSize, FILE *file) {
    uint32_t chunkSize = soundfont_read_size(file);
    if (chunkSize <= 0) {
        printf("Failed to fetch the size of preset mod.");
        return false;
    }
    *pdtaSize -= 4;

    uint16_t MOD_SIZE = 10;
    if (chunkSize % MOD_SIZE != 0) {
        printf("Broken data for preset mod.");
        return false;
    }
    pdta->presetModSize = chunkSize / MOD_SIZE;
    pdta->presetMod = (SoundFontMod *)malloc(sizeof(SoundFontMod) * pdta->presetModSize);

    uint8_t buffer[2];
    for (int i = 0; i < pdta->presetModSize; i++) {
        SoundFontMod *mod = pdta->presetMod + i;

        fread(&buffer, 1, 2, file);
        mod->srcOperator = buffer[0] | buffer[1] << 8;

        fread(&buffer, 1, 2, file);
        mod->destOperator = buffer[0] | buffer[1] << 8;

        fread(&buffer, 1, 2, file);
        mod->amount = buffer[0] | buffer[1] << 8;

        fread(&buffer, 1, 2, file);
        mod->amtSrcOperator = buffer[0] | buffer[1] << 8;

        fread(&buffer, 1, 2, file);
        mod->transOperator = buffer[0] | buffer[1] << 8;
    }

    *pdtaSize -= chunkSize;

    return true;
}

static void print_pdta_preset_mod(SoundFontPdtaData *pdta) {
    printf("\n=== PDTA PRESET MOD START ===\n");
    for (int i = 0; i < pdta->presetModSize; i++) {
        SoundFontMod *mod = pdta->presetMod + i;
        printf("srcOperator : %d ", mod->srcOperator);
        printf("destOperator : %d ", mod->destOperator);
        printf("amount : %d ", mod->amount);
        printf("amtSrcOperator : %d ", mod->amtSrcOperator);
        printf("transOperator : %d\n", mod->transOperator);
    }
    printf("=== PDTA PRESET MOD END   ===\n");
}

static bool read_pdta_preset_gen(SoundFontPdtaData *pdta, uint32_t *pdtaSize, FILE *file) {
    uint32_t chunkSize = soundfont_read_size(file);
    if (chunkSize <= 0) {
        printf("Failed to fetch the size of preset generator.");
        return false;
    }
    *pdtaSize -= 4;

    uint16_t GEN_SIZE = 4;
    if (chunkSize % GEN_SIZE != 0) {
        printf("Broken data for preset generator.");
        return false;
    }
    pdta->presetGenSize = chunkSize / GEN_SIZE;
    pdta->presetGen = (SoundFontGen *)malloc(sizeof(SoundFontGen) * pdta->presetGenSize);

    uint8_t buffer[2];
    for (int i = 0; i < pdta->presetGenSize; i++) {
        SoundFontGen *gen = pdta->presetGen + i;

        fread(&buffer, 1, 2, file);
        gen->operator= buffer[0] | buffer[1] << 8;

        fread(&buffer, 1, 2, file);
        gen->amount = buffer[0] | buffer[1] << 8;
    }

    *pdtaSize -= chunkSize;

    return true;
}

static void print_pdta_preset_gen(SoundFontPdtaData *pdta) {
    printf("\n=== PDTA PRESET GENERATOR START ===\n");
    for (int i = 0; i < pdta->presetGenSize; i++) {
        SoundFontGen *gen = pdta->presetGen + i;
        printf("operator : %d amount : %d\n", gen->operator, gen->amount);
    }
    printf("=== PDTA PRESET GENERATOR END   ===\n");
}

static bool read_pdta_preset_inst(SoundFontPdtaData *pdta, uint32_t *pdtaSize, FILE *file) {
    uint32_t chunkSize = soundfont_read_size(file);
    if (chunkSize <= 0) {
        printf("Failed to fetch the size of preset instructment names and indices.");
        return false;
    }
    *pdtaSize -= 4;

    uint16_t INST_SIZE = 22;
    if (chunkSize % INST_SIZE != 0) {
        printf("Broken data for preset instructment names and indices.");
        return false;
    }
    pdta->presetInstSize = chunkSize / INST_SIZE;
    pdta->presetInst = (SoundFontPresetInst *)malloc(sizeof(SoundFontPresetInst) * pdta->presetInstSize);

    uint8_t buffer[2];
    for (int i = 0; i < pdta->presetInstSize; i++) {
        SoundFontPresetInst *inst = pdta->presetInst + i;

        if (20 != fread(inst->name, 1, 20, file)) {
            return false;
        }

        fread(&buffer, 1, 2, file);
        inst->index = buffer[0] | buffer[1] << 8;
    }

    *pdtaSize -= chunkSize;

    return true;
}

static void print_pdta_preset_inst(SoundFontPdtaData *pdta) {
    printf("\n=== PDTA PRESET INSTRUMENT NAME AND INDICES START ===\n");
    for (int i = 0; i < pdta->presetInstSize; i++) {
        SoundFontPresetInst *inst = pdta->presetInst + i;
        printf("name : %s index : %d\n", inst->name, inst->index);
    }
    printf("=== PDTA PRESET INSTRUMENT NAME AND INDICES END   ===\n");
}

static bool read_pdta_preset_ibag(SoundFontPdtaData *pdta, uint32_t *pdtaSize, FILE *file) {
    uint32_t chunkSize = soundfont_read_size(file);
    if (chunkSize <= 0) {
        printf("Failed to fetch the size of preset instrument index list.");
        return false;
    }
    *pdtaSize -= 4;

    uint16_t IBAG_SIZE = 4;
    if (chunkSize % IBAG_SIZE != 0) {
        printf("Broken data for preset instrument index list.");
        return false;
    }
    pdta->presetIbagSize = chunkSize / IBAG_SIZE;
    pdta->presetIbag = (SoundFontPresetIbag *)malloc(sizeof(SoundFontPresetIbag) * pdta->presetIbagSize);

    uint8_t buffer[2];
    for (int i = 0; i < pdta->presetIbagSize; i++) {
        SoundFontPresetIbag *ibag = pdta->presetIbag + i;

        fread(&buffer, 1, 2, file);
        ibag->genNdx = buffer[0] | buffer[1] << 8;

        fread(&buffer, 1, 2, file);
        ibag->modNdx = buffer[0] | buffer[1] << 8;
    }

    *pdtaSize -= chunkSize;

    return true;
}

static void print_pdta_preset_ibag(SoundFontPdtaData *pdta) {
    printf("\n=== PDTA PRESET INSTRUMENT INDEX START ===\n");
    for (int i = 0; i < pdta->presetIbagSize; i++) {
        SoundFontPresetIbag *ibag = pdta->presetIbag + i;
        printf("genNdx : %d modNdx : %d\n", ibag->genNdx, ibag->modNdx);
    }
    printf("=== PDTA PRESET INSTRUMENT INDEX END   ===\n");
}

static bool read_pdta_inst_mod(SoundFontPdtaData *pdta, uint32_t *pdtaSize, FILE *file) {
    uint32_t chunkSize = soundfont_read_size(file);
    if (chunkSize <= 0) {
        printf("Failed to fetch the size of instrument mod.");
        return false;
    }
    *pdtaSize -= 4;

    uint16_t MOD_SIZE = 10;
    if (chunkSize % MOD_SIZE != 0) {
        printf("Broken data for instrument mod.");
        return false;
    }
    pdta->iModSize = chunkSize / MOD_SIZE;
    pdta->iMod = (SoundFontMod *)malloc(sizeof(SoundFontMod) * pdta->iModSize);

    uint8_t buffer[2];
    for (int i = 0; i < pdta->iModSize; i++) {
        SoundFontMod *mod = pdta->iMod + i;

        fread(&buffer, 1, 2, file);
        mod->srcOperator = buffer[0] | buffer[1] << 8;

        fread(&buffer, 1, 2, file);
        mod->destOperator = buffer[0] | buffer[1] << 8;

        fread(&buffer, 1, 2, file);
        mod->amount = buffer[0] | buffer[1] << 8;

        fread(&buffer, 1, 2, file);
        mod->amtSrcOperator = buffer[0] | buffer[1] << 8;

        fread(&buffer, 1, 2, file);
        mod->transOperator = buffer[0] | buffer[1] << 8;
    }

    *pdtaSize -= chunkSize;

    return true;
}

static void print_pdta_inst_mod(SoundFontPdtaData *pdta) {
    printf("\n=== PDTA INSTRUMENT MOD START ===\n");
    for (int i = 0; i < pdta->iModSize; i++) {
        SoundFontMod *mod = pdta->iMod + i;
        printf("srcOperator : %d ", mod->srcOperator);
        printf("destOperator : %d ", mod->destOperator);
        printf("amount : %d ", mod->amount);
        printf("amtSrcOperator : %d ", mod->amtSrcOperator);
        printf("transOperator : %d\n", mod->transOperator);
    }
    printf("=== PDTA INSTRUMENT MOD END   ===\n");
}

static bool read_pdta_inst_gen(SoundFontPdtaData *pdta, uint32_t *pdtaSize, FILE *file) {
    uint32_t chunkSize = soundfont_read_size(file);
    if (chunkSize <= 0) {
        printf("Failed to fetch the size of instrument generator.");
        return false;
    }
    *pdtaSize -= 4;

    uint16_t GEN_SIZE = 4;
    if (chunkSize % GEN_SIZE != 0) {
        printf("Broken data for instrument generator.");
        return false;
    }
    pdta->iGenSize = chunkSize / GEN_SIZE;
    pdta->iGen = (SoundFontGen *)malloc(sizeof(SoundFontGen) * pdta->presetGenSize);

    uint8_t buffer[2];
    for (int i = 0; i < pdta->iGenSize; i++) {
        SoundFontGen *gen = pdta->iGen + i;

        fread(&buffer, 1, 2, file);
        gen->operator= buffer[0] | buffer[1] << 8;

        fread(&buffer, 1, 2, file);
        gen->amount = buffer[0] | buffer[1] << 8;
    }

    *pdtaSize -= chunkSize;

    return true;
}

static void print_pdta_inst_gen(SoundFontPdtaData *pdta) {
    printf("\n=== PDTA PRESET INSTRUMENT START ===\n");
    for (int i = 0; i < pdta->iGenSize; i++) {
        SoundFontGen *gen = pdta->iGen + i;
        printf("operator : %d amount : %d\n", gen->operator, gen->amount);
    }
    printf("=== PDTA PRESET INSTRUMENT END   ===\n");
}