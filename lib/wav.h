#ifndef WAV_LIB_H
#define WAV_LIB_H

#include <stdio.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void wav_reserve_header(FILE *fp);

void wav_write_header(FILE *fp, uint32_t sample_rate, uint32_t samples);

#ifdef __cplusplus
}
#endif

static inline void le16(FILE *fp, uint16_t v){
    fputc(v & 0xFF, fp);
    fputc(v >> 8,   fp);
}
static inline void le32(FILE *fp, uint32_t v){
    le16(fp, (uint16_t)(v & 0xFFFF));
    le16(fp, (uint16_t)(v >> 16));
}

#endif
