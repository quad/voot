#ifndef __NPC_H__
#define __NPC_H__

#include <netdb.h>
#include "vars.h"
#include "voot.h"

#define NPC_EVENT_QUEUE_SIZE    50

typedef enum
{
    C_NONE,                 /* No command. */
    C_HELP,                 /* Display the help text. */

    C_CONNECT_SLAVE,        /* Connect to the given slave. */
    C_CONNECT_SERVER,       /* Connect to the given server. */

    C_LISTEN_SERVER,        /* Listens on the server port. */

    C_SET_SLAVE_PORT,       /* Sets the port used for UDP slave connections. */
    C_SET_SERVER_PORT,      /* Sets the port used for TCP reliable connections. */

    C_PACKET_FROM_SLAVE,    /* On reception of a packet from the slave. */
    C_CLOSE_SLAVE,          /* Need to close the slave socket. */

    C_PACKET_FROM_SERVER,   /* On reception of a packet from the server. */
    C_CLOSE_SERVER,         /* Need to close the server socket. */

    C_EXIT                  /* Exit from the client. */
} npc_command;

typedef struct
{
    npc_command     type;

    /*
        C_CONNECT_SLAVE
    */
    char            *text;

    /*
        C_SET_SLAVE_PORT, C_SET_SERVER_PORT
    */
    uint16          port;

    /*
        C_PACKET_FROM_SLAVE, C_PACKET_FROM_SERVER
    */
    voot_packet     *packet;

    /*
        C_EXIT
    */
    int32           code;

} npc_command_t;

typedef struct
{
    char                *slave_name;
    uint16              slave_port;
    int32               slave_socket;

    char                *server_name;
    uint16              server_port;
    volatile int32      server_socket;
    volatile int32      server_socket_wait;
    pthread_t           server_thread;

    npc_command_t*      event_queue[NPC_EVENT_QUEUE_SIZE];
    uint32              event_queue_size;
    uint32              event_queue_tail;
} npc_data_t;

int32 handle_npc_command(npc_command_t *command);
bool npc_add_event_queue(npc_command_t *command);
npc_command_t* npc_get_event_queue(void);
npc_command_t* npc_get_event(void);
bool npc_io_check(int32 socket, npc_command type);
int npc_connect(char *dest_name, uint16 dest_port, int32 conntype);
int npc_server_listen(void);
void npc_exit(int code);
void npc_init(void);
npc_data_t* npc_expose(void);

#endif
