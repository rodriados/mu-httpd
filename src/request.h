/*!
 * \brief The request processing functions.
 * Exposes functions which are relevant for the processing of incoming requests.
 * \author Rodrigo Siqueira <rodriados@gmail.com>
 */
#ifndef MU_HTTP_REQUEST_H
#define MU_HTTP_REQUEST_H

typedef int socket_id;

extern void request_listen(socket_id, int);

#endif