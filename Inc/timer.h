#include "stm32f1xx.h"

#define BUZZER_DISABLED 				0
#define BUZZER_SLOW_BEEP 				1
#define BUZZER_FAST_BEEP 				2
#define BUZZER_VERY_FAST_BEEP 	3
#define BUZZER_PERMANENT_BEEP		4
#define BUZZER_SOS 							5
#define BUZZER_UNMANAGEBLE 			100


extern int powerState;
extern int batLEDstate;
extern int extLEDstate;
extern int buzzerState;
extern volatile int flag_1ms;

void delay_us(uint16_t delay);
void delay_ms(uint16_t delay);
void EnableTimer3(unsigned int value);
void DisableTimer3(void);
void Wait_1ms(void);



