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

    Mon Jan 21 23:25:13 PST 2002    Scott Robinson <scott_np@dsn.itgo.com>
        Added usage of fcntl for non-blocking IO because cygwin might
        support it above MSG_NONBLOCK in recv().

    Tue Jan 22 17:27:15 PST 2002    Scott Robinson <scott_np@dsn.itgo.com>
        Dropped the port configuration events and wrapped them into the
        connection and listening events.

    Tue Jan 22 18:04:17 PST 2002    Scott Robinson <scott_np@dsn.itgo.com>
        Cleaning up syntax and overall code manageability.

    Wed Jan 23 00:31:01 PST 2002    Scott Robinson <scott_np@dsn.itgo.com>
        Removed all direct IO references and centralized the socket closing
        code for debugability.

    Thu Jan 24 13:28:18 PST 2002    Scott Robinson <scott_np@dsn.itgo.com>
        Added detail to socket debugging messages and created a socket
        information utility function.

*/

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>

#include "voot.h"
#include "npc.h"

npc_data_t  npc_system;

static uint16 get_socket_info(int32 socket, char *ip_string, uint32 ip_string_size)
{
    struct sockaddr_in address;
    int32 sizeof_address;

    sizeof_address = sizeof(address);

    if (getsockname(socket, (struct sockaddr *) &address, (socklen_t *) &sizeof_address) < 0)
        return 0;

    if (ip_string && ip_string_size >= sizeof("111.222.333.444"))
        strncpy(ip_string, inet_ntoa((struct in_addr) address.sin_addr), ip_string_size);

    return address.sin_port;
}

static void npc_close_socket(volatile int32 *in_socket, const char *socket_desc)
{
    int32 socket;

    socket = *in_socket;
    if (socket >= 0)
    {
        uint16 socket_port;
        char socket_name[16];

        socket_port = get_socket_info(socket, socket_name, sizeof(socket_name));
        *in_socket = -1;

        if (close(socket))
            NPC_LOG(npc_system, LOG_ALERT, "Unable to close %s socket.", socket_desc);
        else
        {
            NPC_LOG(npc_system, LOG_NOTICE, "Closed %s connection on %s:%u.", socket_desc, socket_name, socket_port);
        }
    }
}

int32 npc_handle_command(npc_command_t *command)
{
    int32 retval;

    retval = 0;

    switch (command->type)
    {
        case C_CONNECT_SLAVE:
        {
            npc_system.slave_name = command->text;
            npc_system.slave_port = command->port;

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
            {
                NPC_LOG(npc_system, LOG_NOTICE, "Unable to connect to slave %s:%u.", npc_system.slave_name, npc_system.slave_port);
            }

            break;
        }

        case C_CONNECT_SERVER:
        {
            npc_system.server_name = command->text;
            npc_system.server_port = command->port;

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
            {
                NPC_LOG(npc_system, LOG_NOTICE, "Unable to connect to server %s:%u.", npc_system.server_name, npc_system.server_port);
            }

            break;
        }

        case C_LISTEN_SOCKET:
        {
            npc_io_check_t  *arg;

            /* Start a listening thread for the server socket. */
            arg = (npc_io_check_t *) malloc(sizeof(npc_io_check_t));
            arg->socket = command->listen_socket;
            arg->type = command->listen_type;

            if (pthread_create(command->listen_socket_thread, NULL, npc_io_check, (void *) arg))
            {
                NPC_LOG(npc_system, LOG_ALERT, "Unable to start polling thread for type %d.", command->type);

                npc_close_socket(command->listen_socket, "IO");
            }

            break;
        }

        case C_LISTEN_SERVER:
        {
            npc_system.server_port = command->port;

            retval = npc_server_listen();

            if (retval)
                NPC_LOG(npc_system, LOG_WARNING, "Unable to start server on port %u.", npc_system.server_port);

            break;
        }

        case C_PACKET_FROM_SLAVE:
        {
            switch (command->packet->header.type)
            {
                case VOOT_PACKET_TYPE_DEBUG:
                {
                    NPC_LOG(npc_system, LOG_INFO, "DEBUG(slave): %s", command->packet->buffer);
                    break;
                }

                case VOOT_PACKET_TYPE_DATA:
                {
                    NPC_LOG(npc_system, LOG_INFO, "DATA(slave): '%s'", command->packet->buffer);

                    voot_send_packet(npc_system.server_socket, command->packet, voot_check_packet_advsize(command->packet, sizeof(voot_packet)));
                    break;
                }

                default:
                    break;
            }
            
            free(command->packet);
            break;
        }

        case C_CLOSE_SLAVE:
        {
            NPC_LOG(npc_system, LOG_ERR, "Received request to disconnect slave socket! Is this even possible?!");

            npc_close_socket(&npc_system.slave_socket, "slave");

            break;
        }

        case C_PACKET_FROM_SERVER:
        {
            switch (command->packet->header.type)
            {
                case VOOT_PACKET_TYPE_DEBUG:
                {
                    NPC_LOG(npc_system, LOG_INFO, "DEBUG(server): %s", command->packet->buffer);
                    break;
                }

                case VOOT_PACKET_TYPE_COMMAND:
                {
                    NPC_LOG(npc_system, LOG_INFO, "COMMAND(server): %c", command->packet->buffer[0]);
                    voot_send_packet(npc_system.slave_socket, command->packet, voot_check_packet_advsize(command->packet, sizeof(voot_packet)));
                    break;
                }
                
                case VOOT_PACKET_TYPE_DATA:
                {
                    NPC_LOG(npc_system, LOG_DEBUG, "DATA(server): '%s'", command->packet->buffer);

                    voot_send_packet(npc_system.slave_socket, command->packet, voot_check_packet_advsize(command->packet, sizeof(voot_packet)));
                    break;
                }
            
                default:
                    break;
            }

            free(command->packet);
            break;
        }

        case C_CLOSE_SERVER:
        {
            npc_close_socket(&npc_system.server_socket, "server");

            break;
        }

        case C_EXIT:
        {
            npc_exit(command->code);
            break;
        }

        case C_NONE:    /* Don't even notify of gutter commands. */
            break;

        default:
        {
            NPC_LOG(npc_system, LOG_WARNING, "Dropping unimplemented npc_command of type %u.", command->type);
            break;
        }
    }

    return retval;
}

