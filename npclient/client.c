/*  client.c

DESCRIPTION

    Console netplay client front-end.

CHANGELOG

    Sun Sep  9 22:46:15 PDT 2001    Scott Robinson <scott_np@dsn.itgo.com>
        Started the code base. Specifically the banner and options handling
        code.

    Wed Sep 12 16:30:00 PDT 2001    Scott Robinson <scott_np@dsn.itgo.com>
        Started coding connection code to np-voot-slave.

    Thu Sep 20 00:07:07 PDT 2001    Scott Robinson <scott_np@dsn.itgo.com>
        Finished extraction of NPC command module and renamed module to
        client.

    Tue Jan 22 13:26:29 PST 2002    Scott Robinson <scott_np@dsn.itgo.com>
        Added readline callback thread and started coding of command parser
        for console.

    Tue Jan 22 17:27:58 PST 2002    Scott Robinson <scott_np@dsn.itgo.com>
        Now accept host:port syntax and custom ports in the listening task.

    Tue Jan 22 23:37:22 PST 2002    Scott Robinson <scott_np@dsn.itgo.com>
        Added the debugging callback functionality.

    Thu Jan 24 00:09:37 PST 2002    Scott Robinson <scott_np@dsn.itgo.com>
        Added injection commands for debugging. strndup() macro equivalenies
        and a bugfix in the command line options parser.

    Thu Jan 24 00:50:21 PST 2002    Scott Robinson <scott_np@dsn.itgo.com>
        Added inter-client communication via the input command parser. If
        it's not a command, it's something to send via the DEBUG data route.

    Sun Feb 24 01:02:31 PST 2002    Scott Robinson <scott_vo@quadhome.com>
        Added the new callback initialization functionality and stub code
        for packet callbacks.

*/

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h> 
#include <pthread.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <stdarg.h>

#include "npc.h"
#include "client.h"

char *prog_name;
bool input_handler_poll;
pthread_t input_poll_thread;

/*
 *  Various frontend texts.
 */

static const char banner_text[] = {
    "Console Netplay VOOT Client (npclient), built " __DATE__ " at " __TIME__ "\n"
    "Copyright (C) 2001, 2002, Scott Robinson. All Rights Reserved.\n"
};

static const char gpl_text[] = {
    "\nThis program is distributed in the hope that it will be useful,\n"
    "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
    "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
    "GNU General Public License for more details.\n"
    "\n"
};

static const char help_text[] = {
    "-c <hostname/IP[:port]>    Connect to the specified slave.\n"
    "-s <hostname/IP[:port]>    Connect to the specified server.\n"
    "-l[port]                   Change into server mode.\n"
    "\n"
};

static const char exit_text[] = {
    "npclient exit!\n"
};

static const char *npc_log_level_desc[] = {
    "EMERGERNCY",
    "ALERT",
    "CRITICAL",
    "ERROR",
    "WARNING",
    "NOTICE",
    "INFO",
    "DEBUG"
};

/*
 * min()/max() macros that also do
 * strict type-checking.. See the
 * "unnecessary" pointer comparison.
 */
#define min(x,y) ({ \
	const typeof(x) _x = (x);	\
	const typeof(y) _y = (y);	\
	(void) (&_x == &_y);		\
	_x < _y ? _x : _y; })

#define max(x,y) ({ \
	const typeof(x) _x = (x);	\
	const typeof(y) _y = (y);	\
	(void) (&_x == &_y);		\
	_x > _y ? _x : _y; })

