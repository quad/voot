/*  voot.c

DESCRIPTION

    VOOT Netplay protocol parser and constructor. It uses file handles and
    memory structures.

CHANGELOG

    Sun Jan  6 19:14:48 PST 2002    Scott Robinson <scott_np@dsn.itgo.com>
        Added a changelog. I figure I should do some cleaning up so when I
        write a final version some day.

    Tue Jan 22 00:31:19 PST 2002    Scott Robinson <scott_np@dsn.itgo.com>
        Changed the recvfrom() to recv(). I think it made a difference in
        the win32 port.

*/

#include <stdlib.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include "voot.h"

static bool voot_check_packet(voot_packet *packet, uint32 size)
{
    /* See if there is as much data as advertised. */
    return ntohs(packet->header.size) == size - sizeof(voot_packet_header);
}

uint32 voot_check_packet_advsize(voot_packet *packet, uint32 size)
{
    uint32 advsize;

    if (size < sizeof(voot_packet_header))
        return 0;

    return ((advsize = (sizeof(voot_packet_header) + ntohs(packet->header.size))) > size) ? size : advsize;
}

voot_packet* voot_parse_buffer(unsigned char *buffer, unsigned int buffer_size)
{
    voot_packet *packet;

    if (!voot_check_packet((voot_packet *) buffer, buffer_size))
        return NULL;

    packet = malloc(sizeof(voot_packet));
    memcpy(packet, buffer, buffer_size);

    return packet;
}

voot_packet* voot_parse_socket(int32 socket)
{
    unsigned char data[BIUDP_SEGMENT_SIZE];
    int rx;

    rx = recv(socket, data, sizeof(data), 0);

    return voot_parse_buffer(data, rx);
}    

int32 voot_send_packet(int32 socket, voot_packet *packet, uint32 size)
{
    if (socket < 0)
        return FALSE;
    else if (!voot_check_packet(packet, size))
        return FALSE;

    return send(socket, packet, size, 0);
}

int32 voot_send_command(int32 socket, uint8 command)
{
    int32 retval;
    voot_packet *packet;

    packet = malloc(sizeof(voot_packet));

    packet->header.type = VOOT_PACKET_TYPE_COMMAND;
    packet->header.size = htons(1);
    packet->buffer[0] = command;

    retval = voot_send_packet(socket, packet, voot_check_packet_advsize(packet, sizeof(voot_packet)));

    free(packet);
    
    return retval;
}
