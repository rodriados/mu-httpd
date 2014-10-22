#include <sys/stat.h>
#include <dirent.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <ctime>

#include "HTTP.h"

using namespace std;
using namespace HTTP;

HTTP::Request::Request(const string& request){

	string line;
	stringstream sreq(request);
	
	getline(sreq, line);
	stringstream(line) >> this->method >> this->target >> this->protocol;

	while(getline(sreq, line)){
		int spos = line.find(' ');
		this->header[line.substr(0, spos - 1)] = line.substr(spos + 1);
	}

}

HTTP::Response::Response(Request& request){

	char tbuffer[80];
	time_t now = time(NULL);
	struct tm gmt = *gmtime(&now);
	strftime(tbuffer, sizeof(tbuffer), "%a, %d %b %Y %H:%M:%S %Z", &gmt);

	this->protocol = "HTTP/1.1";
	this->header["Connection"] = request.header["Connection"];
	this->header["Server"] = "HTTPd by Rodrigo Siqueira, Marcos Iseki e Thiago Ikeda";
	this->header["Date"] = tbuffer;

	this->process(request);

}

void HTTP::Response::process(Request& request){

	if(request.protocol != "HTTP/1.1"){
		this->nothttp();
	}

	else if(request.method != "GET" && request.method != "POST"){
		this->notmethod();
	}

	else if(this->isobj("www" + request.target)){
		this->makeobj("www" + request.target);
	}

/*	else if(this->ismoved(request.target)){
		this->makemoved();
	}
*/
	else{
		this->notfound();
	}

}

void HTTP::Response::nothttp(){

	this->status = 505;
	this->makefile("error/505.html");

}

void HTTP::Response::notfound(){

	this->status = 404;
	this->makefile("error/404.html");

}

void HTTP::Response::notmethod(){

	this->status = 501;
	this->makefile("error/501.html");

}

void HTTP::Response::makeobj(const string& target){

	struct stat st;
	lstat(target.c_str(), &st);

	if(S_ISREG(st.st_mode)){
		this->status = 200;
		this->makefile(target);
	}

	else if(S_ISDIR(st.st_mode)){
		this->status = 200;
		this->makedir(target);
	}

	else{
		this->status = 500;
		this->makefile("error/500.html");
	}

}

void HTTP::Response::makedir(const string& target){

	struct stat st;

	if(stat( (target + "/index.html").c_str() , &st) == 0){
		this->makefile(target + "/index.html");
		return;
	}

	DIR *dir;
	struct dirent *ent;
	stringstream index;

	index << "<!DOCTYPE html>" << endl;
	index << "<html><head><meta charset=\"UTF-8\">" << endl;
	index << "<title>Index of " << target.substr(target.find('/')) << "</title>" << endl;
	index << "</head><body>" << endl;
	index << "<h1>Index of " << target.substr(target.find('/')) << "</h1><ul>" << endl;
	
	dir = opendir(target.c_str());

	while((ent = readdir(dir)) != NULL){
		index << "<li><a href='" << target.substr(target.find('/')) << "/" << ent->d_name << "'>";
		index << ent->d_name << "</a></li>" << endl;
	}

	index << "</ul></body></html>";

	this->content = index.str();
	this->header["Content-Type"] = MIME["html"];
	this->header["Content-Length"] = to_string(this->content.length());

}

void HTTP::Response::makefile(const string& target){

	ifstream file(target);

	this->content.assign(
		istreambuf_iterator<char>(file),
		istreambuf_iterator<char>()
	);

	this->header["Content-Type"] = MIME[target.substr(target.rfind('.') + 1)];
	this->header["Content-Length"] = to_string(this->content.length());

}

bool HTTP::Response::isobj(const string& target) const {

	struct stat st;
	lstat(target.c_str(), &st);

	return S_ISDIR(st.st_mode) || S_ISREG(st.st_mode);

}

int HTTP::Response::generate(string& target){

	stringstream tgt;

	tgt << this->protocol << " " << this->status << " " << Code[this->status] << endl;

	for(auto it = this->header.begin(); it != this->header.end(); it++){
		tgt << it->first << ": " << it->second << endl;
	}

	tgt << endl;
	tgt << endl;
	tgt << this->content;
	target = tgt.str();

	return target.length();

}