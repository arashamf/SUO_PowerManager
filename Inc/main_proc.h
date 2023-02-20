void SystemClock_Config_HSI(void);
void LED_voltageIndication(void);
void UartSendToHost(void);
void HeartBeatLED(void);
void WaitV24riseUp(void);
void WaitV36riseUp(void);
void ButtonHandler(int battery);
void MakeBufferToHost(int flag_shut_dwn);

extern char bufToHost[250];
extern int counterToCom;
extern int lenDataToHost;
extern int flag_switch_off;
