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
		this->makefile("default/500.html");
	}

}

void HTTP::Response::makedir(const string& target){

	struct stat st;

	if(stat( (target + "/index.html").c_str() , &st) == 0){
		this->makefile(target + "/index.html");
		return;
	}

	struct dirent *ent;
	vector<File> folders, files;
	DIR *dir = opendir(target.c_str());

	while( (ent = readdir(dir)) != NULL){
		stat((target + "/" + ent->d_name).c_str(), &st);

		if(S_ISDIR(st.st_mode))
			folders.push_back(File(ent->d_name, st));
		else
			files.push_back(File(ent->d_name, st));
	}

	this->makeindex(target, folders, files);

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

void HTTP::Response::makeindex(const string& target, const vector<File>& dirs, const vector<File>& files){

	stringstream index;

	index << "<!DOCTYPE html><html><head><meta charset=\"utf-8\"><style type=\"text/css\">*{margin:0;padding:0;border:0;out";
	index << "line:0;}a{text-decoration:none;color:#000000;}body{background-color:#FFFFFF;margin:40px;color:#2C3E50;}ul{lis";
	index << "t-style:none;border-top:solid 1px #EEEEEE;}li,.lgd{border-bottom:solid 1px #EEEEEE;padding:10px;position:rela";
	index << "tive;font-family:'Lato',sans-serif;font-size:14px;}.lgd{border-bottom:none !important;}.lg{font-weight:normal";
	index << " !important;}.lgd p{text-align:center;font-weight:bold;}#header{display:inline-block;position:relative;width:";
	index << "100%;padding:30px 0;}#header p{position:relative;width:100%;font-size:23px;text-align:center;font-family:'Lat";
	index << "o',sans-serif;font-style:italic;}.center{position:relative;width:750px;margin:30px auto 0;}.name{width:520px;";
	index << "display:inline-block;font-weight:bold;}.modified{width:130px;display:inline-block;text-align:center;}.size{wi";
	index << "dth:70px;display:inline-block;text-align:center;}li:hover{background:#FDFDFD}</style><title>Index of ";
	index << target.substr(target.find('/') + 1) << (target[target.size() - 1] == '/' ? "" : "/");
	index << "</title></head><body><div id=\"header\"><p>Index of <b>";
	index << target.substr(target.find('/') + 1) << (target[target.size() - 1] == '/' ? "" : "/");
	index << "</b></p></div><div id=\"list\"><div class=\"center\"><div class=\"lgd\"><p>DIRECTORIES</p></div><div class=\"";
	index << "lgd\"><div class=\"lg name\"><span>Name</span></div><div class=\"lg modified\"><span>Last Modified</span></di";
	index << "v><div class=\"lg size\"><span>Size</span></div></div><ul>";

	for(const File& elem : dirs){
		// !!!!!!!!!!!!!!!
		// Essas variáveis terão valor alterado!!
		// !!!!!!!!!!!!!!!
		string modified = "09:40";
		string name = elem.name;
		string size = "916KB";

		if(elem.name == ".")
			continue;
		else if(elem.name == "..")
			name = "Parent Directory";

		index << "<a href=\"" << target.substr(target.find('/')) << (target[target.size() - 1] == '/' ? "" : "/");
		index << elem.name << "\"><li><div class=\"name\"><span>" << name << "</span></div><div class=\"modified\"><sp";
		index << "an>" << modified << "</span></div><div class=\"size\"><span>" << size << "</span></div></li></a>";
	}

	if(files.size() > 0){
		index << "</ul><div class=\"lgd\" style=\"margin-top:70px\"><p>FILES</p></div><ul>";

		for(const File& elem : files){
			// !!!!!!!!!!!!!!!
			// Essas variáveis terão valor alterado!!
			// !!!!!!!!!!!!!!!
			string modified = "23:59";
			string size = "1.2MB";

			index << "<a href=\"" << target.substr(target.find('/')) << (target[target.size() - 1] == '/' ? "" : "/");
			index << elem.name << "\"><li><div class=\"name\"><span>" << elem.name << "</span></div><div class=\"modified\"><sp";
			index << "an>" << modified << "</span></div><div class=\"size\"><span>" << size << "</span></div></li></a>";
		}
	}

	index << "</ul></div></div></body></html>";

	this->content = index.str();
	this->header["Content-Type"] = MIME["html"];
	this->header["Content-Length"] = to_string(this->content.length());

}

bool HTTP::Response::isobj(const string& target) const {

	struct stat st;
	lstat(target.c_str(), &st);

	return S_ISDIR(st.st_mode) || S_ISREG(st.st_mode);

}

void HTTP::Response::nothttp(){

	this->status = 505;
	this->makefile("default/505.html");

}

void HTTP::Response::notfound(){

	this->status = 404;
	this->makefile("default/404.html");

}

void HTTP::Response::notmethod(){

	this->status = 501;
	this->makefile("default/501.html");

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