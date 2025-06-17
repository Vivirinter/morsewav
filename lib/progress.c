#define _POSIX_C_SOURCE 200809L
#include "progress.h"
#include <stdio.h>
#include <time.h>
#include <unistd.h>

static double secs_since(const struct timespec *start)
{
    struct timespec now;
#if defined(_POSIX_TIMERS) && _POSIX_TIMERS>0
    clock_gettime(CLOCK_MONOTONIC, &now);
#else
    timespec_get(&now, TIME_UTC);
#endif
    return (now.tv_sec - start->tv_sec) + (now.tv_nsec - start->tv_nsec)/1e9;
}

void progress_init(Progress *pb, size_t total, bool quiet)
{
    pb->total   = total;
    pb->current = 0;
    pb->enabled = (!quiet && isatty(STDERR_FILENO));
    clock_gettime(CLOCK_MONOTONIC, &pb->start_ts);
    if(pb->enabled){
        fprintf(stderr, "[0/%zu]", total);
        fflush(stderr);
    }
}

void progress_update(Progress *pb, size_t pos)
{
    if(!pb->enabled) return;
    if(pos == pb->current) return; /* no change */
    pb->current = pos;

    double elapsed = secs_since(&pb->start_ts);
    double eta = 0.0;
    if(pb->total && pos){
        eta = elapsed * ((double)pb->total/pos - 1.0);
    }
    int percent = (pb->total) ? (int)(100.0*pos/pb->total + 0.5) : 0;

    if(eta < 1.0){
        int ms = (int)(eta*1000 + 0.5);
        fprintf(stderr, "\r\033[92m%3d%%\033[0m [%zu/%zu] ETA %3d ms", percent, pos, pb->total, ms);
    } else {
        int mins = (int)(eta/60);
        int secs = (int)eta % 60;
        fprintf(stderr, "\r\033[92m%3d%%\033[0m [%zu/%zu] ETA %02d:%02d", percent, pos, pb->total, mins, secs);
    }
    fflush(stderr);
}

void progress_finish(Progress *pb)
{
    if(!pb->enabled) return;
    double elapsed = secs_since(&pb->start_ts);
    fprintf(stderr, "\r\033[92m100%%\033[0m [%zu/%zu] Done", pb->total, pb->total);
    if(elapsed < 1.0){
        int ms = (int)(elapsed*1000 + 0.5);
        fprintf(stderr, " | Elapsed %d ms\n", ms);
    } else {
        int hrs  = (int)(elapsed / 3600);
        int mins = (int)((elapsed / 60)) % 60;
        int secs = (int)elapsed % 60;
        fprintf(stderr, " | Elapsed %02d:%02d:%02d\n", hrs, mins, secs);
    }
}
