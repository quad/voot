/*  client.c

DESCRIPTION

    Console netplay client front-end.

CHANGELOG

    Sun Sep  9 22:46:15 PDT 2001    Scott Robinson <scott_np@dsn.itgo.com>
        Started the code base. Specifically the banner and options handling code.

    Wed Sep 12 16:30:00 PDT 2001    Scott Robinson <scott_np@dsn.itgo.com>
        Started coding connection code to np-voot-slave.

    Thu Sep 20 00:07:07 PDT 2001    Scott Robinson <scott_np@dsn.itgo.com>
        Finished extraction of NPC command module and renamed module to client.

    Tue Jan 22 13:26:29 PST 2002    Scott Robinson <scott_np@dsn.itgo.com>
        Added readline callback thread and started coding of command parser
        for console.

    Tue Jan 22 17:27:58 PST 2002    Scott Robinson <scott_np@dsn.itgo.com>
        Now accept host:port syntax and custom ports in the listening task.

*/

#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h> 
#include <pthread.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "npc.h"
#include "client.h"

char *prog_name;
bool input_handler_poll;
pthread_t input_poll_thread;

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

    rl_callback_handler_install("npclient> ", input_handler);
    input_handler_poll = TRUE;
    pthread_create(&input_poll_thread, NULL, input_poll, NULL);
}

void frontend_cleanup(void)
{
    rl_callback_handler_remove();
    free(prog_name);
}

void client_parse_connect(npc_command_t *command, char *opt_arg, npc_command type, const char *vis_type)
{
    char *tokidx_work;
    char *tokidx;

    command->type = type;
    strtok_r(opt_arg, ":", &tokidx_work);

    if ((tokidx = strtok_r(NULL, ":", &tokidx_work)))
    {
        char *host;
        uint16 port;

        host = strndup(opt_arg, (size_t) (tokidx - opt_arg));
        port = atoi(tokidx);

        command->text = host;
        command->port = port;
    }
    else
    {
        command->text = strdup(opt_arg);
        switch(type)
        {
            case C_CONNECT_SLAVE:
                command->port = VOOT_SLAVE_PORT;
                break;

            case C_CONNECT_SERVER:
                command->port = VOOT_SERVER_PORT;
                break;

            default:
                /* This should never happen, but if it does... ahh well. */
                break;
        }
    }

    printf("%s: connecting to %s %s:%u ...\n", prog_name, vis_type, command->text, command->port);
}

void parse_options(int argc, char *argv[])
{
    int opt;
    bool connect_slave, connect_server, listen_server;

    connect_slave = connect_server = listen_server = FALSE;

    while((opt = getopt(argc, argv, "c:s:hl::")) >= 0)
    {
        npc_command_t *command;

        command = malloc(sizeof(npc_command_t));

        switch(opt)
        {
            case 'c':
                if (connect_slave)
                {
                    printf("%s: ignoring duplicate '-%c' option.\n", prog_name, opt);
                    break;
                }
                connect_slave = TRUE;

                client_parse_connect(command, optarg, C_CONNECT_SLAVE, "slave");

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
                connect_server = TRUE;

                client_parse_connect(command, optarg, C_CONNECT_SERVER, "server");

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

                command->port = optarg ? atoi(optarg) : VOOT_SERVER_PORT;

                printf("%s: starting server on port %u...\n", prog_name, command->port);

                command->type = C_LISTEN_SERVER;
                break;

            case 'h':
                printf(help_text);

                command->type = C_EXIT;
                command->code = 0;
                break;

            case '?':
            default:
                /* getopt() automatically responds with an unknown option message. */
                break;
        }

        if (command->type != C_NONE)
            npc_add_event_queue(command);
    }
}

void* input_poll(void *arg)
{
    while(input_handler_poll)
        rl_callback_read_char();

    return NULL;
}

void input_handler(char *line)
{
    if (line)
    {
        printf("[client] received '%s'.\n", line);
        free(line);
    }
    else
    {
        npc_command_t *event;

        event = malloc(sizeof(npc_command_t));
        event->type = C_EXIT;
        npc_add_event_queue(event);
        
        input_handler_poll = FALSE;
    }
}

int main(int argc, char *argv[])
{
    npc_command_t *event;

    display_start_banner();

    frontend_init(argv[0]);

    npc_init();

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
