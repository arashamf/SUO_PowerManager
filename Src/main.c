// SUO Power manager 2019.12.17
//2021.08.27

#include "main.h"
#include "usb_device.h"
#include "usbd_cdc_if.h"
#include "timer.h"
#include "pins.h"
#include "gpio.h"
#include "uart.h"
#include "clockConfig.h"
#include "tim.h"
#include "adc.h"
#include "sleep.h"
#include "battery.h"
#include "flash.h"
#include "calibration.h"
#include "main_proc.h"

//#define TEST_SWITCH_OFF 1

int main(void)
{
	int cnt=0;
#ifdef TEST_SWITCH_OFF
	int tmp_flag = 0;
#endif
	
	HAL_Init();
  SystemClock_Config();	
  MX_GPIO_Init();
	CHARGE_ENABLE(OFF);
	BAT_ENABLE(OFF);
	V24_ENABLE(OFF);
	LED_HEARTBEAT(ON);

	ADC1_Init();		
	ReadCalibration();
	
	if(firstTimeToken!=0xDEADBEEF) {					// if the device is being used for the first time 
		Calibration();													// enter calibration mode
	}

	delay_ms(20);
	ReadAllValues();
	if(value.v24 < MIN_24V_L1_VOLTAGE) {			// if power supply is still low								
		LED_HEARTBEAT(OFF);
		Init_IWDG(2000);												// keep sleeping (waiting for another IWDT interruption)
		extLEDstate=EXT_LED_OFF;
		Sleep();
	}
	USB_PULLUP(OFF); 
//	delay_ms(1000);
//	USB_PULLUP(ON);														// 1.5k resistor for D+ pin of the USB

	MX_TIM1_Init();
	MX_TIM2_Init();
	MX_TIM4_Init();
	ChrgPWM(ON,0);														// 0% duty for lowest Vchrg
	MX_USART1_UART_Init();
	COM_ENABLE(1);
  MX_USB_DEVICE_Init();
	
	delay_ms(200);
	WaitV24riseUp();
	Generate36V(ON);
	WaitV36riseUp();
	V24_ENABLE(ON);
	WaitV24riseUp();
	powerState=STATE_MAIN;
	delay_ms(500);
	EnableTimer3(1000);												// buzzer, LEDs and other interruptions each 1ms
	USB_PULLUP(ON);

  while (1) {
		ReadAllValues();												// read all voltages and calculate their mean values

#ifdef TEST_SWITCH_OFF
		if(BUT == 0) { tmp_flag = 1;}
		if(tmp_flag != 0) {avgValue.vBat = 20500; }
#endif

		if(value.v24 > MIN_24V_L2_VOLTAGE) {		// main power supply. powerState = 0. Todo: avg???
			if(powerState==STATE_BATTERY) {				// shift to main +24V
				BAT_ENABLE(OFF);
				delay_us(100);
				V24_ENABLE(ON);					
			}
			powerState=STATE_MAIN;
			ChargeBattery();											// battery charging procedure
      ButtonHandler(0);
		}
		
		if(value.v24 < MIN_24V_L1_VOLTAGE) {		// battery power supply. powerState = 1.
			if(powerState==STATE_MAIN) {					// shift to battery
				V24_ENABLE(OFF);
				delay_us(100);
				BAT_ENABLE(ON);				
			}	
			powerState=STATE_BATTERY;
			UseBattery();
			ButtonHandler(1);
		}

		LED_voltageIndication();
		HeartBeatLED();

		if(flag_switch_off != 1)
		 {
		  cnt++;
		  if(cnt>1000)
				{													          // each 1 second
			    cnt=0;
			    MakeBufferToHost(0);										      // let's fill the buffer with current data
			    CDC_Transmit_FS((uint8_t*)bufToHost,lenDataToHost);	// and send it through USB
			  }   	
		  UartSendToHost();     	// send data through UART, 1 byte a cycle, until it finishes
	   }
		Wait_1ms();
	 }
}
