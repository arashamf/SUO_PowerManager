#ifndef FLASH_H
#define FLASH_H

#include "stm32f1xx_hal.h"

#define ADDR_DEFAULT 0x0800FC00

unsigned int Read_Flash(unsigned int adrData);
HAL_StatusTypeDef Write_Flash(unsigned int adrData, unsigned int Data);
HAL_StatusTypeDef SaveCalValues(void);
HAL_StatusTypeDef SaveConfig(void);
void ReadCalibration(void);

#endif
