#pragma once

#include <string>
#include <map>

using namespace std;

namespace HTTP{

class Request{

	public:
		string method;
		string target;
		string protocol;
		map<string, string> header;

	public:
		Request(const string&);

};

class Response{

	protected:
		unsigned int status;

	public:
		string content;
		string protocol;
		map<string, string> header;

	private:
		bool isdir(const string&) const;
		bool isfile(const string&) const;
		bool exists(const string&) const;
		bool wasmoved(const string&) const;

		string movedto(const string&) const;

	public:
		Response(const Request&);

};

static map<int, string> Code = {
	make_pair(200, "Ok"),
	make_pair(301, "Moved Permanently"),
	make_pair(400, "Bad Request"),
	make_pair(404, "Not Found"),
	make_pair(501, "Not Implemented"),
	make_pair(505, "HTTP Version Not Supported")
};

}