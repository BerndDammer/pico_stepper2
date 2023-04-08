/*
 * loop_encoder.c
 *
 *  Created on: 07.04.2023
 *      Author: manni4
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/timer.h"
#include "hardware/watchdog.h"
#include "hardware/clocks.h"

#include "blinker.h"

#define CONSOLE_TIMEOUT 1000000


void menu_encoder(void)
{
    printf("------------------------------------\n");
    printf("Encoder\n");
    printf("ESC  exit\n");
    printf("press key to select\n");
    printf("------------------------------------\n");
}

extern int full_count;
extern int fail_count;

void loop_encoder(void)
{

    volatile int c;  // make visible in debugger; avoid optimize out
    int menu_counter = 0;


    for (;;)
    {
        c = getchar_timeout_us(CONSOLE_TIMEOUT);
        blinker_toggle();

        if (c == PICO_ERROR_TIMEOUT)
        {
            printf("Encoder Counter %i\n", full_count);
            printf("Encoder failures %i\n", fail_count);
            menu_counter++;
        }
        else
        {
            switch (c)
            {
                case ' ':
                case '0':
                break;
                case 27:
                    return;
                break;
                default:
                    menu_encoder();
                break;
            }
        }
    }
}

