/*!
 * \brief Macros for colorful prints.
 * This file contains a list of macros which allow us to print colorful messages
 * on terminal. These macros must be used concatenated to the string to be printed.
 * \author Rodrigo Siqueira <rodriados@gmail.com>
 */
#ifndef MU_HTTP_COLORS_H
#define MU_HTTP_COLORS_H

#define SPACE "    "

#define FOREBLACK   "\033[30m"
#define FORERED     "\033[31m"
#define FOREGREEN   "\033[32m"
#define FOREYELLOW  "\033[33m"
#define FOREBLUE    "\033[34m"
#define FOREMAGENTA "\033[35m"
#define FORECYAN    "\033[36m"
#define FOREWHITE   "\033[37m"
#define FORENORMAL  "\033[39m"

#define BACKBLACK   "\033[40m"
#define BACKRED     "\033[41m"
#define BACKGREEN   "\033[42m"
#define BACKYELLOW  "\033[43m"
#define BACKBLUE    "\033[44m"
#define BACKMAGENTA "\033[45m"
#define BACKCYAN    "\033[46m"
#define BACKWHITE   "\033[47m"
#define BACKNORMAL  "\033[49m"

#define BRIGHT      "\033[1m"
#define DIM         "\033[2m"
#define NORMAL      "\033[22m"
#define RESETALL    "\033[0m"
#define UNDERLINE   "\033[4m"
#define BLINKSLOW   "\033[5m"
#define BLINKRAPID  "\033[6m"
#define ITALIC      "\033[3m"
#define NEGATIVE    "\033[7m"

#endif
