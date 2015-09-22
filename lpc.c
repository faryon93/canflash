#include "lpc.h"


// ----------------------------------------------------------------------------------
//  Lokale Variablen
// ----------------------------------------------------------------------------------

/** Ids der einzelnen LPCs die Programmiert werden koennen. */
static struct {
	char *name;
	uint32_t part_id;
} lpc_names[] =
{
	{"LPC1110FD20",			0x0A07102B},
	{"LPC1110FD20",			0x1A07102B},
	{"LPC1111FDH20/002",	0x0A16D02B},
	{"LPC1111FDH20/002",	0x1A16D02B},
	{"LPC1111FHN33/101",	0x041E502B},
	{"LPC1111FHN33/101",	0x2516D02B},
	{"LPC1111FHN33/102",	0x2516D02B},
	{"LPC1111FHN33/201",	0x0416502B},
	{"LPC1111FHN33/201",	0x2516902B},
	{"LPC1111FHN33/202",	0x2516902B},
	{"LPC1111FHN33/103",	0x00010013},
	{"LPC1111JHN33/103",	0x00010013},
	{"LPC1111FHN33/203",	0x00010012},
	{"LPC1111JHN33/203",	0x00010012},
	{"LPC1112FD20/102",		0x0A24902B},
	{"LPC1112FD20/102",		0x1A24902B},
	{"LPC1112FDH20/102",	0x0A24902B},
	{"LPC1112FDH20/102",	0x1A24902B},
	{"LPC1112FDH28/102",	0x0A24902B},
	{"LPC1112FDH28/102",	0x1A24902B},
	{"LPC1112FHN33/101",	0x042D502B},
	{"LPC1112FHN33/101",	0x2524D02B},
	{"LPC1112FHN33/102",	0x2524D02B},
	{"LPC1112FHI33/102",	0x2524D02B},
	{"LPC1112FHN33/201",	0x0425502B},
	{"LPC1112FHN33/201",	0x2524902B},
	{"LPC1112FHN33/202",	0x2524902B},
	{"LPC1112FHN24/202",	0x2524902B},
	{"LPC1112FHI33/202",	0x2524902B},
	{"LPC1112FHN33/103",	0x00020023},
	{"LPC1112JHN33/103",	0x00020023},
	{"LPC1112FHN33/203",	0x00020022},
	{"LPC1112JHN33/203",	0x00020022},
	{"LPC1112FHI33/203",	0x00020022},
	{"LPC1112JHI33/203",	0x00020022},
	{"LPC1113FHN33/201",	0x0434502B},
	{"LPC1113FHN33/201",	0x2532902B},
	{"LPC1113FHN33/202",	0x2532902B},
	{"LPC1113FHN33/301",	0x0434102B},
	{"LPC1113FHN33/301",	0x2532102B},
	{"LPC1113FHN33/302",	0x2532102B},
	{"LPC1113FBD48/301",	0x0434102B},
	{"LPC1113FBD48/301",	0x2532102B},
	{"LPC1113FBD48/302",	0x2532102B},
	{"LPC1113FBD48/303",	0x00030030},
	{"LPC1113JBD48/303",	0x00030030},
	{"LPC1113FHN33/203",	0x00030032},
	{"LPC1113JHN33/203",	0x00030032},
	{"LPC1113FHN33/303",	0x00030030},
	{"LPC1113JHN33/303",	0x00030030},
	{"LPC1114FDH28/102",	0x0A40902B},
	{"LPC1114FDH28/102",	0x1A40902B},
	{"LPC1114FN28/102",		0x0A40902B},
	{"LPC1114FN28/102",		0x1A40902B},
	{"LPC1114FHN33/201",	0x0444502B},
	{"LPC1114FHN33/201",	0x2540902B},
	{"LPC1114FHN33/202",	0x2540902B},
	{"LPC1114FHN33/301",	0x0444102B},
	{"LPC1114FHN33/301",	0x2540102B},
	{"LPC1114FHN33/302",	0x2540102B},
	{"LPC1114FHI33/302",	0x2540102B},
	{"LPC1114FBD48/301",	0x0444102B},
	{"LPC1114FBD48/301",	0x2540102B},
	{"LPC1114FBD48/302",	0x2540102B},
	{"LPC1114FBD48/303",	0x00040040},
	{"LPC1114JBD48/303",	0x00040040},
	{"LPC1114FHN33/203",	0x00040042},
	{"LPC1114JHN33/203",	0x00040042},
	{"LPC1114FHN33/303",	0x00040040},
	{"LPC1114JHN33/303",	0x00040040},
	{"LPC1114FBD48/323",	0x00040060},
	{"LPC1114JBD48/323",	0x00040060},
	{"LPC1114FBD48/333",	0x00040070},
	{"LPC1114JBD48/333",	0x00040070},
	{"LPC1114FHN33/333",	0x00040070},
	{"LPC1114JHN33/333",	0x00040070},
	{"LPC1114FHI33/303",	0x00040040},
	{"LPC1114JHI33/303",	0x00040040},
	{"LPC11D14FBD100/302",	0x2540102B},
	{"LPC1115FBD48/303",	0x00050080},
	{"LPC1115JBD48/303",	0x00050080},
	{"LPC1115FET48/303",	0x00050080},
	{"LPC1115JET48/303",	0x00050080},
	{"LPC11C12FBD48/301",	0x1421102B},
	{"LPC11C14FBD48/301",	0x1440102B},
	{"LPC11C22FBD48/301",	0x1431102B},
	{"LPC11C24FBD48/301",	0x1430102B}
};


