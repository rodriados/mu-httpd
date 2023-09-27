/*!
 * mu-HTTPd: A very very simple HTTP server.
 * \file The types and functions declarations for the server.
 * \author Rodrigo Siqueira <rodriados@gmail.com>
 * \copyright 2014-present Rodrigo Siqueira
 */
#ifndef MU_HTTPD_SERVER_H
#define MU_HTTPD_SERVER_H

#include <stdint.h>

#include "logger.h"

/*!
 * \enum server_status_t
 * \brief The server status codes.
 * \since 3.0
 */
typedef enum server_status_t {
    SERVER_SUCCESS = 0
  , SERVER_UNINITIALIZED
  , SERVER_FAIL_CREATE_SOCKET
  , SERVER_FAIL_ACCEPT_CLIENT
  , SERVER_STOP_REQUESTED = -1
} server_status_t;

/*!
 * \typedef socket_id_t
 * \brief The identifier for a server socket.
 * \since 3.0
 */
typedef int socket_id_t;

/*!
 * \struct server_t
 * \brief The server type.
 * \since 3.0
 */
typedef struct server_t {
    socket_id_t socket;
    char *address;
    uint16_t port;
    void *_internal;
} server_t;

/*
 * Forward declaration of server functions.
 * These functions are needed for creating and interacting with the server.
 */
extern server_status_t server_create(server_t*, int);
extern server_status_t server_listen(const server_t*, logger_t*, int);
extern const char *server_status_describe(server_status_t);
extern void server_destroy(server_t*);

#endif
