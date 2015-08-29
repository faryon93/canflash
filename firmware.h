#ifndef FIRMWARE_H_
#define FIRMWARE_H_

#include <stdint.h>
#include <stddef.h>


// ----------------------------------------------------------------------------------
//  Typen
// ----------------------------------------------------------------------------------

typedef struct
{
	uint8_t *binary;
	size_t size;
} sector_t;


// ----------------------------------------------------------------------------------
//  Funktionen
// ----------------------------------------------------------------------------------

sector_t *firmware_open(char *path, uint32_t sector_size, size_t *sector_count);

void firmware_free(sector_t *sectors);

#endif
