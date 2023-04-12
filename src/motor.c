#include <stdbool.h>

#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"

#include "motor.h"

#include "hardware/irq.h"

// Pin 21 Slice 0 Channel A GPIO 16
// Pin 22 Slice 0 Channel B GPIO 17
// Pin 19 Slice 7 Channel A GPIO 14
// Pin 20 Slice 7 Channel B GPIO 15

#define SLICE1 0
#define SLICE2 7

#define WRAP 255
#define FPWM 10000
#define OFF_CHANNEL 0
#define ON_CHANNEL 255

#define FLAG 18

//const unsigned char phasetable[256] =
unsigned char phasetable[256] = {

};

typedef struct
{
    uint slice;
    int moth;
    int motl;
    pwm_config pc;

} s_cs_mot;

static s_cs_mot cs_mot1 =
    { SLICE1, COIL1, COIL2, {

    } };

static s_cs_mot cs_mot2 =
    { SLICE2, COIL3, COIL4, {

    } };

volatile int irq_counter = 0;
volatile int phase_angle = 0;
volatile int phase_omega = 100;

void on_pwm_wrap(void)
{
    gpio_put(FLAG, true);
    irq_counter++;
    pwm_clear_irq(cs_mot1.slice);
    //gpio_xor_mask(1 << FLAG);

    phase_angle += phase_omega;

    // intentionally use of byte wrap around
    volatile unsigned char alpha = phase_angle >> 16;

    volatile unsigned short a, b, c, d;
    a = phasetable[alpha];
    alpha -= 64;
    b = phasetable[alpha];
    alpha -= 64;
    c = phasetable[alpha];
    alpha -= 64;
    d = phasetable[alpha];
    alpha -= 64;

    pwm_set_chan_level(cs_mot1.slice, PWM_CHAN_A, a);
    pwm_set_chan_level(cs_mot2.slice, PWM_CHAN_A, b);
    pwm_set_chan_level(cs_mot1.slice, PWM_CHAN_B, c);
    pwm_set_chan_level(cs_mot2.slice, PWM_CHAN_B, d);

    /*
     pwm_set_chan_level(cs_mot1.slice, PWM_CHAN_A, phasetable[alpha]);
     alpha += 64;
     pwm_set_chan_level(cs_mot1.slice, PWM_CHAN_B, phasetable[alpha]);
     alpha += 64;
     pwm_set_chan_level(cs_mot2.slice, PWM_CHAN_A, phasetable[alpha]);
     alpha += 64;
     pwm_set_chan_level(cs_mot2.slice, PWM_CHAN_B, phasetable[alpha]);
     */
    gpio_put(FLAG, false);
}

void motor_table(void)
{
    int i;
    for (i = 0; i < 64; i++)
    {
//        phasetable[i] = i << 2;
        phasetable[i] = i * 3;
    }
    for (i = 0; i < 64; i++)
    {
//        phasetable[i + 64] = (~i & 63) << 2;
        phasetable[i + 64] = (~i & 63) * 3;
    }
    for (i = 128; i < 256; i++)
    {
        phasetable[i] = 0;
    }
}

void motor_irq_init(void)
{
    motor_table();

    irq_set_exclusive_handler(PWM_IRQ_WRAP, on_pwm_wrap);
    irq_set_enabled(PWM_IRQ_WRAP, true);
    pwm_set_irq_enabled(cs_mot1.slice, true);
    pwm_set_irq_mask_enabled(1, true);  // enable slice 0
    //irq_set_priority(PWM_IRQ_WRAP, PICO_DEFAULT_IRQ_PRIORITY);

    gpio_init(FLAG);
    gpio_set_function(FLAG, GPIO_FUNC_SIO);
    gpio_set_dir(FLAG, true);
}

void motor_init2(s_cs_mot *s)
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

    //s->pc.div = 0XFFFFFF;
    s->pc.top = WRAP - 1;

    pwm_init(s->slice, &s->pc, false);

    pwm_set_chan_level(s->slice, PWM_CHAN_A, OFF_CHANNEL);
    pwm_set_chan_level(s->slice, PWM_CHAN_B, OFF_CHANNEL);

    pwm_set_enabled(s->slice, true);
}

