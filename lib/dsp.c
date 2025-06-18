#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include "dsp.h"
#include "wav.h"

#ifndef RAMP_MS
#define RAMP_MS 5.0
#endif


#define BUFSIZE 4096

uint64_t dsp_write_silence(FILE *fp, uint64_t samples)
{
    static int16_t zbuf[BUFSIZE] = {0};
    uint64_t remaining = samples;
    while (remaining) {
        size_t chunk = (remaining > BUFSIZE) ? BUFSIZE : (size_t)remaining;
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
        fwrite(zbuf, sizeof(int16_t), chunk, fp);
#else
        /* fall back to le16 helper for big-endian hosts */
        for (size_t i = 0; i < chunk; ++i) {
            le16(fp, 0);
        }
#endif
        remaining -= chunk;
    }
    return samples;
}

uint64_t dsp_write_tone(FILE *fp, uint64_t samples, double *phase,
                        double phase_inc, double vol, unsigned sr,
                        DspFilter filter)
{
    static int16_t buf[BUFSIZE];
    uint64_t written = 0;

    const uint64_t rampSamples = (uint64_t)(RAMP_MS/1000.0 * sr);
    uint64_t ramp = (rampSamples*2 > samples) ? samples/2 : rampSamples;
    if(samples < 2) ramp = 0; /* too short for envelope */

    double prev1 = 0.0, prev2 = 0.0;

    uint64_t remaining = samples;
    while (remaining) {
        size_t chunk = (remaining > BUFSIZE) ? BUFSIZE : (size_t)remaining;
        for (size_t i = 0; i < chunk; ++i) {
            uint64_t globalIdx = written + i;
            double env = 1.0;
            if (globalIdx < ramp)
                env = 0.5 * (1 - cos(M_PI * globalIdx / (double)ramp));
            else if (globalIdx >= samples - ramp)
                env = 0.5 * (1 - cos(M_PI * (samples - globalIdx - 1) / (double)ramp));

            double raw = sin(*phase) * vol * env;
            *phase += phase_inc;
            if (*phase >= 2*M_PI) *phase -= 2*M_PI;

            double out;
            if (filter == DSP_FILTER_HANN3) {
                out = 0.25*prev2 + 0.5*prev1 + 0.25*raw;
            } else {
                out = raw;
            }
            prev2 = prev1;
            prev1 = raw;

            buf[i] = (int16_t)(out * (double)INT16_MAX);
        }
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
        fwrite(buf, sizeof(int16_t), chunk, fp);
#else
        for (size_t i = 0; i < chunk; ++i) {
            le16(fp, (uint16_t)buf[i]);
        }
#endif
        remaining -= chunk;
        written   += chunk;
    }
    return samples;
}
