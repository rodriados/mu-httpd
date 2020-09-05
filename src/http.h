/*!
 * \brief The HTTP parsing and processing functions and structures.
 * Declares functions and defines structures relevant to HTTP requests.
 * \author Rodrigo Siqueira <rodriados@gmail.com>
 */
#ifndef MU_HTTP_HTTP_H
#define MU_HTTP_HTTP_H

/*!
 * \enum http_method_t
 * \brief Enumerates all HTTP methods so they can be easily referenced in code.
 */
enum http_method_t {
    HTTP_GET            = 0x0001
  , HTTP_POST           = 0x0002
  , HTTP_PUT            = 0x0004
  , HTTP_DELETE         = 0x0008
  , HTTP_HEAD           = 0x0010
  , HTTP_OPTIONS        = 0x0020
  , HTTP_TRACE          = 0x0040
  , HTTP_CONNECT        = 0x0080
  , HTTP_METHOD_UNKNOWN = 0x0000
};

/*!
 * \enum http_error_t
 * \brief Enumerates HTTP parsing error status.
 */
enum http_error_t {
    HTTP_ERROR_OK = 0
  , HTTP_ERROR_METHOD_INVALID
  , HTTP_ERROR_PATH_INVALID
  , HTTP_ERROR_URI_QUERY_INVALID
};

struct http_param_bag_t {
    char *key;
    char *value;
};

/*!
 * \struct http_uri_t
 * \brief Stores the HTTP request's path and query values.
 */
struct http_uri_t {
    char *path;
    char *query;
};

/*!
 * \struct http_request_t
 * \brief Groups up relevant info about a HTTP request.
 */
struct http_request_t {
    struct http_uri_t uri;
    enum http_method_t method;
    char protocol[16];
    struct http_param_bag_t *header;
    char *contents;
    size_t length;
    char *raw;
};

extern struct http_request_t http_request_parse(enum http_error_t *, char *, size_t);
extern void http_request_free(struct http_request_t *);

#endif