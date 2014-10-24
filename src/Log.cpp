/*! \file Log.cpp
 * \brief Arquivo de implementação Log.cpp
 * \author Marcos Paulo Massao Iseki <mm.iseki@gmail.com>
 * \author Rodrigo Albuquerque de Oliveira Siqueira <rodriados@gmail.com>
 * \author Thiago Yasutaka Ikeda <thiagoyasutaka@gmail.com>
 * \date 23 Oct 2014
 *
 * Contém a declaração das bibliotecas externas utilizadas e a implementação dos
 * métodos da classe Log.
 * \see Log
 */
#include <string>
#include <ctime>

#include "colors.h"
#include "Log.h"

using namespace std;

Log logall("log/all.log");
Log logerr("log/error.log");

//! Construtor da classe de Log
/*! \fn Log(const string&)
 * Constrói objeto responsável pela administração e manipulação de arquivos
 * de log do servidor.
 * \param filename Nome do arquivo a ser utilizado como log.
 */
Log::Log(const string& filename){

	this->file = new fstream(filename.c_str(), fstream::out|fstream::app);

}

//! Destrutor da classe Log
/*! \fn ~Log()
 * Destrói a instância da classe Log. Método invocado automaticamente
 * para a liberação de memória e posterior reuso.
 */
Log::~Log(){

	delete this->file;

}

//! Escreve uma linha de log ao arquivo
/*! \fn operator<<(const string&)
 * Método responsável por adicionar uma nova linha de log ao arquivo permitindo,
 * assim, o controle de todas as transições feita pelo servidor.
 * \param data Dados a serem adicionados na linha de log.
 */
void Log::operator<< (const string& data){

	char buffer[80];
	time_t rawtime;

	time(&rawtime);
	strftime(buffer, 80, "[%d.%m.%Y %H:%M:%S] ", localtime(&rawtime));
	
	(*this->file) << buffer << data << endl;

}