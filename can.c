#include "can.h"
#include <errno.h>
#include <sys/socket.h>
#include <linux/can/raw.h>
#include <net/if.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>


// ----------------------------------------------------------------------------------
//  Konstanten
// ----------------------------------------------------------------------------------

#define MAX_BYTES_PER_FRAME	8


// ----------------------------------------------------------------------------------
//  Funktionen
// ----------------------------------------------------------------------------------

int can_init(can_t *can, const char *ifname)
{
	// create the socket
	can->socket = socket(PF_CAN, SOCK_RAW, CAN_RAW);
	if (can->socket < 0)
		return errno;

	// find the interface by name
	struct ifreq ifr;
	strcpy(ifr.ifr_name, ifname);
	if (ioctl(can->socket, SIOCGIFINDEX, &ifr) < 0)
		return errno;

	// address to bind the socket to
	struct sockaddr_can addr;
	addr.can_family = AF_CAN;
	addr.can_ifindex = ifr.ifr_ifindex;
	if (bind(can->socket, (struct sockaddr *)&addr, sizeof(addr)) < 0)
		return errno;

	return CAN_SUCCESS;
}

int can_send(const can_t *can, uint16_t can_id, void *buffer, size_t size)
{
	if (size > MAX_BYTES_PER_FRAME)
		return CAN_FAILURE;

	// build the can frame
	struct can_frame frame;
	frame.can_id = can_id;
	frame.can_dlc = size;
	memcpy(frame.data, buffer, size);

	// write the can frame to the interface
	size_t bytes_written = write(can->socket, &frame, sizeof(frame));

	// ensure that all bytes are sent
	return bytes_written == sizeof(frame);	
}

void can_read(const can_t *can, struct can_frame *frame)
{
	// TODO: implement a timeout as optional parameter
	read(can->socket, frame, sizeof(*frame));
}

int can_close(const can_t *can)
{
	return close(can->socket);
}