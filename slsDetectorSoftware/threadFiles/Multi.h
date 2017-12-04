#pragma once


#include "Global.h"

#include <string>
using namespace std;

class Single;
class ThreadPool;

class Multi {
public:

	Multi();
	~Multi();

	string executeCommand(int argc,char* argv[]);

	int printNumber(int inum);
	string printString(string s);
	char* printCharArray(char a[]);

	int createThreadPool();
	int destroyThreadPool();

protected:
	Single* singles[MAX_SINGLES];
	int numSingles;
	ThreadPool* threadpool;



};


