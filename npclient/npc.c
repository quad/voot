/*  npc.c

DESCRIPTION

    NetPlayCommand system. This is a prototype for a interface the netplay
    VO system for reliability clients.

CHANGELOG

    Wed Sep 19 23:44:40 PDT 2001    Scott Robinson <scott_np@dsn.itgo.com>
        Imported code from original npclient module.

    Sun Nov 25 23:32:04 PST 2001    Scott Robinson <scott_np@dsn.itgo.com>
        Start writing slave interface logic.

TODO

    Create the socket interface logic.
    Remove console system assumptions. All IO logic should be handled by the client itself.

*/

#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>

#include "voot.h"
#include "npc.h"

#define NPC_DEBUG   1

npc_data_t  npc_system;

unsigned int handle_npc_command(npc_command_t *command)
{
    unsigned int retval;

    retval = 0;

    switch (command->type)
    {
        case C_SET_SLAVE_PORT:
            npc_system.slave_port = command->port;
            retval = npc_system.slave_port;

            #ifdef NPC_DEBUG
                fprintf(stderr, "[npc] set slave port to %u.\n", npc_system.slave_port);
            #endif
            break;

        case C_SET_SERVER_PORT:
            npc_system.server_port = command->port;
            retval = npc_system.server_port;

            #ifdef NPC_DEBUG
                fprintf(stderr, "[npc] set server port to %u.\n", npc_system.server_port);
            #endif
            break;

        case C_CONNECT_SLAVE:
            npc_system.slave_name = command->text;

            retval = npc_slave_connect();
            
            #ifdef NPC_DEBUG
                if (retval)
                    fprintf(stderr, "[npc] unable to connect to slave %s:%u.\n", npc_system.slave_name, npc_system.slave_port);
            #endif
            break;

        case C_CONNECT_SERVER:
            npc_system.server_name = command->text;

            retval = npc_server_connect();

            #ifdef NPC_DEBUG
                if (retval)
                    fprintf(stderr, "[npc] unable to connect to server %s:%u.\n", npc_system.server_name, npc_system.server_port);
            #endif
            break;

        case C_LISTEN_SERVER:
            retval = npc_server_listen();

            #ifdef NPC_DEBUG
                if (retval)
                    fprintf(stderr, "[npc] unable to start server on port %u.\n", npc_system.server_port);
            #endif
            break;

        case C_PACKET_FROM_SLAVE:
            switch (command->packet->header.type)
            {
                case VOOT_PACKET_TYPE_DEBUG:
                    fprintf(stderr, "[npc] DEBUG: %s", command->packet->buffer);
                    break;
            
                default:
                    break;
            }
            
            free(command->packet);
            break;

        case C_EXIT:
            npc_exit(command->code);
            break;

        case C_NONE:    /* Don't even notify of gutter commands. */
            break;

        default:
            fprintf(stderr, "[npc] dropping unimplemented npc_command %u.\n", command->type);
            break;
    }

    return retval;
}

npc_command_t* npc_get_event(void)
{
    npc_command_t *event;

    if ((event = npc_io_check(npc_system.slave_socket, C_PACKET_FROM_SLAVE)));
    else if ((event = npc_io_check(npc_system.server_socket, C_PACKET_FROM_SERVER)));
    else if (!event)
    {
        event = (npc_command_t *) malloc(sizeof(npc_command_t));
        event->type = C_NONE;
    }

    return event;
}

npc_command_t* npc_io_check(int32 socket, npc_command type)
{
    fd_set read_fds;
    voot_packet *packet;
    npc_command_t *event;

    if (socket < 0)
        return NULL;

    /* Check if we have received data on the socket. */
    FD_ZERO(&read_fds);
    FD_SET(socket, &read_fds);
    if (select(socket + 1, &read_fds, NULL, NULL, 0) < 0)
        return NULL;

    packet = voot_parse_socket(socket);

    if (!packet)
        return NULL;

    event = (npc_command_t *) malloc(sizeof(npc_command_t));
    event->type = type;
    event->packet = packet;

    return event;
}

