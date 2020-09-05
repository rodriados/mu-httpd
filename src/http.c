/*!
 * \brief Parses and processes HTTP requests.
 * Implements functions relevant to the processing of the HTTP protocol.
 * \author Rodrigo Siqueira <rodriados@gmail.com>
 */
#include <stdbool.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>

#include "http.h"

size_t http_request_parse_method(enum http_error_t *, struct http_request_t *, char *);
size_t http_request_parse_uri(enum http_error_t *, struct http_request_t *, char *);
size_t http_request_parse_protocol(enum http_error_t *, struct http_request_t *, char *);
size_t http_request_parse_headers(enum http_error_t *, struct http_request_t *, char *);
size_t http_request_parse_contents(enum http_error_t *, struct http_request_t *, char *, size_t);

/*!
 * \fn struct http_request_t http_request_parse(enum http_error_t *, char *, size_t)
 * \brief Parses an incoming HTTP request, and builds an instance representing it.
 * \param error The error status return for the current parsing.
 * \param raw The raw request contents.
 * \param size The request's total size.
 * \return The parsed HTTP request instance.
 */
struct http_request_t http_request_parse(enum http_error_t *error, char *raw, size_t size)
{
    size_t consumed = 0;
    struct http_request_t http_request = { .raw = raw };

    *error = HTTP_ERROR_OK;

    consumed += http_request_parse_method(error, &http_request, raw);
    consumed += http_request_parse_uri(error, &http_request, raw + consumed);
    consumed += http_request_parse_protocol(error, &http_request, raw + consumed);
    consumed += http_request_parse_headers(error, &http_request, raw + consumed);
    consumed += http_request_parse_contents(error, &http_request, raw + consumed, size - consumed);

    return http_request;
}

enum http_method_t http_request_parse_map_method(const char *);

size_t http_request_parse_uri_path(enum http_error_t *, char **, char *);
size_t http_request_parse_uri_query(enum http_error_t *, struct http_param_bag_t **, char *);

/*!
 * \fn size_t http_request_parse_method(enum http_error_t *, struct http_request_t *, const char *)
 * \brief Parses the HTTP method out of the raw socket request.
 * \param error The error status return for the current parsing.
 * \param request The HTTP request being currently parsed.
 * \param raw The raw request contents.
 * \return The amount of consumed characters.
 */
size_t http_request_parse_method(enum http_error_t *error, struct http_request_t *request, char *raw)
{
    char buffer[16];

    if (*error != HTTP_ERROR_OK)
        return 0;

    size_t consumed = sscanf(raw, "%16s ", buffer);
    request->method = http_request_parse_map_method(buffer);

    if (request->method == HTTP_METHOD_UNKNOWN)
        *error = HTTP_ERROR_METHOD_INVALID;

    return consumed;
}

/*!
 * \fn enum http_method_t http_request_parse_map_method(const char *)
 * \brief Maps a method name to its corresponding enumeration value.
 * \param method The method name to be mapped.
 * \return The corresponding method value.
 */
enum http_method_t http_request_parse_map_method(const char *method)
{
    if (strcmp(method, "GET")      == 0) return HTTP_GET;
    if (strcmp(method, "POST")     == 0) return HTTP_POST;
    if (strcmp(method, "PUT")      == 0) return HTTP_PUT;
    if (strcmp(method, "DELETE")   == 0) return HTTP_DELETE;
    if (strcmp(method, "HEAD")     == 0) return HTTP_HEAD;
    if (strcmp(method, "OPTIONS")  == 0) return HTTP_OPTIONS;
    if (strcmp(method, "TRACE")    == 0) return HTTP_TRACE;
    if (strcmp(method, "CONNECT")  == 0) return HTTP_CONNECT;
    
    return HTTP_METHOD_UNKNOWN;
}

/*!
 * \fn size_t http_request_parse_uri(enum http_error_t *, struct http_request_t *, char *)
 * \brief Parses the incoming HTTP request URI.
 * \param error The error status return for the current parsing.
 * \param request The HTTP request structure to output the parsing to.
 * \param raw The raw request contents.
 * \return The amount of bytes consumed.
 */
size_t http_request_parse_uri(enum http_error_t *error, struct http_request_t *request, char *raw)
{
    if (*error != HTTP_ERROR_OK)
        return 0;

    size_t consumed = 0;
    struct http_uri_t *uri = &request->uri;

    char *end = strtok(raw, "\n\r ");
    size_t total_size = end - raw;

    uri->path = NULL;
    uri->query = NULL;

    consumed += http_request_parse_uri_path(error, &uri->path, raw);
    consumed += http_request_parse_uri_query(error, &uri->query, raw + consumed);

    return total_size + 1;
}

/*!
 * \fn void http_request_parse_uri_decode_special(char *, size_t)
 * \brief Decodes special characters sent in hexadecimal blobs.
 * \param raw The raw request contents.
 * \param size The total URI size.
 */
void http_request_parse_uri_decode_special(char *raw, size_t size)
{
    char decoded;
    size_t decoded_size = 0;

    for (size_t i = 0; i < size; ++i) {
        bool special_char = (i < size - 2) && (raw[i] == '%');

        if (special_char && isxdigit(raw[i + 1]) && isxdigit(raw[i + 2])) {
            char hex[] = {raw[i + 1], raw[i + 2], 0};
            decoded = (char) strtol(hex, NULL, 16);
            i += 2;
        } else {
            decoded = raw[i];
        }

        raw[decoded_size] = decoded;
        ++decoded_size;
    }

    raw[decoded_size] = (char) 0;
}

/*!
 * \fn size_t http_request_parse_uri_path(enum http_error_t *, char **, char *)
 * \brief Captures the HTTP request's URI path.
 * \param error The error status return for the current parsing.
 * \param path The output parameter for the parsed URI path.
 * \param raw The raw request contents.
 * \return The amount of bytes consumed.
 */
size_t http_request_parse_uri_path(enum http_error_t *error, char **path, char *raw)
{
    if (*error != HTTP_ERROR_OK)
        return 0;

    char *end = strtok(raw, "? ");
    size_t total_size = end != NULL ? end - raw : 0;

    if (total_size == 0) {
        *error = HTTP_ERROR_PATH_INVALID;
    } else {
        http_request_parse_uri_decode_special(*path = raw, total_size);
    }

    return total_size + 1;
}

/*!
 * \fn size_t http_request_parse_uri_query(enum http_error_t *, char **, char *)
 * \brief Captures the HTTP request's URI query string.
 * \param error The error status return for the current parsing.
 * \param query The output parameter for the parsed URI query string.
 * \param raw The raw request contents.
 * \return The amount of bytes consumed.
 */
size_t http_request_parse_uri_query(enum http_error_t *error, char **query, char *raw)
{
    if (*error != HTTP_ERROR_OK)
        return 0;

    char *end = strtok(raw, " ");
    size_t total_size = end != NULL ? end - raw : 0;

    if (end == NULL) {
        *error = HTTP_ERROR_URI_QUERY_INVALID;
    } else if (total_size > 1) {
        http_request_parse_uri_decode_special(*query = raw, total_size);
    }

    return total_size + 1;
}
