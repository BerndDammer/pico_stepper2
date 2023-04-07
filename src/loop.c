#include "loop.h"
#include "blinker.h"
#include "motor.h"

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/timer.h"
#include "hardware/watchdog.h"
#include "hardware/clocks.h"

#define CONSOLE_TIMEOUT (1000*1000)
//#define STEP_MAX 3000
//#define STEP_STEP 300
//#define STEP_START 300
#define DSTEP 1.2

void menu(void)
{
    printf("------------------------------------\n");
    printf("Microstepping\n");
    printf("+ Increase Frequency\n");
    printf("- Decrease Frequency\n");
    printf("r Reverse direction\n");
    printf("ESC  exit\n");
    printf("press key to select\n");
    printf("------------------------------------\n");
}

/*
 void loop_it(void)
 {
 int c;
 int step = 1;
 printf("Stepping\n");
 for (;;)
 {
 c = getchar_timeout_us(50 * 1000);
 blinker_toggle();

 if (c != PICO_ERROR_TIMEOUT)
 {
 return;
 }
 else
 {
 step++;
 if (step > 4)
 step = 1;
 motor_set_single(step);
 }
 }
 printf("Stepping stopped\n");
 }
 */

void loop(void)
{
    volatile int c; // make visible in debugger; avoid optimize out
    int counter = 0;
    double f = 1.0;

    motor_init();
    menu();
    motor_set_frequency(f);

    for (;;)
    {
        c = getchar_timeout_us(CONSOLE_TIMEOUT);
        blinker_toggle();

        if (c == PICO_ERROR_TIMEOUT)
        {
            printf("Microstepping Loop Counter %i\n", counter);
            counter++;
        }
        else
        {
            switch (c)
            {
            case '+':
                f *= DSTEP;
                motor_set_frequency(f);
                printf("New frequency %f\n", f);
                break;
            case '-':
                f /= DSTEP;
                motor_set_frequency(f);
                printf("New frequency %f\n", f);
                break;
            case 'r':
                f = -f;
                motor_set_frequency(f);
                printf("New frequency %f\n", f);
                break;
            case 27:
                motor_deinit();
                return;
                break;
            default:
                menu();
                break;
            }
        }
    }
}
