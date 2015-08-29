#ifndef UTIL_IO_
#define UTIL_IO_

#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>

uint8_t *io_read_file(char *name, size_t *size);

#endif