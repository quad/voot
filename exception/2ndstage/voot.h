#ifndef __VOOT_H__
#define __VOOT_H__

#include "vars.h"
#include "net.h"

/*

Total Guesstimated System Memory:
    8CCF9C00 - 8CCFAFFF (0x13ff / 5119d)

Initial Focus:
    8CCF9EE0 - 8CCF9EFF

*/

#define VOOT_MEM_START      0x8CCF9ECC
#define VOOT_MEM_END        0x8CCFA2CC

void voot_handle_packet(ether_info_packet_t *frame, udp_header_t *udp, uint16 udp_data_length);

#endif
