#ifndef CLI_LIB_H
#define CLI_LIB_H

#include <stdbool.h>
#include <stdint.h>
#include "dsp.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef DEF_FREQ
#define DEF_FREQ  750.0
#endif
#ifndef DEF_WPM
#define DEF_WPM   15.0
#endif
#ifndef DEF_SR
#define DEF_SR    16000
#endif
#ifndef DEF_VOL
#define DEF_VOL   1.0
#endif
#ifndef DEF_OUTFILE
#define DEF_OUTFILE "morse.wav"
#endif

typedef struct Params {
    /* input */
    const char *text;          /* literal text or NULL if reading from file */
    const char *infile;        /* path or "-" for stdin */

    /* output */
    const char *outfile;       /* path or "-" for stdout (raw PCM) */

    /* synthesis parameters */
    double freq;               /* carrier frequency, Hz */
    double wpm;                /* words-per-minute speed */
    double vol;                /* 0..1 full-scale */
    double dot_ms;             /* explicit dot length (ms), 0 = derive from WPM */
    double farns;              /* Farnsworth multiplier (>1 => longer gaps) */
    unsigned sr;               /* sample-rate, Hz */
    DspFilter filter;          /* selected audio filter */

    /* flags */
    bool raw;                  /* stream raw PCM to stdout */
    bool play;                 /* play result after generation */
    bool quiet;                /* suppress progress / info output */
    bool version;              /* print version and exit */
} Params;

bool cli_parse(int argc, char **argv, Params *p);

void cli_usage(const char *prog);

#ifdef __cplusplus
}
#endif

#endif
