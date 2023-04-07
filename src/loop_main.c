#include "loop_main.h"
#include "loop.h"
#include "loop_bitbang.h"
#include "loop_single.h"
#include "loop_encoder.h"
#include "blinker.h"

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/timer.h"
#include "hardware/watchdog.h"
#include "hardware/clocks.h"

#define CONSOLE_TIMEOUT 1000000

void menu_main(void)
{
    printf("------------------------------------\n");
    printf("b bitbang\n");
    printf("m microstepping\n");
    printf("s single\n");
    printf("e encoder\n");
    printf("press key to select\n");
    printf("------------------------------------\n");
}


void loop_main(void)
{
    volatile int c; // make visible in debugger; avoid optimize out
    int counter = 0;

    menu_main();

    for (;;)
    {
        c = getchar_timeout_us(CONSOLE_TIMEOUT);
        blinker_toggle();

        if (c == PICO_ERROR_TIMEOUT)
        {
            printf("Main Loop Counter %i\n", counter);
            counter++;
        }
        else
        {
            switch (c)
            {
            case 'b':
                loop_bitbang();
                break;
            case 'm':
                loop();
                break;
            case 's':
                loop_single();
                break;
            case 'e':
                loop_encoder();
                break;
            default:
                menu_main();
                break;
            }
        }
    }
}
