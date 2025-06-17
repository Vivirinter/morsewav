#ifndef MORSE_H
#define MORSE_H

#define MORSE_DIT_UNITS   1u
#define MORSE_DAH_UNITS   3u
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
