#include "internalFlash.h"
#include "string.h"

// use last page for simulated eeprom
#define SIMULEATED_EEPROM_SIZE (FLASH_PAGE_SIZE/8)
#define FLASH_USER_START_ADDR   (FLASH_BASE + (FLASH_PAGE_NB - 1) * FLASH_PAGE_SIZE)   	/* Start @ of user Flash area 	*/

static int simulatedEEPROM_read(const u16 addr, u8 *buff, const u16 bytes);
static int simulatedEEPROM_write(const u16 addr, const u8 *buff, const u16 bytes);
static int simulatedEEPROM_firstEmptyAddr256();
static int simulatedEEPROM_read256(const u16 addr, u8 *buff);
static int simulatedEEPROM_write256(const u16 addr, const u8 *buff);

void setupSimulatedEEPROM(sEEPROM_t* dev){
	dev->read = simulatedEEPROM_read;
	dev->write = simulatedEEPROM_write;
	dev->read256= simulatedEEPROM_read256;
}

static int simulatedEEPROM_read(const u16 addr, u8 *buff, const u16 bytes){
	s32 i;
	u8 xBuff[256];

	if(addr >= SIMULEATED_EEPROM_SIZE){	return -1;	}
	// seek a avalid 256 section
	i = simulatedEEPROM_firstEmptyAddr256();
	if(i < 0){	simulatedEEPROM_read256(7*256, xBuff);	}
	else if(i >= 256){	simulatedEEPROM_read256(i-256, xBuff);	}
	else{	memset(xBuff,0xff,256);	}
	memcpy(buff, &xBuff[addr], bytes);
	return i;
}

static int simulatedEEPROM_write(const u16 addr, const u8 *buff, const u16 bytes){
	int32_t i;
	u8 xBuff[256] = {0xff};

	if(addr >= SIMULEATED_EEPROM_SIZE){	return -1;	}

	// seek a avalid 256 section
	i = simulatedEEPROM_firstEmptyAddr256();

	if (i < 0){
		simulatedEEPROM_read256(7*256, xBuff);
		i = 0;	// roll, program at head
	}
	else if (i >= 256){
		simulatedEEPROM_read256(i-256, xBuff);
	}

	memcpy(&xBuff[addr], buff, bytes);
	return(simulatedEEPROM_write256(i, xBuff));
}

static int simulatedEEPROM_firstEmptyAddr256() {
	u32 tmp,i,j,k;
	u32 iFlshAddr = FLASH_USER_START_ADDR + 0;

	for(k=0;k<8;k++){
		for(i=0,j=0;i<SIMULEATED_EEPROM_SIZE/4;i++)
		{
			tmp = *(__IO uint32_t *)iFlshAddr;
			iFlshAddr += 4;
			if(tmp == 0xffffffff){	j++;	}
		}
		if(j == SIMULEATED_EEPROM_SIZE/4){
			return k*256;
		}
	}
	return -1;
}

static int simulatedEEPROM_read256(const u16 addr, u8 *buff){
	u8 *p = buff;
	u32 tmp,i;
	u32 iFlshAddr = FLASH_USER_START_ADDR + addr;

	for(i=0;i<256/4;i++)	//SIMULEATED_EEPROM_SIZE
	{
		tmp = *(__IO uint32_t *)iFlshAddr;
		iFlshAddr += 4;
		p[0] = tmp;
		p[1] = tmp>>8;
		p[2] = tmp>>16;
		p[3] = tmp>>24;
		p += 4;
	}
	return 0;
}

static int simulatedEEPROM_write256(const u16 addr, const u8 *buff){
	FLASH_EraseInitTypeDef EraseInitStruct = {0};
	uint32_t PageError = 0;
	u32 iFlshAddr = FLASH_USER_START_ADDR + addr;

	if(addr > (FLASH_PAGE_SIZE-256)){	return -1;	}

	/* Unlock the Flash to enable the flash control register access *************/
	HAL_FLASH_Unlock();
	while (HAL_FLASH_Program(FLASH_TYPEPROGRAM_FAST, iFlshAddr, (u32)buff) != HAL_OK){
		iFlshAddr = FLASH_USER_START_ADDR;
		EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
		EraseInitStruct.Page        = FLASH_PAGE_NB-1;
		EraseInitStruct.NbPages     = 1;
		if (HAL_FLASHEx_Erase(&EraseInitStruct, &PageError) != HAL_OK){		return -2;	}
	}
	HAL_FLASH_Lock();
	return 0;
}
