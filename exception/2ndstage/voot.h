#ifndef __VOOT_H__
#define __VOOT_H__

#include "vars.h"
#include "net.h"

void voot_handle_packet(ether_info_packet_t *frame, udp_header_t *udp, uint16 udp_data_length);

#endif
