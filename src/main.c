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
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "colors.h"
#include "request.h"

/*!
 * \fn int main(int, char **)
 * \brief The software's entry point.
 * \param argc The number of command line arguments.
 * \param argv The list of command line arguments.
 */
int main(int argc, char **argv)
{
    socket_id server;
    sockaddr_in localaddr;

    memset(&localaddr, 0, sizeof(sockaddr_in));

    localaddr.sin_family = AF_INET;
    localaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    localaddr.sin_port = htons(argc > 1 ? atoi(argv[1]) : 8080);

    server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (server == INVALID_SOCKET) {
        printf("Could not create socket!\n");
        return 1;
    }

    bind(server, (sockaddr *)&localaddr, sizeof(sockaddr_in));
    listen(server, 100);

    char *address = inet_ntoa(localaddr.sin_addr);
    int port = ntohs(localaddr.sin_port);

    printf(BRIGHT "μHTTP Hipertext Transfer Protocol Server\n" RESETALL);
    printf(BACKGREEN " LISTENING " RESETALL " @ %s:%d\n\n", address, port);

    request_listen(server, 50);
    close(server);

    printf(RESETALL);

    return 0;
}
