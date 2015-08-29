#include <stdio.h>
#include <sys/socket.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>

#include "lpc.h"
#include "firmware.h"

#define int(x) (*((int*)x))
#define uint32_t(x) (*((uint32_t*)x))

int s = 0;

int can_init(const char *ifname)
{
	// create the socket
	s = socket(PF_CAN, SOCK_RAW, CAN_RAW);
	if (s < 0)
		return errno;

	// find the interface by name
	struct ifreq ifr;
	strcpy(ifr.ifr_name, ifname);
	if (ioctl(s, SIOCGIFINDEX, &ifr) < 0)
		return -1;

	// address to bind the socket to
	struct sockaddr_can addr;
	addr.can_family = AF_CAN;
	addr.can_ifindex = ifr.ifr_ifindex;

	if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		return errno;
	}
}

void can_close(void)
{
	close(s);
}

uint8_t sdo_upload(uint16_t node_id, uint16_t index, uint8_t sub_index, void *data)
{
	// send request
	struct can_frame cf;
	cf.can_id = 0x600 + node_id;
	cf.can_dlc = 8;
	cf.data[0] = 0x40;
	cf.data[1] = index & 0xff;
	cf.data[2] = (index >> 8) & 0xff;
	cf.data[3] = sub_index;
	cf.data[4] = 0x00;
	cf.data[5] = 0x00;
	cf.data[6] = 0x00;
	cf.data[7] = 0x00;
	write(s, &cf, sizeof(cf));

	// read the response
	read(s, &cf, sizeof(cf));
	memcpy(data, &cf.data[4], 4);

	if (cf.data[0] == 0x80)
	{
		printf("sdo error: %x\n", *((int*)&cf.data[4]));
		exit(-1);
	}
}

uint8_t sdo_download(uint16_t node_id, uint16_t index, uint8_t sub_index, uint8_t *data, uint8_t size)
{
	struct can_frame cf;
	cf.can_id = 0x600 + node_id;
	cf.can_dlc = 8;
	cf.data[0] = 0x20 | ((4 - size) << 2) | 0x03;
	cf.data[1] = index & 0xff;
	cf.data[2] = (index >> 8) & 0xff;
	cf.data[3] = sub_index;

	for (int i = 0; i < 4; i++)
	{
		cf.data[4 + i] = (i < size)
			? data[i]
			: 0x00;

	}
	write(s, &cf, sizeof(cf));

	// error check
	read(s, &cf, sizeof(cf));
	if (cf.data[0] == 0x80)
	{
		printf("sdo error: %x\n", *((int*)&cf.data[4]));
		exit(-1);
	}
}

void sdo_init_download(uint16_t node_id, uint16_t index, uint8_t sub_index)
{
	struct can_frame cf;
	cf.can_id = 0x600 + node_id;
	cf.can_dlc = 8;
	cf.data[0] = 0x21;
	cf.data[1] = index & 0xff;
	cf.data[2] = (index >> 8) & 0xff;
	cf.data[3] = sub_index;	
	memset(&cf.data[4], 0, 4);
	write(s, &cf, sizeof(cf));

	// error check
	read(s, &cf, sizeof(cf));
	if (cf.data[0] == 0x80)
	{
		printf("sdo error: %x\n", *((int*)&cf.data[4]));
		exit(-1);
	}
}

void sdo_download_seg(uint16_t node_id, uint8_t *data, size_t size, uint8_t last)
{
	static uint8_t toggle = 0;

	struct can_frame cf;
	cf.can_id = 0x600 + node_id;
	cf.can_dlc = 8;
	cf.data[0] = ((toggle & 1) << 4) | (((7 - size) & 0x7) << 1) | (last & 1);
	for (int i = 0; i < 7; i++)
	{
		cf.data[i + 1] = (i < size)
			? data[i]
			: 0;
	}
	write(s, &cf, sizeof(cf));

	toggle = (toggle == 0)
		? 1
		: 0;

	// error check
	read(s, &cf, sizeof(cf));
	if (cf.data[0] == 0x80)
	{
		printf("sdo error: %x\n", *((int*)&cf.data[4]));
		exit(-1);
	}
}


#define MAX_BYTES_PER_FRAME	7

void sdo_download_buffer(uint16_t node_id, uint16_t index, uint8_t sub_index, uint8_t *data, size_t size)
{
	sdo_init_download(0x7D, index, sub_index);

	const size_t rest_bytes = size % MAX_BYTES_PER_FRAME;
	const size_t full_frames = size / MAX_BYTES_PER_FRAME;

	// download all frames which are fully filled with data
	for (size_t i = 0; i < full_frames; i++)
	{
		// the frame is only the last frame if no packet has to be transmitted
		// which is smaller than 7 bytes
		uint8_t is_last_frame = (i == (full_frames - 1) && rest_bytes == 0)
			? 1
			: 0;

		sdo_download_seg(0x7D, &data[i * MAX_BYTES_PER_FRAME], MAX_BYTES_PER_FRAME, is_last_frame);

		printf("\r\tdownload to uC ram: [%04d]", i*7);
	}

	// the last frame is smaller
	if (rest_bytes != 0)
	{
		sdo_download_seg(0x7D, &data[full_frames * MAX_BYTES_PER_FRAME], rest_bytes, 1);	
	}

	printf("\r\tdownload to uC ram: [%04d]\n", size);
}

