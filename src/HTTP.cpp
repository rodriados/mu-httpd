/*! \file HTTP.cpp
 * \brief Arquivo de implementação HTTP.cpp
 * \author Marcos Paulo Massao Iseki <mm.iseki@gmail.com>
 * \author Rodrigo Albuquerque de Oliveira Siqueira <rodriados@gmail.com>
 * \author Thiago Yasutaka Ikeda <thiagoyasutaka@gmail.com>
 * \date 23 Oct 2014
 *
 * Contém a declaração das bibliotecas externas utilizadas e a implementação dos
 * métodos da namespace HTTP.
 * \see HTTP, Request, Response
 */
#include <dirent.h>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <cstring>
#include <string>
#include <cctype>
#include <ctime>

#include "Log.h"
#include "HTTP.h"

using namespace std;
using namespace HTTP;

//! Construtor da classe Request
/*! \fn Request(const string&, const AddressIn)
 * Contrói objeto responsável pelo parseamento e processamento da requisição
 * feita pelo usuário. Permitindo, assim, gerar a resposta de forma satisfatória.
 * \param request Requisição recebida do cliente.
 * \param client Endereço do cliente.
 */
HTTP::Request::Request(const string& request, const AddressIn client)
	: client(client)
{
	string line;
	stringstream sreq(request);
	
	getline(sreq, line);
	stringstream(line) >> this->method >> this->target >> this->protocol;
	this->urldecode();

	while(getline(sreq, line) && line.size() > 1) {
		int spos = line.find(':');
		this->header[line.substr(0, spos)] = line.substr(spos + 2);
	}

	getline(sreq, this->post);
}

//! Destrutor da classe Request
/*! \fn ~Request()
 * Destrói a instância da classe Request. Método invocado automaticamente
 * para a liberação de memória e posterior reuso.
 */
HTTP::Request::~Request()
{}

//! Decodificor de URL
/*! \fn urldecode()
 * Normalmente, as URLs costumam chegar um pouco codificadas para devido
 * a caracteres especiais existentes no endereço que está sendo requisitado.
 * Esse método visa reverter essa codificação para que o arquivo seja 
 * acessado normalmente internamente pelo servidor.
 */
void HTTP::Request::urldecode()
{
	string dec, aux;
	string& enc = this->target;
	int i, size = this->target.size();

	for(i = 0; i < size; ++i)
		if(i < size - 2 && enc[i] == '%' && isxdigit(enc[i+1]) && isxdigit(enc[i+2])) {
			aux = string() + "0x" + enc[i + 1] + enc[i + 2];
			dec += (char)strtol(aux.c_str(), NULL, 16);
			i = i + 2;
		}

		else {
			dec += enc[i];
		}

	int spos = dec.find('?');
	this->get = (spos > -1 ? dec.substr(spos + 1) : "");
	this->target = dec.substr(0, spos);
}

//! Construtor da classe Response
/*! \fn Response(Request&)
 * Constrói o objeto responsável pela formatação da resposta do servidor à requisição
 * feita pelo cliente seguindo o protocolo de internet HTTP.
 * \param request Objeto de requisição feita pelo cliente.
 */
HTTP::Response::Response(Request& request)
	: request(request)
{
	char tbuffer[80];
	time_t now = time(NULL);
	struct tm gmt = *gmtime(&now);
	strftime(tbuffer, sizeof(tbuffer), "%a, %d %b %Y %H:%M:%S %Z", &gmt);

	this->protocol = "HTTP/1.1";
	this->header["Connection"] = "closed";
	this->header["Server"] = "HTTPd by Rodrigo Siqueira, Marcos Iseki e Thiago Ikeda";
	this->header["Date"] = tbuffer;

	this->process();
}

//! Destrutor da classe Response
/*! \fn ~Response()
 * Destrói a instância da classe Response. Método invocado automaticamente
 * para a liberação de memória e posterior reuso.
 */
HTTP::Response::~Response()
{}

//! Interpreta requisição e gera a resposta ao cliente
/*! \fn process()
 * Interpreta a requisição realizada pelo cliente e tenta gerar uma resposta
 * aos dados recebidos. Caso a requisição não possa ser corretamente interpretada,
 * um erro HTTP será exibido.
 */
void HTTP::Response::process()
{
	if(this->request.protocol != "HTTP/1.1") {
		this->makeerror(505);
	}

	else if(this->request.method != "GET" && this->request.method != "POST") {
		this->makeerror(501);
	}

	else if(this->isobj("www" + this->request.target)) {
		this->makeobj("www" + this->request.target);
	}

	else if(this->ismoved(this->request.target)) {
		this->makemoved();
	}

	else {
		this->makeerror(404);
	}
}

//! Produz uma resposta ao objeto requisitado
/*! \fn makeobj(const string&)
 * Procura o objeto requisitado pelo nome e tenta produzir a resposta
 * esperada pelo cliente. Os objetos podem ser um arquivo ou pasta.
 * \param target Nome do objeto requisitado pelo cliente.
 */
