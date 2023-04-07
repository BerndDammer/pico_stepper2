#define COIL1 16
#define COIL2 17
#define COIL3 14
#define COIL4 15

void motor_single_init(void);
void motor_single_deinit(void);
void motor_single_set(int coil_number, int pwm_val);


