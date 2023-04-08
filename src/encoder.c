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
#include "hardware/irq.h"

#include "hardware/watchdog.h"
#include "hardware/clocks.h"

#include "encoder.h"

#define PHASE_A 2
#define PHASE_B 3

#define FLAG 18

int full_count = 0;
static int last_read = 0;
int fail_count = 0;

#define TIMER_NUM 2
#define TIMER_IRQ TIMER_IRQ_2
#define SCAN_DELAY_US 20

static int scan_time;
volatile static long long counter = 0;

static void scan_encoder_lines(void)
{
    gpio_put(FLAG, true);
    counter++;

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

    hw_clear_bits(&timer_hw->intr, 1u << TIMER_NUM);

    // stops after some ints ....
    //scan_time = scan_time + SCAN_DELAY_US;
    //timer_hw->alarm[TIMER_NUM] = scan_time;

    // sometimes a bit later
    timer_hw->alarm[TIMER_NUM] = timer_hw->timerawl + SCAN_DELAY_US;

    gpio_put(FLAG, false);
}

//void encoder_irq_init(void)
//{
//    add_repeating_timer_us( 20, check, NULL, &encoder_timer);
//}

void encoder_irq_init(void)
{
    hardware_alarm_claim(TIMER_NUM);

    irq_set_exclusive_handler(TIMER_IRQ, scan_encoder_lines);
    irq_set_enabled(TIMER_IRQ, true);
    uint64_t target = timer_hw->timerawl + SCAN_DELAY_US;

    scan_time = (uint32_t) target;
    timer_hw->alarm[TIMER_NUM] = scan_time;
    hw_set_bits(&timer_hw->inte, 1u << TIMER_NUM);
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

int get_full_counter(void)
{
    return full_count;
}