// ----------------------------------------------------------------------------------
//  Funktionen
// ----------------------------------------------------------------------------------

char *lpc_part_name(uint32_t part_id)
{
	for (int i = 0; i < sizeof(lpc_names) / sizeof(lpc_names[0]); i++)
		if (lpc_names[i].part_id == part_id)
			return lpc_names[i].name;

	return 0;
}

size_t lpc_next_sector_size(size_t size)
{
	if (size <= 256)
		return 256;
	else if (size <= 512)
		return 512;
	else if (size <= 1024)
		return 1024;
	else if (size <= 4096)
		return 4096;
	else
		return -1;
}

void lpc_prepare_sector(can_t *can, uint8_t start_sector, uint8_t end_sector)
{
	uint16_t sectors = ((end_sector << 8) | start_sector);
	sdo_download_exp(can, LPC_SDO_NODE_ID, LPC_SDO_PREP_SECTOR_IDX, 0x00, (uint8_t*)&sectors, 2);
}

void lpc_erase_sector(can_t *can, uint8_t start_sector, uint8_t end_sector)
{
	uint16_t sectors = ((end_sector << 8) | start_sector);

	// prepare and erase the sectors
	sdo_download_exp(can, LPC_SDO_NODE_ID, LPC_SDO_PREP_SECTOR_IDX, 0x00, (uint8_t*)&sectors, 2);
	sdo_download_exp(can, LPC_SDO_NODE_ID, LPC_SDO_ERASE_IDX, 0x00, (uint8_t*)&sectors, 2);
}

uint32_t lpc_ram_addr(can_t *can)
{
	uint32_t ram_addr;
	sdo_upload(can, LPC_SDO_NODE_ID, LPC_SDO_RAM_ADDR_IDX, 0x00, &ram_addr);
	
	return ram_addr;
}

uint32_t lpc_go(can_t *can, uint32_t addr)
{
	uint32_t err;
	uint8_t go = 1;

	// download the address we want to execute at
	err =  sdo_download_exp(can, LPC_SDO_NODE_ID, 0x5070, 0x01, (void*)&addr, 4);
	if (err != CAN_SUCCESS)
		return err;

	// execute the code
	err = sdo_download_exp(can, LPC_SDO_NODE_ID, 0x1F51, 0x01, &go, 1);
	if (err != CAN_SUCCESS)
		return err;

	return CAN_SUCCESS;
}