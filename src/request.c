/*!
 * mu-HTTPd: A very very simple HTTP server.
 * \file The core implementation for processing user requests.
 * \author Rodrigo Siqueira <rodriados@gmail.com>
 * \copyright 2014-present Rodrigo Siqueira
 */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

#include "config.h"
#include "response.h"
#include "http.h"
#include "logger.h"

#include "request.h"

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

/*!
 * \fn void request_write_response(struct request_t *, struct http_response_t *)
 * \brief Sends a response back to the request client.
 * \param request The request to be responded.
 * \param response The request's response.
 */
void request_write_response(struct request_t *request, struct http_response_t *response)
{
    char buffer[2048];
    const char *status_str = response_status_string(response->status_code);

    sprintf(buffer, "%s %d %s\r\n", response->protocol, response->status_code, status_str);
    send(request->client, buffer, strlen(buffer), MSG_MORE);

    for (size_t i = 0; i < response->count_headers; ++i) {
        sprintf(buffer, "%s: %s\r\n", response->header[i].key, response->header[i].value);
        send(request->client, buffer, strlen(buffer), MSG_MORE);
    }

    send(request->client, "\r\n", 2, MSG_MORE);
    send(request->client, response->content, response->length, 0);
}

/*!
 * \fn void request_process(request_t*, logger_writer_t*)
 * \brief Processes a request and sends a response to user.
 * \param request The request to be processed.
 * \param logger_writer The logger writer instance to log to.
 */
extern void request_process(request_t *request, logger_writer_t *logger_writer)
{
    size_t length;
    char *request_buffer;
    time_t t = time(NULL);

    enum http_error_t error = request_read(request, &request_buffer, &length);
    struct http_request_t http_request = http_request_parse(&error, request_buffer, length);

    struct http_response_t http_response = error == HTTP_ERROR_OK
        ? response_process(&http_request)
        : response_make_error(error);

    logger_entry_t log_entry = {
        .level = LOGGER_LEVEL_INFO
      , .datetime = *localtime(&t)
      , .http_method = http_request.method
      , .http_code = http_response.status_code
      , .http_uri = http_request.uri
    };

    request_write_response(request, &http_response);
    logger_write(logger_writer, &log_entry);

    http_request_free(&http_request);
    response_free(&http_response);
}
