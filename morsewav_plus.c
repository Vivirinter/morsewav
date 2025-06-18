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
#include <sys/wait.h>   /* waitpid */
#include <signal.h>
#include <fcntl.h>
#include "lib/wav.h"
#include "lib/morse.h"
#include "lib/dsp.h"
#include "lib/cli.h"
#include "lib/progress.h"
#include "lib/common.h"
#include "version.h"

static void play_file(const char *path){
#if defined(_WIN32)
    /* Windows: fallback to PowerShell.  Still vulnerable to paths with quotes but Win32 API is verbose */
    char cmd[PATH_MAX+128];
    snprintf(cmd, sizeof(cmd),
             "powershell -c \"(New-Object Media.SoundPlayer '%s').PlaySync()\"",
             path);
    system(cmd);
#elif defined(__APPLE__)
    pid_t pid = fork();
    if(pid == 0){
        /* Redirect stdout/stderr to /dev/null to suppress aplay/afplay errors on SIGINT */
        int fd = open("/dev/null", O_RDWR);
        if(fd >= 0){ dup2(fd, STDOUT_FILENO); dup2(fd, STDERR_FILENO); if(fd>2) close(fd);}    
        execlp("afplay", "afplay", path, (char*)NULL);
        _exit(127);
    }
    if(pid > 0) {
        /* Parent ignores SIGINT during playback so Ctrl+C stops player only */
        struct sigaction sa_old, sa_ign = {0};
        sa_ign.sa_handler = SIG_IGN;
        sigaction(SIGINT, &sa_ign, &sa_old);
        int status; (void)waitpid(pid, &status, 0);
        sigaction(SIGINT, &sa_old, NULL);
    }
#else
    pid_t pid = fork();
    if(pid == 0){
        int fd = open("/dev/null", O_RDWR);
        if(fd >= 0){ dup2(fd, STDOUT_FILENO); dup2(fd, STDERR_FILENO); if(fd>2) close(fd);}    
        execlp("aplay", "aplay", "--", path, (char*)NULL);
        _exit(127);
    }
    if(pid > 0) {
        struct sigaction sa_old, sa_ign = {0};
        sa_ign.sa_handler = SIG_IGN;
        sigaction(SIGINT, &sa_ign, &sa_old);
        int status; (void)waitpid(pid, &status, 0);
        sigaction(SIGINT, &sa_old, NULL);
    }
#endif
}

#ifdef __GNUC__
#   define NODISCARD  __attribute__((warn_unused_result))
#   define PURE_ATTR  __attribute__((pure))
#   define PACKED_ATTR __attribute__((packed))
#elif defined(__has_c_attribute) && __has_c_attribute(nodiscard)
#   define NODISCARD [[nodiscard]]
#   define PURE_ATTR
#   define PACKED_ATTR
#else
#   define NODISCARD
#   define PURE_ATTR
#   define PACKED_ATTR
#endif

#ifndef restrict
#   define restrict __restrict
#endif

#include <assert.h>
static_assert(sizeof(uint16_t)==2, "uint16_t must be 16-bit");
static_assert(sizeof(int16_t)==2,  "int16_t must be 16-bit");


static char *read_text(const Params *pr) {
    if(pr->text) return strdup(pr->text);

    FILE *in;
    if(strcmp(pr->infile, "-") == 0) in = stdin; else {
        in = fopen(pr->infile, "rb");
        if(!in){ perror("fopen"); return NULL; }
    }

    if(fseeko(in, 0, SEEK_END) != 0){ perror("fseeko"); if(in!=stdin) fclose(in); return NULL; }
    off_t sz = ftello(in);
    if(sz < 0){ perror("ftello"); if(in!=stdin) fclose(in); return NULL; }
    rewind(in);

    char *buf = (char*)malloc((size_t)sz + 1);
    if(!buf){ perror("malloc"); if(in!=stdin) fclose(in); return NULL; }

    size_t got = fread(buf, 1, (size_t)sz, in);
    if(got != (size_t)sz){
        perror("fread");
        free(buf);
        if(in!=stdin) fclose(in);
        return NULL;
    }

    if(in != stdin) fclose(in);
    buf[sz] = '\0';
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
                uint64_t tone_len = (code[j]=='.') ? dot_samp : dot_samp*MORSE_DASH_UNITS;
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
        if(total > 0xFFFFFFFFu){
            if(!p.quiet)
                fprintf(stderr, "[INFO] Using RF64 header for large file (>4 GiB)\n");
            wav_write_header64(fp, p.sr, total);
        } else {
            wav_write_header(fp, p.sr, (uint32_t)total);
        }
        fclose(fp);
    }
    free(text);
    if(!p.raw) printf("Wrote %s (%.2f s)\n", p.outfile, total / (double)p.sr);
    if(p.play && !p.raw){
        play_file(p.outfile);
        if(!p.keep){
            if(remove(p.outfile) != 0 && !p.quiet){
                perror("remove");
            }
        }
    }
    return EXIT_SUCCESS;
}
