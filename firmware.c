#include "firmware.h"
#include "util/io.h"


// ----------------------------------------------------------------------------------
//  Funktionen
// ----------------------------------------------------------------------------------

sector_t *firmware_open(char *path, uint32_t sector_size, size_t *sector_count)
{
	// load the firmware from the filesystem
	size_t firmware_size;
	uint8_t *firmware = io_read_file(path, &firmware_size);	
	if (firmware == 0)
		return 0;

	// compute the count of full sectors, and the amount of rest bytes
	const size_t full_sector_count = firmware_size / sector_size;
	const size_t rest_bytes = firmware_size % sector_size;

	// allocate a new sector_t array
	size_t array_size = full_sector_count;
	if (rest_bytes > 0)
		array_size += 1;
	sector_t *sectors = (sector_t*)malloc((array_size) * sizeof(sector_t));

	// add the full sectors to the array
	for (int i = 0; i < full_sector_count; i++)
	{
		sectors[i].binary = &firmware[i * sector_size];
		sectors[i].size = sector_size;
	}

	// if a rest sector is issued by the firmware file add it
	if (rest_bytes > 0)
	{
		sectors[full_sector_count].binary = &firmware[full_sector_count * sector_size];
		sectors[full_sector_count].size = rest_bytes;
	}

	// return the result
	*sector_count = array_size;
	return sectors;
}

void firmware_free(sector_t *sectors)
{
	free(sectors[0].binary);
	free(sectors);
}
