#ifndef PROGRESS_LIB_H
#define PROGRESS_LIB_H

#include <stdbool.h>
#include <stddef.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef struct {
    size_t   total;
    size_t   current;
    struct timespec start_ts;
    bool     enabled;
} Progress;

void progress_init(Progress *pb, size_t total, bool quiet);

void progress_update(Progress *pb, size_t pos);

void progress_finish(Progress *pb);

#ifdef __cplusplus
}
#endif

#endif
