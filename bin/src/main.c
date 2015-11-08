#include <msp430.h>

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <libmsp/mem.h>
#include <libio/log.h>
#include <msp-math.h>
#include <msp-builtins.h>

#ifdef CONFIG_EDB
#include <libedb/edb.h>
#else
#define ENERGY_GUARD_BEGIN()
#define ENERGY_GUARD_END()
#endif

#ifdef DINO
#include <libmsp/mem.h>
#include <dino.h>
#endif

#include "libcuckoo/cuckoo.h"
#include "libcuckoo/print.h"

#include "pins.h"

// #define SHOW_PROGRESS_ON_LED

#define NUM_KEYS 10
#define INIT_KEY 0x1

/* This is for progress reporting only */
#define SET_CURTASK(t) curtask = t

#define TASK_GENERATE_KEY 1
#define TASK_INSERT 2
#define TASK_LOOKUP 3

static __nv unsigned curtask;

static hash_t djb_hash(uint8_t* data, unsigned len)
{
   uint32_t hash = 5381;
   unsigned int i;

   for(i = 0; i < len; data++, i++)
      hash = ((hash << 5) + hash) + (*data);

   return hash & 0xFFFF;
}

static index_t hash_fp_to_index(fingerprint_t fp)
{
    hash_t hash = djb_hash((uint8_t *)&fp, sizeof(fingerprint_t));
    return hash & (NUM_BUCKETS - 1); // NUM_BUCKETS must be power of 2
}

static index_t hash_key_to_index(value_t fp)
{
    hash_t hash = djb_hash((uint8_t *)&fp, sizeof(value_t));
    return hash & (NUM_BUCKETS - 1); // NUM_BUCKETS must be power of 2
}

static fingerprint_t hash_to_fingerprint(value_t key)
{
    return djb_hash((uint8_t *)&key, sizeof(value_t));
}

static value_t generate_key(value_t prev_key)
{
    SET_CURTASK(TASK_GENERATE_KEY);

    // insert pseufo-random integers, for testing
    // If we use consecutive ints, they hash to consecutive DJB hashes...
    // NOTE: we are not using rand(), to have the sequence available to verify
    // that that are no false negatives (and avoid having to save the values).
    return (prev_key + 1) * 17;
}

static bool insert(fingerprint_t *filter, value_t key)
{
    fingerprint_t fp1, fp2, fp_victim, fp_next_victim;
    index_t index_victim, fp_hash_victim;
    unsigned relocation_count = 0;

    SET_CURTASK(TASK_INSERT);

    fingerprint_t fp = hash_to_fingerprint(key);
    index_t index1 = hash_key_to_index(key);
    index_t fp_hash = hash_fp_to_index(fp);
    index_t index2 = index1 ^ fp_hash;

    LOG("insert: key %04x fp %04x h %04x idx1 %u idx2 %u\r\n",
        key, fp, fp_hash, index1, index2);

    fp1 = filter[index1];
    LOG("insert: fp1 %04x\r\n", fp1);
    if (!fp1) { // slot 1 is free
        filter[index1] = fp;
    } else {
        fp2 = filter[index2];
        LOG("insert: fp2 %04x\r\n", fp2);
        if (!fp2) { // slot 2 is free
            filter[index2] = fp;
        } else { // both slots occupied, evict
            if (rand() % 2) {
                index_victim = index1;
                fp_victim = fp1;
            } else {
                index_victim = index2;
                fp_victim = fp2;
            }

            LOG("insert: evict [%u] = %04x\r\n", index_victim, fp_victim);
            filter[index_victim] = fp; // evict victim

            do { // relocate victim(s)
                fp_hash_victim = hash_fp_to_index(fp_victim);
                index_victim = index_victim ^ fp_hash_victim;
                fp_next_victim = filter[index_victim];
                filter[index_victim] = fp_victim;

                LOG("insert: moved %04x to %u; next victim %04x\r\n",
                    fp_victim, index_victim, fp_next_victim);

                fp_victim = fp_next_victim;
            } while (fp_victim && ++relocation_count < MAX_RELOCATIONS);

            if (fp_victim) {
                LOG("insert: max relocations (%u) exceeded\r\n", MAX_RELOCATIONS);
                return false;
            }
        }
    }

    return true;
}

static bool lookup(fingerprint_t *filter, value_t key)
{
    SET_CURTASK(TASK_LOOKUP);

    fingerprint_t fp = hash_to_fingerprint(key);
    index_t index1 = hash_key_to_index(key);
    index_t fp_hash = hash_fp_to_index(fp);
    index_t index2 = index1 ^ fp_hash;

    LOG("lookup: key %04x fp %04x h %04x idx1 %u idx2 %u\r\n",
        key, fp, fp_hash, index1, index2);

    return filter[index1] == fp || filter[index2] == fp;
}

void init()
{
    WISP_init();

#ifdef CONFIG_EDB
    debug_setup();
#endif

    INIT_CONSOLE();

    __enable_interrupt();

    GPIO(PORT_LED_1, DIR) |= BIT(PIN_LED_1);
    GPIO(PORT_LED_2, DIR) |= BIT(PIN_LED_2);
#if defined(PORT_LED_3)
    GPIO(PORT_LED_3, DIR) |= BIT(PIN_LED_3);
#endif

#if defined(PORT_LED_3) // when available, this LED indicates power-on
    GPIO(PORT_LED_3, OUT) |= BIT(PIN_LED_3);
#endif

#ifdef SHOW_PROGRESS_ON_LED
    blink(1, SEC_TO_CYCLES * 5, LED1 | LED2);
#endif

    EIF_PRINTF(".%u.\r\n", curtask);
}

int main()
{
    unsigned i;
    value_t key;

    // Mementos can't handle globals: it restores them to .data, when they are
    // in .bss... So, for now, just keep all data on stack.
    fingerprint_t filter[NUM_BUCKETS];

    // can't use C initializer because it gets converted into memset,
    // but the memset linked in by GCC is of the wrong calling convention,
    // but we can't override with our own memset because C runtime
    // calls memset with GCC's calling convention. Catch 22.
    for (i = 0; i < NUM_BUCKETS; ++i)
        filter[i] = 0;

#ifndef MEMENTOS
    init();
#endif

    while (1) {
        key = INIT_KEY;
        unsigned inserts = 0;
        for (i = 0; i < NUM_KEYS; ++i) {
            key = generate_key(key);
            bool success = insert(filter, key);
            LOG("insert: key %04x success %u\r\n", key, success);
            log_filter(filter);

            inserts += success;

#ifdef CONT_POWER
            volatile uint32_t delay = 0x8ffff;
            while (delay--);
#endif
        }
        LOG("inserts/total: %u/%u\r\n", inserts, NUM_KEYS);

        key = INIT_KEY;
        unsigned members = 0;
        for (i = 0; i < NUM_KEYS; ++i) {
            key = generate_key(key);
            bool member = lookup(filter, key);
            LOG("lookup: key %04x member %u\r\n", key, member);
            members += member;
        }
        LOG("members/total: %u/%u\r\n", members, NUM_KEYS);

        print_filter(filter);
        print_stats(inserts, members, NUM_KEYS);

#ifdef CONT_POWER
        volatile uint32_t delay = 0x8ffff;
        while (delay--);
#endif
    }

    return 0;
}
