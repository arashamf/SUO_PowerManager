#include "adc.h"
#include "sleep.h"
#include "pins.h"
#include "tim.h"
#include "timer.h"
#include "battery.h"

int PWMduty=0;
int chrgMode=MODE_CC;
int batLevelToHost=BAT_LEVEL_TO_HOST_HIGH;
int fShutDown=0;


// set the charging current as close to targetCurrent as possible.
// return 0 if success
// 1 - current is too low, could not increase
// 2 - current is too high, suspect short circuit
// 3 - battery voltage is too high, suspect battery is disconnected
int SetChrgCurrent(int targetCurrent)
{
	int res=0;
	
	if(avgValue.chrgCurrent<targetCurrent) {	// if charge current is too low
		if(PWMduty<100) {
			PWMduty++;											// increase current
		}
		else {														// failed to increase
			res=1;
		}											
	}
	else {															// if charge current is too high
		if(PWMduty>0) {
			PWMduty--;											// reduce current
		}
		else {														// failed to reduce
			res=2;
		}											
	}
	if(avgValue.vBat>NO_BATTERY_VOLTAGE) {
		if(PWMduty>0) {
			PWMduty--;
		}
		res=3;
	}
	ChrgPWM(ON,PWMduty);								// set the PWM duty value
	
	return res;
}


// set charge voltage as close to the terget as possible
void SetChrgVoltage(int targetVoltage)
{
	if(avgValue.vChrg<targetVoltage) {	// if charge voltage is too low
		if(PWMduty<100) {
			PWMduty++;											// increase current
		}										
	}
	else {															// if charge current is too high
		if(PWMduty>0) {
			PWMduty--;											// reduce current
		}										
	}
	ChrgPWM(ON,PWMduty);								// set the PWM duty value
}


void ChargeBattery(void)
{
	static int tmp=0;
	static int t=0;
	
	tmp++;
	if(tmp<10) { return; }		// each 10ms only
	tmp=0;
	
	CHARGE_ENABLE(ON);
	buzzerState=BUZZER_DISABLED;
	extLEDstate=EXT_LED_ON;
	fShutDown=0;
	
	if(chrgMode==MODE_CC) {
		if(avgValue.vBat<BAT_CHRG_L1) {
			if(SetChrgCurrent(CC_MODE_CURRENT)!=0) {		// set charge current
				batLEDstate = BAT_LED_CHRG_SHORT_CIRCUIT;	// high current means short circuit. Indicate the error
			}
			else {
				batLEDstate = BAT_LED_BLINK_1_3;	// 1 LED is blinking
			}
			t=0;
		}
		if((avgValue.vBat>BAT_CHRG_L1)&&(avgValue.vBat<BAT_CHRG_L2)) {
			SetChrgCurrent(CC_MODE_CURRENT); 		// set charge current
			batLEDstate = BAT_LED_BLINK_2_3;		// 2 LEDs are running
			t=0;
		}
		if((avgValue.vBat>BAT_CHRG_L2)&&(avgValue.vBat<BAT_CHRG_L3)) {
			SetChrgCurrent(CC_MODE_CURRENT); 		// set charge current
			batLEDstate = BAT_LED_BLINK_3_3;		// 3 LEDs are running
			t=0;
		}
		if(avgValue.vBat>BAT_CHRG_L3) {
			SetChrgCurrent(CC_MODE_CURRENT);
			batLEDstate = BAT_LED_BLINK_3_3;
			if(t<100) {								// give 100ms for battery 
				t++;	
			}
			else {
				t=0;
				chrgMode=MODE_CV;
			}
		}
	}
	if(chrgMode==MODE_CV) {
		SetChrgVoltage(CV_MODE_VOLTAGE); 			// set charge voltage
		batLEDstate = BAT_LED_3_3;						// 3 LEDs are on
		if((avgValue.vBat<BAT_CHRG_L1)||(avgValue.chrgCurrent>CC_MODE_CURRENT)) {	// we can get back to the CC mode here
			chrgMode=MODE_CC;
		}
		t=0;
	}
}


void UseBattery(void)
{
	static int cnt=0;
	
	CHARGE_ENABLE(OFF);			// disabling charge circuit 
	PWMduty = 0;
	ChrgPWM(ON,PWMduty);
	chrgMode=MODE_CC;				// next time let's start charging with CC mode
	extLEDstate=EXT_LED_BLINK;
	
	if(fShutDown==0) {
		if(avgValue.vBat>BAT_LEVEL_HIGH) {
			batLevelToHost=BAT_LEVEL_TO_HOST_HIGH;
			buzzerState=BUZZER_SLOW_BEEP;
			batLEDstate = BAT_LED_3_3;
			cnt=0;
			return;
		}
		if((avgValue.vBat>BAT_LEVEL_MIDDLE)&&(avgValue.vBat<BAT_LEVEL_HIGH)) {
			batLevelToHost=BAT_LEVEL_TO_HOST_MIDDLE;
			buzzerState=BUZZER_FAST_BEEP;
			batLEDstate = BAT_LED_2_3;
			cnt=0;
			return;
		}
		if((avgValue.vBat>BAT_LEVEL_LOW)&&(avgValue.vBat<BAT_LEVEL_MIDDLE)) {
			batLevelToHost=BAT_LEVEL_TO_HOST_LOW;
			buzzerState=BUZZER_VERY_FAST_BEEP;
			batLEDstate = BAT_LED_1_3;
			cnt=0;
			return;
		}
		if(avgValue.vBat<BAT_LEVEL_LOW) {
			fShutDown=1;			// no way out. We will shut down
			cnt=0;
		}
	}
	if(fShutDown==1) {		// no matter what happen, we will shut down soon.
		batLevelToHost=BAT_LEVEL_TO_HOST_SHUT_DOWN;
		buzzerState=BUZZER_VERY_FAST_BEEP;
		batLEDstate = BAT_LEDS_OFF;
		cnt++;
		if((cnt>60000)||(avgValue.vBat<BAT_LEVEL_CRITICAL)) {	// 1 minute for the host shutting down, but sooner if voltage gets critical
			CHARGE_ENABLE(OFF);
			V24_ENABLE(OFF);
			BAT_ENABLE(OFF);
			Generate36V(OFF);
			delay_ms(20);
			buzzerState=BUZZER_PERMANENT_BEEP;
			delay_ms(2000);
			fShutDown=0;
			cnt=0;
			Buzzer(OFF);
			EnterSleepMode();
		}
	}
}

