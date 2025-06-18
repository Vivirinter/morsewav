#ifndef MORSE_H
#pragma once
#define MORSE_H

#define MORSE_DOT_UNITS   1u
#define MORSE_DASH_UNITS  3u

#define MORSE_DIT_UNITS   MORSE_DOT_UNITS
#define MORSE_DAH_UNITS   MORSE_DASH_UNITS

#define MORSE_GAP_SYM_UNITS 1u
#define MORSE_GAP_CHAR_UNITS 3u
#define MORSE_GAP_WORD_UNITS 7u

#ifdef __cplusplus
extern "C" {
#endif

const char *morse_lookup(char c);

#ifdef __cplusplus
}
#endif

#endif
