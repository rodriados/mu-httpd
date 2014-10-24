/*! \file httpd.cpp
 * \brief Arquivo principal do software.
 * 
 * Este arquivo contém a função main do programa, responsável pela inicialização
 * e o loop do servidor. Contém as referências de bibliotecas externas utilizadas.
 * Recebe como parâmetro a porta de comunicação a ser utilizada por AddressIn.
 * Executa as funções de configuração básica do servidor (socket, bind e listen).
 *
 * \mainpage Relatório do Trabalho
 * \author Marcos Paulo Massao Iseki <mm.iseki@gmail.com>
 * \author Rodrigo Albuquerque de Oliveira Siqueira <rodriados@gmail.com>
 * \author Thiago Yasutaka Ikeda <thiagoyasutaka@gmail.com>
 * \date 23 Oct 2014
 *
 * \section intro_sec Introdução
 * O objetivo deste trabalho foi a implementação da funcionalidade básica de um servidor Web.
 * O servidor deve permitir que clientes HTTP (Firefox, IE, etc.) se conectem ao servidor
 * e façam downloads de arquivos.
 * O servidor para o protocolo HTTP deve implementar os métodos de consulta GET e POST
 * usando os protocolos TCP/IP para transferência das páginas HTML.
 *
 * \section implement_sec Implementações
 * As seguintes funcionalidades foram implementadas:
 * \li Método GET
 * \li Método POST
 * \li Navegação em diretórios
 * \li Atendimento concorrente
 * \li Redirecionamento permanente
 * \li Registro de log
 *
 * \section makefile_sec Como compilar e executar
 * Para compilar, num terminal de comandos, execute o arquivo MakeFile em um computador
 * que rode uma distribuição do SO Linux com compilador GNU com suporte a C++11.
 *
 * Para executar, use ./httpd [porta]
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

//! Função principal do software
/*! \fn main(int, const char **)
 * Inicializa, administra e finaliza a execução do software. Função de entrada
 * do software e responsável pela administração e execução do mesmo desde seu início até
 * o término.
 * \param argc Quantidade de argumentos de linha de comando.
 * \param argv Os argumentos passados pela linha de comando.
 */
int main(int argc, const char **argv){

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

//! Função de saudação.
/*! \fn hello()
 * Saúda o usuário com uma mensagem de boas-vindas e identificação
 * do nome do software que está iniciando.
 */
void hello(){

	style(BRIGHT);
	cout << endl;
	cout << SPACE << "HTTPd Hipertext Transfer Protocol Server." << endl;
	cout << endl;

	style(RESETALL);

}

//! Função de mensagem de erro
/*! \fn abort(AddressIn)
 * Mostra uma mensagem de erro, e finaliza o programa com status de erro.
 * Ainda assim, não são dadas mais informações sobre o erro.
 * \param addr Endereço de servidor que tentou ser conectado.
 */
void abort(AddressIn addr){

	foreground(RED);
	style(BRIGHT);
	cout << SPACE << "Erro:" << endl;
	cout << SPACE << "Não foi possível vincular-se ao endereço ";
	cout << inet_ntoa(addr.sin_addr) << ":" << ntohs(addr.sin_port) << endl;

	style(RESETALL);
 	cout << endl;

 	exit(1);

}

//! Função de confirmação
/*!
 * Mostra ao usuário que o software foi inicilizado corretamente e pode
 * prosseguir tranquilamente com seu uso cotidiano.
 * \param addr Endereço de servidor sendo utilizado para conectar-se.
 */
void confirm(AddressIn addr){

	foreground(GREEN);
	style(BRIGHT);
	cout << SPACE << "Sucesso!" << endl;
	cout << SPACE << "Esperando conexões no endereço ";
	cout << inet_ntoa(addr.sin_addr) << ":" << ntohs(addr.sin_port) << endl;

	style(RESETALL);
 	cout << endl;

}
