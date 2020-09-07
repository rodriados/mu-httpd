/*!
 * \brief Functions responsible for managing request logs.
 * Implements a requests log. These log files are useful when trying to trace down
 * errors or unusual behaviour from clients.
 * \author Rodrigo Siqueira <rodriados@gmail.com>
 */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <time.h>

#include "log.h"
#include "http.h"

/*!
 * \var logfile
 * \brief The pointer to the file in which the log will be stored.
 */
FILE *logfile;

/*!
 * \fn void log_init(const char *)
 * \brief Initializes the log at the given file.
 * \param filename The name of file to use as log.
 */
void log_init(const char *filename)
{
    logfile = fopen(filename, "a");
}


/*!
 * \fn void log_finalize()
 * \brief Finalizes the log and closes the file.
 */
void log_finalize()
{
    fclose(logfile);
}

/*!
 * \fn const char *log_method_to_str(enum http_method_t)
 * \brief Gets the verb string of the a HTTP request method.
 * \param method The method to get the verb of.
 * \return The requested method's verb string.
 */
const char *log_method_to_str(enum http_method_t method)
{
    switch (method) {
        case HTTP_GET:      return "GET";
        case HTTP_POST:     return "POST";
        case HTTP_PUT:      return "PUT";
        case HTTP_DELETE:   return "DELETE";
        case HTTP_HEAD:     return "HEAD";
        case HTTP_OPTIONS:  return "OPTIONS";
        case HTTP_TRACE:    return "TRACE";
        case HTTP_CONNECT:  return "CONNECT";
        default:            return "METHOD_UNKNOWN";
    }
}

/*!
 * \fn void log_write(struct sockaddr_in *, struct http_request_t *, struct http_response_t *)
 * \brief Writes a line to the HTTP requests log.
 * \param remote The client's IP address.
 * \param request The incoming HTTP request to be logged.
 * \param response The outgoing HTTP response to be logged.
 */
void log_write(struct sockaddr_in *remote, struct http_request_t *request, struct http_response_t *response)
{
    char tbuffer[80];
    time_t now;

    time(&now);
    strftime(tbuffer, 80, "[%d.%m.%Y %H:%M:%S]", localtime(&now));

    fprintf(
        logfile
      , "%s %d %s %s %s\n"
      , tbuffer
      , response->status_code
      , inet_ntoa(remote->sin_addr)
      , log_method_to_str(request->method)
      , request->uri.path
    );
}
