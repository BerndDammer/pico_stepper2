#include "loop_single.h"
#include "blinker.h"
#include "motor_single.h"
#include "encoder.h"

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/timer.h"
#include "hardware/clocks.h"

#define CONSOLE_TIMEOUT 1000000
#define STEP_MAX 255
#define STEP_STEP 16
#define STEP_START 20

void menu_single(void)
{
    printf("------------------------------------\n");
    printf("1 2 3 4 select coil\n");
    printf("+ pwm up\n");
    printf("- pwm down\n");
    printf("0 all coils off\n");
    printf("ESC exit\n");
    printf("press key to select\n");
    printf("------------------------------------\n");
}

void echo_single(int *coil, int *pwm_val)
{
    if (*pwm_val < 0)
        *pwm_val = 0;
    if (*pwm_val > STEP_MAX)
        *pwm_val = STEP_MAX;
    motor_single_set(*coil, *pwm_val);
    printf("Set Coil %i to pwm value %i\n", *coil, *pwm_val);
}
void loop_single(void)
{
    //////////////////////////////////
    /// /////////////////////////////////////
    volatile int c; // make visible in debugger; avoid optimize out
    int counter = 0;
    int pwm_val = STEP_START;
    int coil = 0;

    gpio_pull_up(18);


    motor_single_init();
    echo_single(&coil, &pwm_val);

    menu_single();

    for (;;)
    {
        c = getchar_timeout_us(CONSOLE_TIMEOUT);
        blinker_toggle();

        if (c == PICO_ERROR_TIMEOUT)
        {
            //printf("Single Counter %i\n", counter);
            counter++;

            int count = get_full_counter();
            int phase = count % 80;
            int step = (phase / 10) & 7;
            printf("counter %i phase %i step %i\n", count, phase, step);
        }
        else
        {
            switch (c)
            {
            case '1':
                coil = 1;
                break;
            case '2':
                coil = 2;
                break;
            case '3':
                coil = 3;
                break;
            case '4':
                coil = 4;
                break;
            case '+':
                pwm_val += STEP_STEP;
                break;
            case '-':
                pwm_val -= STEP_STEP;
                break;
            case ' ':
            case '0':
                coil = 0;
                break;
            case 27:
                motor_single_deinit();
                return;
                break;
            default:
                menu_single();
                break;
            }
            echo_single(&coil, &pwm_val);
        }
    }
}