void lpc_prepare_sector(uint8_t start_sector, uint8_t end_sector)
{
	uint16_t sectors = ((end_sector << 8) | start_sector);
	sdo_download(0x7D, LPC_SDO_PREP_SECTOR_IDX, 0x00, (uint8_t*)&sectors, 2);
}

void lpc_erase_sector(uint8_t start_sector, uint8_t end_sector)
{
	uint16_t sectors = ((end_sector << 8) | start_sector);

	// prepare and erase the sectors
	sdo_download(0x7D, LPC_SDO_PREP_SECTOR_IDX, 0x00, (uint8_t*)&sectors, 2);
	sdo_download(0x7D, LPC_SDO_ERASE_IDX, 0x00, (uint8_t*)&sectors, 2);
}

uint32_t lpc_ram_addr()
{
	uint32_t ram_addr;
	sdo_upload(0x7D, LPC_SDO_RAM_ADDR_IDX, 0x00, &ram_addr);
	
	return ram_addr;
}

int main(int argc, char *argv[])
{
	// command line arguments
	if (argc < 3)
	{
		printf("invalid arguments: ./lpc_can_flash <interface> <filename>\n");
		exit(-1);
	}

	// load the firmware file
	size_t sector_count;
	sector_t *sectors = firmware_open(argv[2], LPC_11C24_SECTOR_SIZE, &sector_count);
	printf("loaded firmware file \"%s\"\n\n", argv[2]);

	// print the anatomy of the firmware file and stats
	printf("------------------------------------------------------------\n");
	printf("firmware file analysis:\n");
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
		printf("error: firmware does not fit into microprocessor\n");
		exit(-1);
	}


	// start the download process
	printf("\n------------------------------------------------------------\n");
	printf("firmware download processs\n");
	printf("------------------------------------------------------------\n");

	// initialize the can interface
	printf("opening CAN interface: ...");
	can_init(argv[1]);
	printf("\ropening CAN interface: done\n");

	// verify the lpc processors part id
	printf("verifying lpc part id: ...");
	uint32_t lpc_part_id;
	sdo_upload(0x7D, LPC_SDO_IDENTITY_IDX, LPC_SDO_PARTID_SUBIDX, &lpc_part_id);
	if (lpc_part_id == 0x1430102B)
	{
		printf("\rverifying lpc part id: [OK] %s\n", lpc_part_name(lpc_part_id));
	} else {
		printf("\rverifying lpc part id: [FAILED] %s\n", lpc_part_name(lpc_part_id));
		exit(-1);
	}

	// send unlock code
	printf("sending unlock code: ...");
	uint16_t code = LPC_UNLOCK_CODE;
	sdo_download(0x7D, LPC_SDO_UNLOCK_IDX, LPC_SDO_UNLOCK_SUBIDX, (uint8_t*)&code, 2);
	printf("\rsending unlock code: done\n");

	// erase all sectors
	printf("erasing sector 0-7: ...");
	lpc_erase_sector(0, 7);
	printf("\rerasing sector 0-7: done\n\n");

	// gather some information we need later
	const uint32_t ram_start_addr = lpc_ram_addr();

	// write the firmware sector by sector to the uC
	for (int i = 0; i < sector_count; i++)
	{
		printf("Processing sector %d\n", i);
		printf("\tdownload to uC ram: [----]");
		uint16_t bytes_to_copy = 0;
	
		// download the sectors content
		sdo_download_buffer(0x7D, 0x1F50, 0x01, sectors[i].binary, sectors[i].size);

		// this is a full sector -> straight-forward copy
		if (sectors[i].size == LPC_11C24_SECTOR_SIZE)
		{
			bytes_to_copy = sectors[i].size;
		
		// not a full sector, we have to fill with zeroes util next 2^n size
		} else {
			const size_t next_block_size = pow(2, ceil(log(sectors[i].size)/log(2)));
			size_t zero_count = next_block_size - sectors[i].size;
			uint8_t zeroes[zero_count];
			memset(zeroes, 0xFF, zero_count);
			sdo_download_buffer(0x7D, 0x1F50, 0x01, zeroes, zero_count);
			bytes_to_copy = next_block_size;
		}

		// prepare the sector and copy to flash
		printf("\twriting ram to flash: ...");
		lpc_prepare_sector(i, i);
		sdo_download(0x7D, 0x5050, 0x03, (uint8_t*)&bytes_to_copy, 2);
		printf("\r\twriting ram to flash: done\n");

		// reset ram address and set next flash addr
		sdo_download(0x7D, LPC_SDO_RAM_ADDR_IDX, 0x00, (uint8_t*)&ram_start_addr, 4);
		uint32_t next_flash_addr = (i + 1) * LPC_11C24_SECTOR_SIZE;
		sdo_download(0x7D, 0x5050, 0x01, (uint8_t*)&next_flash_addr, 4);

		printf("\n");
	}

	// close
	firmware_free(sectors);
	can_close();
	return 0;
}
