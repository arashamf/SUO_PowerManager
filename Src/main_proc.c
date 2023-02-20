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
#include "switch_off.h"

void LED_voltageIndication(void);
void MakeBufferToHost(int flag_shut_dwn);
void UartSendToHost(void);

char bufToHost[250];
int counterToCom=0;
int lenDataToHost=0;
int flag_switch_off=0;

void ButtonHandler(int battery)
{
	static int cnt=0;
	int i;
	int j;
	
	if(BUT==0) {cnt++;}
	else {cnt=0;}

	if(battery == 1)
 	 {
//Питание от батареи
		 if((cnt>3000)||(flag_switch_off == 1)) {
		 buzzerState=BUZZER_SOUND_SWITCH_OFF;
		 batLevelToHost=BAT_LEVEL_TO_HOST_SHUT_DOWN;

		 if(flag_switch_off == 1) {i=2;}		//Если пропала сеть, но все уже было выключено, то отработаем только 2 секунды
		 else {i=DELAY_BEFORE_SW_OFF;}			//Отработаем нормальную задержку 
		 delay_ms(100);
		 while(i>0)
		  {
			  MakeBufferToHost(0);																// let's fill the buffer with current data
			  CDC_Transmit_FS((uint8_t*)bufToHost,lenDataToHost);	// and send it through USB
				for(j=0; j<1000; j++)																// and send it through COM
					{
						delay_us(1000);
						UartSendToHost();
					}
				i--;
	    }

		 CHARGE_ENABLE(OFF);
		 V24_ENABLE(OFF);
		 BAT_ENABLE(OFF);
		 Generate36V(OFF);
		 delay_ms(20);
		 Buzzer(OFF);
		 EnterSleepMode();
	  }
	 } 
  else
   {
//Питание от сети
		 if(cnt>3000) {
//Проверим, а не стоит ли флаг выключено
		 if(flag_switch_off == 1) //Если флаг выключено стоит, то надо сделать ресет	 
		  {
						buzzerState=BUZZER_PERMANENT_BEEP;
						delay_ms(1000);
						NVIC_SystemReset();
		  }
			 
		 buzzerState=BUZZER_SOUND_SWITCH_OFF;

			delay_ms(100);
		 for(i=0;i<DELAY_BEFORE_SW_OFF;i++)
		  {
		   MakeBufferToHost(1);																	// let's fill the buffer with current data
			 CDC_Transmit_FS((uint8_t*)bufToHost,lenDataToHost);	// and send it through USB
				for(j=0; j<1000; j++)																// and send it through COM
					{
						delay_us(1000);
						UartSendToHost();
					}
      }

		 CHARGE_ENABLE(OFF);
		 V24_ENABLE(OFF);
		 BAT_ENABLE(OFF);
		 Generate36V(OFF);
		 USB_PULLUP(OFF);
		 delay_ms(20);
		 buzzerState=BUZZER_DISABLED;
		 Buzzer(OFF);
		 cnt = 0;

		 while(BUT == 0) {}
		 flag_switch_off=1;
		 }
	  }
}

void MakeBufferToHost(int flag_shut_dwn)
{
	char psBuf[150];
	int iChrg;
	
	if(avgValue.chrgCurrent>0) {iChrg = avgValue.chrgCurrent;}
	else {iChrg = 0;}
	
	if(powerState==STATE_MAIN)
	{
		switch(chrgMode)
		{
			case MODE_CC:
				if(flag_shut_dwn != 1){sprintf(psBuf,"State=main, charge mode=CC, Ichrg=%dmA", iChrg);}
				else                  {sprintf(psBuf,"State=main, charge mode=CC, level=shut down, Ichrg=%dmA", iChrg);}
				break;
			case MODE_CV:
				if(flag_shut_dwn != 1){sprintf(psBuf,"State=main, charge mode=CV, Ichrg=%dmA", iChrg);}
				else                  {sprintf(psBuf,"State=main, charge mode=CV, level=shut down, Ichrg=%dmA", iChrg);}
				break;
			default: break;
		}
	}
	if(powerState==STATE_BATTERY) {
		switch (batLevelToHost) {
			case BAT_LEVEL_TO_HOST_HIGH:
				strcpy(psBuf,"State=battery, level=high");			// high level. Life's easy
			break;
			case BAT_LEVEL_TO_HOST_MIDDLE:
				strcpy(psBuf,"State=battery, level=middle");		// middle level. Watch out!
			break;
			case BAT_LEVEL_TO_HOST_LOW:
				strcpy(psBuf,"State=battery, level=low");				// low battery. The host must shut down itself
			break;
			case BAT_LEVEL_TO_HOST_SHUT_DOWN:
				strcpy(psBuf,"State=battery, level=shut down");	// less that 1 minute remain. Shut down immediately.
			break;
			default: break;
		}
	}
	sprintf(bufToHost,"%c%s, Vbat=%dmV, Vchrg=%dmV, V24=%dmV, Vprnt=%dmV, V12=%dmV, Vpc=%dmV, V36=%dmV \r\n", 
	0x02, psBuf, avgValue.vBat, avgValue.vChrg, avgValue.v24, avgValue.vPrnt, avgValue.v12, avgValue.v19, avgValue.v36);	
	
	lenDataToHost = strlen(bufToHost);
	counterToCom = 0;
}


