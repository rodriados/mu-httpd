/*!
 * \brief The functions responsible for creating a log of HTTP requests.
 * Declares functions and defines structures relevant to logging HTTP requests.
 * \author Rodrigo Siqueira <rodriados@gmail.com>
 */
#ifndef MU_HTTPD_LOG_H
#define MU_HTTPD_LOG_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "http.h"

extern void log_init(const char *);
extern void log_finalize();
extern void log_write(struct sockaddr_in *, struct http_request_t *, struct http_response_t *);

#endif
