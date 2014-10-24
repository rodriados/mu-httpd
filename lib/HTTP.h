/*! \file HTTP.h
 * \brief Arquivo de cabeçalho de HTTP
 * \author Marcos Paulo Massao Iseki <mm.iseki@gmail.com>
 * \author Rodrigo Albuquerque de Oliveira Siqueira <rodriados@gmail.com>
 * \author Thiago Yasutaka Ikeda <thiagoyasutaka@gmail.com>
 * \date 23 Oct 2014
 *
 * Contém a declaração das bibliotecas externas utilizadas e da namespace HTTP,
 * que possui as classes Request e Response.
 * \see HTTP, Resquest, Response
 */
#pragma once

#include <sys/stat.h>
#include <string>
#include <deque>
#include <map>

#include "run.h"

using namespace std;

//! Espaço de objetos HTTP
/*! \namespace HTTP
 * Esta namespace contém as componentes HTTP Request e
 * Response, que são as classes que recebem e respondem,
 * respectivamente, às requisições do cliente (navegador).
 * Contém, ainda, os mapeamentos Code, que define a resposta
 * da requisição a partir de seu código, e MIME, que define o
 * formato MIME da mensagem a partir de sua extensão.
 * \see Request, Response
 */
namespace HTTP{
	class Request;
	class Response;

	//! Mapeamento de código
	/*! \var Code
	 * O mapeamento Code associa um status de resposta à sua
	 * respectiva descrição de resposta.
	 */
	static map<int, string> Code = {
		make_pair(200, "Ok"),
		make_pair(301, "Moved Permanently"),
		make_pair(400, "Bad Request"),
		make_pair(404, "Not Found"),
		make_pair(500, "Internal Server Error"),
		make_pair(501, "Not Implemented"),
		make_pair(505, "HTTP Version Not Supported")
	};

	//! Mapeamento MIME
	/*! \var MIME
	 * O mapeamento MIME associa uma extensção de arquivo com
	 * o MIME que representa tal conteúdo.
	 */
	static map<string, string> MIME = {
		make_pair("html", "text/html"),
		make_pair("txt", "text/plain"),
		make_pair("jpe", "image/jpeg"),
		make_pair("jpg", "image/jpeg"),
		make_pair("jpeg", "image/jpeg"),
		make_pair("png", "image/png"),
		make_pair("gif", "image/gif"),
		make_pair("css", "text/css"),
		make_pair("js", "text/javascript"),
		make_pair("pdf", "application/pdf")
	};

}

//! Classe leitora de requisição
/*! \class Request
 * A classe Request é responsável por receber cadeias de caracteres,
 * analisar seu formato e conteúdo para extrair informações sobre
 * o método, o alvo e o protocolo da requisição caso seja adequadamente
 * apresentada no formato HTTP.
 */
class HTTP::Request{

	/*
	 * Declaração de propriedades da classe para permitir o armazenamento
	 * e encapsulamento de valores e objetos sob o escopo da classe.
	 */
	public:
		string method;
		string target;
		string protocol;
		map<string, string> header;
		AddressIn client;

	public:
		string get;
		string post;

	/*
	 * Declaração de métodos de classe para permitir a manipulação do objeto
	 * dos valores nele contido.
	 */
	private:
		void urldecode();

	public:
		Request(const string&, const AddressIn);
		~Request();

};

//! Classe replicante de requisição
/*! \class Response
 * A classe Response é responsável por formatar a resposta do servidor ao
 * cliente no formato adequado de resposta HTTP.
 */
class HTTP::Response{

	/*
	 * Declaração de objetos aninhados e encapsulados sob o escopo do 
	 * objeto principal. Os controles de visibilidade são ditados pelo
	 * objeto exterior.
	 */
	private:
		struct File;

	/*
	 * Declaração de propriedades da classe para permitir o armazenamento
	 * e encapsulamento de valores e objetos sob o escopo da classe.
	 */
	protected:
		string content;
		string protocol;
		unsigned int status;
		map<string, string> header;
		Request& request;

	/*
	 * Declaração de métodos de classe para permitir a manipulação do objeto
	 * dos valores nele contido.
	 */
	private:
		void process();

		void makeobj(const string&);
		void makedir(const string&);
		void makefile(const string&);
		void makeindex(const string&, const deque<File>&, const deque<File>&);
		void makemoved();

		bool isobj(const string&) const;
		bool ismoved(const string&);
		
		void makeerror(int);

	public:
		Response(Request&);
		~Response();

		int generate(string&);

};

//! Estrutura de informações de arquivo
/*! \struct File
 * Estrutura responsável pelo armazenamento das informações de um arquivo.
 * As informações contidas são obtidas através da função de sistema \a stat .
 */
struct HTTP::Response::File{
	string name;
	struct stat meta;

	File(const string& name, const struct stat& meta)
		: name(name), meta(meta) {}

};