/*!
 * \brief The server's configuration and limit values.
 * Here you can see your personal preferences and configurations.
 * \author Rodrigo Siqueira <rodriados@gmail.com>
 */
#ifndef MU_HTTPD_CONFIG_H
#define MU_HTTPD_CONFIG_H

#define MAX_THREADS         50
#define MAX_CONNECTIONS     50

#define DEFAULT_ADDR        "127.0.0.1"
#define DEFAULT_PORT        8080

#define PAGE_SIZE           4096
#define BUFFER_SIZE         2048
#define MAX_REQUEST_SIZE    52428800
#define MAX_URL_SIZE        2048

#define PUBLIC_FOLDER       "www"
#define LOG_FILE            "log/requests.txt"

#endif