#define my_strndup(s, n)                                                      \
  (__extension__                                                              \
    ({                                                                        \
      const char *__old = (s);                                                \
      size_t __len = min(strlen (__old), n);                                  \
      char *__new = (char *) malloc (__len + 1);                              \
      __new[__len] = '\0';                                                    \
      (char *) memcpy (__new, __old, __len);                                  \
    }))

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

        host = my_strndup(opt_arg, (size_t) (tokidx - opt_arg));
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
            {
                command->port = VOOT_SLAVE_PORT;
                break;
            }

            case C_CONNECT_SERVER:
            {
                command->port = VOOT_SERVER_PORT;
                break;
            }

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
            {
                if (connect_slave)
                {
                    printf("%s: ignoring duplicate '-%c' option.\n", prog_name, opt);
                    break;
                }

                connect_slave = TRUE;

                client_parse_connect(command, optarg, C_CONNECT_SLAVE, "slave");

                break;
            }

            case 's':
            {
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
            }

            case 'l':
            {
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
            }

            case 'h':
            {
                printf(help_text);

                command->type = C_EXIT;
                command->code = 0;

                break;
            }

            case '?':
            default:
            {
                /* getopt() automatically responds with an unknown option message. */
                command->type = C_NONE;
                break;
            }
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
        int32 string_index;
        npc_data_t *system;

        system = npc_expose();

        /* First, lowercase the damn string. */
        string_index = 0;
        while((line[string_index] = tolower(line[string_index])))
            string_index++;

        add_history(line);

        /* Now parse simple commands. */
        if (!strcmp(line, "c-inject"))
            voot_send_command(system->slave_socket, VOOT_COMMAND_TYPE_INJECTTST);
        else if (!strcmp(line, "c-printf"))
            voot_send_command(system->slave_socket, VOOT_COMMAND_TYPE_PRINTFTST);
        else if (!strcmp(line, "c-malloc"))
            voot_send_command(system->slave_socket, VOOT_COMMAND_TYPE_MALLOCTST);
        else if (!strcmp(line, "c-netstat"))
            voot_send_command(system->slave_socket, VOOT_COMMAND_TYPE_NETSTAT);
        else if (!strcmp(line, "c-health"))
            voot_send_command(system->slave_socket, VOOT_COMMAND_TYPE_HEALTH);
        else if (!strcmp(line, "c-time"))
            voot_send_command(system->slave_socket, VOOT_COMMAND_TYPE_TIME);
        else if (!strcmp(line, "c-version"))
            voot_send_command(system->slave_socket, VOOT_COMMAND_TYPE_VERSION);
        else if (!strcmp(line, "c-passive-on"))
            voot_send_command(system->slave_socket, VOOT_COMMAND_TYPE_PASVON);
        else if (!strcmp(line, "c-screenshot"))
            voot_send_command(system->slave_socket, VOOT_COMMAND_TYPE_SCREEN);
        else if (!strcmp(line, "inject"))
        {
            char data[] = "012345678901234567890123456789012345678901234567890123456789";

            voot_send_data(system->slave_socket, VOOT_PACKET_TYPE_DATA, data, sizeof(data));
        }
        else
            voot_send_data(system->server_socket, VOOT_PACKET_TYPE_DEBUG, line, strlen(line) + 1);
          
        free(line);
    }
    else
    {
        npc_command_t *event;

        input_handler_poll = FALSE;

        event = malloc(sizeof(npc_command_t));
        event->type = C_EXIT;

        npc_add_event_queue(event);
    }
}

void logger_callback(npc_log_level severity, const char *format, ...)
{
    va_list args;

    printf("%s: [npc|%s] ", prog_name, npc_log_level_desc[severity]); 

    va_start(args, format);
    vprintf(format, args);
    va_end(args);

    printf("\n");

    rl_redisplay();
}

bool packet_callback(uint8 type, const voot_packet *packet)
{
    static uint32 dump_file = 0;

    if (type == C_PACKET_FROM_SLAVE)
    {
        if (packet->header.type == VOOT_PACKET_TYPE_COMMAND)
        {
            switch (packet->buffer[0])
            {
                case VOOT_COMMAND_TYPE_DUMPON:
                    if (!dump_file)
                    {
                        char template[] = "npc-dump.XXXXXX";

                        dump_file = mkstemp(template);

                        printf("%s: [dump] Opened dump file. (%d)\n", prog_name, dump_file);

                    }
                    break;

                case VOOT_COMMAND_TYPE_DUMPOFF:
                    if (dump_file)
                        close(dump_file);
                    dump_file = 0;

                    printf("%s: [dump] Closed dump file.\n", prog_name);

                    break;
            }
        }
        else if (packet->header.type == VOOT_PACKET_TYPE_DUMP && dump_file)
        {
            int out;

            //printf("%s: [dump] Writing dump data...\n", prog_name);

            out = write(dump_file, packet->buffer, (ntohs(packet->header.size) - 1));

            if (!out)
                printf("%s: [dump] Error in writing data the dump IO file.\n", prog_name);
        }
    }

    return FALSE;
}

int main(int argc, char *argv[])
{
    npc_command_t *event;
    npc_callbacks_t callbacks;

    display_start_banner();

    frontend_init(argv[0]);

    callbacks.log = logger_callback;
    callbacks.packet = packet_callback;

    npc_init(&callbacks);

    parse_options(argc, argv);

    while((event = npc_get_event()))
    {
        npc_handle_command(event);

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
