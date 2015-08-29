#include "sdo.h"
#include <string.h>
#include <stdio.h>


// ----------------------------------------------------------------------------------
//  Konstanten
// ----------------------------------------------------------------------------------

#define SEGMENT_MAX_PAYLOAD	7


// ----------------------------------------------------------------------------------
//  Funktionen
// ----------------------------------------------------------------------------------

uint32_t sdo_upload(can_t *can, uint16_t node_id, uint16_t index, uint8_t sub_index, void *data)
{
	// can payload for sdo upload request
	uint8_t frame[] =
	{
		0x40,
		index & 0xFF,
		(index >> 8) & 0xFF,
		sub_index,
		0, 0, 0, 0
	};
	if (!can_send(can, node_id, frame, 8))
		return CAN_FAILURE;

	// read the response
	struct can_frame response;
	can_read(can, &response);

	// an sdo error occoured
	if (response.data[0] == 0x80)
	{
		return uint32_t(response.data[4]);

	// fine -> copy payload to user buffer
	} else {
		memcpy(data, &response.data[4], 4);
		return CAN_SUCCESS;
	}
}

uint32_t sdo_download_exp(can_t *can, uint16_t node_id, uint16_t index, uint8_t sub_index, void *data, uint8_t size)
{
	// can payload for sdo upload request
	uint8_t frame[] =
	{
		0x20 | ((4 - size) << 2) | 0x03,
		index & 0xFF,
		(index >> 8) & 0xFF,
		sub_index,
		0, 0, 0, 0
	};

	// copy the user payload to the can frame
	// but 
	memcpy(&frame[4], data, size);
	if (!can_send(can, node_id, frame, 8))
		return CAN_FAILURE;

	// read the response
	struct can_frame response;
	can_read(can, &response);

	// check for errors from node
	return (response.data[0] == 0x80)
		? uint32_t(response.data[4])
		: CAN_SUCCESS;

}

uint32_t sdo_download_init(can_t *can, uint16_t node_id, uint16_t index, uint8_t sub_index)
{
	// can payload for sdo upload request
	uint8_t frame[] =
	{
		0x21,
		index & 0xFF,
		(index >> 8) & 0xFF,
		sub_index,
		0, 0, 0, 0
	};
	if (!can_send(can, node_id, frame, 8))
		return CAN_FAILURE;

	// read the response
	struct can_frame response;
	can_read(can, &response);

	// check for errors from node
	return (response.data[0] == 0x80)
		? uint32_t(response.data[4])
		: CAN_SUCCESS;

}

uint32_t sdo_download_seg(can_t *can, uint16_t node_id, void *data, size_t size, bool last)
{
	static uint8_t toggle = 0;

	// can payload for sdo upload request
	uint8_t frame[] =
	{
		((toggle & 1) << 4) | (((7 - size) & 0x7) << 1) | (last & 1),
		0, 0, 0, 0, 0, 0, 0
	};

	// copy user payload and send frame
	memcpy(&frame[1], data, SEGMENT_MAX_PAYLOAD);
	if (!can_send(can, node_id, frame, 8))
		return CAN_FAILURE;

	// flip the toggle bit
	toggle = (toggle == 0)
		? 1
		: 0;

	// read the response
	struct can_frame response;
	can_read(can, &response);

	// check for errors from node
	return (response.data[0] == 0x80)
		? uint32_t(response.data[4])
		: CAN_SUCCESS;
}

uint32_t sdo_download_buffer(can_t *can, uint16_t node_id, uint16_t index, uint8_t sub_index, uint8_t *data, size_t size, void(*update)(int, int))
{
	sdo_download_init(can, node_id, index, sub_index);

	const size_t rest_bytes = size % SEGMENT_MAX_PAYLOAD;
	const size_t full_frames = size / SEGMENT_MAX_PAYLOAD;

	// download all frames which are fully filled with data
	for (size_t i = 0; i < full_frames; i++)
	{
		// the frame is only the last frame if no packet has to be transmitted
		// which is smaller than 7 bytes
		uint8_t is_last_frame = (i == (full_frames - 1) && rest_bytes == 0)
			? 1
			: 0;

		sdo_download_seg(can, node_id, &data[i * SEGMENT_MAX_PAYLOAD], SEGMENT_MAX_PAYLOAD, is_last_frame);

		if (update != 0)
			update((rest_bytes == 0) ? full_frames : (full_frames + 1), i);
	}

	// the last frame is smaller
	if (rest_bytes != 0)
	{
		sdo_download_seg(can, node_id, &data[full_frames * SEGMENT_MAX_PAYLOAD], rest_bytes, 1);	
		update(full_frames + 1, full_frames + 1);
	}
}