/*!
 * \brief The response processing functions.
 * Exposes functions which are relevant for the processing of outgoing responses.
 * \author Rodrigo Siqueira <rodriados@gmail.com>
 */
#ifndef MU_HTTPD_RESPONSE_H
#define MU_HTTPD_RESPONSE_H

#include <stdint.h>

#include "http.h"

extern struct http_response_t response_process(struct http_request_t *);
extern struct http_response_t response_make_error(enum http_error_t);
extern void response_free(struct http_response_t *);
extern const char *response_status_string(enum http_code_t);

#endif
