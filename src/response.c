/*!
 * \brief Creates a response for a HTTP request.
 * Implements functions relevant to the processing of a response. Each HTTP request
 * must produce a request for the client.
 * \author Rodrigo Siqueira <rodriados@gmail.com>
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "http.h"
#include "config.h"
#include "response.h"

bool response_check_moved_object(char *, const char *);
bool response_check_public_object(char *, const char *);
struct http_response_t response_make_error_view(enum http_code_t);
struct http_response_t response_make_moved_view(const char *);
struct http_response_t response_make_object_view(const char *);

/*!
 * \fn struct http_response_t response_process(struct http_request_t *)
 * \brief Processes a HTTP request and produces a response for it.
 * \param http_request The HTTP request to be processed.
 * \return The produced HTTP response.
 */
struct http_response_t response_process(struct http_request_t *http_request)
{
    char target[BUFFER_SIZE];

    if (http_request->method & ~(HTTP_GET | HTTP_POST))
        return response_make_error_view(HTTP_RESPONSE_NOT_IMPLEMENTED);

    if (response_check_moved_object(target, http_request->uri.path))
        return response_make_moved_view(target);

    if (response_check_public_object(target, http_request->uri.path))
        return response_make_object_view(target);

    return response_make_error_view(HTTP_RESPONSE_NOT_FOUND);
}

/*!
 * \fn struct http_response_t response_make_error(enum http_error_t)
 * \brief Produces a response to an error detected by the server.
 * \param error The detected error to return to the client.
 * \return The HTTP response for the given error status.
 */
struct http_response_t response_make_error(enum http_error_t error)
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

/*!
 * \fn const char *response_status_string(enum http_code_t)
 * \brief Maps a HTTP response status code to its corresponding description.
 * \param code The code to have its string description returned.
 * \return The status code's description.
 */
const char *response_status_string(enum http_code_t code)
{
    switch (code) {
        case HTTP_RESPONSE_OK:                    return "Ok";
        case HTTP_RESPONSE_MOVED_PERMANENTLY:     return "Moved Permanently";
        case HTTP_RESPONSE_BAD_REQUEST:           return "Bad Request";
        case HTTP_RESPONSE_NOT_FOUND:             return "Not Found";
        case HTTP_RESPONSE_INTERNAL_SERVER_ERROR: return "Internal Server Error";
        case HTTP_RESPONSE_NOT_IMPLEMENTED:       return "Not Implemented";
        case HTTP_RESPONSE_VERSION_NOT_SUPPORTED: return "HTTP Version Not Supported";
        default:                                  return "Unknown";
    }
}

void response_add_header(struct http_response_t *, const char *, const char *);
void response_update_header(struct http_response_t *, const char *, const char *);
void response_add_common_headers(struct http_response_t *);
void response_add_file_header(struct http_response_t *, const char *, size_t);

/*!
 * \fn char *response_read_file(const char *, size_t *)
 * \brief Reads a whole file into memory for sending it as content.
 * \param filename The name of file to be loaded and sent as content.
 * \param length The file's total content length.
 * \return The file contents.
 */
unsigned char *response_read_file(const char *filename, size_t *length)
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
 * \fn struct http_response_t response_make_basic(enum http_code_t)
 * \brief Creates a basic HTTP response with given status code.
 * \param status The status code to respond HTTP request with.
 * \return The new basic response structure.
 */
struct http_response_t response_make_basic(enum http_code_t status)
{
    return (struct http_response_t) {
        .protocol       = "HTTP/1.1"
      , .status_code    = status
      , .header         = malloc(sizeof(struct http_header_t) * 100)
      , .count_headers  = 0
    };
}

/*!
 * \fn struct http_response_t response_make_file_view(enum http_code_t, const char *)
 * \brief Creates a HTTP response of a file.
 * \param status The response's HTTP status code.
 * \param filename The name of file to be returned.
 * \return The HTTP response for the requested file.
 */
struct http_response_t response_make_file_view(enum http_code_t status, const char *filename)
{
    struct http_response_t response = response_make_basic(status);

    response.content = response_read_file(filename, &response.length);

    response_add_common_headers(&response);
    response_add_file_header(&response, filename, response.length);

    return response;
}

