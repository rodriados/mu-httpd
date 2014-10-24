#pragma once

#include <iostream>
#include <fstream>
#include <string>

using namespace std;

class Log{

	protected:
		fstream *file;

	public:
		Log(const string&);
		~Log();

		void operator<< (const string&);

};

extern Log logall;
extern Log logerr;