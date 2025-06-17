#include <ctype.h>
#include <errno.h>
#include <getopt.h>
#include <math.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include "lib/wav.h"
#include "lib/morse.h"
#include "lib/dsp.h"
#include "lib/cli.h"
#include "lib/progress.h"
#include "lib/common.h"
#include "version.h"

static void play_file(const char *path){
#if defined(__APPLE__)
    char cmd[PATH_MAX+64];
    snprintf(cmd,sizeof(cmd),"afplay '%s' >/dev/null 2>&1", path);
#elif defined(_WIN32)
    /* Use PowerShell to play sound invisibly */
    char cmd[PATH_MAX+128];
    snprintf(cmd,sizeof(cmd),"powershell -c \"(New-Object Media.SoundPlayer '%s').PlaySync()\"", path);
#else /* Linux & other POSIX */
    char cmd[PATH_MAX+64];
    snprintf(cmd,sizeof(cmd),"aplay '%s' >/dev/null 2>&1", path);
#endif

    system(cmd);
}

#if defined(__has_c_attribute) && defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 202311L) && __has_c_attribute(nodiscard)
#  define NODISCARD [[nodiscard]]
#elif defined(__GNUC__) || defined(__clang__)
#  define NODISCARD __attribute__((warn_unused_result))
#else
#  define NODISCARD /* no-op */


#if defined(__GNUC__) || defined(__clang__)
#  define PURE_ATTR  __attribute__((pure))
#  define PACKED_ATTR __attribute__((packed))
#else
#  define PURE_ATTR
#  define PACKED_ATTR

#ifndef restrict
#  define restrict __restrict

#endif
#endif
#endif

#include <assert.h>
static_assert(sizeof(uint16_t)==2, "uint16_t must be 16-bit");
static_assert(sizeof(int16_t)==2,  "int16_t must be 16-bit");


static char *read_text(const Params *pr) {
    if(pr->text) return strdup(pr->text);

    FILE *in;
    if(strcmp(pr->infile,"-")==0) in=stdin; else {
        in=fopen(pr->infile,"rb"); if(!in){perror("fopen"); return NULL;}
    }
    fseek(in,0,SEEK_END); long sz=ftell(in); if(sz<0){perror("ftell");return NULL;}
    rewind(in);
    char *buf=(char*)malloc((size_t)sz+1);
    if(!buf){perror("malloc");return NULL;}
    fread(buf,1,(size_t)sz,in);
    if(in!=stdin) fclose(in);
    buf[sz]='\0';
    return buf;
}

int main(int argc,char **argv){
        Params p; if(!cli_parse(argc,argv,&p)){ cli_usage(argv[0]); return EXIT_FAILURE; }
    if(p.version){
        printf("morsewav %s (%s)\n", MORSEWAV_VERSION, MORSEWAV_BUILD_TYPE);
        return EXIT_SUCCESS;
    }

    char *text=read_text(&p); if(!text) return EXIT_FAILURE;

    const double dot_sec = (p.dot_ms>0)? p.dot_ms/1000.0 : 1.2/p.wpm;
    const uint64_t dot_samp=(uint64_t)(dot_sec*p.sr+0.5);
    const double phase_inc = 2*M_PI*p.freq/p.sr;

    FILE *fp;
    if(p.raw) fp=stdout; else {
        fp=fopen(p.outfile,"wb"); if(!fp){perror("fopen"); free(text); return EXIT_FAILURE;}
        wav_reserve_header(fp);
    }

    uint64_t total=0; 
    double phase=0.0;

    size_t txtLen = strlen(text);
    Progress pb; progress_init(&pb, txtLen, p.quiet || p.raw);

    for(size_t idx=0; idx<txtLen; ++idx){
        char ch = text[idx];
        if(ch==' '){
            total += dsp_write_silence(fp, (uint64_t)(dot_samp*MORSE_GAP_WORD_UNITS*p.farns));
        } else {
            const char *code = morse_lookup(ch);
            if(!code) continue;
            for(size_t j=0; code[j]; ++j){
                uint64_t tone_len = (code[j]=='.') ? dot_samp : dot_samp*MORSE_DAH_UNITS;
                total += dsp_write_tone(fp, tone_len, &phase, phase_inc, p.vol, p.sr, p.filter);
                total += dsp_write_silence(fp, dot_samp);
            }
            total += dsp_write_silence(fp, (uint64_t)(dot_samp*(MORSE_GAP_CHAR_UNITS - MORSE_GAP_SYM_UNITS)*p.farns));
        }
        if(pb.enabled){
            progress_update(&pb, idx+1);
        }
    }

    progress_finish(&pb);

    if(!p.raw){
        if(!p.quiet && total>0xFFFFFFFFu){
            fprintf(stderr,"[WARN] File too big for standard WAV (>4GB)\n");
        }
        wav_write_header(fp,p.sr,(uint32_t)(total & 0xFFFFFFFFu));
        fclose(fp);
    }
    free(text);
    if(!p.raw) printf("Wrote %s (%.2f s)\n",p.outfile,total/(double)p.sr);
    if(p.play && !p.raw){
        play_file(p.outfile);
    }
    return EXIT_SUCCESS;
}
