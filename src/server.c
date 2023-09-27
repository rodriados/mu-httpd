/*!
 * mu-HTTPd: A very very simple HTTP server.
 * \file The core implementation for the server.
 * \author Rodrigo Siqueira <rodriados@gmail.com>
 * \copyright 2014-present Rodrigo Siqueira
 */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "logger.h"
#include "request.h"

#include "server.h"

/*!
 * \typedef server_request_channel_t
 * \brief The channel type by which requests are passed to server workers.
 * \since 3.0
 */
typedef request_t *server_request_channel_t;

/*!
 * \struct server_worker_t
 * \brief The payload sent by the server to initialize a worker.
 * \since 3.0
 */
typedef struct server_worker_t {
    const server_t *server;
    logger_writer_t *logger;
    server_request_channel_t *request_channel;
    uint32_t id;
} server_worker_t;

/*!
 * \struct server_internal_t
 * \brief The internal server struct for private server functions.
 * \since 3.0
 */
typedef struct server_internal_t {
    server_request_channel_t request_channel;
} server_internal_t;

/*!
 * \var g_server_mutex
 * \brief The global server lock.
 * \since 3.0
 */
static pthread_mutex_t g_server_mutex;

/*!
 * \var g_server_consumer_fence
 * \brief The condition for signaling a request can be consumed from a channel.
 * \since 3.0
 */
static pthread_cond_t g_server_consumer_fence;

/*!
 * \var g_server_producer_fence
 * \brief The condition for signaling that a request may be produced on a channel.
 * \since 3.0
 */
static pthread_cond_t g_server_producer_fence;

/*!
 * \var g_server_status
 * \brief The global server status. Used to signal user cancellation.
 * \since 3.0
 */
server_status_t g_server_status = SERVER_UNINITIALIZED;

/*!
 * \fn server_status_t server_create(server_t*, int)
 * \brief Creates a server and opens a new TCP socket.
 * \param server The server instance to be created.
 * \param connections The server's maximum number of simultaneous connections.
 * \return The server status after its creation.
 */
extern server_status_t server_create(server_t *server, int connections)
{
    struct sockaddr_in localaddr;

    memset(&localaddr, 0, sizeof(struct sockaddr_in));

    localaddr.sin_family = AF_INET;
    localaddr.sin_addr.s_addr = inet_addr(server->address);
    localaddr.sin_port = htons(server->port);

    server->socket = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP);

    bind(server->socket, (struct sockaddr*) &localaddr, sizeof(struct sockaddr_in));
    listen(server->socket, connections);

    g_server_status = errno || server->socket == -1
        ? SERVER_FAIL_CREATE_SOCKET
        : SERVER_SUCCESS;

    server->address = inet_ntoa(localaddr.sin_addr);
    server->port = ntohs(localaddr.sin_port);

    if (g_server_status == SERVER_SUCCESS)
        server->_internal = calloc(1, sizeof(server_internal_t));

    return g_server_status;
}

/*!
 * \fn request_t *server_request_channel_receive(server_request_channel_t*)
 * \brief Waits for a new request from the channel within a worker.
 * \param channel The channel to wait a new request from.
 * \return The received request connection.
 */
request_t *server_request_channel_receive(server_request_channel_t *channel)
{
    pthread_mutex_lock(&g_server_mutex);
    pthread_cond_wait(&g_server_consumer_fence, &g_server_mutex);

    request_t *request = *channel;
    *channel = (request_t*) NULL;

    pthread_cond_signal(&g_server_producer_fence);
    pthread_mutex_unlock(&g_server_mutex);

    return request;
}

/*!
 * \fn void server_request_channel_post(server_request_channel_t*, request_t*)
 * \brief Posts a new request to the channel and waits for worker to consume it.
 * \param channel The channel to send the request to.
 * \param request The request to be sent to a worker.
 */
void server_request_channel_post(server_request_channel_t *channel, request_t *request)
{
    pthread_mutex_lock(&g_server_mutex);

    *channel = request;

    pthread_cond_signal(&g_server_consumer_fence);
    pthread_cond_wait(&g_server_producer_fence, &g_server_mutex);
    pthread_mutex_unlock(&g_server_mutex);
}

/*!
 * \typedef server_cleanup_func
 * \brief The type of a server cleanup function.
 * \since 3.0
 */
typedef void (*server_cleanup_func)(void *);

/*!
 * \fn void server_cleanup_request(request_t*)
 * \brief Cleans-up a request and frees all of its resources.
 * \param request The request to cleaned-up.
 */
void server_cleanup_request(request_t *request)
{
    close(request->client);
    free(request);
}

/*!
 * \fn void server_cleanup_worker(server_worker_t*)
 * \brief Cleans-up a worker and frees all of its resources.
 * \param worker The worker to be cleaned-up.
 */
void server_cleanup_worker(server_worker_t *worker)
{
    logger_writer_finalize(worker->logger);
    free(worker);
}

/*!
 * \fn void server_worker_request_wait(server_worker_t*)
 * \brief Waits for a new request from channel and processes it.
 * \param worker The worker to wait for a request.
 */
void server_worker_request_wait(server_worker_t *worker)
{
    request_t *request = server_request_channel_receive(worker->request_channel);

    if (request != NULL) {
        pthread_cleanup_push((server_cleanup_func) &server_cleanup_request, request);
        request_process(request, worker->logger);
        pthread_cleanup_pop(true);
    }
}

/*!
 * \fn void *server_worker_thread_run(void*)
 * \brief The server's worker routine.
 * \param worker The worker to be executed.
 */
