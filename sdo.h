#ifndef SDO_H_
#define	SDO_H_

#include <stdint.h>
#include <stdbool.h>
#include "can.h"


// ----------------------------------------------------------------------------------
//  Konstanten
// ----------------------------------------------------------------------------------


// ----------------------------------------------------------------------------------
//  Funktionen
// ----------------------------------------------------------------------------------

uint32_t sdo_upload(can_t *can, uint16_t node_id, uint16_t index, uint8_t sub_index, void *data);

uint32_t sdo_download_exp(can_t *can, uint16_t node_id, uint16_t index, uint8_t sub_index, void *data, uint8_t size);

uint32_t sdo_download_init(can_t *can, uint16_t node_id, uint16_t index, uint8_t sub_index);
uint32_t sdo_download_seg(can_t *can, uint16_t node_id, void *data, size_t size, bool last);

uint32_t sdo_download_buffer(can_t *can, uint16_t node_id, uint16_t index, uint8_t sub_index, uint8_t *data, size_t size, void(*update)(int, int));

#endif