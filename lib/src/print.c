#include <libio/log.h>

#include "cuckoo.h"
#include "print.h"

void print_filter(fingerprint_t *filter)
{
    unsigned i;
    BLOCK_PRINTF_BEGIN();
    for (i = 0; i < NUM_BUCKETS; ++i) {
        BLOCK_PRINTF("%04x ", filter[i]);
        if (i > 0 && (i + 1) % 8 == 0)
            BLOCK_PRINTF("\r\n");
    }
    BLOCK_PRINTF_END();
}

void log_filter(fingerprint_t *filter)
{
    unsigned i;
    BLOCK_LOG_BEGIN();
    for (i = 0; i < NUM_BUCKETS; ++i) {
        BLOCK_LOG("%04x ", filter[i]);
        if (i > 0 && (i + 1) % 8 == 0)
            BLOCK_LOG("\r\n");
    }
    BLOCK_LOG_END();
}

// TODO: to avoid having to wrap every thing printf macro (to prevent
// a mementos checkpoint in the middle of it, which could be in the
// middle of an EDB energy guard), make printf functions in libio
// and exclude libio from Mementos passes
void print_stats(unsigned inserts, unsigned members, unsigned total)
{
    PRINTF("stats: inserts %u members %u total %u\r\n",
           inserts, members, total);
}
