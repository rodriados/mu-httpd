/*!
 * \brief The response processing functions.
 * Exposes functions which are relevant for the processing of outgoing responses.
 * \author Rodrigo Siqueira <rodriados@gmail.com>
 */
#ifndef MU_HTTPD_RESPONSE_H
#define MU_HTTPD_RESPONSE_H

#include <stdint.h>

#include "http.h"

/*!
 * \enum response_code_t
 * \brief Enumerates HTTP response codes.
 */
enum response_code_t {
    HTTP_RESPONSE_OK                    = 200
  , HTTP_RESPONSE_MOVED_PERMANENTLY     = 301
  , HTTP_RESPONSE_BAD_REQUEST           = 400
  , HTTP_RESPONSE_NOT_FOUND             = 404
  , HTTP_RESPONSE_INTERNAL_SERVER_ERROR = 500
  , HTTP_RESPONSE_NOT_IMPLEMENTED       = 501
  , HTTP_RESPONSE_VERSION_NOT_SUPPORTED = 505
};

/*!
 * \struct response_t
 * \brief Describes a HTTP response for an incoming request.
 */
struct response_t {
    char protocol[16];
    enum response_code_t status_code;
    struct http_header_t *header;
    size_t count_headers;
    unsigned char *content;
    size_t length;
};

extern struct response_t response_process(struct http_request_t *);
extern struct response_t response_make_error(enum http_error_t);
extern void response_free(struct response_t *);

#endif
