/*!
 * \brief Creates a response for a HTTP request.
 * Implements functions relevant to the processing of a response. Each HTTP request
 * must produce a request for the client.
 * \author Rodrigo Siqueira <rodriados@gmail.com>
 */
#include <time.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "http.h"
#include "response.h"

bool response_check_public_object(char **, const char *);
bool response_check_moved_object(char **, const char *);
struct response_t response_make_error_view(enum response_code_t);
struct response_t response_make_object_view(const char *);
struct response_t response_make_moved_view(const char *, char *);

/*!
 * \fn struct response_t response_process(struct http_request_t *)
 * \brief Processes a HTTP request and produces a response for it.
 * \param http_request The HTTP request to be processed.
 * \return The produced HTTP response.
 */
struct response_t response_process(struct http_request_t *http_request)
{
    char *target;

    if (http_request->method & ~(HTTP_GET | HTTP_POST))
        return response_make_error_view(HTTP_RESPONSE_NOT_IMPLEMENTED);

    if (response_check_public_object(&target, http_request->uri.path))
        return response_make_object_view(target);

    if (response_check_moved_object(&target, http_request->uri.path))
        return response_make_moved_view(http_request->uri.path, target);

    return response_make_error_view(HTTP_RESPONSE_NOT_FOUND);
}

/*!
 * \fn struct response_t response_make_error(enum http_error_t)
 * \brief Produces a response to an error detected by the server.
 * \param error The detected error to return to the client.
 * \return The HTTP response for the given error status.
 */
struct response_t response_make_error(enum http_error_t error)
{
    switch (error) {
        case HTTP_ERROR_METHOD_INVALID:
            return response_make_error_view(HTTP_RESPONSE_NOT_IMPLEMENTED);

        case HTTP_ERROR_URI_EMPTY:
        case HTTP_ERROR_URI_TOO_LONG:
        case HTTP_ERROR_REQUEST_TOO_LONG:
        case HTTP_ERROR_HEADERS_EMPTY:
            return response_make_error_view(HTTP_RESPONSE_BAD_REQUEST);

        case HTTP_ERROR_PROTOCOL_INVALID:
            return response_make_error_view(HTTP_RESPONSE_VERSION_NOT_SUPPORTED);

        default:
            return response_make_error_view(HTTP_RESPONSE_INTERNAL_SERVER_ERROR);
    }
}

void response_add_common_headers(struct response_t *);
void response_add_file_header(struct response_t *, const char *, size_t);

/*!
 * \fn char *response_read_file(char *, size_t *)
 * \brief Reads a whole file into memory for sending it as content.
 * \param filename The name of file to be loaded and sent as content.
 * \param length The file's total content length.
 * \return The file contents.
 */
unsigned char *response_read_file(char *filename, size_t *length)
{
    FILE *file = fopen(filename, "rb");

    fseek(file, 0L, SEEK_END);
    *length = ftell(file);

    unsigned char *content = malloc(sizeof(unsigned char) * (*length + 1));

    rewind(file);
    fread(content, sizeof(unsigned char), *length, file);
    fclose(file);

    return content;
}

/*!
 * \fn struct response_t response_make_basic(enum response_code_t)
 * \brief Creates a basic HTTP response with given status code.
 * \param status The status code to respond HTTP request with.
 * \return The new basic response structure.
 */
struct response_t response_make_basic(enum response_code_t status)
{
    return {
        .protocol       = "HTTP/1.1"
      , .status_code    = status
      , .header         = malloc(sizeof(struct http_header_t) * 100)
      , .count_headers  = 0
    };
}

/*!
 * \fn struct response_t response_make_file_view(enum response_code_t, const char *)
 * \brief Creates a HTTP response of a file.
 * \param status The response's HTTP status code.
 * \param filename The name of file to be returned.
 * \return The HTTP response for the requested file.
 */
struct response_t response_make_file_view(enum response_code_t status, const char *filename)
{
    struct response_t response = response_make_basic(status);

    response.content = response_read_file(filename, &response.length);

    response_add_common_headers(&response);
    response_add_file_header(&response, filename, length);

    return response;
}

