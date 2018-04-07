/*! \file run.cpp
 * \brief Arquivo de cabeçalho run.cpp
 * \author Marcos Paulo Massao Iseki <mm.iseki@gmail.com>
 * \author Rodrigo Albuquerque de Oliveira Siqueira <rodriados@gmail.com>
 * \author Thiago Yasutaka Ikeda <thiagoyasutaka@gmail.com>
 * \date 23 Oct 2014
 *
 * Contém a declaração das bibliotecas externas utilizadas e da função
 * de loop do servidor.
 * \see run, execute, process
 */
#include <iostream>
#include <cstdlib>
#include <sstream>
#include <thread>

#include "colors.h"
#include "http.hpp"
#include "log.hpp"
#include "run.h"

using namespace std;

void execute(Socket, const AddressIn&, const string&);
void process(Socket, const AddressIn&, thread *);

//! Loop de execução do execução
/*! \fn run(Socket)
 * Aceita conexões com clientes e administra as requisições por elas
 * feitas paralelamente atravez de um thread.
 * \param server Socket de rede do servidor.
 */
void run(Socket server)
{
	Socket client;
	AddressIn remoteaddr;
	thread *parallel;
	unsigned int length = sizeof(remoteaddr);

	while(true) {
		client = accept(server, (Address *)&remoteaddr, &length);
		parallel = new thread(process, client, remoteaddr, parallel);
	}
}

//! Recebe e processa requisição de cliente
/*! \fn process(Socket, const AddressIn&, thread *)
 * Recebe todos os dados da requisição feita pelo cliente e a processa
 * para preparar uma resposta ao cliente.
 * \param client Socket de rede do cliente.
 * \param remote Endereço de rede do cliente.
 * \param thread Referência de thread para mantê-lo ativo.
 */
void process(Socket client, const AddressIn& remote, thread *actual)
{
	int bytes;
	char buffer[1000];
	string received;

	received.clear();
	bytes = recv(client, buffer, 999, 0);

	while(bytes > 0) {
		buffer[bytes] = '\0';
		received += buffer;
		bytes = recv(client, buffer, 999, MSG_DONTWAIT);
	}

	execute(client, remote, received);
	close(client);
}

//! Executa uma requisição de cliente
/*!	\fn execute(Socket, const AddressIn&, const string&)
 * Executa e interpreta a requisição enviada pelo cliente; produz e envia
 * uma resposta para o que foi recebido.
 * \param client Socket de rede do cliente.
 * \param remote Endereço de rede do cliente.
 * \param received Dados recebidos como requisição.
 */
void execute(Socket client, const AddressIn& remote, const string& received)
{
	HTTP::Request request(received, remote);
	HTTP::Response response(request);

	string content;
	int length = response.generate(content);

	stringstream logstr;

	logstr << inet_ntoa(remote.sin_addr);
	logstr << " " << request.method + " " + request.target;

	logall << logstr.str();

	send(client, content.c_str(), length, 0);

	if(request.get.size())
		cout << SPACE << request.target << " GET " << request.get << endl;

	if(request.post.size())
		cout << SPACE << request.target << " POST " << request.post << endl;
}