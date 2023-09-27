/*!
 * mu-HTTPd: A very very simple HTTP server.
 * \file The implementation of core logger functions.
 * \author Rodrigo Siqueira <rodriados@gmail.com>
 * \copyright 2014-present Rodrigo Siqueira
 */
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "http.h"
#include "logger.h"

#define SINK_INCREMENT 5

/*!
 * \struct logger_sink_group_t
 * \brief Describes a group of sinks linked to a logger instance.
 * \since 3.0
 */
typedef struct logger_sink_group_t {
    uint32_t capacity;
    logger_sink_t *sink_list;
} logger_sink_group_t;

/*!
 * \var g_logger_mutex
 * \brief The global logger lock.
 * \since 3.0
 */
pthread_mutex_t g_logger_mutex;

/*!
 * \fn logger_t logger_initialize()
 * \brief Initializes a new logger instance.
 * \return The new logger instance.
 */
extern logger_t logger_initialize()
{
    logger_t logger;

    pthread_mutex_init(&g_logger_mutex, NULL);

    logger.sink_count = 0;
    logger.logged_lines = 0;
    logger._internal = calloc(1, sizeof(logger_sink_group_t));

    return logger;
}

/*!
 * \fn void logger_file_sink_add(logger_t*, logger_sink_t)
 * \brief Links a new sink to an existing logger instance.
 * \param logger The logger instance to add the new sink to.
 * \param sink The sink to be linked to the logger.
 */
extern void logger_file_sink_add(logger_t *logger, logger_sink_t sink)
{
    logger_sink_group_t *group = (logger_sink_group_t*) logger->_internal;

    if (logger->sink_count + 1 > group->capacity) {
        group->capacity += SINK_INCREMENT;
        group->sink_list = realloc(group->sink_list, group->capacity * sizeof(logger_sink_t));
    }

    group->sink_list[logger->sink_count] = sink;
    ++logger->sink_count;
}

/*!
 * \fn logger_writer_t *logger_writer_initialize(logger_t*)
 * \brief Creates a new log-writer from a logger instance.
 * \param logger The logger instance to create writer from.
 * \return The new logger writer instance.
 */
extern logger_writer_t *logger_writer_initialize(logger_t *logger)
{
    logger_writer_t *writer = malloc(sizeof(logger_writer_t));

    writer->logger = logger;
    writer->_internal = logger->_internal;

    return writer;
}

const char *logger_describe_http_method(enum http_method_t);
const char *logger_describe_level(logger_level_t);

/*!
 * \fn void logger_write_entry_to_sink(logger_sink_t*, const logger_entry_t*)
 * \brief Writes a new entry to a logger sink.
 * \param sink The sink to write a new entry to.
 * \param entry The entry to be logged.
 */
void logger_write_entry_to_sink(logger_sink_t *sink, const logger_entry_t *entry)
{
    char datetime_buffer[128];
    strftime(datetime_buffer, 128, "%c", &entry->datetime);

    fprintf(
        (FILE*) *sink
      , "%s [%s] %d %s %s\n"
      , datetime_buffer
      , logger_describe_level(entry->level)
      , entry->http_code
      , logger_describe_http_method(entry->http_method)
      , entry->http_uri.path
    );
}

/*!
 * \fn void logger_write(const logger_writer_t*, const logger_entry_t*)
 * \brief Writes a new entry to every sink linked to the logger writer.
 * \param writer The logger writer instance to write a new entry to.
 * \param entry The entry to be logged.
 */
extern void logger_write(const logger_writer_t *writer, const logger_entry_t *entry)
{
    logger_sink_group_t *group = (logger_sink_group_t*) writer->_internal;

    pthread_mutex_lock(&g_logger_mutex);

    for (int i = 0; i < writer->logger->sink_count; ++i) {
        logger_sink_t *sink = &group->sink_list[i];
        logger_write_entry_to_sink(sink, entry);
    }

    ++writer->logger->logged_lines;
    pthread_mutex_unlock(&g_logger_mutex);
}

/*!
 * \fn void logger_writer_finalize(logger_writer_t*)
 * \brief Closes and finalizes execution for a logger writer instance.
 * \param writer The logger writer instance to finalize.
 */
extern void logger_writer_finalize(logger_writer_t *writer)
{
    free(writer);
}

/*!
 * \fn void logger_finalize(logger_t*)
 * \brief Closes and finalizes execution for a logger instance.
 * \param logger The logger instance to finalize.
 */
extern void logger_finalize(logger_t *logger)
{
    logger_sink_group_t *group = (logger_sink_group_t*) logger->_internal;

    pthread_mutex_destroy(&g_logger_mutex);

    free(group->sink_list);
    free(group);
}

/*!
 * \fn const char *logger_describe_http_method(enum http_method_t)
 * \brief Gets the verb string of the a HTTP request method.
 * \param method The method to get the verb of.
 * \return The requested method's verb string.
 */
const char *logger_describe_http_method(enum http_method_t method)
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
 * \fn const char *logger_describe_level(logger_level_t)
 * \brief Describes a logger level.
 * \param level The level to be described.
 * \return The logger level description.
 */
const char *logger_describe_level(logger_level_t level)
{
    switch (level) {
        case LOGGER_LEVEL_INFO:     return "INFO";
        case LOGGER_LEVEL_WARNING:  return "WARN";
        case LOGGER_LEVEL_ERROR:    return "ERROR";
        case LOGGER_LEVEL_FATAL:    return "FATAL";
        case LOGGER_LEVEL_DEBUG:
        default:                    return "DEBUG";
    }
}
