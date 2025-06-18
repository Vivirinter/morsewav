#ifndef DSP_LIB_H
#pragma once
#define DSP_LIB_H

#include <stdio.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Simple FIR/Window choices */
typedef enum {
    DSP_FILTER_NONE = 0,
    DSP_FILTER_HANN3   /* 3-tap Hann window: 0.25 0.5 0.25 */
} DspFilter;

/* Generate N samples of silence (PCM 16-bit LE) */
uint64_t dsp_write_silence(FILE *fp, uint64_t samples);

uint64_t dsp_write_tone(FILE *fp, uint64_t samples, double *phase,
                        double phase_inc, double vol, unsigned sr,
                        DspFilter filter);

#ifdef __cplusplus
}
#endif

#endif