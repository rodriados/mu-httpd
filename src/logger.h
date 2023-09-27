/*!
 * mu-HTTPd: A very very simple HTTP server.
 * \file The types and functions declarations for the logger.
 * \author Rodrigo Siqueira <rodriados@gmail.com>
 * \copyright 2014-present Rodrigo Siqueira
 */
#ifndef MU_HTTPD_LOG_H
#define MU_HTTPD_LOG_H

#include <stdint.h>
#include <stdio.h>
#include <time.h>

#include "http.h"

/*!
 * \enum logger_level_t
 * \brief The logger level enumeration.
 * \since 3.0
 */
typedef enum logger_level_t {
    LOGGER_LEVEL_DEBUG = 0
  , LOGGER_LEVEL_INFO  = 1
  , LOGGER_LEVEL_WARNING
  , LOGGER_LEVEL_ERROR
  , LOGGER_LEVEL_FATAL
} logger_level_t;

/*!
 * \typedef logger_sink_t
 * \brief The type for a logger sink.
 * \since 3.0
 */
typedef FILE *logger_sink_t;

/*!
 * \struct logger_t
 * \brief The logger type.
 * \since 3.0
 */
typedef struct logger_t {
    uint32_t sink_count;
    uint32_t logged_lines;
    void *_internal;
} logger_t;

/*!
 * \struct logger_writer_t
 * \brief The logger writer type.
 * \since 3.0
 */
typedef struct logger_writer_t {
    logger_t *logger;
    void *_internal;
} logger_writer_t;

/*!
 * \struct logger_entry_t
 * \brief An entry describes information of a line to be written to a sink.
 * \since 3.0
 */
typedef struct logger_entry_t {
    logger_level_t level;
    struct tm datetime;
    enum http_method_t http_method;
    enum http_code_t http_code;
    struct http_uri_t http_uri;
} logger_entry_t;

/*
 * Forward declaration of logger instance functions.
 * These functions are needed for creating and interacting with the logger and sinks.
 */
extern logger_t logger_initialize();
extern void logger_file_sink_add(logger_t*, logger_sink_t);
extern void logger_finalize(logger_t*);

/*
 * Forward declaration of logger writer instance functions.
 * These functions are needed to create and use logger writers.
 */
extern logger_writer_t *logger_writer_initialize(logger_t*);
extern void logger_write(const logger_writer_t*, const logger_entry_t*);
extern void logger_writer_finalize(logger_writer_t*);

#endif
