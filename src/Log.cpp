#include <string>
#include <ctime>

#include "colors.h"
#include "Log.h"

using namespace std;

Log logall("log/all.log");
Log logerr("log/error.log");

Log::Log(const string& filename){

	this->file = new fstream(filename.c_str(), fstream::out|fstream::app);

}

Log::~Log(){

	delete this->file;

}

void Log::operator<< (const string& data){

	char buffer[80];
	time_t rawtime;

	time(&rawtime);
	strftime(buffer, 80, "[%d.%m.%Y %H:%M:%S] ", localtime(&rawtime));
	
	(*this->file) << buffer << data << endl;

}