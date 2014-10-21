#include <iostream>
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

	stringstream sres(this->content);

}