/*!
 * \fn response_add_listing_directory(char **, size_t *, size_t *, const char *, time_t)
 * \brief Adds a directory to the index listing.
 * \param buffer The buffer containing the index listing.
 * \param bufsize The listing buffer size.
 * \param consumed The total amount of bytes already consumed in buffer.
 * \param objname The name of object to be included on listing.
 * \param modified The object's last modified time.
 */
void response_add_listing_directory(
    char **buffer
  , size_t *bufsize
  , size_t *consumed
  , const char *objname
  , time_t modified
) {
    size_t bytes;
    char internal[2048];
    sprintf(internal, "addDir(\"%s\", %lu);%zn", objname, modified, &bytes);

    *consumed += bytes;

    while (*consumed > *bufsize)
        *buffer = realloc(sizeof(char) * (*bufsize *= 2));

    strcat(*buffer, internal);
}

/*!
 * \fn response_add_listing_file(char **, size_t *, size_t *, const char *, time_t, int)
 * \brief Adds a file to the index listing.
 * \param buffer The buffer containing the index listing.
 * \param bufsize The listing buffer size.
 * \param consumed The total amount of bytes already consumed in buffer.
 * \param objname The name of object to be included on listing.
 * \param modified The object's last modified time.
 * \param size The file's total size in bytes.
 */
void response_add_listing_file(
    char **buffer
  , size_t *bufsize
  , size_t *consumed
  , const char *objname
  , time_t modified
  , int size
) {
    int bytes;
    char internal[2048];
    sprintf(internal, "addFile(\"%s\", %lu, %d);%zn", objname, modified, size, &bytes);

    *consumed += bytes;

    while (*consumed > *bufsize)
        *buffer = realloc(sizeof(char) * (*bufsize *= 2));

    strcat(*buffer, internal);
}

/*!
 * void response_make_directory_listing(const char *, char **, size_t *)
 * \brief Creates a HTTP response of a directory index listing.
 * \param dirname The directory to be listed.
 * \param buffer The buffer to be used as listing.
 * \param bufsize The total size of given buffer.
 */
void response_make_directory_listing(const char *dirname, char **buffer, size_t *bufsize)
{
    char objname[2048];
    size_t consumed = strlen(*buffer);

    struct dirent *directory;
    DIR *dir = opendir(dirname);

    while ((directory = readdir(dir)) != NULL) {
        struct stat st;
        sprintf(objname, "%s/%s", dirname, directory->d_name);
        stat(objname, &st);

        if (S_ISDIR(st.st_mode))
            response_add_listing_directory(buffer, bufsize, &consumed, directory->d_name, st.st_mtime);
        else if (S_ISREG(st.st_mode))
            response_add_listing_file(buffer, bufsize, &consumed, directory->d_name, st.st_mtime, st.st_size);
    }

    *bufsize = consumed;
}

/*!
 * \fn struct response_t response_make_directory_view(enum response_code_t, const char *)
 * \brief Creates a directory index as a response to client.
 * \param status The HTTP status code to be returned.
 * \param dirname The directory to be listed.
 * \return The HTTP response created.
 */
struct response_t response_make_directory_view(enum response_code_t status, const char *dirname)
{
    char *buffer;
    struct stat objstat;

    buffer = malloc(sizeof(char) * 4096);
    sprintf(buffer, "%s/index.html", dirname);

    if (stat(buffer, &objstat) == 0)
        return response_make_file_view(buffer);

    size_t bufsize = 4096;
    
    strcpy(buffer, "<script>addTitle(\"");
    strcat(buffer, dirname);
    strcat(buffer, "\");");

    struct response_t response = response_make_file_view(status, "default/directory.html");
    response_make_directory_listing(dirname, &buffer, &bufsize);

    response.content = realloc(sizeof(unsigned char) * (response.length + bufsize + 12));
    response.content = strcat(response.content, buffer);
    response.content = strcat(response.content, "</script>");

    free(buffer);
}

/*!
 * \fn struct response_t response_make_error_view(enum response_code_t)
 * \brief Creates a response for a HTTP error status.
 * \param status The HTTP status to generate error page for.
 * \return The HTTP response for the given status.
 */
