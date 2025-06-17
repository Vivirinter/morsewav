#include <ctype.h>
#include <stddef.h>
#include "morse.h"

typedef struct { char ch; const char *code; } Morse;
static const Morse MORSE_TAB[] = {
    { 'A', ".-" },   { 'B', "-..." }, { 'C', "-.-." }, { 'D', "-.." },  { 'E', "." },
    { 'F', "..-." }, { 'G', "--." },  { 'H', "...." }, { 'I', ".." },   { 'J', ".---" },
    { 'K', "-.-" },  { 'L', ".-.." }, { 'M', "--" },   { 'N', "-." },   { 'O', "---" },
    { 'P', ".--." }, { 'Q', "--.-" }, { 'R', ".-." },  { 'S', "..." },  { 'T', "-" },
    { 'U', "..-" }, { 'V', "...-" }, { 'W', ".--" },  { 'X', "-..-" }, { 'Y', "-.--" },
    { 'Z', "--.." },
    /* Digits */
    { '0', "-----"}, { '1', ".----"}, { '2', "..---"}, { '3', "...--"},
    { '4', "....-"}, { '5', "....."}, { '6', "-...."}, { '7', "--..."},
    { '8', "---.."}, { '9', "----."},
    /* Punctuation */
    { '.', ".-.-.-" }, { ',', "--..--" }, { '?', "..--.." }, { '/', "-..-." },
    { '-', "-....-" }, { '(', "-.--." },  { ')', "-.--.-" }, { '\'', ".----." },
    { '"', ".-..-." }, { ':', "---..." }, { ';', "-.-.-." }, { '=', "-...-" },
    { '+', ".-.-." },  { '@', ".--.-." }
};
static const size_t MORSE_COUNT = sizeof(MORSE_TAB)/sizeof(MORSE_TAB[0]);

const char *morse_lookup(char c)
{
    c = (char)toupper((unsigned char)c);
    for(size_t i = 0; i < MORSE_COUNT; ++i)
        if(MORSE_TAB[i].ch == c)
            return MORSE_TAB[i].code;
    return NULL;
}
