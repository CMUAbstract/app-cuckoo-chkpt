#include <msp430.h>

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

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

#include "pins.h"

// #define SHOW_PROGRESS_ON_LED

/* This is for progress reporting only */
#define SET_CURTASK(t) curtask = t

static __nv unsigned curtask;

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
#ifndef MEMENTOS
    init();
#endif

    while (1) {
        PRINTF("hello world\r\n");

        volatile uint32_t delay = 0x8ffff;
        while (delay--);
    }

    return 0;
}
