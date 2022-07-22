#ifndef _INTERNAL_FLASH_H
#define _INTERNAL_FLASH_H

#include "misc.h"
/***
 * use lase page(2048bytes) to simulate a EEPROM
 * */

/* Includes ------------------------------------------------------------------*/
typedef struct{
	int (*read)(const u16 addr, u8 *buff, const u16 bytes);
	int (*write)(const u16 addr, const u8 *buff, const u16 bytes);
	int (*read256)(const u16 addr, u8 *buff);
} sEEPROM_t;

void setupSimulatedEEPROM(sEEPROM_t* dev);

#endif


