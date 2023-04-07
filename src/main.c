#include "timer.h"
#include "loop_main.h"
#include "blinker.h"
#include "motor.h"
#include "encoder.h"

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/timer.h"
#include "hardware/watchdog.h"
#include "hardware/clocks.h"




int main()
{
    stdio_init_all();

    timer_init();
    blinker_init();

    encoder_init();

    loop_main();

    return 0;
}
