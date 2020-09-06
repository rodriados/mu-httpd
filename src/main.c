/*!
 * \brief The server's entry point.
 * This is the file in which one can find the software's main function. Here, the
 * server is initialized and some basic checks are made.
 *
 * This paper's objective was the implementation of a webserver's basic functionalities.
 * The webserver must allow HTTP clients (using browsers such Firefox, Chrome or
 * IE) to connect to the server and download files from it. The webserver also had
 * to implement the methods GET and POST as part of the HTTP protocol, using the
 * TCP/IP protocols to transfer HTML pages or files.
 *
 * \author Rodrigo Siqueira <rodriados@gmail.com>
 */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

#include "config.h"
#include "colors.h"
#include "request.h"

/*!
 * \var server
 * \brief This is the descriptor of the socket used by the server.
 */
socket_id server;

void abort_listening(int);

/*!
 * \fn int main(int, char **)
 * \brief The software's entry point.
 * \param argc The number of command line arguments.
 * \param argv The list of command line arguments.
 */
int main(int argc, char **argv)
{
    struct sockaddr_in localaddr;

    memset(&localaddr, 0, sizeof(struct sockaddr_in));

    localaddr.sin_family = AF_INET;
    localaddr.sin_addr.s_addr = inet_addr(DEFAULT_ADDR);
    localaddr.sin_port = htons(argc > 1 ? atoi(argv[1]) : DEFAULT_PORT);

    server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    bind(server, (struct sockaddr *)&localaddr, sizeof(struct sockaddr_in));
    listen(server, MAX_CONNECTIONS);

    char *address = inet_ntoa(localaddr.sin_addr);
    int port = ntohs(localaddr.sin_port);

    printf(BRIGHT "μHTTP Hipertext Transfer Protocol Server\n\n" RESETALL);

    if (errno) {
        printf(BACKRED " ERROR " RESETALL " Could not create socket! Bailing out.\n");
        return 1;
    }

    signal(SIGINT, abort_listening);

    printf(BACKGREEN " LISTENING " RESETALL " @ %s:%d\n\n", address, port);

    request_listen(server, MAX_THREADS);
    close(server);

    printf(RESETALL);

    return 0;
}

/*!
 * \fn void abort_listening(int)
 * \brief Handles termination signal and closes the server.
 */
void abort_listening(int _)
{
    close(server);
    exit(0);
}
