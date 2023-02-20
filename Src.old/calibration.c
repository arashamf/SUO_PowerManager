#include "adc.h"
#include "sleep.h"
#include "pins.h"
#include "tim.h"
#include "timer.h"
#include "battery.h"
#include "calibration.h"
#include "flash.h"

#include "usb_device.h"
#include "usbd_cdc_if.h"

unsigned int firstTimeToken;		// virgin/calibrated device token
unsigned int calV24;						// calibration factor for 24V


void Beep(int value)
{
	while(value--!=0) {
		Buzzer(ON);
		delay_ms(50);
		Buzzer(OFF);
		delay_ms(50);
	}
}


void WaitButOff(void)
{
	int tmp=0;
	
	while(tmp<100) {
		if(BUT==0) {tmp=0;}
		else {tmp++;}
	}
}

char tmpBuf[200];

void Calibration(void)
{
	int tmp, sum;
	
	MX_TIM1_Init();
	MX_TIM2_Init();
	MX_TIM4_Init();
	
	MX_USB_DEVICE_Init();
	
	buzzerState=BUZZER_UNMANAGEBLE;
	batLEDstate=LED_CALIBRATION_MODE;	
	EnableTimer3(1000);
	
	for(tmp=0;tmp<100;tmp++) {
		delay_us(1000);
		ReadAllValues();
	}
	Beep(3);
	
	sprintf(tmpBuf,"Calibration start. vBat=%dmV, Vchrg=%dmV, 24V=%dmV, Vprnt=%dmV, 12V=%dmV, Vpc=%dmV, 36V=%dmV, I=%dmA \r\n", 
	avgValue.vBat, avgValue.vChrg, avgValue.v24, avgValue.vPrnt, avgValue.v12, avgValue.v19, avgValue.v36, avgValue.chrgCurrent);	
	CDC_Transmit_FS((uint8_t*)tmpBuf,strlen(tmpBuf));
	
	while(1) {
		delay_us(1000);
		ReadAllValues();
		if((avgValue.v24<22000)||(avgValue.v24>26000)) {		// tolerance is more than 6%
			batLEDstate=LED_CALIBRATION_IMPOSSIBLE;
		}
		else {
			batLEDstate=LED_CALIBRATION_MODE;		
			if(BUT==0) {
				Beep(1);
				sum=0;
				for(tmp=0;tmp<1000;tmp++) {
					delay_us(1000);
					ReadAllValues();
					sum+=value.v24;
				}
				sum = sum/1000;
				calV24 = (24000*1000)/sum;		// calculate the factor for 24V
				WaitButOff();
				break;
			}
		}
	}
		
	for(tmp=0;tmp<100;tmp++) {
		delay_us(1000);
		ReadAllValues();
	}
	
	sprintf(tmpBuf,"Calibration done. vBat=%dmV, Vchrg=%dmV, 24V=%dmV, Vprnt=%dmV, 12V=%dmV, Vpc=%dmV, 36V=%dmV, I=%dmA \r\n", 
	avgValue.vBat, avgValue.vChrg, avgValue.v24, avgValue.vPrnt, avgValue.v12, avgValue.v19, avgValue.v36, avgValue.chrgCurrent);	
	CDC_Transmit_FS((uint8_t*)tmpBuf,strlen(tmpBuf));
	
	sprintf(tmpBuf,"result: calV24=%d\r\n",calV24);
	CDC_Transmit_FS((uint8_t*)tmpBuf,strlen(tmpBuf));
	
	firstTimeToken=0xDEADBEEF;
	SaveCalValues();
	delay_ms(500);
	Beep(3);	
	batLEDstate=0;
	buzzerState=0;
	delay_ms(500);
	WaitButOff();
	
	NVIC_SystemReset();
}
