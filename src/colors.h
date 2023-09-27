/*!
 * mu-HTTPd: A very very simple HTTP server.
 * \file Macros for coloring messages sent to a user's terminal.
 * \author Rodrigo Siqueira <rodriados@gmail.com>
 * \copyright 2014-present Rodrigo Siqueira
 */
#ifndef MU_HTTPD_COLORS_H
#define MU_HTTPD_COLORS_H

#define BRIGHT    "\033[1m"
#define BACKRED   "\033[41m"
#define BACKGREEN "\033[42m"
#define RESETALL  "\033[0m"

#define FG_INFO(str)    BRIGHT str RESETALL
#define BG_WARNING(str) BACKRED str RESETALL
#define BG_SUCCESS(str) BACKGREEN str RESETALL

#endif
