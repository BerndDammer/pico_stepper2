#include "loop_bitbang.h"
#include "blinker.h"
#include "coils.h"
#include "encoder.h"

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/timer.h"
#include "hardware/watchdog.h"
#include "hardware/clocks.h"

#define TIMEOUT_MIN 100
#define TIMEOUT_MAX 500000
#define DSTEP 1.2

#define STEPS_PER_ELECTRIC_WAVE 8
#define POLE_PAIR_COUNT 50
#define GEAR 1

void menu_bitbang(void)
{
    printf("------------------------------------\n");
    printf("Bitbang\n");
    printf("1 Full stepping\n");
    printf("2 Half Stepping\n");
    printf("4 Quarter Stepping\n");
    printf("5 Gap Stepping\n");
    printf("+ Increase Frequency / Push\n");
    printf("- Decrease Frequency / Push\n");
    printf("r Reverse direction\n");
    printf("c Countdown\n");
    printf("e encoder mode\n");
    printf("ESC  exit\n");
    printf("press key to select\n");
    printf("------------------------------------\n");
}

const int pins[4] =
    {
    COIL_AP,
    COIL_AM,
    COIL_BP,
    COIL_BM, };

void show_frequency(int delay)
{
    double f = 1e6 / (double) delay / (double) 8;
    printf("Electric frequency %8.1f HZ\n", f);
}

// mode step pin
int outs[5][8][4] =
    {
        {  // OFF
            { false, false, false, false },  //
            { false, false, false, false },  //
            { false, false, false, false },  //
            { false, false, false, false },  //
            { false, false, false, false },  //
            { false, false, false, false },  //
            { false, false, false, false },  //
            { false, false, false, false }  //
        },  //
        {  // full stepping
            { true, false, true, false },
            { true, false, true, false },
            { false, true, true, false },
            { false, true, true, false },
            { false, true, false, true },
            { false, true, false, true },
            { true, false, false, true },
            { true, false, false, true }, },
        {  // half step

            { true, false, false, false },  // A+B0
            { true, false, true, false },  // A+B+
            { false, false, true, false },  // A0B+
            { false, true, true, false },  // A-B+
            { false, true, false, false },  // A-B0
            { false, true, false, true },  // A-B-
            { false, false, false, true },  // A0B-
            { true, false, false, true }  // A+B-
        },
        {  // quarter step
            { true, false, false, false },
            { true, false, false, false },
            { false, false, true, false },
            { false, false, true, false },
            { false, true, false, false },
            { false, true, false, false },
            { false, false, false, true },
            { false, false, false, true }  //
        },
        {  // gap step
            { true, false, false, false },
            { false, false, false, false },
            { false, false, true, false },
            { false, false, false, false },
            { false, true, false, false },
            { false, false, false, false },
            { false, false, false, true },
            { false, false, false, false }  //
        }  //
    };

void bitbang_init(void)
{
    for (int i = 0; i < 4; i++)
    {
        int pin = pins[i];

        gpio_init(pin);
        gpio_set_function(pin, GPIO_FUNC_SIO);
        gpio_set_dir(pin, GPIO_OUT);
        gpio_put(pin, false);
    }
}

void bitbang_deinit(void)
{
    for (int i = 0; i < 4; i++)
    {
        int pin = pins[i];

        gpio_init(pin);
        gpio_set_function(pin, GPIO_FUNC_NULL);
        gpio_set_dir(pin, GPIO_IN);
        gpio_put(pin, false);
    }
}

void loop_bitbang(void)
{
    volatile int c;  // make visible in debugger; avoid optimize out
    int menu_counter = 0;
    int delay = TIMEOUT_MAX;
    int forward = true;
    int countdown = 0;
    int mode = 0;
    int step = 0;
    int encoder_mode = false;
    int push = 0;

    menu_bitbang();
    bitbang_init();

    for (;;)
    {
        c = getchar_timeout_us(delay);
        blinker_toggle();

        if (c == PICO_ERROR_TIMEOUT)
        {
            if (encoder_mode)
            {
                int phase = get_full_counter();
                phase += push;
                phase %= 80;
                phase /= 10;
                phase &= 7;

                for (int i = 0; i < 4; i++)
                {
                    volatile int p = pins[i];
                    volatile int v = outs[mode][phase][i];
                    gpio_put(p, v);
                    //gpio_put(pins[i], outs[mode][step][i]);
                }
            }
            else
            {
                menu_counter++;
                for (int i = 0; i < 4; i++)
                {
                    volatile int p = pins[i];
                    volatile int v = outs[mode][step][i];
                    gpio_put(p, v);
                    //gpio_put(pins[i], outs[mode][step][i]);
                }
                step = (forward ? ++step : --step) & 7;
                if (countdown != 0)
                {
                    countdown--;
                    if (countdown == 0)
                    {
                        mode = 0;
                    }
                }
            }
        }
        else
        {
            switch (c)
            {
                case '1':
                    mode = 1;
                break;
                case '2':
                    mode = 2;
                break;
                case '4':
                    mode = 3;
                break;
                case '5':
                    mode = 4;
                break;
                case '+':
                    if (encoder_mode)
                    {
                        printf("New Push value %i\n", ++push);
                    }
                    else
                    {
                        delay = (int) ((double) delay / DSTEP);
                        if (delay < TIMEOUT_MIN)
                            delay = TIMEOUT_MIN;
                        show_frequency(delay);
                    }
                break;
                case '-':
                    if (encoder_mode)
                    {
                        printf("New Push value %i\n", --push);
                    }
                    else
                    {
                        delay = (int) ((double) delay * DSTEP);
                        if (delay > TIMEOUT_MAX)
                            delay = TIMEOUT_MAX;
                        show_frequency(delay);
                    }
                break;
                case 'r':
                    if(encoder_mode)
                    {
                        if( push > 0)
                            push -= 40;
                        else
                            push += 40;
                    }
                    else
                    {
                        forward = !forward;
                    }
                break;
                case 'c':
                    countdown = STEPS_PER_ELECTRIC_WAVE * POLE_PAIR_COUNT * GEAR;
                break;
                case 'e':
                    if (encoder_mode)
                    {
                        delay = TIMEOUT_MAX;
                        step = 0;
                        encoder_mode = false;
                    }
                    else
                    {
                        delay = TIMEOUT_MIN;
                        encoder_mode = true;
                        push = 0;
                    }
                break;
                case ' ':
                case '0':
                    mode = 0;
                break;
                case 27:
                    goto deinit;
                break;
                default:
                    menu_bitbang();
                break;
            }
        }
    }
    deinit: bitbang_deinit();
}
