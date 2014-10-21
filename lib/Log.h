#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <mutex>

using namespace std;

class Log{

	protected:
		fstream *file;
		mutex mtx;

	public:
		Log(const string&);
		~Log();

		void operator<< (const string&);

};

extern Log logall;
extern Log logerr;