#define COIL1 16
#define COIL2 17
#define COIL3 14
#define COIL4 15

void motor_init(void);
void motor_deinit(void);
void motor_set_frequency(double f);

void motor_set(int pwm_val);
void motor_set_single(int coil_number);



