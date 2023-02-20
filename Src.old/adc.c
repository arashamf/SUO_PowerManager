#include "adc.h"
#include "pins.h"
#include "tim.h"
#include "timer.h"
#include "sleep.h"
#include "calibration.h"


v_t value;			// current values
v_t avgValue;		// averaged values


void ADC1_Init(void)
{
  RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;  
	RCC->CFGR |= RCC_CFGR_ADCPRE;        
	RCC->CFGR |= RCC_CFGR_ADCPRE_DIV8;	

	ADC1->CR2 |= ADC_CR2_CAL;          //Calibration
	while (!(ADC1->CR2 & ADC_CR2_CAL)){}
	    	
	ADC1->SQR3 &= ~ADC_SQR3_SQ2; 

	ADC1->CR2 |= (ADC_CR2_EXTSEL_0 | ADC_CR2_EXTSEL_1 | ADC_CR2_EXTSEL_2 | ADC_CR2_EXTTRIG);
	ADC1->CR2 |= ADC_CR2_ADON;						
		
	GPIOA->CRL &= ~GPIO_CRL_CNF1;                        
	GPIOA->CRL &= ~GPIO_CRL_MODE1;
}


unsigned int ADC1_getValue (void)
{
	ADC1->CR2 |= (1<<30)|(1<<0);								// SWSTART and ADON to start single ADC conversion
	while(!(ADC1->SR&(1<<1)));
	return (uint16_t) ADC1->DR;
	
}


void ADC1_ChannelSelect(unsigned int channel)
{
	channel = channel & 0x1f;
	ADC1->SQR3 = channel;
}


// the result is voltage in (mV)
unsigned int GetVoltage(int channel)
{
	unsigned int v;
	
	ADC1_ChannelSelect(channel);
	v=ADC1_getValue();
	v=((3300*v*(330+24))/4095/24);		// resistor voltage divider is 330k/24k
	return v;
}


void ReadAllValues(void)
{
	value.v24 = 		GetVoltage(ADC_24V_CHANNEL)*calV24/1000;
	value.vBat = 		GetVoltage(ADC_BAT_CHANNEL)*calV24/1000;
	value.v12 = 		GetVoltage(ADC_12V_CHANNEL)*calV24/1000;
	value.vPrnt = 	GetVoltage(ADC_PRNT_CHANNEL)*calV24/1000;
	value.vChrg = 	GetVoltage(ADC_CHRG_CHANNEL)*calV24/1000;
	value.v36 = 		GetVoltage(ADC_36V_CHANNEL)*calV24/1000;
	value.v19 = 		GetVoltage(ADC_19V_CHANNEL)*calV24/1000;
	
	avgValue.v24 =  	(avgValue.v24*9+value.v24)/10;
	avgValue.vBat =  	(avgValue.vBat*9+value.vBat)/10;
	avgValue.v12 =  	(avgValue.v12*9+value.v12)/10;
	avgValue.vPrnt = 	(avgValue.vPrnt*9+value.vPrnt)/10;
	avgValue.vChrg = 	(avgValue.vChrg*9+value.vChrg)/10;
	avgValue.v36 =  	(avgValue.v36*9+value.v36)/10;
	avgValue.v19 =  	(avgValue.v19*9+value.v19)/10;
	
	avgValue.chrgCurrent = (avgValue.vChrg - avgValue.vBat)/2;	// charge current=(Vcharge-Vbat)/2R  [mA]
	avgValue.chrgCurrent -= 20;		// the board is supplied with Vcharge if Vcharge > 24V. Let's subtract 20mA
}

