/*!
 * \brief Processes requests in a multi-threaded environment.
 * Implements functions relevant to the processing of a request. Each request is
 * treated independently in its own thread.
 * \author Rodrigo Siqueira <rodriados@gmail.com>
 */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

#include "config.h"
#include "request.h"
#include "http.h"

void *request_process(void *);
enum http_error_t request_read(struct request_t *, char **, size_t *);
void request_write_response(struct request_t *, struct response_t *);

/*!
 * \fn void request_listen(socket_id, size_t)
 * \brief Listens to the socket and processes requests.
 * \param server The server's socket id.
 * \param max_threads The maximum number of threads to be spawned.
 */
void request_listen(socket_id server, size_t max_threads)
{
    size_t current_id = 0;
    struct request_t *request_list = calloc(max_threads, sizeof(struct request_t));
    socklen_t length = sizeof(struct sockaddr_in);

    while(1) {
        struct request_t *request = &request_list[current_id];

        if (request->thread)
            pthread_join(request->thread, NULL);

        request->client = accept(server, (struct sockaddr *) &request->remoteaddr, &length);
        pthread_create(&request->thread, NULL, request_process, (void *) request);

        current_id = (current_id + 1) % max_threads;
    }

    pthread_exit(NULL);
    free(request_list);
}

/*!
 * \fn void request_process(void *)
 * \brief Processes a request and returns a response to client.
 * \param raw_request The request to be processed.
 */
void *request_process(void *raw_request)
{
    size_t length;
    char *request_buffer;
    enum http_error_t error = HTTP_ERROR_OK;

    struct response_t response;
    struct request_t *request = (struct request_t *) raw_request;

    error = request_read(request, &request_buffer, &length);
    struct http_request_t http_request = http_request_parse(&error, request_buffer, length);

    if (error == HTTP_ERROR_OK) response = response_process(&http_request);
    else                        response = response_make_error(error);

    request_write_response(request, response);

    http_request_free(&http_request);
    response_free(&response);

    return NULL;
}

/*!
 * \fn enum http_error_t request_read(struct request_t *, char **, size_t *)
 * \brief Reads the request into memory.
 * \param request The request to be read into memory.
 * \param buffer The request's raw buffer.
 * \param length The total request length.
 * \return The error status for reading the request.
 */
enum http_error_t request_read(struct request_t *request, char **buffer, size_t *length)
{
    size_t offset = 0;

    *buffer = malloc(sizeof(char) * PAGE_SIZE);
    size_t bytes_read = recv(request->client, *buffer, PAGE_SIZE, 0);

    while (bytes_read >= PAGE_SIZE) {
        offset += bytes_read;

        if (offset + PAGE_SIZE > MAX_REQUEST_SIZE)
            return HTTP_ERROR_REQUEST_TOO_LONG;

        *buffer = realloc(*buffer, sizeof(char) * (offset + PAGE_SIZE));
        bytes_read = recv(request->client, *buffer + offset, PAGE_SIZE, MSG_DONTWAIT);
    }

    *length = offset + bytes_read;
    *(*buffer + *length) = (char) 0;

    return HTTP_ERROR_OK;
}
