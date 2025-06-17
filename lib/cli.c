#include "cli.h"
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

bool cli_parse(int argc, char **argv, Params *p)
{
    *p = (Params){
        .text    = NULL,
        .infile  = NULL,
        .outfile = DEF_OUTFILE,
        .freq    = DEF_FREQ,
        .wpm     = DEF_WPM,
        .vol     = DEF_VOL,
        .dot_ms  = 0.0,
        .farns   = 1.0,
        .sr      = DEF_SR,
        .filter  = DSP_FILTER_NONE,
        .raw     = false,
        .play    = false,
        .quiet   = false,
        .version = false
    };

    /* use values >255 for long-only opts */
    enum { OPT_QUIET = 1000, OPT_FILTER, OPT_FARNS, OPT_VERSION };

    static const struct option long_opts[] = {
        {"outfile", required_argument, 0, 'o'},
        {"raw",     no_argument,       0, 'R'},
        {"freq",    required_argument, 0, 'f'},
        {"wpm",     required_argument, 0, 'w'},
        {"rate",    required_argument, 0, 'r'},
        {"vol",     required_argument, 0, 'v'},
        {"input",   required_argument, 0, 'i'},
        {"dot",     required_argument, 0, 'd'},
        {"play",    no_argument,       0, 'P'},
        {"quiet",   no_argument,       0, OPT_QUIET},
        {"filter",  required_argument, 0, OPT_FILTER},
        {"farns",   required_argument, 0, OPT_FARNS},
        {"version", no_argument,       0, OPT_VERSION},
        {"help",    no_argument,       0, 'h'},
        {0,0,0,0}
    };

    int opt;
    while((opt = getopt_long(argc, argv, "o:Rf:w:r:v:i:d:PhV", long_opts, NULL)) != -1){
        switch(opt){
        case 'o':
            p->outfile = optarg;
            if(strcmp(optarg, "-") == 0) p->raw = true;
            break;
        case 'R':
            p->outfile = "-";
            p->raw = true;
            break;
        case 'f': p->freq  = strtod(optarg, NULL); break;
        case 'w': p->wpm   = strtod(optarg, NULL); break;
        case 'r': p->sr    = (unsigned)strtoul(optarg, NULL, 10); break;
        case 'v': p->vol   = strtod(optarg, NULL); break;
        case 'i': p->infile = optarg; break;
        case 'd': p->dot_ms = strtod(optarg, NULL); break;
        case 'P': p->play = true; break;
        case OPT_QUIET: p->quiet = true; break;
        case OPT_FILTER:
            if(strcmp(optarg, "none") == 0)      p->filter = DSP_FILTER_NONE;
            else if(strcmp(optarg, "hann3") == 0) p->filter = DSP_FILTER_HANN3;
            else return false; /* unknown filter */
            break;
        case OPT_FARNS:
            p->farns = strtod(optarg, NULL);
            if(p->farns < 1.0) p->farns = 1.0;
            break;
        case 'V': p->version = true; break;
        case OPT_VERSION: p->version = true; break;
        case 'h':
        default:
            return false;
        }
    }

    if(optind < argc) p->text = argv[optind];

    if(p->filter == DSP_FILTER_NONE && p->sr < 12000)
        p->filter = DSP_FILTER_HANN3;

    /* basic sanity checks */
    if(!p->version && !p->text && !p->infile) return false;
    if(p->freq <= 0 || p->wpm <= 0 || p->sr < 4000 || p->vol <= 0 || p->vol > 1)
        return false;

    return true;
}

void cli_usage(const char *prog)
{
    fprintf(stderr,
        "Usage: %s \"TEXT\" [options]\n"
        "Options:\n"
        "  -o, --outfile <file>   output WAV file (default %s) or '-' for stdout raw\n"
        "  -R                     shorthand for -o - (raw PCM)\n"
        "  -f, --freq <Hz>        tone frequency (default %.0f)\n"
        "  -w, --wpm <N>          speed in words per minute (default %.0f)\n"
        "  -r, --rate <Hz>        sample rate (default %u)\n"
        "  -v, --vol <0..1>       volume (default %.1f)\n"
        "  -i, --input <file>     read text from file or '-' (stdin)\n"
        "  -d, --dot <ms>         dot duration in milliseconds (overrides -w)\n"
        "  -P, --play             play result via system player\n"
        "      --quiet            suppress progress/info output\n"
        "      --filter=<name>    audio filter: none | hann3 (default auto)\n"
        "      --farns=<N>        Farnsworth timing multiplier (>1)\n"
        "  -V, --version           show program version\n"
        "  -h, --help             show this help\n",
        prog, DEF_OUTFILE, DEF_FREQ, DEF_WPM, DEF_SR, DEF_VOL);
}
