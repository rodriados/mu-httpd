#pragma once

#include <sys/stat.h>
#include <string>
#include <vector>
#include <map>

#include "run.h"

using namespace std;

namespace HTTP{
	class Request;
	class Response;

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
		make_pair("js", "text/javascript"),
		make_pair("pdf", "application/pdf")
	};

}

class HTTP::Request{

	public:
		string method;
		string target;
		string protocol;
		map<string, string> header;
		AddressIn client;

	public:
		string get;
		string post;

	private:
		void urldecode();

	public:
		Request(const string&, const AddressIn);
		~Request();

};

class HTTP::Response{

	private:
		struct File;

	protected:
		string content;
		string protocol;
		unsigned int status;
		map<string, string> header;
		Request& request;

	private:
		void process();

		void makeobj(const string&);
		void makedir(const string&);
		void makefile(const string&);
		void makeindex(const string&, const vector<File>&, const vector<File>&);
		void makemoved();

		bool isobj(const string&) const;
		bool ismoved(const string&);
		
		void makeerror(int);

	public:
		Response(Request&);
		~Response();

		int generate(string&);

};

struct HTTP::Response::File{
	string name;
	struct stat meta;

	File(const string& name, const struct stat& meta)
		: name(name), meta(meta) {}

};