bool npc_add_event_queue(npc_command_t *command)
{
    if (npc_system.event_queue_size >= NPC_EVENT_QUEUE_SIZE)
    {
        NPC_LOG(npc_system, LOG_ERR, "Event queue overflow!");
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
    int32 socket;
    volatile int32 *check_socket;
    npc_command type;
    fd_set read_fds;
    voot_packet *packet;
    npc_command_t *event;
    bool poll_done;

    /* Obtain the full parameters from the passed arg. */
    arg = (npc_io_check_t *) in_arg;
    check_socket = arg->socket;
    type = arg->type;
    free(arg);

    poll_done = FALSE;

    while(*check_socket >= 0 && !poll_done)
    {
        socket = *check_socket;

        /* Check if we have received data on the socket. */
        FD_ZERO(&read_fds);
        FD_SET(socket, &read_fds);
        if (select(FD_SETSIZE, &read_fds, NULL, NULL, NULL) > 0)
        {
            uint32 out_type;

            packet = voot_parse_socket(socket);

            if (!packet)
            {
                fcntl(socket, F_SETFL, O_NONBLOCK);
                if (!recv(socket, NULL, 0, MSG_PEEK))
                {
                    fcntl(socket, F_SETFL, NULL);
                    out_type = type + 1;         /* this is a bad hack, but I'm essentially "closing" the socket for cleanup issues. */
                    poll_done = TRUE;
                }
                else
                {
                    fcntl(socket, F_SETFL, NULL);
                    continue;          /* skip on the whole event sending. */
                }
            }
            else
            {
                out_type = type;
            }

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
        NPC_LOG(npc_system, LOG_INFO, "Could not resolve '%s'.", dest_name);
        return -1;
    }

    address.sin_family = host->h_addrtype;
    memcpy((char *) &address.sin_addr.s_addr, host->h_addr_list[0], host->h_length);
    address.sin_port = htons(dest_port);

    new_socket = socket(AF_INET, conntype, 0);
    if (new_socket < 0)
    {
        NPC_LOG(npc_system, LOG_ALERT, "Unable to open a socket.");
        return -2;
    }

    /* If we're dealing with a stream, be sure to disable Nagle's algorithm. */
    if (conntype == SOCK_STREAM)
    {
        int opt_length;

        opt_length = 1;

        if(!setsockopt(new_socket, SOL_TCP, TCP_NODELAY, (char *) &opt_length, sizeof(opt_length)))
            NPC_LOG(npc_system, LOG_CRIT, "Could not set TCP_NODELAY on socket %d.", new_socket);
    }

    if (connect(new_socket, (struct sockaddr *) &address, sizeof(address)))
    {
        close(new_socket);
        NPC_LOG(npc_system, LOG_INFO, "Unable to connect a socket to %s:%u.", dest_name, dest_port);
        return -3;
    }

    NPC_LOG(npc_system, LOG_INFO, "Connected to %s:%u and sending VERSION command.", dest_name, dest_port);
    voot_send_command(new_socket, VOOT_COMMAND_TYPE_VERSION);

    return new_socket;
}

static void* npc_server_accept_task(void *arg)
{
    int32 wait_socket;
    int32 new_socket;

    wait_socket = (int32) arg;

    NPC_LOG(npc_system, LOG_NOTICE, "Waiting for connection as server.");

    while((new_socket = accept(wait_socket, NULL, 0)))
    {
        if (new_socket >= 0 && (npc_system.server_socket < 0))
        {
            npc_command_t *event;
            char socket_name[16];
            uint16 socket_port;
            
            npc_system.server_socket = new_socket;

            event = (npc_command_t *) malloc(sizeof(npc_command_t));
            event->type = C_LISTEN_SOCKET;
            event->listen_type = C_PACKET_FROM_SERVER;
            event->listen_socket = &(npc_system.server_socket);
            event->listen_socket_thread = &(npc_system.server_poll_thread);

            npc_add_event_queue(event);

            socket_port = get_socket_info(new_socket, socket_name, sizeof(socket_name));
            NPC_LOG(npc_system, LOG_NOTICE, "Accepted connection from %s:%u.", socket_name, socket_port);
        }
        else
        {
            close(new_socket);
            NPC_LOG(npc_system, LOG_INFO, "Dropped incoming connection for server.");
        }
    }

    NPC_LOG(npc_system, LOG_NOTICE, "No longer accepting connections.");

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
        NPC_LOG(npc_system, LOG_ALERT, "Unable to open a socket for serving.");
        return -1;
    }

    my_address.sin_family = AF_INET;
    my_address.sin_addr.s_addr = htonl(INADDR_ANY);
    my_address.sin_port = htons(npc_system.server_port);

    if (!bind(server_socket, (struct sockaddr *) &my_address, sizeof(my_address)) < 0)
    {
        close(server_socket);
        NPC_LOG(npc_system, LOG_CRIT, "Unable to bind a socket for serving.");
        return -2;
    }

    if (listen(server_socket, 1))
    {
        close(server_socket);
        NPC_LOG(npc_system, LOG_CRIT, "Unable to listen on port %u for serving.", npc_system.server_port);
        return -3;
    }

    if (pthread_create(&(npc_system.server_listen_thread), NULL, npc_server_accept_task, (void *) server_socket))
    {
        close(server_socket);
        NPC_LOG(npc_system, LOG_ALERT, "Unable to start acceptance thread for serving.");
        return -4;
    }

    /* Update npc_system with socket and addressing information. */
    npc_system.server_socket_wait = server_socket;

    return 0;
}

void npc_exit(int code)
{
    /* Clean up both the slave and server sockets. */

    free(npc_system.slave_name);
    npc_close_socket(&npc_system.slave_socket, "slave");

    free(npc_system.server_name);
    npc_close_socket(&npc_system.server_socket, "server");
}

void npc_init(npc_log_callback_f *log_callback_function)
{
    memset(&npc_system, 0x0, sizeof(npc_system));
    npc_system.slave_socket = npc_system.server_socket = npc_system.server_socket_wait = -1;

    npc_system.log_callback_function = log_callback_function;

    pthread_mutex_init(&npc_system.event_queue_busy, NULL);
}

npc_data_t* npc_expose(void)
{
    /* We don't really ever want this to be done. If there is an outside
        function needs access to one of the internal data types, hopefully
        we'll have created an accessor function for it. However, I'll leave
        the functionality in just-in-case. */

    /* DEBUG: Notify the programmer when npc_system is exposed. */
    //NPC_LOG(npc_system, LOG_DEBUG, "npc_system exposed!");

    return &npc_system;
}