/*!
 * \fn void response_append_contents(struct http_response_t *, const unsigned char *, size_t)
 * \brief Appends content to the end of a HTTP response.
 * \param response The response to have data appended to.
 * \param buffer The buffer to be appended to the end of the request.
 * \param length The length of buffer to be appended to response.
 */
void response_append_contents(struct http_response_t *response, const unsigned char *buffer, size_t length)
{
    size_t prev_size = response->length;
    size_t final_size = prev_size + length;

    response->content = realloc(response->content, sizeof(unsigned char) * (final_size + 1));
    response->length = final_size;

    char length_str[15];
    sprintf(length_str, "%zu", final_size);

    memcpy(response->content + prev_size, buffer, length);
    response_update_header(response, "Content-Length", length_str);
}

/*!
 * void response_make_directory_listing(struct http_response_t *, const char *)
 * \brief Creates a HTTP response of a directory index listing.
 * \param response The response to which the listing must be appended.
 * \param dirname The directory to be listed.
 */
void response_make_directory_listing(struct http_response_t *response, const char *dirname)
{
    struct dirent *obj;
    char tmp[BUFFER_SIZE];

    size_t length = 0, capacity = BUFFER_SIZE;
    unsigned char *buffer = malloc(sizeof(unsigned char) * capacity);

    DIR *dir = opendir(dirname);

    while ((obj = readdir(dir)) != NULL) {
        size_t size;
        struct stat st;
        sprintf(tmp, "%s/%s", dirname, obj->d_name);

        stat(tmp, &st);

        if (S_ISDIR(st.st_mode))
            sprintf(tmp, "<script>d(\"%s\", %lu);</script>%zn", obj->d_name, st.st_mtime, &size);
        else
            sprintf(tmp, "<script>f(\"%s\", %lu, %ld);</script>%zn", obj->d_name, st.st_mtime, st.st_size, &size);

        while (length + size + 1 > capacity)
            buffer = realloc(buffer, sizeof(unsigned char) * (capacity *= 2));

        memcpy(buffer + length, tmp, size + 1);
        length += size;
    }

    response_append_contents(response, buffer, length);
    free(buffer);
}

/*!
 * \fn struct http_response_t response_make_directory_view(enum http_code_t, const char *)
 * \brief Creates a directory index as a response to client.
 * \param status The HTTP status code to be returned.
 * \param dirname The directory to be listed.
 * \return The HTTP response created.
 */
struct http_response_t response_make_directory_view(enum http_code_t status, const char *dirname)
{
    char indexfile[BUFFER_SIZE];
    struct stat objstat;

    sprintf(indexfile, "%s/index.html", dirname);

    if (stat(indexfile, &objstat) == 0)
        return response_make_file_view(status, indexfile);

    struct http_response_t response = response_make_file_view(status, "default/directory.html");
    response_make_directory_listing(&response, dirname);

    return response;
}

/*!
 * \fn struct http_response_t response_make_error_view(enum http_code_t)
 * \brief Creates a response for a HTTP error status.
 * \param status The HTTP status to generate error page for.
 * \return The HTTP response for the given status.
 */
struct http_response_t response_make_error_view(enum http_code_t status)
{
    char filename[150];
    sprintf(filename, "default/%d.html", status);

    return response_make_file_view(status, filename);
}

/*!
 * \fn bool response_check_moved_object(char *, const char *)
 * \brief Checks whethe the given object has suffered a permanent move.
 * \param target The object's redirection target.
 * \param objname The name of object being currenly requested.
 * \return Has the object been moved?
 */
bool response_check_moved_object(char *target, const char *objname)
{
    char origin[BUFFER_SIZE];
    FILE *db_moved = fopen("default/.moved", "r");

    while (!feof(db_moved)) {
        fscanf(db_moved, "%s %s ", origin, target);

        if (strcmp(origin, objname) == 0)
            return true;
    }

    return false;
}

/**
 * \fn bool response_check_public_object(char *, const char *)
 * \brief Checks whether the given name is a public object.
 * \param target The final target name.
 * \param objname The object name to be checked.
 * \return Is the requested object public?
 */
bool response_check_public_object(char *target, const char *objname)
{
    struct stat st;

    sprintf(target, PUBLIC_FOLDER "%s", objname);
    stat(target, &st);

    return S_ISDIR(st.st_mode) || S_ISREG(st.st_mode);
}

