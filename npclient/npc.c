/*  npc.c

DESCRIPTION

    NetPlayCommand system. This is a prototype for a interface the netplay
    VO system for reliability clients.

CHANGELOG

    Wed Sep 19 23:44:40 PDT 2001    Scott Robinson <scott_np@dsn.itgo.com>
        Imported code from original npclient module.

    Sun Nov 25 23:32:04 PST 2001    Scott Robinson <scott_np@dsn.itgo.com>
        Start writing slave interface logic.

    Thu Nov 29 00:39:23 PST 2001    Scott Robinson <scott_np@dsn.itgo.com>
        Finished initial version of all event logic. Need to add VOOT
        protocol linkages and accept() thread.

    Fri Nov 30 10:39:17 PST 2001    Scott Robinson <scott_np@dsn.itgo.com>
        Improved the stability of socket relations. More specificially, we
        can handle disconnected sockets.

    Fri Jan 11 00:39:32 PST 2002    Scott Robinson <scott_np@dsn.itgo.com>
        Added a full event queue and started attaching events into the
        queue. Now we can pass cleanly between sockets?

    Mon Jan 21 16:24:12 PST 2002    Scott Robinson <scott_np@dsn.itgo.com>
        We rely completely on the event queue now. Now directly passing
        packets between server and slave sockets via VOOT protocol library.

    Mon Jan 21 18:50:59 PST 2002    Scott Robinson <scott_np@dsn.itgo.com>
        Threadify the socket polling.

    Mon Jan 21 20:03:46 PST 2002    Scott Robinson <scott_np@dsn.itgo.com>
        Added proper pthread mutexing to the event queue.

TODO

    Remove console system assumptions. All IO logic should be handled by the client itself.

*/

#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

#include "voot.h"
#include "npc.h"

#define NPC_DEBUG    1

npc_data_t  npc_system;

