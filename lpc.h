#ifndef LPC_H_
#define LPC_H_

#include <stdint.h>
#include "sdo.h"


// ----------------------------------------------------------------------------------
//  Konstanten
// ----------------------------------------------------------------------------------

#define LPC_SDO_NODE_ID			0x67D

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

size_t lpc_next_sector_size(size_t size);

void lpc_prepare_sector(can_t *can, uint8_t start_sector, uint8_t end_sector);
void lpc_erase_sector(can_t *can, uint8_t start_sector, uint8_t end_sector);
uint32_t lpc_ram_addr(can_t *can);

#endif