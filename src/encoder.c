/*
 * encoder.c
 *
 *  Created on: 06.04.2023
 *      Author: manni4
 */
#include <stdio.h>
#include <stdbool.h>

#include "pico/stdlib.h"
#include "hardware/timer.h"
#include "hardware/watchdog.h"
#include "hardware/clocks.h"

#include "encoder.h"

#define PHASE_A 2
#define PHASE_B 3

#define FLAG 18

int full_count = 0;
static int last_read = 0;
int fail_count = 0;

static repeating_timer_t encoder_timer;

static bool check(repeating_timer_t *rt)
{
    gpio_put(FLAG, true);

    int act = (gpio_get_all() & 0X000C) >> 2;
    switch (act << 2 | last_read)
    {
        case 0:
        case 5:
        case 10:
        case 15:
            // no change;
        break;
        case 4:
        case 13:
        case 11:
        case 2:
            full_count++;
        break;
        case 12:
        case 9:
        case 3:
        case 6:
            fail_count++;
        break;
        case 1:
        case 7:
        case 8:
        case 14:
            full_count--;
        break;

    }
    last_read = act;
    gpio_put(FLAG, false);
    return true;
}

void encoder_irq_init(void)
{
    add_repeating_timer_us( 20, check, NULL, &encoder_timer);
}

void encoder_init(void)
{
    gpio_init(PHASE_A);
    gpio_set_function(PHASE_A, GPIO_FUNC_SIO);
    gpio_set_dir(PHASE_A, GPIO_IN);

    gpio_init(PHASE_B);
    gpio_set_function(PHASE_B, GPIO_FUNC_SIO);
    gpio_set_dir(PHASE_B, GPIO_IN);

    // last two bits of counter must reflect phase states
    int act = (gpio_get_all() & 0X000C) >> 2;
    switch (act)
    {
        case 0:
            full_count = 0;
            last_read = 0;
        break;
        case 1:
            full_count = 1;
            last_read = 1;
        break;
        case 2:
            full_count = 3;
            last_read = 2;
        break;
        case 3:
            full_count = 2;
            last_read = 3;
        break;
    }

    gpio_init(FLAG);
    gpio_set_function(FLAG, GPIO_FUNC_SIO);
    gpio_set_dir(FLAG, true);
    gpio_put(FLAG, false);

    encoder_irq_init();
}

