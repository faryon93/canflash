#ifndef LPC_H_
#define LPC_H_

#include <stdint.h>


// ----------------------------------------------------------------------------------
//  Konstanten
// ----------------------------------------------------------------------------------

#define LPC_SDO_UNLOCK_IDX		0x5000
#define LPC_SDO_UNLOCK_SUBIDX	0x00

#define LPC_SDO_IDENTITY_IDX	0x1018
#define LPC_SDO_PARTID_SUBIDX	0x02

#define LPC_SDO_PREP_SECTOR_IDX	0x5020

#define LPC_SDO_ERASE_IDX		0x5030

#define LPC_SDO_RAM_ADDR_IDX	0x5015

#define LPC_UNLOCK_CODE			0x5A5A

#define LPC_11C24_SECTOR_SIZE	4096
#define LPC_11C24_SECTOR_COUT	8
#define LPC_FLASH_SIZE			32000


// ----------------------------------------------------------------------------------
//  Funktionen
// ----------------------------------------------------------------------------------

char *lpc_part_name(uint32_t part_id);


#endif