/*!
 * \fn void response_add_header(struct http_response_t *, const char *, const char *)
 * \brief Adds a new header to the HTTP response.
 * \param response The response to have a header added to.
 * \param key The name of header being addded.
 * \param value The header's value.
 */
void response_add_header(struct http_response_t *response, const char *key, const char *value)
{
    size_t count = response->count_headers;
    response->header[count].key   = malloc(sizeof(char) * (strlen(key) + 1));
    response->header[count].value = malloc(sizeof(char) * (strlen(value) + 1));

    strcpy(response->header[count].key, key);
    strcpy(response->header[count].value, value);

    ++response->count_headers;
}

/*!
 * \fn void response_update_header(struct http_response_t *, const char *, const char *)
 * \brief Updates a header or adds it if it does not exist.
 * \param response The response to have a header updated.
 * \param key The name of header being updated.
 * \param value The header's new value.
 */
void response_update_header(struct http_response_t *response, const char *key, const char *value)
{
    for (size_t i = 0; i < response->count_headers; ++i) {
        if (strcmp(response->header[i].key, key) == 0) {
            response->header[i].value = realloc(response->header[i].value, sizeof(char) * (strlen(value) + 1));
            strcpy(response->header[i].value, value);
            return;
        }
    }

    response_add_header(response, key, value);
}

/**
 * \fn struct http_response_t response_make_moved_view(const char *)
 * \brief Creates a HTTP response for an object permanently moved.
 * \param target The object's new redirection target.
 * \return The newly created HTTP request.
 */
struct http_response_t response_make_moved_view(const char *target)
{
    struct http_response_t response = response_make_basic(HTTP_RESPONSE_MOVED_PERMANENTLY);
    
    response_add_common_headers(&response);
    response_add_header(&response, "Location", target);

    return response;
}

/*!
 * \fn struct http_response_t response_make_object_view(const char *)
 * \brief Creates a HTTP response of a public object.
 * \param objname The name of the object to be returned to client.
 * \return The created HTTP response with corresponding object.
 */
struct http_response_t response_make_object_view(const char *objname)
{
    struct stat objstat;
    stat(objname, &objstat);

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
        if (strcmp(extension + 1, "html") == 0)  return "text/html";
        if (strcmp(extension + 1, "txt")  == 0)  return "text/plain";
        if (strcmp(extension + 1, "jpe")  == 0)  return "image/jpeg";
        if (strcmp(extension + 1, "jpg")  == 0)  return "image/jpeg";
        if (strcmp(extension + 1, "jpeg") == 0)  return "image/jpeg";
        if (strcmp(extension + 1, "png")  == 0)  return "image/png";
        if (strcmp(extension + 1, "gif")  == 0)  return "image/gif";
        if (strcmp(extension + 1, "css")  == 0)  return "text/css";
        if (strcmp(extension + 1, "js")   == 0)  return "text/javascript";
        if (strcmp(extension + 1, "pdf")  == 0)  return "application/pdf";
    }

    return "application/octet-stream";
}

/*!
 * \fn void response_add_common_headers(struct http_response_t *)
 * \brief Adds headers to response that are common to all responses.
 * \param response The target response to which headers must be added to.
 */
void response_add_common_headers(struct http_response_t *response)
{
    char tbuffer[80];
    time_t now = time(NULL);
    struct tm gmt = *gmtime(&now);
    strftime(tbuffer, sizeof(tbuffer), "%a, %d %b %Y %H:%M:%S %Z", &gmt);

    response_add_header(response, "Connection", "closed");
    response_add_header(response, "Server", "μHTTPd Webserver");
    response_add_header(response, "Date", tbuffer);
}

/*!
 * \fn void response_add_file_header(struct http_response_t *, const char *, size_t)
 * \brief Adds headers to response related to the file being returned.
 * \param response The target response to which headers must be added to.
 * \param filename The name of the file being returned by the response.
 * \param length The file's content length.
 */
void response_add_file_header(struct http_response_t *response, const char *filename, size_t length)
{
    char length_str[25];
    char *extension = strrchr(filename, '.');

    sprintf(length_str, "%zu", length);

    response_add_header(response, "Content-Type", response_get_mime(extension));
    response_add_header(response, "Content-Length", length_str);
}

/*!
 * \fn void response_free(struct http_response_t *)
 * \brief Frees up resources used by the response structure.
 * \param response The structure to have its resources freed up.
 */
void response_free(struct http_response_t *response)
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
