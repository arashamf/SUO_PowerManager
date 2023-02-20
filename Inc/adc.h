#include "main.h"

#define ADC_CHRG_CHANNEL	0			
#define ADC_BAT_CHANNEL 	1	
#define ADC_24V_CHANNEL 	2	
#define ADC_PRNT_CHANNEL 	3	
#define ADC_12V_CHANNEL 	4	
#define ADC_36V_CHANNEL 	5
#define ADC_19V_CHANNEL 	6


typedef struct{
	int v24;
	int vBat;
	int v12;
	int vPrnt;
	int vChrg;
	int v36;
	int v19;
	int chrgCurrent;
}v_t;

extern v_t value;			
extern v_t avgValue;	

void ADC1_Init(void);
unsigned int ADC1_getValue (void);
void ADC1_ChannelSelect(unsigned int channel);
unsigned int GetVoltage(int channel);
void ReadAllValues(void);