void *server_worker_thread_run(void *worker)
{
    pthread_cleanup_push((server_cleanup_func) &server_cleanup_worker, worker);

    while (g_server_status == SERVER_SUCCESS)
        server_worker_request_wait((server_worker_t*) worker);

    pthread_cleanup_pop(1);
    return NULL;
}

/*!
 * \fn pthread_t server_worker_initialize(const server_t*, logger_t*, int)
 * \brief Initializes a new server worker a sets it ready to process requests.
 * \param server The server instance to start a new worker off.
 * \param logger The logger instance to which the worker must log to.
 * \param id The new worker id.
 * \return The new worker thread id.
 */
pthread_t server_worker_initialize(const server_t *server, logger_t *logger, int id)
{
    pthread_t worker_thread;

    server_worker_t *worker = malloc(sizeof(server_worker_t));
    server_internal_t *internal = (server_internal_t*) server->_internal;

    worker->server = server;
    worker->logger = logger_writer_initialize(logger);
    worker->request_channel = &internal->request_channel;
    worker->id = id;

    pthread_create(&worker_thread, NULL, &server_worker_thread_run, (void*) worker);

    return worker_thread;
}

/*!
 * \fn server_status_t server_connection_wait(const server_t*)
 * \brief Sets the server to wait for a new client request.
 * \param server The server instance to wait for a new request.
 * \return The current server status.
 */
server_status_t server_connection_wait(const server_t *server)
{
    struct sockaddr_in client_address;

    socklen_t l = sizeof(struct sockaddr_in);
    socket_id_t client_socket = accept(server->socket, (struct sockaddr*) &client_address, &l);

    if (client_socket == -1)
        return (errno != EAGAIN && errno != EWOULDBLOCK)
            ? SERVER_FAIL_ACCEPT_CLIENT
            : SERVER_SUCCESS;

    request_t *request = malloc(sizeof(request_t));
    server_internal_t *internal = (server_internal_t*) server->_internal;

    request->client = client_socket;
    request->origin = client_address;

    // Posting the request to the channel, so a worker can consume it.
    // Control is only returned when it is confirmed that a worker has consumed from
    // the channel and consequently will process the request.
    server_request_channel_post(&internal->request_channel, request);

    return SERVER_SUCCESS;
}

/*!
 * \fn void server_force_stop(int)
 * \brief Forces the server to stop and wakes up all waiting threads.
 * \param signal (ignored)
 */
void server_force_stop(int signal)
{
    g_server_status = SERVER_STOP_REQUESTED;

    pthread_cond_broadcast(&g_server_consumer_fence);
    pthread_cond_broadcast(&g_server_producer_fence);
}

/*!
 * \fn void server_worker_finalize(pthread_t*)
 * \brief Cancels the execution of a worker thread and waits for it to finalize.
 * \param worker_thread The worker thread to be cancelled.
 */
void server_worker_finalize(pthread_t *worker_thread)
{
    pthread_cancel(*worker_thread);
    pthread_join(*worker_thread, NULL);
}

/*!
 * \fn server_status_t server_listen(const server_t*, logger_t*, int)
 * \brief Listens to requests on the created server.
 * \param server The server instance to listen to requests.
 * \param logger The logger instance to log to.
 * \param workers The number of server workers to spawn.
 * \return The server status.
 */
extern server_status_t server_listen(const server_t *server, logger_t *logger, int workers)
{
    if (g_server_status != SERVER_SUCCESS)
        return g_server_status;

    server_status_t server_status = SERVER_SUCCESS;
    pthread_t *worker_thread = calloc(workers, sizeof(pthread_t));

    pthread_mutex_init(&g_server_mutex, NULL);
    pthread_cond_init(&g_server_producer_fence, NULL);
    pthread_cond_init(&g_server_consumer_fence, NULL);

    signal(SIGINT, &server_force_stop);

    for (int i = 0; i < workers; ++i)
        worker_thread[i] = server_worker_initialize(server, logger, i);

    while ((g_server_status | server_status) == SERVER_SUCCESS)
        server_status = server_connection_wait(server);

    server_force_stop(0);

    for (int i = 0; i < workers; ++i)
        server_worker_finalize(&worker_thread[i]);

    pthread_cond_destroy(&g_server_producer_fence);
    pthread_cond_destroy(&g_server_consumer_fence);
    pthread_mutex_destroy(&g_server_mutex);

    free(worker_thread);

    return server_status == SERVER_SUCCESS
        ? g_server_status
        : server_status;
}

/*!
 * \fn const char *server_status_describe(server_status_t)
 * \brief Describes a server status with a meaning message.
 * \param status The status to be described.
 * \return The status description.
 */
extern const char *server_status_describe(server_status_t status)
{
    switch (status) {
        case SERVER_SUCCESS:
            return "Server is successful.";
        case SERVER_UNINITIALIZED:
            return "Server is uninitialized.";
        case SERVER_FAIL_CREATE_SOCKET:
            return "Server could not start on the given address and/or port.";
        case SERVER_FAIL_ACCEPT_CLIENT:
            return "Server detected an error while waiting for requests.";
        case SERVER_STOP_REQUESTED:
            return "Server stopped as requested by the user.";
        default:
            return "Unknown server error.";
    }
}

/*!
 * \fn void server_destroy(server_t*)
 * \brief Destroys a server instance and closes its connection.
 * \param server The server instance to be destroyed.
 */
extern void server_destroy(server_t *server)
{
    free(server->_internal);
    close(server->socket);
}
