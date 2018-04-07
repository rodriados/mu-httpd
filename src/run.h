/*! \file run.h
 * \brief Arquivo de cabeçalho run.h
 * \author Marcos Paulo Massao Iseki <mm.iseki@gmail.com>
 * \author Rodrigo Albuquerque de Oliveira Siqueira <rodriados@gmail.com>
 * \author Thiago Yasutaka Ikeda <thiagoyasutaka@gmail.com>
 * \date 23 Oct 2014
 *
 * Contém a declaração das bibliotecas externas utilizadas e da função
 * de loop do servidor.
 * \see run
 */
#pragma once

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

typedef sockaddr Address;
typedef sockaddr_in AddressIn;
typedef int Socket;

extern void run(Socket);