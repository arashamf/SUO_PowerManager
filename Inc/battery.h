#include "main.h"

#define MIN_24V_L1_VOLTAGE 20000  // voltage to shift to battery power supply [mV]
#define MIN_24V_L2_VOLTAGE 22500  // voltage to get back to main power supply [mV]

#define STATE_MAIN 		0						// main power supply mode
#define STATE_BATTERY	1						// battery power supply mode

#define BAT_CHRG_L1 				24500	// CC mode charging level 1 	[mV]
#define BAT_CHRG_L2	 				26000	// CC mode charging level 2 	[mV]
#define BAT_CHRG_L3 				28000	// voltage to enter CV mode		[mV]
#define NO_BATTERY_VOLTAGE 	30000	// voltage to detect battery disconnected	during charging [mV]

#define BAT_LEVEL_CRITICAL	20000	// shut down immediately to save battery's life
#define BAT_LEVEL_LOW				21000	// voltage is too low, let the host shut down, give 1 minute
#define BAT_LEVEL_MIDDLE		22000	// mid level
#define BAT_LEVEL_HIGH			23500	// high level

#define BAT_LEVEL_TO_HOST_HIGH			0	// battery level for the host
#define BAT_LEVEL_TO_HOST_MIDDLE		1
#define BAT_LEVEL_TO_HOST_LOW				2
#define BAT_LEVEL_TO_HOST_SHUT_DOWN 3

#define MODE_CC	0									// Constant Current charging mode
#define MODE_CV	1									// Constant Voltage charging mode

#define CC_MODE_CURRENT 700				// [mA]
#define CV_MODE_VOLTAGE 27400			// [mV]

#define BAT_LEDS_OFF				0
#define BAT_LED_BLINK_1_3		1
#define BAT_LED_BLINK_2_3		2
#define BAT_LED_BLINK_3_3		3
#define BAT_LED_1_3					4
#define BAT_LED_2_3					5
#define BAT_LED_3_3					6
#define BAT_LED_CHRG_SHORT_CIRCUIT		7
#define BAT_LED_CHRG_NO_BATTERY 			8
#define LED_CALIBRATION_MODE					9
#define LED_CALIBRATION_IMPOSSIBLE		10

#define EXT_LED_OFF			0
#define EXT_LED_ON			1
#define EXT_LED_BLINK		2

extern int PWMduty;
extern int chrgMode;
extern int batLevelToHost;
extern int fShutDown;

int SetChrgCurrent(int current);
void SetChrgVoltage(int voltage);
void ChargeBattery(void);
void UseBattery(void);



