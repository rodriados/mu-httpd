/*!
 * \brief The HTTP parsing and processing functions and structures.
 * Declares functions and defines structures relevant to HTTP requests.
 * \author Rodrigo Siqueira <rodriados@gmail.com>
 */
#ifndef MU_HTTPD_HTTP_H
#define MU_HTTPD_HTTP_H

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
  , HTTP_ERROR_URI_EMPTY
  , HTTP_ERROR_URI_TOO_LONG
  , HTTP_ERROR_REQUEST_TOO_LONG
  , HTTP_ERROR_PROTOCOL_INVALID
  , HTTP_ERROR_HEADERS_EMPTY
};

/*!
 * \struct http_header_t
 * \brief Stores a HTTP header key-value pair.
 */
struct http_header_t {
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
    struct http_header_t *header;
    size_t count_headers;
    char *contents;
    size_t length;
    char *raw;
};

/*!
 * \enum http_code_t
 * \brief Enumerates HTTP response codes.
 */
enum http_code_t {
    HTTP_RESPONSE_OK                    = 200
  , HTTP_RESPONSE_MOVED_PERMANENTLY     = 301
  , HTTP_RESPONSE_BAD_REQUEST           = 400
  , HTTP_RESPONSE_NOT_FOUND             = 404
  , HTTP_RESPONSE_INTERNAL_SERVER_ERROR = 500
  , HTTP_RESPONSE_NOT_IMPLEMENTED       = 501
  , HTTP_RESPONSE_VERSION_NOT_SUPPORTED = 505
};

/*!
 * \struct http_response_t
 * \brief Describes a HTTP response for an incoming request.
 */
struct http_response_t {
    char protocol[16];
    enum http_code_t status_code;
    struct http_header_t *header;
    size_t count_headers;
    unsigned char *content;
    size_t length;
};

extern struct http_request_t http_request_parse(enum http_error_t *, char *, size_t);
extern void http_request_free(struct http_request_t *);

#endif
