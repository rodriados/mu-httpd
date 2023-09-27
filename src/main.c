/*!
 * mu-HTTPd: A very very simple HTTP server.
 * \file The server's entry point implementation.
 * This is the file in which one can find the software's main function. Here, the
 * server is initialized and some basic checks are made.
 *
 * This paper's objective was the implementation of a web-server's basic functionalities.
 * The web-server must allow HTTP clients, using browsers such Firefox or Chrome,
 * to connect to the server and download files from it. The web-server also had to
 * implement the methods GET and POST as part of the HTTP protocol, using the TCP/IP
 * protocols to transfer HTML pages or files.
 * \author Rodrigo Siqueira <rodriados@gmail.com>
 * \copyright 2014-present Rodrigo Siqueira
 */
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

#include "config.h"
#include "colors.h"
#include "logger.h"
#include "server.h"

void report_success(const char *, uint16_t);
void report_failure_and_exit(enum server_status_t);

/*!
 * \fn int main(int, char **)
 * \brief The software's entry point.
 * \param argc The number of command line arguments.
 * \param argv The list of command line arguments.
 */
int main(int argc, char **argv)
{
    printf(FG_INFO("μHTTPd Hipertext Transfer Protocol Server\n"));

    server_t server = {
        .address = DEFAULT_ADDR
      , .port = argc > 1 ? atoi(argv[1]) : DEFAULT_PORT
    };

    server_status_t server_status =
        server_create(&server, MAX_CONNECTIONS);

    if (server_status != SERVER_SUCCESS)
        report_failure_and_exit(server_status);

    FILE *logfile = fopen(LOG_FILE, "a");
    logger_t logger = logger_initialize();

    logger_file_sink_add(&logger, logfile);
    logger_file_sink_add(&logger, stdout);

    report_success(server.address, server.port);

    server_status = server_listen(&server, &logger, MAX_THREADS);

    server_destroy(&server);
    logger_finalize(&logger);
    fclose(logfile);

    printf(RESETALL);

    return 0;
}

#define HTTPD_SUCCESS_MSG BG_SUCCESS(" LISTENING ") FG_INFO(" at %s:%hu\n")
#define HTTPD_FAILURE_MSG BG_WARNING(" ERROR ") " %s\n"

/*!
 * \fn void report_success(const char*, uint16_t)
 * \brief Reports a successful server connection message to the terminal.
 * \param address The address to which the server is listening to.
 * \param port The port at which the server is listening to.
 */
void report_success(const char *address, uint16_t port)
{
    fprintf(stderr, HTTPD_SUCCESS_MSG, address, port);
}

/*!
 * \fn void report_failure_and_exit(server_status_t)
 * \brief Reports a failure server connection to the terminal.
 * \param status The server status to be reported.
 */
void report_failure_and_exit(server_status_t status)
{
    fprintf(stderr, HTTPD_FAILURE_MSG, server_status_describe(status));
    exit(EXIT_FAILURE);
}