int npc_slave_connect(void)
{
    struct hostent *host;
    struct sockaddr_in slave_address, my_address;
    int slave_socket;

    bzero(&slave_address, sizeof(slave_address));
    bzero(&my_address, sizeof(my_address));

    /* Try to resolve the slave host name. */
    host = gethostbyname(npc_system.slave_name);
    if (!host)
    {
        fprintf(stderr, "[npc] unable to resolve slave %s.\n", npc_system.slave_name);
        return 1;
    }

    slave_address.sin_family = host->h_addrtype;
    memcpy((char *) &slave_address.sin_addr.s_addr, host->h_addr_list[0], host->h_length);
    slave_address.sin_port = htons(npc_system.slave_port);

    /* Open a UDP socket and bind to any port. */
    slave_socket = socket(AF_INET, SOCK_DGRAM, 0);  /* ??? I'm using the 'ip' protocol. Shouldn't I be using the 'udp' protocol? */
    if (slave_socket < 0)
    {
        fprintf(stderr, "[npc] unable to open a socket for the slave.\n");
        return 2;
    }

    my_address.sin_family = slave_address.sin_family;
    my_address.sin_addr.s_addr = htonl(INADDR_ANY);
    my_address.sin_port = htons(0);

    if (!bind(slave_socket, (struct sockaddr *) &my_address, sizeof(my_address)) < 0)
    {
        close(slave_socket);
        fprintf(stderr, "[npc] unable to bind a socket for the slave.\n");
        return 3;
    }

    /* FIXME: !!! Send version command to slave. */
    {
        char v_string[] = "c  v";

        sendto(slave_socket, v_string, strlen(v_string), 0, (struct sockaddr *) &slave_address, sizeof(slave_address));
    }

    /* Update npc_system with socket and addressing information. */
    npc_system.slave_socket = slave_socket;
    npc_system.slave_address = slave_address;

    return 0;
}

int npc_server_connect(void)
{
    return 1;
}

int npc_server_listen(void)
{
    int server_socket;
    struct sockaddr_in my_address;

    bzero(&my_address, sizeof(my_address));

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0)
    {
        fprintf(stderr, "[npc] unable to open a socket for serving.\n");
        return 1;
    }

    my_address.sin_family = AF_INET;
    my_address.sin_addr.s_addr = htonl(INADDR_ANY);
    my_address.sin_port = htons(npc_system.server_port);

    if (!bind(server_socket, (struct sockaddr *) &my_address, sizeof(my_address)) < 0)
    {
        close(server_socket);
        fprintf(stderr, "[npc] unable to bind a socket for serving.\n");
        return 2;
    }

    if (listen(server_socket, 1))
    {
        close(server_socket);
        fprintf(stderr, "[npc] unable to listen on port %u for serving.\n", npc_system.server_port);
        return 3;
    }

    /* Update npc_system with socket and addressing information. */
    npc_system.server_socket_wait = server_socket;

    return 0;
}

void npc_exit(int code)
{
    /* Clean up the sockets, if necessary. */
    if (npc_system.slave_socket >= 0)
    {
        if (close(npc_system.slave_socket))
            fprintf(stderr, "[npc] unable to close slave socket.\n");
        else
            fprintf(stderr, "[npc] closed slave socket.\n");
    }
}

void npc_init(void)
{
    npc_command_t command;

    memset(&npc_system, 0x0, sizeof(npc_system));
    npc_system.slave_socket = npc_system.server_socket = npc_system.server_socket_wait = -1;

    command.type = C_SET_SLAVE_PORT;
    command.port = VOOT_SLAVE_PORT;
    handle_npc_command(&command);

    command.type = C_SET_SERVER_PORT;
    command.port = VOOT_SERVER_PORT;
    handle_npc_command(&command);
}

npc_data_t* npc_expose(void)
{
    /* We don't really ever want this to be done. If there is an outside
        function needs access to one of the internal data types, hopefully
        we'll have created an accessor function for it. However, I'll leave
        the functionality in just-in-case. */

    fprintf(stderr, "[npc] npc_system exposed!\n");

    return &npc_system;
}