void UartSendToHost(void)
{
	if(counterToCom>=lenDataToHost) { return;}
	UART1_SendByte(bufToHost[counterToCom++]);	//send 1 byte from buffer
}


void WaitV36riseUp(void)
{
	int tmp=0;
	
	while(1) {
		ReadAllValues();
		delay_us(1000);
		tmp++;
		if(avgValue.v36<33000) {
			if(tmp<50) {	LED_36V(OFF);	}
			else {	LED_36V(ON);	}
			if(tmp>100) {tmp=0;}
		}
		else {break;}
	}
	LED_36V(OFF);
}


void WaitV24riseUp(void)
{
	int tmp=0;
	
	while((tmp++) < 1000) {
		ReadAllValues();
		delay_us(1000);
		if(avgValue.v24 > MIN_24V_L2_VOLTAGE) {
			return;
		}
	}
	CHARGE_ENABLE(OFF);
	V24_ENABLE(OFF);
	BAT_ENABLE(OFF);
	Generate36V(OFF);
	LED_HEARTBEAT(OFF);
	Init_IWDG(2000);	
	extLEDstate=EXT_LED_OFF;	
	Sleep();
}


void HeartBeatLED(void)
{
	static int cnt=0;
	
	cnt++;
	if(cnt<500) {
		LED_HEARTBEAT(ON);
	}
	else {
		LED_HEARTBEAT(OFF);
	}
	if(cnt>1000) {
		cnt=0;
	}
}

void LED_voltageIndication(void)
{
	int tmp;
	static int cntL=0;
	static int cntS=0;
	
	if(cntL>1000) {cntL=0;}
	else {cntL++;}
	if(cntS>100) {cntS=0;}
	else {cntS++;}
	
	tmp = avgValue.vBat;
	if(tmp<22000) {
		if(cntL<50) { LED_BAT(ON); }
		else { LED_BAT(OFF); }
	}
	else if(tmp>29000) {
		if(cntS<50) { LED_BAT(ON); }
		else { LED_BAT(OFF); }
	}
	else {LED_BAT(1);}
	
	tmp = avgValue.v24;
	if(tmp<23500) {
		if(cntL<50) { LED_24V(ON); }
		else { LED_24V(OFF); }
	}
	else if(tmp>24500) {
		if(cntS<50) { LED_24V(ON); }
		else { LED_24V(OFF); }
	}
	else {LED_24V(1);}
	
	tmp = avgValue.vChrg;
	if((tmp<23000)&&(powerState==STATE_MAIN)){
		if(cntL<50) { LED_CHRG(ON); }
		else { LED_CHRG(OFF); }
	}
	else if(tmp>30000) {
		if(cntS<50) { LED_CHRG(ON); }
		else { LED_CHRG(OFF); }
	}
	else {LED_CHRG(1);}
	
	tmp = avgValue.vPrnt;
	if(tmp<22000) {
		if(cntL<50) { LED_PRNT(ON); }
		else { LED_PRNT(OFF); }
	}
	else if(tmp>25000) {
		if(cntS<50) { LED_PRNT(ON); }
		else { LED_PRNT(OFF); }
	}
	else {LED_PRNT(1);}
	
	tmp = avgValue.v12;
	if(tmp<11500) {
		if(cntL<50) { LED_12V(ON); }
		else { LED_12V(OFF); }
	}
	else if(tmp>12500) {
		if(cntS<50) { LED_12V(ON); }
		else { LED_12V(OFF); }
	}
	else {LED_12V(1);}
	
	tmp = avgValue.v36;
	if(tmp<34000) {
		if(cntL<50) { LED_36V(ON); }
		else { LED_36V(OFF); }
	}
	else if(tmp>38000) {
		if(cntS<50) { LED_36V(ON); }
		else { LED_36V(OFF); }
	}
	else {LED_36V(1);}
	
	tmp = avgValue.v19;
	if(((tmp>11500)&&(tmp<12500))||((tmp>18500)&&(tmp<19500))) {
		LED_19V(ON);
	}
	else {
		if(tmp>19500) {
			if(cntS<50) { LED_19V(ON); }
			else { LED_19V(OFF); }
		}
		else {
			if(cntL<50) { LED_19V(ON); }
			else { LED_19V(OFF); }
		}
	}
}

void SystemClock_Config_HSI(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV512;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV16;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV16;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}


