#ifndef SDO_H_
#define	SDO_H_

#include <linux/can.h>

#define SDO_NODE_OFFSET	600

typedef enum
{
	CCS_SEGMENT_DOWNLOAD,
	CCS_INIT_DOWNLAOD,
	CCS_INIT_UPLOAD,
	CCS_SEGMENT_UPLOAD,
	CCS_ABORT
} ccs_t;

typedef struct
{
	ccs_t ccs: 3;
	uint8_t n_bytes: 2;
	uint8_t expedited: 1;
	uint8_t size_set: 1;

	uint16_t node_id;
	uint16_t index;
	uint8_t sub_index;
	uint8_t data[4];
} sdo_message;

void sdo_can_frame(sdo_message *sdo, struct can_frame *cf)
{
	cf->can_id = SDO_NODE_OFFSET + sdo->node_id;
	cf->can_dlc = 8;

	memset(cf->data, 0, 8);
	cf->data[0] = sdo->size_set 
				| (sdo->expedited << 1)
				| (sdo->n_bytes << 2)
				| (sdo->ccs << 5);
	cf->data[1] = (sdo->index) & 0xFF
	cf->data[2] = (sdo->index >> 8) & 0xFF;
	cf->data[3] = sdo->sub_index;
	memcpy(&cf->data[4], sdo->data, 4);
}

#endif