/*! \file Log.h
 * \brief Arquivo de cabeçalho Log.h
 * \author Marcos Paulo Massao Iseki <mm.iseki@gmail.com>
 * \author Rodrigo Albuquerque de Oliveira Siqueira <rodriados@gmail.com>
 * \author Thiago Yasutaka Ikeda <thiagoyasutaka@gmail.com>
 * \date 23 Oct 2014
 *
 * Contém a declaração das bibliotecas externas utilizadas e da classe
 * Log, que é responsável pela criação e manutenção do registro das
 * atividades do programa.
 * \see Log
 */
#pragma once

#include <iostream>
#include <fstream>
#include <string>

using namespace std;

//! Classe de controle de Log
/* \class Log
 * A classe Log é responsável pela criação e manipulação de arquivos de
 * log do servidor, sendo atribuída a ela toda a responsabilidade da
 * interação com os arquivos de log.
 */
class Log
{
	/*
	 * Declaração de propriedades da classe para permitir o armazenamento
	 * e encapsulamento de valores e objetos sob o escopo da classe.
	 */
	protected:
		fstream *file;

	/*
	 * Declaração de métodos de classe para permitir a manipulação do objeto
	 * dos valores nele contido.
	 */
	public:
		Log(const string&);
		~Log();

		void operator<< (const string&);
};

extern Log logall;
extern Log logerr;