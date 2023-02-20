#include "timer.h"
#include "adc.h"
#include "tim.h"
#include "pins.h"
#include "battery.h"

int powerState=0;
int batLEDstate=0;
int extLEDstate=0;
int buzzerState=0;
volatile int flag_1ms=0;

#define DELAY_8 __NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();
#define DELAY_1US DELAY_8; DELAY_8; DELAY_8; DELAY_8; DELAY_8; DELAY_8; DELAY_8; DELAY_8; __NOP();__NOP();


void delay_us(uint16_t delay)
{
	while(delay--) {
		DELAY_1US;
	}
}


void delay_ms(uint16_t delay)
{
	while(delay--) {delay_us(1000);}
}


//enable timer4: 1 us
void EnableTimer3(unsigned int value)
{
	RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
	TIM3->EGR |= TIM_EGR_UG;
	TIM3->PSC = 72000000/1000000-1; 
	TIM3->ARR = value;
	TIM3->CNT=0;
	TIM3->CR1 |= TIM_CR1_CEN;
	TIM3->DIER |= TIM_DIER_UIE;
	NVIC_EnableIRQ(TIM3_IRQn); 
}


void DisableTimer3(void)
{
	RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
	TIM3->CR1 &= ~TIM_CR1_CEN;
	TIM3->DIER &= ~TIM_DIER_UIE;
	NVIC_DisableIRQ(TIM3_IRQn); 
	TIM3->SR &= ~TIM_SR_UIF;
}


