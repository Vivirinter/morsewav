#include "wav.h"
#include "common.h"
#include <string.h>
#include <inttypes.h>

#define RF64_CHUNK_ID   "RF64"
#define WAVE_ID         "WAVE"
#define DS64_ID         "ds64"

static inline void le64(FILE *fp, uint64_t v){
    le32(fp, (uint32_t)(v & 0xFFFFFFFFu));
    le32(fp, (uint32_t)(v >> 32));
}

void wav_reserve_header(FILE *fp){
    static const unsigned char zeros[WAV_HEADER_BYTES] = {0};
    fwrite(zeros,1,WAV_HEADER_BYTES,fp);
}

void wav_write_header(FILE *fp, uint32_t sr, uint32_t samples){
    const uint16_t channels = 1;
    const uint16_t bits_per_sample = 16;
    const uint16_t block_align = channels * bits_per_sample / 8;
    const uint32_t byte_rate   = sr * block_align;
    const uint32_t data_size   = samples * block_align;

    rewind(fp);

    fwrite("RIFF",1,4,fp); le32(fp, 36 + data_size);
    fwrite("WAVE",1,4,fp);

    fwrite("fmt ",1,4,fp); le32(fp, 16);
    le16(fp, 1);
    le16(fp, channels);
    le32(fp, sr);
    le32(fp, byte_rate);
    le16(fp, block_align);
    le16(fp, bits_per_sample);

    fwrite("data",1,4,fp);
    le32(fp, data_size);
}

void wav_write_header64(FILE *fp, uint32_t sr, uint64_t samples){
    const uint16_t channels = 1;
    const uint16_t bits_per_sample = 16;
    const uint16_t block_align = channels * bits_per_sample / 8;

    const uint64_t data_size64 = samples * (uint64_t)block_align;

    rewind(fp);

    fwrite(RF64_CHUNK_ID,1,4,fp); le32(fp, 0xFFFFFFFFu);
    fwrite(WAVE_ID,1,4,fp);

    fwrite(DS64_ID,1,4,fp); le32(fp, 28);
    le64(fp, 36 + 28 + data_size64); /* riffSize64 */
    le64(fp, data_size64);           /* dataSize64 */
    le64(fp, samples);               /* sampleCount64 */
    le32(fp, 0);                     /* table length = 0 */

    fwrite("fmt ",1,4,fp); le32(fp, 16);
    le16(fp, 1);
    le16(fp, channels);
    le32(fp, sr);
    le32(fp, sr * block_align);
    le16(fp, block_align);
    le16(fp, bits_per_sample);

    fwrite("data",1,4,fp); le32(fp, 0xFFFFFFFFu);
}
