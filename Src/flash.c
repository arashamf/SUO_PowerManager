#include <stdio.h>
#include "stm32f1xx_hal.h"
#include "flash.h"
#include "calibration.h"


FLASH_EraseInitTypeDef EraseInitStruct;


unsigned int Read_Flash(unsigned int adrData)
{
	return *(unsigned int*) adrData;
}


HAL_StatusTypeDef Erase_Flash(void) 
{
	EraseInitStruct.Banks = FLASH_BANK_1;
	EraseInitStruct.NbPages = 1;
	EraseInitStruct.PageAddress = ADDR_DEFAULT;
	EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
	unsigned int SectorError = 0;
	
	if(HAL_FLASH_Unlock() == HAL_ERROR) {
		return HAL_ERROR;
	}
	if(HAL_FLASHEx_Erase(&EraseInitStruct, &SectorError) == HAL_ERROR) {
		return HAL_ERROR;
	}
	if(HAL_FLASH_Lock()){
		return HAL_ERROR;
	}
	return HAL_OK;
}


HAL_StatusTypeDef Write_Flash(unsigned int adrData, unsigned int Data) 
{
	if(HAL_FLASH_Unlock() == HAL_ERROR) {
		return HAL_ERROR;
	}
	if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, adrData, Data) == HAL_ERROR) {
		return HAL_ERROR;
	}
	if(HAL_FLASH_Lock()){
		return HAL_ERROR;
	}
	return HAL_OK;
}


HAL_StatusTypeDef SaveCalValues(void)
{
	Erase_Flash();

	if(Write_Flash(ADDR_DEFAULT, firstTimeToken) == HAL_ERROR) {
		return HAL_ERROR;
	}
	if(Write_Flash(ADDR_DEFAULT+4, calV24) == HAL_ERROR) {
		return HAL_ERROR;
	}
	return HAL_OK;
}


void ReadCalibration(void)			
{
	firstTimeToken=Read_Flash(ADDR_DEFAULT);
	if(firstTimeToken!=0xDEADBEEF) {
		calV24 = 1000;
	}
	else {
		calV24=Read_Flash(ADDR_DEFAULT+4);
	}
}