void HTTP::Response::makeobj(const string& target)
{
	struct stat st;
	lstat(target.c_str(), &st);

	if(S_ISREG(st.st_mode)) {
		this->status = 200;
		this->makefile(target);
	}

	else if(S_ISDIR(st.st_mode)) {
		this->status = 200;
		this->makedir(target);
	}

	else {
		this->status = 500;
		this->makefile("default/500.html");
	}
}

//! Produz uma resposta a uma requisição de diretório
/*! \fn makedir(const string&)
 * Procura um diretório pelo nome, cria uma lista de objetos contidos
 * em seu interior e produz uma resposta à requisição do cliente.
 * \param target Nome do diretório requisitado pelo cliente.
 */
void HTTP::Response::makedir(const string& target)
{
	struct stat st;

	if(stat((target + "/index.html").c_str(), &st) == 0) {
		this->makefile(target + "/index.html");
		return;
	}

	struct dirent *ent;
	deque<File> folders, files;
	DIR *dir = opendir(target.c_str());

	while((ent = readdir(dir)) != NULL) {
		stat((target + "/" + ent->d_name).c_str(), &st);

		if(strcmp(ent->d_name, "..") == 0)
			folders.push_front(File(ent->d_name, st));
		else if(S_ISDIR(st.st_mode))
			folders.push_back(File(ent->d_name, st));
		else
			files.push_back(File(ent->d_name, st));
	}

	this->makeindex(target, folders, files);
}

//! Produz uma resposta a uma requisição de arquivo
/*! \fn makefile(const string&)
 * Procura um arquivo pelo nome e produz como resposta o conteúdo do
 * arquivo requisitado pelo cliente.
 * \param target Nome do arquivo requisitado pelo cliente.
 */
void HTTP::Response::makefile(const string& target)
{
	ifstream file(target);

	this->content.assign(
		istreambuf_iterator<char>(file),
		istreambuf_iterator<char>()
	);

	this->header["Content-Type"] = MIME[target.substr(target.rfind('.') + 1)];
	this->header["Content-Length"] = to_string(this->content.length());
}

//! Produz um índice de um diretório como resposta da requisição
/*! \fn makeindex(const string&, const deque<File>&, const deque<File>&)
 * Gera um índice de conteúdo de um diretório e permite a navegação recursiva
 * entre os diretórios contidos em seu interior.
 * \param target Nome do diretório a ser indexado.
 * \param dirs Lista de diretórios a serem listados.
 * \param files Lista de arquivos a serem listados.
 */
void HTTP::Response::makeindex(const string& target, const deque<File>& dirs, const deque<File>& files)
{
	char tmodified[81];
	stringstream index;
	time_t now = time(0), timediff;
	struct tm timeinfo;
	int magnitude;
	double size;

	index << "<!DOCTYPE html><html><head><meta charset=\"utf-8\"><style type=\"text/css\">*{margin:0;padding:0;border:0;out";
	index << "line:0;}a{text-decoration:none;color:#000000;}body{background-color:#FFFFFF;margin:40px;color:#2C3E50;}ul{lis";
	index << "t-style:none;border-top:solid 1px #EEEEEE;}li,.lgd{border-bottom:solid 1px #EEEEEE;padding:10px;position:rela";
	index << "tive;font-family:'Lato',sans-serif;font-size:14px;}.lgd{border-bottom:none !important;}.lg{font-weight:normal";
	index << " !important;}.lgd p{text-align:center;font-weight:bold;}#header{display:inline-block;position:relative;width:";
	index << "100%;padding:30px 0;}#header p{position:relative;width:100%;font-size:23px;text-align:center;font-family:'Lat";
	index << "o',sans-serif;font-style:italic;}.center{position:relative;width:750px;margin:30px auto 0;}.name{width:520px;";
	index << "display:inline-block;font-weight:bold;}.modified{width:130px;display:inline-block;text-align:center;}.size{wi";
	index << "dth:70px;display:inline-block;text-align:center;}li:hover{background:#FDFDFD}</style><title>Index of ";
	index << target.substr(target.find('/') + 1) << (target[target.size() - 1] == '/' && target != "www/" ? "" : "/");
	index << "</title></head><body><div id=\"header\"><p>Index of <b>";
	index << target.substr(target.find('/') + 1) << (target[target.size() - 1] == '/' && target != "www/" ? "" : "/");
	index << "</b></p></div><div id=\"list\"><div class=\"center\"><div class=\"lgd\"><p>DIRECTORIES</p></div><div class=\"";
	index << "lgd\"><div class=\"lg name\"><span>Name</span></div><div class=\"lg modified\"><span>Last Modified</span></di";
	index << "v><div class=\"lg size\"><span>Size</span></div></div><ul>";

	for(const File& elem : dirs) {
		if(elem.name == "." || (elem.name == ".." && target == "www/"))
			continue;

		timediff = now - elem.meta.st_mtime;
		timeinfo = *localtime(&elem.meta.st_mtime);

		if(timediff < 86400)
			strftime(tmodified, 80, "Today <b>%H:%M</b>", &timeinfo);
		else if(timediff < 172800)
			strftime(tmodified, 80, "Yesterday <b>%H:%M</b>", &timeinfo);
		else
			strftime(tmodified, 80, "%d %b %Y <b>%H:%M</b>", &timeinfo);

		index << "<a href=\"" << target.substr(target.find('/')) << (target[target.size() - 1] == '/' ? "" : "/");
		index << elem.name << "\"><li><div class=\"name\"><span>" << (elem.name == ".." ? "Parent Directory " : elem.name);
		index << "</span></div><div class=\"modified\"><span>" << (elem.name == ".." ? "-" : tmodified) << "</span></div>";
		index << "<div class=\"size\"><span>-</span></div></li></a>";
	}

	if(files.size() > 0) {
		index << "</ul><div class=\"lgd\" style=\"margin-top:70px\"><p>FILES</p></div><ul>";
		string mags[] = {"", "KB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB"};

		for(const File& elem : files) {
			timediff = now - elem.meta.st_mtime;
			timeinfo = *localtime(&elem.meta.st_mtime);
			size = elem.meta.st_size;

			if(timediff < 86400)
				strftime(tmodified, 80, "Today <b>%H:%M</b>", &timeinfo);
			else if(timediff < 172800)
				strftime(tmodified, 80, "Yesterday <b>%H:%M</b>", &timeinfo);
			else
				strftime(tmodified, 80, "%d %b %Y <b>%H:%M</b>", &timeinfo);

			for(magnitude = 0; size > 1024; ++magnitude)
				size /= 1024;

			index << "<a href=\"" << target.substr(target.find('/')) << (target[target.size() - 1] == '/' ? "" : "/");
			index << elem.name << "\"><li><div class=\"name\"><span>" << elem.name << "</span></div><div class=\"modified\"";
			index << "><span>" << tmodified << "</span></div><div class=\"size\"><span>" << (int)size << " ";
			index << mags[magnitude] << "</span></div></li></a>";
		}
	}

	index << "</ul></div></div></body></html>";

	this->content = index.str();
	this->header["Content-Type"] = MIME["html"];
	this->header["Content-Length"] = to_string(this->content.length());
}

