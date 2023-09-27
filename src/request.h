/*!
 * mu-HTTPd: A very very simple HTTP server.
 * \file The types and functions declarations for a user request.
 * \author Rodrigo Siqueira <rodriados@gmail.com>
 * \copyright 2014-present Rodrigo Siqueira
 */
#ifndef MU_HTTPD_REQUEST_H
#define MU_HTTPD_REQUEST_H

#include <netinet/in.h>

#include "server.h"
#include "logger.h"

/*!
 * \struct request_t
 * \brief The type for an accepted user request.
 * \since 2.0
 */
typedef struct request_t {
    socket_id_t client;
    struct sockaddr_in origin;
} request_t;

/*
 * Forward declaration of request processing function.
 * This function is responsible for truly processing a request.
 */
extern void request_process(request_t *, logger_writer_t*);

#endif