void motor_init(void)
{
    motor_init2(&cs_mot1);
    motor_init2(&cs_mot2);
    motor_irq_init();
}

void motor_set_frequency(double f)
{
    f = f * 16777216.0 / (double) FPWM;
    phase_omega = (int) f;
}

void motor_deinit(void)
{
    irq_set_enabled(PWM_IRQ_WRAP, false);
    pwm_set_irq_enabled(cs_mot1.slice, false);
    irq_remove_handler(PWM_IRQ_WRAP, on_pwm_wrap);

    pwm_set_enabled(cs_mot1.slice, false);
    pwm_set_enabled(cs_mot2.slice, false);

    gpio_init(FLAG);
    gpio_init(cs_mot1.moth);
    gpio_init(cs_mot1.motl);
    gpio_init(cs_mot2.moth);
    gpio_init(cs_mot2.motl);
}

void motor_set_single(int coil_number)
{
    switch (coil_number)
    {
        case 1:
            pwm_set_chan_level(cs_mot1.slice, PWM_CHAN_A, ON_CHANNEL);
            pwm_set_chan_level(cs_mot1.slice, PWM_CHAN_B, OFF_CHANNEL);
            pwm_set_chan_level(cs_mot2.slice, PWM_CHAN_A, OFF_CHANNEL);
            pwm_set_chan_level(cs_mot2.slice, PWM_CHAN_B, OFF_CHANNEL);
        break;
        case 2:
            pwm_set_chan_level(cs_mot1.slice, PWM_CHAN_A, OFF_CHANNEL);
            pwm_set_chan_level(cs_mot1.slice, PWM_CHAN_B, ON_CHANNEL);
            pwm_set_chan_level(cs_mot2.slice, PWM_CHAN_A, OFF_CHANNEL);
            pwm_set_chan_level(cs_mot2.slice, PWM_CHAN_B, OFF_CHANNEL);
        break;
        case 3:
            pwm_set_chan_level(cs_mot1.slice, PWM_CHAN_A, OFF_CHANNEL);
            pwm_set_chan_level(cs_mot1.slice, PWM_CHAN_B, OFF_CHANNEL);
            pwm_set_chan_level(cs_mot2.slice, PWM_CHAN_A, ON_CHANNEL);
            pwm_set_chan_level(cs_mot2.slice, PWM_CHAN_B, OFF_CHANNEL);
        break;
        case 4:
            pwm_set_chan_level(cs_mot1.slice, PWM_CHAN_A, OFF_CHANNEL);
            pwm_set_chan_level(cs_mot1.slice, PWM_CHAN_B, OFF_CHANNEL);
            pwm_set_chan_level(cs_mot2.slice, PWM_CHAN_A, OFF_CHANNEL);
            pwm_set_chan_level(cs_mot2.slice, PWM_CHAN_B, ON_CHANNEL);
        break;
        default:
            pwm_set_chan_level(cs_mot1.slice, PWM_CHAN_A, OFF_CHANNEL);
            pwm_set_chan_level(cs_mot1.slice, PWM_CHAN_B, OFF_CHANNEL);
            pwm_set_chan_level(cs_mot2.slice, PWM_CHAN_A, OFF_CHANNEL);
            pwm_set_chan_level(cs_mot2.slice, PWM_CHAN_B, OFF_CHANNEL);
        break;
    }
}

void motor_set(int pwm_val)
{
    if (pwm_val > 0)
    {
        pwm_set_chan_level(cs_mot1.slice, PWM_CHAN_A, pwm_val);
        pwm_set_chan_level(cs_mot1.slice, PWM_CHAN_B, OFF_CHANNEL);
    }
    else
    {
        pwm_set_chan_level(cs_mot1.slice, PWM_CHAN_A, OFF_CHANNEL);
        pwm_set_chan_level(cs_mot1.slice, PWM_CHAN_B, -pwm_val);
    }
}