//! Implementa um redirecionamento permanente
/*! \fn makemoved()
 * Gera, como resposta, um redirecionameto. Isto é, a requisição do usuário já
 * existiu nesse endereço mas foi transferido permanentemente para outra
 * localização. O comportamento normal do browser, nessas condições, é fazer
 * outra requisição do novo endereço.
 */
void HTTP::Response::makemoved()
{
	this->status = 301;
	this->header["Location"] = this->content;
	this->content = "";
}

//! Testa existencia de alvo de requisição
/*! \fn isobj(const string&)
 * Verifica se o alvo da requisição feita do cliente é um objeto, isto é, um
 * arquivo ou um diretório.
 * \param target Alvo de requisição a ser testada.
 * \return A requisição é de um arquivo ou diretório existente?
 */
bool HTTP::Response::isobj(const string& target) const
{
	struct stat st;
	lstat(target.c_str(), &st);

	return S_ISDIR(st.st_mode) || S_ISREG(st.st_mode);
}

//! Testa se alvo de requisição já existiu e foi movido
/*! \fn ismoved(const string&)
 * Verifica se o alvo da requisição já existiu e/ou foi movido permanentemente
 * para outra localidade. Só serão reconhecidos movimentos permanentes que
 * forem explicitamente indicados ao servidor atravez do arquivo \a .moved .
 * \param target Alvo de requisição a ser testada.
 * \return A requisição é de um objeto que foi movido permanentemente?
 */
bool HTTP::Response::ismoved(const string& target)
{
	string origin, destiny;
	ifstream movfile("default/.moved");

	while(movfile.good()) {
		movfile >> origin >> destiny;

		if(origin == target) {
			this->content = destiny;
			return true;
		}
	}

	return false;
}

//! Produz uma mensagem de erro à requisição
/*! \fn makeerror(int)
 * Retorna, como resposta à requisição do cliente, uma página de erro. Isso
 * pode acontecer caso a requisição tenha sido mal-formada ou possui como
 * alvo um objeto problemático ou não existente.
 * \param code Código de erro a ser retornado como resposta.
 */
void HTTP::Response::makeerror(int code)
{
	stringstream filename, logstr;
	filename << "default/" << code << ".html";

	this->status = code;
	this->makefile(filename.str());

	logstr << inet_ntoa(this->request.client.sin_addr);
	logstr << " " << this->request.method << " ";
	logstr << this->request.target << " " << code;

	logerr << logstr.str();
}

//! Gera a resposta à requisição
/*! \fn generate(string&)
 * Formata a resposta processada e a retorna em \s target para que possa,
 * finalmente, ser enviada ao cliente.
 * \param target Variável responsável por receber a resposta.
 * \return Tamanho, em caracteres, da resposta gerada.
 */
int HTTP::Response::generate(string& target)
{
	stringstream tgt;

	tgt << this->protocol << " " << this->status << " " << Code[this->status] << endl;

	for(auto it = this->header.begin(); it != this->header.end(); it++) {
		tgt << it->first << ": " << it->second << endl;
	}

	tgt << endl;
	tgt << this->content;
	target = tgt.str();

	return target.length();
}