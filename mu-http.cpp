/*! \file mu-http.cpp
 * \brief Software's main file.
 * 
 * This file contains the software's entry point, responsible for its initialization,
 * and the server's loop. It keeps references to external libraries used. Its should
 * get as parameter the communication port to be used by AddressIn. Runs the server's
 * basic configuration routines (socket, bind and listen). Although this file's
 * documentation is in English, all of original comments were in Portuguese.
 *
 * \mainpage Paper Report
 * \author Marcos Paulo Massao Iseki <mm.iseki@gmail.com>
 * \author Rodrigo Albuquerque de Oliveira Siqueira <rodriados@gmail.com>
 * \author Thiago Yasutaka Ikeda <thiagoyasutaka@gmail.com>
 * \date 23 Oct 2014
 *
 * \section intro_sec Introduction.
 * This paper's objective was the implementation of a webserver's basic functionalities.
 * The webserver must allow HTTP clients (using browsers such Firefox, Chrome or IE) to
 * connect to the server and download files from it.
 * The webserver must implement the methods GET and POST as part of the HTTP protocol,
 * using the TCP/IP protocols to transfer HTML pages or files.
 *
 * \section implement_sec Implementations
 * The following functionalities were implemented:
 * \li HTTP GET method
 * \li HTTP POST method
 * \li Directory browsing
 * \li Concurrency
 * \li Permanent redirection
 * \li Log registry
 *
 * \section makefile_sec How to compile and run
 * To compile, using a command line terminal, run the makefile in a Linux computer
 * with a GNU C++11 compiler support.
 *
 * To run, use ./mu-http [port]
 * \see main, run, HTTP
 */
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <cerrno>

#include "run.h"
#include "colors.h"

using namespace std;

void hello();
void abort(AddressIn);
void confirm(AddressIn);

//! Software's main function.
/*! \fn main(int, const char **)
 * Starts, manages and finishes the software's execution.
 * \param argc Number of arguments passed by command line.
 * \param argv The arguments passed by command line.
 */
int main(int argc, const char **argv)
{
	Socket server;
	AddressIn localaddr;

	hello();

	memset(&localaddr, 0, sizeof(AddressIn));
	localaddr.sin_family = AF_INET;
	localaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	localaddr.sin_port = htons(argc > 1 ? atoi(argv[1]) : 8080);

	server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	bind(server, (Address *)&localaddr, sizeof(AddressIn));
	listen(server, 20);

	errno ? abort(localaddr) : confirm(localaddr);

	run(server);
	close(server);

	style(RESETALL);

	return 0;
}

//! Greeting function.
/*! \fn hello()
 * Greets the user with a welcome message and software identification.
 */
void hello()
{
	style(BRIGHT);
	cout << endl;
	cout << SPACE << "μHTTP Hipertext Transfer Protocol Server." << endl;
	cout << endl;

	style(RESETALL);
}

//! Abort message function.
/*! \fn abort(AddressIn)
 * Shows an error message, and finishes the software with an error status.
 * Nevertheless, no information about the error is given.
 * \param addr The address the server tried to bind to.
 */
void abort(AddressIn addr)
{
	foreground(RED);
	style(BRIGHT);
	cout << SPACE << "Error:" << endl;
	cout << SPACE << "It was not possible to bind to adress ";
	cout << inet_ntoa(addr.sin_addr) << ":" << ntohs(addr.sin_port) << endl;

	style(RESETALL);
 	cout << endl;

 	exit(1);
}

//! Confirmation message function.
/*! \fn confirm(AddressIn)
 * Shows a success message, and tells the user the software was correctly
 * booted and can keep up with its work.
 * \param addr The address reserved to the webserver's connections.
 */
void confirm(AddressIn addr)
{
	foreground(GREEN);
	style(BRIGHT);
	cout << SPACE << "Sucesso!" << endl;
	cout << SPACE << "Esperando conexões no endereço ";
	cout << inet_ntoa(addr.sin_addr) << ":" << ntohs(addr.sin_port) << endl;

	style(RESETALL);
 	cout << endl;
}
