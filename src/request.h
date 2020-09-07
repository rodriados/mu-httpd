/*!
 * \brief The request processing functions.
 * Exposes functions which are relevant for the processing of incoming requests.
 * \author Rodrigo Siqueira <rodriados@gmail.com>
 */
#ifndef MU_HTTPD_REQUEST_H
#define MU_HTTPD_REQUEST_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <unistd.h>
#include <stdint.h>

/*!
 * \type socket_id
 * \brief The socket's identification type.
 */
typedef int socket_id;

/*!
 * \struct request_t
 * \brief Groups together all information about an incoming request.
 */
struct request_t {
    socket_id client;
    struct sockaddr_in remoteaddr;
    pthread_t thread;
};

extern void request_listen(socket_id, size_t);

#endif