// interruption each 1ms
void TIM3_IRQHandler(void) {	  
	TIM3->SR &= ~TIM_SR_UIF;
	
	static int ledCnt=0, buzzerCnt=0, extLedCnt=0;
	
	flag_1ms = 1;
	
	switch(buzzerState) {		// buzzer handler
		case BUZZER_DISABLED:
			Buzzer(OFF);
		break;
		case BUZZER_SLOW_BEEP:
			buzzerCnt++;
			if(buzzerCnt==1950){
				Buzzer(ON);
			}
			if(buzzerCnt>2000){
				Buzzer(OFF);
				buzzerCnt=0;
			}	
		break;
		case BUZZER_FAST_BEEP:
			buzzerCnt++;
			if(buzzerCnt==450){
				Buzzer(ON);
			}
			if(buzzerCnt>500){
				Buzzer(OFF);
				buzzerCnt=0;
			}	
		break;
		case BUZZER_VERY_FAST_BEEP:
			buzzerCnt++;
			if(buzzerCnt==100){
				Buzzer(ON);
			}
			if(buzzerCnt>150){
				Buzzer(OFF);
				buzzerCnt=0;
			}	
		break;
		case BUZZER_PERMANENT_BEEP:
			Buzzer(ON);
		break;
		case BUZZER_SOS:
			buzzerCnt++;
			if(buzzerCnt==1		){	Buzzer(ON);		}	// .
			if(buzzerCnt==100	){	Buzzer(OFF);	}
			if(buzzerCnt==200	){	Buzzer(ON);		} // .
			if(buzzerCnt==300	){	Buzzer(OFF);	}
			if(buzzerCnt==400	){	Buzzer(ON);		} // .
			if(buzzerCnt==500	){	Buzzer(OFF);	}
			
			if(buzzerCnt==700		){	Buzzer(ON);		}	// -
			if(buzzerCnt==900		){	Buzzer(OFF);	}
			if(buzzerCnt==1000	){	Buzzer(ON);		}	// -
			if(buzzerCnt==1200	){	Buzzer(OFF);	}
			if(buzzerCnt==1300	){	Buzzer(ON);		}	// -
			if(buzzerCnt==1500	){	Buzzer(OFF);	}
			
			if(buzzerCnt==1700	){	Buzzer(ON);		}	// .
			if(buzzerCnt==1800	){	Buzzer(OFF);	}
			if(buzzerCnt==1900	){	Buzzer(ON);		} // .
			if(buzzerCnt==2000	){	Buzzer(OFF);	}
			if(buzzerCnt==2100	){	Buzzer(ON);		} // .
			if(buzzerCnt==2200	){	Buzzer(OFF);	}
			
			if(buzzerCnt>4000){
				Buzzer(OFF);
				buzzerCnt=0;
			}	
		break;
		default: break;
	}

	switch(batLEDstate) {				// battery LEDs handler
		case BAT_LEDS_OFF: 				// all LEDs OFF
			LED_BAT_1(OFF); LED_BAT_2(OFF); LED_BAT_3(OFF);
			ledCnt = 0;
		break;
		case BAT_LED_BLINK_1_3:		// LED1 blinking
			ledCnt++;
			if(ledCnt<500) {
				LED_BAT_1(OFF); LED_BAT_2(OFF); LED_BAT_3(OFF);
			}
			else {
				LED_BAT_1(ON); LED_BAT_2(OFF); LED_BAT_3(OFF);
			}
			if(ledCnt>1000) {
				ledCnt = 0;
			}
		break;
		case BAT_LED_BLINK_2_3:		// LED1, LED2 running
			ledCnt++;
			if(ledCnt<500) {
				LED_BAT_1(OFF); LED_BAT_2(OFF); LED_BAT_3(OFF);
			}
			if((ledCnt>500)&&(ledCnt<1000)) {
				LED_BAT_1(ON); LED_BAT_2(OFF); LED_BAT_3(OFF);
			}
			if((ledCnt>1000)&&(ledCnt<1500)) {
				LED_BAT_1(ON); LED_BAT_2(ON); LED_BAT_3(OFF);
			}
			if(ledCnt>1500) {
				ledCnt=0;
			}
		break;
		case BAT_LED_BLINK_3_3:		// LED1, LED2, LED3 running
			ledCnt++;
			if(ledCnt<500) {
				LED_BAT_1(OFF); LED_BAT_2(OFF); LED_BAT_3(OFF);
			}
			if((ledCnt>500)&&(ledCnt<1000)) {
				LED_BAT_1(ON); LED_BAT_2(OFF); LED_BAT_3(OFF);
			}
			if((ledCnt>1000)&&(ledCnt<1500)) {
				LED_BAT_1(ON); LED_BAT_2(ON); LED_BAT_3(OFF);
			}
			if((ledCnt>1500)&&(ledCnt<2000)) {
				LED_BAT_1(ON); LED_BAT_2(ON); LED_BAT_3(ON);
			}
			if(ledCnt>2000) {
				ledCnt=0;
			}
		break;
		case BAT_LED_1_3:					// LED1 on
			LED_BAT_1(ON); LED_BAT_2(OFF); LED_BAT_3(OFF);
		break;
		case BAT_LED_2_3:					// LED1,LED2 on
			LED_BAT_1(ON); LED_BAT_2(ON); LED_BAT_3(OFF);
		break;
		case BAT_LED_3_3:					// LED1,LED2,LED3 on
			LED_BAT_1(ON); LED_BAT_2(ON); LED_BAT_3(ON);
		break;
		case BAT_LED_CHRG_SHORT_CIRCUIT:			// all LEDS bit "1"
			ledCnt++;
			if(ledCnt<50) {
				LED_BAT_1(ON); LED_BAT_2(ON); LED_BAT_3(ON);
			}
			else {
				LED_BAT_1(OFF); LED_BAT_2(OFF); LED_BAT_3(OFF);
			}
			if(ledCnt>1000) {
				ledCnt=0;
			}
		break;
		case BAT_LED_CHRG_NO_BATTERY:					// all LEDS bit "2"
			ledCnt++;
			if(ledCnt<50) {
				LED_BAT_1(ON); LED_BAT_2(ON); LED_BAT_3(ON);
			}
			if((ledCnt>50)&&(ledCnt<150)) {
				LED_BAT_1(OFF); LED_BAT_2(OFF); LED_BAT_3(OFF);
			}
			if((ledCnt>150)&&(ledCnt<200)) {
				LED_BAT_1(ON); LED_BAT_2(ON); LED_BAT_3(ON);
			}
			if((ledCnt>200)&&(ledCnt<1000)) {
				LED_BAT_1(OFF); LED_BAT_2(OFF); LED_BAT_3(OFF);
			}
			if(ledCnt>1000) {
				ledCnt=0;
			}
		break;
		case LED_CALIBRATION_MODE:					// voltage LEDs are running
			ledCnt++;
			if((ledCnt>0)&&(ledCnt<50)) {
				LED_24V(OFF); LED_BAT(OFF); LED_12V(OFF); LED_PRNT(OFF); LED_CHRG(OFF); LED_36V(ON); LED_19V(OFF);
			}
			if((ledCnt>50)&&(ledCnt<100)) {
				LED_24V(OFF); LED_BAT(OFF); LED_12V(OFF); LED_PRNT(OFF); LED_CHRG(ON); LED_36V(OFF); LED_19V(OFF);
			}
			if((ledCnt>100)&&(ledCnt<150)) {
				LED_24V(OFF); LED_BAT(OFF); LED_12V(OFF); LED_PRNT(ON); LED_CHRG(OFF); LED_36V(OFF); LED_19V(OFF);
			}
			if((ledCnt>150)&&(ledCnt<200)) {
				LED_24V(OFF); LED_BAT(OFF); LED_12V(ON); LED_PRNT(OFF); LED_CHRG(OFF); LED_36V(OFF); LED_19V(OFF);
			}
			if((ledCnt>200)&&(ledCnt<250)) {
				LED_24V(OFF); LED_BAT(ON); LED_12V(OFF); LED_PRNT(OFF); LED_CHRG(OFF); LED_36V(OFF); LED_19V(OFF);
			}
			if((ledCnt>250)&&(ledCnt<300)) {
				LED_24V(ON); LED_BAT(OFF); LED_12V(OFF); LED_PRNT(OFF); LED_CHRG(OFF); LED_36V(OFF); LED_19V(OFF);
			}
			if((ledCnt>300)&&(ledCnt<350)) {
				LED_24V(OFF); LED_BAT(OFF); LED_12V(OFF); LED_PRNT(OFF); LED_CHRG(OFF); LED_36V(OFF); LED_19V(ON);
			}
			if(ledCnt>400) {
				ledCnt=0;
			}
		break;
		case LED_CALIBRATION_IMPOSSIBLE:
			ledCnt++;
			if((ledCnt>0)&&(ledCnt<50)) {
				LED_24V(ON); LED_BAT(ON); LED_12V(ON); LED_PRNT(ON); LED_CHRG(ON); LED_36V(ON); LED_19V(ON);
			}
			if((ledCnt>50)&&(ledCnt<1000)) {
				LED_24V(OFF); LED_BAT(OFF); LED_12V(OFF); LED_PRNT(OFF); LED_CHRG(OFF); LED_36V(OFF); LED_19V(OFF);
			}
			if(ledCnt>1000) {
				ledCnt=0;
			}
		break;
		default: break;
	}
	
	switch(extLEDstate) {				// external LED handler
		case EXT_LED_OFF:
			LED_EXT(OFF);		
			extLedCnt=0;
		break;
		case EXT_LED_ON:
			LED_EXT(ON);		
			extLedCnt=0;		
		break;
		case EXT_LED_BLINK:
			extLedCnt++;
			if(extLedCnt<500) {LED_EXT(OFF);}
			else {LED_EXT(ON);}
			if(extLedCnt>1000) {extLedCnt=0;}
		break;
		default:	break;
	}
}


void Wait_1ms(void)
{
	while(flag_1ms==0);
	flag_1ms=0;
}


