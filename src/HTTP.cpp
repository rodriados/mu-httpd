#include <dirent.h>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <string>
#include <cctype>
#include <ctime>

#include "Log.h"
#include "HTTP.h"

using namespace std;
using namespace HTTP;

HTTP::Request::Request(const string& request, const AddressIn client)
: client(client) {

	string line;
	stringstream sreq(request);
	
	getline(sreq, line);
	stringstream(line) >> this->method >> this->target >> this->protocol;
	this->urldecode();

	while(getline(sreq, line) && line.size() > 1){
		int spos = line.find(':');
		this->header[line.substr(0, spos)] = line.substr(spos + 2);
	}

	getline(sreq, this->post);

}

HTTP::Request::~Request(){
	;
}

void HTTP::Request::urldecode(){

	string dec, aux;
	string& enc = this->target;
	int i, size = this->target.size();

	for(i = 0; i < size; ++i)
		if(i < size - 2 && enc[i] == '%' && isxdigit(enc[i+1]) && isxdigit(enc[i+2])){
			aux = string() + "0x" + enc[i + 1] + enc[i + 2];
			dec += (char)strtol(aux.c_str(), NULL, 16);
			i = i + 2;
		}

		else{
			dec += enc[i];
		}

	int spos = dec.find('?');
	this->get = (spos > -1 ? dec.substr(spos + 1) : "");
	this->target = dec.substr(0, spos);

}

HTTP::Response::Response(Request& request)
: request(request) {

	char tbuffer[80];
	time_t now = time(NULL);
	struct tm gmt = *gmtime(&now);
	strftime(tbuffer, sizeof(tbuffer), "%a, %d %b %Y %H:%M:%S %Z", &gmt);

	this->protocol = "HTTP/1.1";
	this->header["Connection"] = request.header["Connection"];
	this->header["Server"] = "HTTPd by Rodrigo Siqueira, Marcos Iseki e Thiago Ikeda";
	this->header["Date"] = tbuffer;

	this->process();

}

HTTP::Response::~Response(){
	;
}

void HTTP::Response::process(){

	if(this->request.protocol != "HTTP/1.1"){
		this->makeerror(505);
	}

	else if(this->request.method != "GET" && this->request.method != "POST"){
		this->makeerror(501);
	}

	else if(this->isobj("www" + this->request.target)){
		this->makeobj("www" + this->request.target);
	}

	else if(this->ismoved(this->request.target)){
		this->makemoved();
	}

	else{
		this->makeerror(404);
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

	for(const File& elem : dirs){
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

	if(files.size() > 0){

		index << "</ul><div class=\"lgd\" style=\"margin-top:70px\"><p>FILES</p></div><ul>";
		string mags[] = {"", "KB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB"};

		for(const File& elem : files){
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

void HTTP::Response::makemoved(){

	this->status = 301;
	this->header["Location"] = this->content;
	this->content = "";

}

bool HTTP::Response::isobj(const string& target) const {

	struct stat st;
	lstat(target.c_str(), &st);

	return S_ISDIR(st.st_mode) || S_ISREG(st.st_mode);

}

bool HTTP::Response::ismoved(const string& target){

	string origin, destiny;
	ifstream movfile("default/.moved");

	while(movfile.good()){
		movfile >> origin >> destiny;

		if(origin == target){
			this->content = destiny;
			return true;
		}
	}

	return false;

}

void HTTP::Response::makeerror(int code){

	stringstream filename, logstr;
	filename << "default/" << code << ".html";

	this->status = code;
	this->makefile(filename.str());

	logstr << inet_ntoa(this->request.client.sin_addr);
	logstr << " " << this->request.method << " ";
	logstr << this->request.target << " " << code;

	logerr << logstr.str();

}

int HTTP::Response::generate(string& target){

	stringstream tgt;

	tgt << this->protocol << " " << this->status << " " << Code[this->status] << endl;

	for(auto it = this->header.begin(); it != this->header.end(); it++){
		tgt << it->first << ": " << it->second << endl;
	}

	tgt << endl;
	tgt << this->content;
	target = tgt.str();

	return target.length();

}