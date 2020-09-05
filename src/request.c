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
#include <signal.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

#include "request.h"

#define PAGE_SIZE 4096

void request_process(void *);
char *request_read(struct request_t *);
void request_free(char *);

/*!
 * \var keep_running
 * \brief Informs whether the server should keep running and listening to new requests.
 */
static volatile int keep_running = 1;

/*!
 * \fn void server_interrupt(int)
 * \brief Interrupts the server execution.
 * \param _ Unused parameter.
 */
void server_interrupt(int _)
{
    keep_running = 0;
}

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

    signal(SIGINT, server_interrupt);

    while(keep_running) {
        struct request_t *request = &request_list[current_id];

        if (request->thread)
            pthread_join(request->thread, NULL);

        request->client = accept(server, (sockaddr *) &request->remoteaddr, sizeof(sockaddr_in));
        pthread_create(&request->thread, NULL, request_process, (void *) request);

        current_id = (current_id + 1) % max_threads;
    }

    pthread_exit(NULL);
    free(request_list);
}

/*!
 * \fn void request_process(void *)
 * \brief Processes a request and returns a response to client.
 * \param request The request to be processed.
 */
void request_process(void *request)
{
    size_t length;
    enum http_error_t error;

    char *request_buffer = request_read((struct request_t *) request, &length);
    struct http_request_t http_request = http_request_parse(&error, request_buffer, length);

    http_request_free(&http_request);
}

/*!
 * \fn char *request_read(struct request_t *, size_t *)
 * \brief Reads the request into memory.
 * \param request The request to be read into memory.
 * \param length The total request length.
 * \return The request's buffer.
 */
char *request_read(struct request_t *request, size_t *length)
{
    size_t offset = 0;

    char *buffer = malloc(sizeof(char) * PAGE_SIZE);
    size_t bytes_read = recv(request->client, buffer, PAGE_SIZE, 0);

    while (bytes_read >= PAGE_SIZE) {
        offset += bytes_read;
        buffer = realloc(buffer, sizeof(char) * (offset + PAGE_SIZE));
        bytes_read = recv(request->client, &buffer[offset], PAGE_SIZE, MSG_DONTWAIT);
    }

    *length = offset + bytes_read;
    buffer[*length] = (char) 0;

    return buffer;
}

/*!
 * \fn void request_free(char *)
 * \brief Frees resources needed for reading the request into memory.
 * \param request The target request to be freed.
 */
void request_free(char *request)
{
    if (request != NULL) {
        free(request);
    }
}