int32 handle_npc_command(npc_command_t *command)
{
    int32 retval;

    retval = 0;

    switch (command->type)
    {
        case C_SET_SLAVE_PORT:
            npc_system.slave_port = command->port;
            retval = npc_system.slave_port;

            fprintf(stderr, "[npc] set slave port to %u.\n", npc_system.slave_port);
            break;

        case C_SET_SERVER_PORT:
            npc_system.server_port = command->port;
            retval = npc_system.server_port;

            fprintf(stderr, "[npc] set server port to %u.\n", npc_system.server_port);
            break;

        case C_CONNECT_SLAVE:
            npc_system.slave_name = command->text;

            retval = npc_connect(npc_system.slave_name, npc_system.slave_port, SOCK_DGRAM);

            if (retval >= 0)
            {
                npc_command_t *event;

                npc_system.slave_socket = retval;

                event = (npc_command_t *) malloc(sizeof(npc_command_t));
                event->type = C_LISTEN_SOCKET;
                event->listen_type = C_PACKET_FROM_SLAVE;
                event->listen_socket = &(npc_system.slave_socket);
                event->listen_socket_thread = &(npc_system.slave_poll_thread);

                npc_add_event_queue(event);
            }
            else
                fprintf(stderr, "[npc] unable to connect to slave %s:%u.\n", npc_system.slave_name, npc_system.slave_port);
            break;

        case C_CONNECT_SERVER:
            npc_system.server_name = command->text;

            retval = npc_connect(npc_system.server_name, npc_system.server_port, SOCK_STREAM);

            if (retval >= 0)
            {
                npc_command_t *event;
                npc_system.server_socket = retval;

                event = (npc_command_t *) malloc(sizeof(npc_command_t));
                event->type = C_LISTEN_SOCKET;
                event->listen_type = C_PACKET_FROM_SERVER;
                event->listen_socket = &(npc_system.server_socket);
                event->listen_socket_thread = &(npc_system.server_poll_thread);

                npc_add_event_queue(event);
            }
            else
                fprintf(stderr, "[npc] unable to connect to server %s:%u.\n", npc_system.server_name, npc_system.server_port);
            break;

        case C_LISTEN_SOCKET:
            {
                npc_io_check_t  *arg;

                /* Start a listening thread for the server socket. */
                arg = (npc_io_check_t *) malloc(sizeof(npc_io_check_t));
                arg->socket = command->listen_socket;
                arg->type = command->listen_type;

                if (pthread_create(command->listen_socket_thread, NULL, npc_io_check, (void *) arg))
                {
                    close(*(command->listen_socket));
                    fprintf(stderr, "[npc] unable to start polling thread for type %d.\n", command->type);
                    *(command->listen_socket) = -1;
                }
            }
            break;

        case C_LISTEN_SERVER:
            retval = npc_server_listen();

            if (retval)
                fprintf(stderr, "[npc] unable to start server on port %u.\n", npc_system.server_port);
            break;

        case C_PACKET_FROM_SLAVE:
            switch (command->packet->header.type)
            {
                case VOOT_PACKET_TYPE_DEBUG:
                    fprintf(stderr, "[npc] DEBUG(slave): %s", command->packet->buffer);
                    break;

                case VOOT_PACKET_TYPE_DATA:
                    fprintf(stderr, "[npc] DATA(slave): '%c'... [passing]\n", command->packet->buffer[0]);
                    voot_send_packet(npc_system.server_socket, command->packet, voot_check_packet_advsize(command->packet, sizeof(voot_packet)));
                    break;
            
                default:
                    break;
            }
            
            free(command->packet);
            break;

        case C_CLOSE_SLAVE:
            fprintf(stderr, "[npc] received request to disconnect slave socket! Is this even possible?!\n");
            break;

        case C_PACKET_FROM_SERVER:
            switch (command->packet->header.type)
            {
                case VOOT_PACKET_TYPE_DEBUG:
                    fprintf(stderr, "[npc] DEBUG(server): %s", command->packet->buffer);
                    break;

                case VOOT_PACKET_TYPE_COMMAND:
                    fprintf(stderr, "[npc] COMMAND(server): '%c'... [passing]\n", command->packet->buffer[0]);
                    voot_send_packet(npc_system.slave_socket, command->packet, voot_check_packet_advsize(command->packet, sizeof(voot_packet)));
                    break;

                case VOOT_PACKET_TYPE_DATA:
                    fprintf(stderr, "[npc] DATA(server): '%c'... [passing]\n", command->packet->buffer[0]);
                    voot_send_packet(npc_system.slave_socket, command->packet, voot_check_packet_advsize(command->packet, sizeof(voot_packet)));
                    break;
            
                default:
                    break;
            }

            free(command->packet);
            break;

        case C_CLOSE_SERVER:
            fprintf(stderr, "[npc] closing server socket.\n");

            close(npc_system.server_socket);
            npc_system.server_socket = -1;

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

bool npc_add_event_queue(npc_command_t *command)
{
    if (npc_system.event_queue_size >= NPC_EVENT_QUEUE_SIZE)
    {
        fprintf(stderr, "[npc] Event queue overflow!\n");
        return TRUE;
    }
    else if(pthread_mutex_lock(&(npc_system.event_queue_busy)));

    npc_system.event_queue[(npc_system.event_queue_tail + npc_system.event_queue_size) % NPC_EVENT_QUEUE_SIZE] = command;
    npc_system.event_queue_size++;

    pthread_mutex_unlock(&(npc_system.event_queue_busy));

    return FALSE;
}

npc_command_t* npc_get_event_queue(void)
{
    npc_command_t *command;

    if (!npc_system.event_queue_size)
        return NULL;
    else if(pthread_mutex_lock(&(npc_system.event_queue_busy)));

    command = npc_system.event_queue[npc_system.event_queue_tail];

    npc_system.event_queue_tail = ++npc_system.event_queue_tail % NPC_EVENT_QUEUE_SIZE;
    npc_system.event_queue_size--;

    pthread_mutex_unlock(&(npc_system.event_queue_busy));

    return command;
}

npc_command_t* npc_get_event(void)
{
    npc_command_t *event;

    if ((event = npc_get_event_queue()));
    else if (!event)
    {
        event = (npc_command_t *) malloc(sizeof(npc_command_t));
        event->type = C_NONE;
    }

    return event;
}

void* npc_io_check(void *in_arg)
{
    npc_io_check_t *arg;
    volatile int32 *socket;
    npc_command type;
    fd_set read_fds;
    voot_packet *packet;
    npc_command_t *event;
    bool poll_done;

    /* Obtain the full parameters from the passed arg. */
    arg = (npc_io_check_t *) in_arg;
    socket = arg->socket;
    type = arg->type;
    free(arg);

    poll_done = FALSE;

    while(*socket >= 0 && !poll_done)
    {
        /* Check if we have received data on the socket. */
        FD_ZERO(&read_fds);
        FD_SET(*socket, &read_fds);
        if (select(*socket + 1, &read_fds, NULL, NULL, NULL) > 0)
        {
            uint32 out_type;

            packet = voot_parse_socket(*socket);

            if (!packet)
            {
                if (!recv(*socket, NULL, 0, MSG_DONTWAIT | MSG_PEEK | MSG_TRUNC))
                {
                    out_type = type + 1;         /* this is a bad hack, but I'm essentially "closing" the socket for cleanup issues. */
                    poll_done = TRUE;
                }
                else
                    continue;          /* skip on the whole event sending. */
            }
            else
                out_type = type;

            event = (npc_command_t *) malloc(sizeof(npc_command_t));
            event->type = out_type;
            event->packet = packet;

            npc_add_event_queue(event);
        }
    }

    return NULL;
}

int npc_connect(char *dest_name, uint16 dest_port, int32 conntype)
{
    struct hostent *host;
    struct sockaddr_in address;
    int new_socket;

    bzero(&address, sizeof(address));

    host = gethostbyname(dest_name);
    if (!host)
    {
        fprintf(stderr, "[npc] unable to resolve %s.\n", dest_name);
        return -1;
    }

    address.sin_family = host->h_addrtype;
    memcpy((char *) &address.sin_addr.s_addr, host->h_addr_list[0], host->h_length);
    address.sin_port = htons(dest_port);

    new_socket = socket(AF_INET, conntype, 0);
    if (new_socket < 0)
    {
        fprintf(stderr, "[npc] unable to open a socket.\n");
        return -2;
    }

    if (connect(new_socket, (struct sockaddr *) &address, sizeof(address)))
    {
        close(new_socket);
        fprintf(stderr, "[npc] unable to connect to %s:%u.\n", dest_name, dest_port);
        return -3;
    }

    fprintf(stderr, "[npc] connected to %s:%u and sending VERSION command.\n", dest_name, dest_port);
    voot_send_command(new_socket, VOOT_COMMAND_TYPE_VERSION);

    return new_socket;
}

static void* npc_server_accept_task(void *arg)
{
    int32 wait_socket;
    int32 new_socket;

    wait_socket = (int32) arg;

    fprintf(stderr, "[npc] waiting for connection on server.\n");

    while((new_socket = accept(wait_socket, NULL, 0)))
    {
        if (new_socket >= 0 && (npc_system.server_socket < 0))
        {
            npc_command_t *event;
            
            npc_system.server_socket = new_socket;

            event = (npc_command_t *) malloc(sizeof(npc_command_t));
            event->type = C_LISTEN_SOCKET;
            event->listen_type = C_PACKET_FROM_SERVER;
            event->listen_socket = &(npc_system.server_socket);
            event->listen_socket_thread = &(npc_system.server_poll_thread);

            npc_add_event_queue(event);
            
            fprintf(stderr, "[npc] accepted connection for server! [%d]\n", new_socket);
        }
        else
        {
            close(new_socket);
            fprintf(stderr, "[npc] dropped incoming connection for server.\n");
        }
    }

    fprintf(stderr, "[npc] no longer accepting connections.\n");

    return NULL;
}

int32 npc_server_listen(void)
{
    int32 server_socket;
    struct sockaddr_in my_address;

    bzero(&my_address, sizeof(my_address));

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0)
    {
        fprintf(stderr, "[npc] unable to open a socket for serving.\n");
        return -1;
    }

    my_address.sin_family = AF_INET;
    my_address.sin_addr.s_addr = htonl(INADDR_ANY);
    my_address.sin_port = htons(npc_system.server_port);

    if (!bind(server_socket, (struct sockaddr *) &my_address, sizeof(my_address)) < 0)
    {
        close(server_socket);
        fprintf(stderr, "[npc] unable to bind a socket for serving.\n");
        return -2;
    }

    if (listen(server_socket, 1))
    {
        close(server_socket);
        fprintf(stderr, "[npc] unable to listen on port %u for serving.\n", npc_system.server_port);
        return -3;
    }

    if (pthread_create(&(npc_system.server_listen_thread), NULL, npc_server_accept_task, (void *) server_socket))
    {
        close(server_socket);
        fprintf(stderr, "[npc] unable to start acceptance thread for serving.\n");
        return -4;
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

    if (npc_system.server_socket >= 0)
    {
        if (close(npc_system.server_socket))
            fprintf(stderr, "[npc] unable to close server socket.\n");
        else
            fprintf(stderr, "[npc] closed server socket.\n");
    }
}

void npc_init(void)
{
    npc_command_t command;

    memset(&npc_system, 0x0, sizeof(npc_system));
    npc_system.slave_socket = npc_system.server_socket = npc_system.server_socket_wait = -1;

    pthread_mutex_init(&npc_system.event_queue_busy, NULL);

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
