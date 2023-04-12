#include <stdbool.h>

#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"

#include "motor_single.h"


#define SLICE1 0
#define SLICE2 7

#define WRAP 255
#define FPWM 10000
#define OFF_CHANNEL 0
#define ON_CHANNEL 255

#define FLAG 18

typedef struct
{
    uint slice;
    int moth;
    int motl;
    pwm_config pc;

} s_cs_mot;

static s_cs_mot cs_mot1 =
{ SLICE1, COIL1, COIL2,
{

} };

static s_cs_mot cs_mot2 =
{ SLICE2, COIL3, COIL4,
{

} };



void motor_single_init2(s_cs_mot *s)
{
    // Tell GPIO they are allocated to the PWM
    gpio_set_function(s->moth, GPIO_FUNC_PWM);
    gpio_set_function(s->motl, GPIO_FUNC_PWM);

    s->pc = pwm_get_default_config();
    pwm_config_set_clkdiv_mode(&s->pc, PWM_DIV_FREE_RUNNING);

    double div;
    {
        div = (double) clock_get_hz(clk_sys) / (double) FPWM / (double) WRAP;
    }
    pwm_config_set_clkdiv(&s->pc, div);

    s->pc.top = WRAP - 1;

    pwm_init(s->slice, &s->pc, false);

    pwm_set_chan_level(s->slice, PWM_CHAN_A, OFF_CHANNEL);
    pwm_set_chan_level(s->slice, PWM_CHAN_B, OFF_CHANNEL);

    pwm_set_enabled(s->slice, true);
}

void motor_single_init(void)
{
    motor_single_init2(&cs_mot1);
    motor_single_init2(&cs_mot2);
}

void motor_single_deinit(void)
{
    pwm_set_enabled(cs_mot1.slice, false);
    pwm_set_enabled(cs_mot2.slice, false);
}


void motor_single_set(int coil_number, int pwm_val)
{
    switch (coil_number)
    {
    case 1:
        pwm_set_chan_level(cs_mot1.slice, PWM_CHAN_A, pwm_val);
        break;
    case 2:
        pwm_set_chan_level(cs_mot1.slice, PWM_CHAN_B, pwm_val);
        break;
    case 3:
        pwm_set_chan_level(cs_mot2.slice, PWM_CHAN_A, pwm_val);
        break;
    case 4:
        pwm_set_chan_level(cs_mot2.slice, PWM_CHAN_B, pwm_val);
        break;
    default:
        pwm_set_chan_level(cs_mot1.slice, PWM_CHAN_A, OFF_CHANNEL);
        pwm_set_chan_level(cs_mot1.slice, PWM_CHAN_B, OFF_CHANNEL);
        pwm_set_chan_level(cs_mot2.slice, PWM_CHAN_A, OFF_CHANNEL);
        pwm_set_chan_level(cs_mot2.slice, PWM_CHAN_B, OFF_CHANNEL);
        break;
    }
}
