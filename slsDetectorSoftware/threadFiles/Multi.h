#pragma once


#include "Global.h"

#include <string>

class Single;
class ThreadPool;

class Multi {
public:

	Multi();
	~Multi();

	std::string executeCommand(int argc,char* argv[]);

	int printNumber(int inum);
	std::string printString(std::string s);
	char* printCharArray(char a[]);

	int createThreadPool();
	int destroyThreadPool();

protected:
	Single* singles[MAX_SINGLES];
	int numSingles;
	ThreadPool* threadpool;



};


