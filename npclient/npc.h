#ifndef __NPC_H__
#define __NPC_H__

#include <netdb.h>
#include "vars.h"
#include "voot.h"

#define VOOT_SLAVE_PORT     5007
#define VOOT_SERVER_PORT    5008

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
        C_PACKET_FROM_SLAVE
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
    struct sockaddr_in  slave_address;

    char                *server_name;
    uint16              server_port;
    int32               server_socket;
} npc_data_t;

uint32 handle_npc_command(npc_command_t *command);

npc_command_t* npc_get_event(void);
npc_command_t* npc_slave_io_check(void);
int npc_slave_connect(void);
int npc_server_connect(void);
int npc_server_listen(void);
void npc_exit(int code);
void npc_init(void);
npc_data_t* npc_expose(void);

#endif
