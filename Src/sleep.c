#include "adc.h"
#include "sleep.h"
#include "pins.h"
#include "tim.h"
#include "timer.h"
#include "battery.h"


void EnterSleepMode(void)
{
	LED_HEARTBEAT(ON);
	delay_ms(10);
	Buzzer(OFF);
	ChrgPWM(OFF,0);
	DisableTimer3();
	delay_ms(2000);
	LED_HEARTBEAT(OFF);
	extLEDstate=EXT_LED_OFF;
	USB_PULLUP(OFF); 
	
	Init_IWDG(2000);
	Sleep();
}


void Sleep(void)
{
	RCC->APB1ENR |= RCC_APB1ENR_PWREN;//??? ???????????? PWR
	SCB->SCR |= 0x04; //??? M3 ????????? sleepdeep
	PWR->CR |= PWR_CR_PDDS;//???????? ????? Power Down Deepsleep
	PWR->CR |= PWR_CR_CWUF ; //??????? wakeup flag
	PWR->CSR |= PWR_CSR_EWUP; //wake up по переднему фронту A0
	__WFE();  //   wait for event      __WFI(wait for interrupt)
}


//https://blog.myelectronics.com.ua/stm32-%D1%81%D1%82%D0%BE%D1%80%D0%BE%D0%B6%D0%B5%D0%B2%D1%8B%D0%B5-%D1%82%D0%B0%D0%B9%D0%BC%D0%B5%D1%80%D1%8B-wdt/
//  IWDG Independent Watchdog
void Init_IWDG(unsigned int tw) // ???????? tw ?? 7?? ?? 26200??
{
	// For IWDG_PR=7 Tmin=6,4ms RLR=Tms*40/256
	IWDG->KR=0x5555; 			// Key access
	IWDG->PR=7; 					// refresh IWDG_PR
	IWDG->RLR=tw*40/256; 	// Load register
	IWDG->KR=0xAAAA; 			// Reload
	IWDG->KR=0xCCCC; 			// Run
}


// ??????? ???????????? ??????????? ??????? IWDG
void IWDG_res(void)
{
	IWDG->KR=0xAAAA; // ????????????
}


int get_system_reset_cause(void)
{
	int reset_cause = 0;
		
	if (__HAL_RCC_GET_FLAG(RCC_FLAG_LPWRRST)) {
		reset_cause = 1; //LOW POWER RESET
	}
	else if (__HAL_RCC_GET_FLAG(RCC_FLAG_WWDGRST))	{
		reset_cause = 2; //WWWD
	}
	else if (__HAL_RCC_GET_FLAG(RCC_FLAG_IWDGRST))	{
		reset_cause = 3; //IWDT 
	}
	else if (__HAL_RCC_GET_FLAG(RCC_FLAG_SFTRST))	{
		reset_cause = 4; //soft reset NVIC_SystemReset()
	}
	else if (__HAL_RCC_GET_FLAG(RCC_FLAG_PORRST))	{
		reset_cause = 5; //POWER ON RESET
	}
	else if (__HAL_RCC_GET_FLAG(RCC_FLAG_PINRST))	{
		reset_cause = 6; //EXTERNAL PIN RESET
	}
	else	{
		reset_cause = 8; 
	}

	// Clear all the reset flags or else they will remain set during future resets until system power is fully removed.
	__HAL_RCC_CLEAR_RESET_FLAGS();
	return reset_cause; 
}

