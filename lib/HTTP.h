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
		string content;
		string protocol;
		unsigned int status;
		map<string, string> header;

	private:
		void process(Request&);

		void nothttp();
		void notfound();
		void notmethod();

		void makeobj(const string&);
		void makedir(const string&);
		void makefile(const string&);
//		void makemoved();

		bool isobj(const string&) const;
//		bool ismoved(const string&) const;

	public:
		Response(Request&);

		int generate(string&);

};

static map<int, string> Code = {
	make_pair(200, "Ok"),
	make_pair(301, "Moved Permanently"),
	make_pair(400, "Bad Request"),
	make_pair(404, "Not Found"),
	make_pair(500, "Internal Server Error"),
	make_pair(501, "Not Implemented"),
	make_pair(505, "HTTP Version Not Supported")
};

static map<string, string> MIME = {
	make_pair("html", "text/html"),
	make_pair("txt", "text/plain"),
	make_pair("jpe", "image/jpeg"),
	make_pair("jpg", "image/jpeg"),
	make_pair("jpeg", "image/jpeg"),
	make_pair("png", "image/png"),
	make_pair("gif", "image/gif"),
	make_pair("css", "text/css"),
	make_pair("js", "text/javascript")
};

}