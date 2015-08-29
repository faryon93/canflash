#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "sdo.h"
#include "lpc.h"
#include "firmware.h"
#include "progressbar.h"

can_t can;
size_t current_sector = 0;
char *current_action = "downloading to ram";

void progress_print(int max, int current)
{
	char tmp[50];
	sprintf(tmp, "sector %d", current_sector);
	print_progress(max, current, tmp, current_action);
}

int main(int argc, char *argv[])
{
	uint32_t err;

	// command line arguments
	if (argc < 3)
	{
		printf("invalid arguments: ./lpc_can_flash <interface> <filename>\n");
		exit(-1);
	}

	// load the firmware file
	size_t sector_count;
	sector_t *sectors = firmware_open(argv[2], LPC_11C24_SECTOR_SIZE, &sector_count);

	// print the anatomy of the firmware file and stats
	printf("------------------------------------------------------------\n");
	printf("firmware file %s:\n", argv[2]);
	printf("------------------------------------------------------------\n");
	printf("file anatomy:\n");
	size_t firmware_size = 0;
	for (int i = 0; i < sector_count; i++)
	{
		printf("\tsector %d, size: %d, addr: 0x%04x - 0x%04x\n", i, sectors[i].size, i*LPC_11C24_SECTOR_SIZE, (i*LPC_11C24_SECTOR_SIZE + sectors[i].size - 1));
		firmware_size += sectors[i].size;
	}
	printf("used space: %d bytes, free space: %d byte\n", firmware_size, (LPC_FLASH_SIZE - firmware_size));

	// check firmware plausibility
	if (firmware_size <= LPC_FLASH_SIZE && sector_count <= LPC_11C24_SECTOR_COUT)
	{
		printf("firmware ok, download is possible\n");

	} else {
		printf("error: firmware does not fit into flash\n");
		exit(-1);
	}


	// start the download process
	printf("\n------------------------------------------------------------\n");
	printf("firmware download:\n");
	printf("------------------------------------------------------------\n");

	// initialize the can interface
	printf("opening CAN interface: ...");
	can_init(&can, argv[1]);
	printf("\ropening CAN interface: done\n");

	// verify the lpc processors part id
	printf("verifying lpc part id: ...");
	uint32_t lpc_part_id = 0;
	err = sdo_upload(&can, LPC_SDO_NODE_ID, LPC_SDO_IDENTITY_IDX, LPC_SDO_PARTID_SUBIDX, &lpc_part_id);
	if (lpc_part_id == 0x1430102B)
	{
		printf("\rverifying lpc part id: [OK] %s\n", lpc_part_name(lpc_part_id));
	} else {
		printf("\rverifying lpc part id: [FAILED] %s\n", lpc_part_name(lpc_part_id));
		printf("err: %d, %x\n", err, lpc_part_id);
		exit(-1);
	}

	// send unlock code
	printf("sending unlock code: ...");
	uint16_t code = LPC_UNLOCK_CODE;
	sdo_download_exp(&can, LPC_SDO_NODE_ID, LPC_SDO_UNLOCK_IDX, LPC_SDO_UNLOCK_SUBIDX, (uint8_t*)&code, 2);
	printf("\rsending unlock code: done\n");

	// erase all sectors
	printf("erasing sector 0-7: ...");
	lpc_erase_sector(&can, 0, 7);
	printf("\rerasing sector 0-7: done\n\n");

	// gather some information we need later
	const uint32_t ram_start_addr = lpc_ram_addr(&can);

	// write the firmware sector by sector to the uC
	for (int i = 0; i < sector_count; i++)
	{
		// number of bytes to write to flash
		uint16_t bytes_to_copy = 0;
	
		// download the sectors content
		err = sdo_download_buffer(&can, LPC_SDO_NODE_ID, 0x1F50, 0x01, sectors[i].binary, sectors[i].size, progress_print);
		if (err != CAN_SUCCESS)
		{
			printf("\nerror in download_sector_payload: 0x%x\n", err);
			exit(-1);
		}

		// this is a full sector -> straight-forward copy
		if (sectors[i].size == LPC_11C24_SECTOR_SIZE)
		{
			bytes_to_copy = sectors[i].size;
		
		// not a full sector, we have to fill with zeroes util next 2^n size
		} else {
			current_action = "filling unused ram";

			// calculate the next matching block size and fill with 0xFF
			const size_t next_block_size = lpc_next_sector_size(sectors[i].size);
			size_t zero_count = next_block_size - sectors[i].size;
			uint8_t zeroes[zero_count];
			memset(zeroes, 0xFF, zero_count);
			err = sdo_download_buffer(&can, LPC_SDO_NODE_ID, 0x1F50, 0x01, zeroes, zero_count, progress_print);
			if (err != CAN_SUCCESS) {
				printf("\nerror in download_zeroes: 0x%x\n", err);
				exit(-1);
			}
			bytes_to_copy = next_block_size;
		}

		// prepare the sector and copy to flash
		char tmp[50];
		sprintf(tmp, "sector %d", current_sector);
		print_progress(1, 1, tmp, "copy to flash");
		lpc_prepare_sector(&can, i, i);
		err = sdo_download_exp(&can, LPC_SDO_NODE_ID, 0x5050, 0x03, (uint8_t*)&bytes_to_copy, 2);
		if (err != CAN_SUCCESS)
		{
			printf("\nerror in copy_to_flash: 0x%x [count: %d]\n", err, bytes_to_copy);
			exit(-1);
		}

		// reset ram address and set next flash addr
		sdo_download_exp(&can, LPC_SDO_NODE_ID, LPC_SDO_RAM_ADDR_IDX, 0x00, (uint8_t*)&ram_start_addr, 4);
		uint32_t next_flash_addr = (i + 1) * LPC_11C24_SECTOR_SIZE;
		err = sdo_download_exp(&can, LPC_SDO_NODE_ID, 0x5050, 0x01, (uint8_t*)&next_flash_addr, 4);
		if (err != CAN_SUCCESS)
		{
			printf("\nerror in rest_ram_addr: 0x%x\n", err);
			exit(-1);
		}

		// increment the sector number for the progressbar
		current_sector++;
		current_action = "downloading to ram";
		printf("\n");
	}

	printf("firmware download finished\n");

	// execute user code
	printf("executing user application at 0x00000004: ...");
	uint8_t go = 1;
	uint32_t go_address = 0x00000004;
	sdo_download_exp(&can, LPC_SDO_NODE_ID, 0x5070, 0x01, &go_address, 4);
	sdo_download_exp(&can, LPC_SDO_NODE_ID, 0x1F51, 0x01, &go, 1);
	printf("\rexecuting user application at 0x00000004: done\n");

	// close
	firmware_free(sectors);
	can_close(&can);
	return 0;
}