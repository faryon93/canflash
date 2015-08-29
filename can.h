#ifndef CAN_H_
#define CAN_H_

#include <stddef.h>
#include <stdint.h>
#include <linux/can.h>


// ----------------------------------------------------------------------------------
//  Konstanten
// ----------------------------------------------------------------------------------

#define CAN_SUCCESS	0
#define CAN_FAILURE	1


// ----------------------------------------------------------------------------------
//  Makros
// ----------------------------------------------------------------------------------

#define int(x) (*((int*)&x))
#define uint32_t(x) (*((uint32_t*)&x))


// ----------------------------------------------------------------------------------
//  Typen
// ----------------------------------------------------------------------------------

typedef struct
{
	int socket;
} can_t;


// ----------------------------------------------------------------------------------
//  Funktionen
// ----------------------------------------------------------------------------------

int can_init(can_t *can, const char *ifname);

int can_close(const can_t *can);

int can_send(const can_t *can, uint16_t can_id, void *buffer, size_t size);

void can_read(const can_t *can, struct can_frame *frame);

#endif