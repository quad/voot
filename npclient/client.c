/*  client.c

    Console netplay client.

CHANGELOG:

    Sun Sep  9 22:46:15 PDT 2001    SSR
        Started the code base. Specifically the banner and options handling code.

    Wed Sep 12 16:30:00 PDT 2001    SSR
        Started coding connection code to np-voot-slave.

    Thu Sep 20 00:07:07 PDT 2001    SSR
        Finished extraction of NPC command module and renamed module to client.

TODO:

    Make my code more ugly.
*/

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h> 

#include "npc.h"
#include "client.h"

char *prog_name;

void display_start_banner(void)
{
    printf(banner_text);
    printf(gpl_text);
}

void display_exit_banner(void)
{
    printf(exit_text);
}

void frontend_init(char *pname)
{
    prog_name = strdup(pname);
}

void frontend_cleanup(void)
{
    free(prog_name);
}


void parse_options(int argc, char *argv[])
{
    int opt;
    bool connect_slave, connect_server, listen_server;

    connect_slave = connect_server = listen_server = FALSE;

    while((opt = getopt(argc, argv, "c:s:hl")) >= 0)
    {
        npc_data_t *npc_data;
        npc_command_t command;

        switch(opt)
        {
            case 'c':
                if (connect_slave)
                {
                    printf("%s: ignoring duplicate '-%c' option.\n", prog_name, opt);
                    break;
                }
                connect_slave = TRUE;

                npc_data = npc_expose();

                printf("%s: connecting to slave %s:%u...\n", prog_name, optarg, npc_data->slave_port);

                command.type = C_CONNECT_SLAVE;
                command.text = strdup(optarg);

                break;

            case 's':
                if (connect_server)
                {
                    printf("%s: ignoring duplicate '-%c' option.\n", prog_name, opt);
                    break;
                }
                else if (listen_server)
                {
                    printf("%s: ignoring conflicting '-%c' option.\n", prog_name, opt);
                    break;
                }
                connect_server++;

                npc_data = npc_expose();

                printf("%s: connecting to server %s:%u...\n", prog_name, optarg, npc_data->server_port);

                command.type = C_CONNECT_SERVER;
                command.text = strdup(optarg);
                break;

            case 'l':
                if (listen_server)
                {
                    printf("%s: ignoring duplicate '-%c' option.\n", prog_name, opt);
                    break;
                }
                else if (connect_server)
                {
                    printf("%s: ignoring conflicting '-%c' option.\n", prog_name, opt);
                    break;
                }
                listen_server++;

                npc_data = npc_expose();

                printf("%s: starting server on port %u...\n", prog_name, npc_data->server_port);

                command.type = C_LISTEN_SERVER;
                break;

            case 'h':
                printf(help_text);

                command.type = C_EXIT;
                command.code = 0;
                break;

            case '?':
            default:
                /* getopt() automatically responds with an unknown option message. */
                command.type = C_NONE;
                break;
        }

        if (command.type != C_NONE)
            handle_npc_command(&command);
    }
}

int main(int argc, char *argv[])
{
    npc_command_t *event;

    display_start_banner();

    npc_init();

    frontend_init(argv[0]);

    parse_options(argc, argv);

    while((event = npc_get_event()))
    {
        handle_npc_command(event);

        /* NPC will have already done its cleanup. */
        if (event->type == C_EXIT)
        {
            free(event);
            break;
        }

        free(event);
    }

    frontend_cleanup();

    display_exit_banner();
    
    return 0;
}