struct response_t response_make_error_view(enum response_code_t status)
{
    char filename[150];
    sprintf(filename, "default/%d.html", status);

    return response_make_file_view(status, filename);
}

/*!
 * \fn struct response_t response_make_object_view(const char *)
 * \brief Creates a HTTP response of a public object.
 * \param objname The name of the object to be returned to client.
 * \return The created HTTP response with corresponding object.
 */
struct response_t response_make_object_view(const char *objname)
{
    struct stat objstat;
    lstat(objname, &objstat);

    if (S_ISREG(objstat.st_mode))
        return response_make_file_view(HTTP_RESPONSE_OK, objname);

    if (S_ISDIR(objstat.st_mode))
        return response_make_directory_view(HTTP_RESPONSE_OK, objname);

    return response_make_error_view(HTTP_RESPONSE_INTERNAL_SERVER_ERROR);
}

/*!
 * \fn const char *response_get_mime(const char *)
 * \brief Maps file extensions to MIME types.
 * \param extension The file extension to have its MIME type found.
 * \return The corresponding extension MIME type.
 */
const char *response_get_mime(const char *extension)
{
    if (extension != NULL) {
        if (strcmp(extension, "html") == 0)  return "text/html";
        if (strcmp(extension, "txt")  == 0)  return "text/plain";
        if (strcmp(extension, "jpe")  == 0)  return "image/jpeg";
        if (strcmp(extension, "jpg")  == 0)  return "image/jpeg";
        if (strcmp(extension, "jpeg") == 0)  return "image/jpeg";
        if (strcmp(extension, "png")  == 0)  return "image/png";
        if (strcmp(extension, "gif")  == 0)  return "image/gif";
        if (strcmp(extension, "css")  == 0)  return "text/css";
        if (strcmp(extension, "js")   == 0)  return "text/javascript";
        if (strcmp(extension, "pdf")  == 0)  return "application/pdf";
    }

    return "application/octet-stream";
}

/*!
 * \fn void response_add_header(struct response_t *, const char *, const char *)
 * \brief Adds a new header to the HTTP response.
 * \param response The response to have a header added to.
 * \param key The name of header being addded.
 * \param value The header's value.
 */
void response_add_header(struct response_t *response, const char *key, const char *value)
{
    size_t count = response->count_headers;
    response->header[count].key   = malloc(sizeof(char) * strlen(key));
    response->header[count].value = malloc(sizeof(char) * strlen(value));

    strcpy(response->header[count].key, key);
    strcpy(response->header[count].value, value);

    ++response->count_headers;
}

/*!
 * \fn void response_add_common_headers(struct response_t *)
 * \brief Adds headers to response that are common to all responses.
 * \param response The target response to which headers must be added to.
 */
void response_add_common_headers(struct response_t *response)
{
    char tbuffer[80];
    time_t now = time(NULL);
    struct tm gmt = *gmtime(&now);
    strftime(time_buffer, sizeof(time_buffer), "%a, %d %b %Y %H:%M:%S %Z", &gmt);

    response_add_header(response, "Connection", "closed");
    response_add_header(response, "Server", "μHTTPd Webserver");
    response_add_header(response, "Date", time_buffer);
}

/*!
 * \fn void response_add_file_header(struct response_t *, const char *, size_t)
 * \brief Adds headers to response related to the file being returned.
 * \param response The target response to which headers must be added to.
 * \param filename The name of the file being returned by the response.
 * \param length The file's content length.
 */
void response_add_file_header(struct response_t *response, const char *filename, size_t length)
{
    char length_str[10];
    char *extension = strrchr(filename, '.');

    sprintf(length_str, "%zu", length);

    response_add_header(response, "Content-Type", response_get_mime(extension));
    response_add_header(response, "Content-Length", length_str);
}

/*!
 * \fn void response_free(struct response_t *)
 * \brief Frees up resources used by the response structure.
 * \param response The structure to have its resources freed up.
 */
void response_free(struct response_t *response)
{
    if (response != NULL) {
        for (size_t i = 0; i < response->count_headers; ++i) {
            free(response->header[i].key);
            free(response->header[i].value);
        }

        free(response->header);
        free(response->content);
    }
}
