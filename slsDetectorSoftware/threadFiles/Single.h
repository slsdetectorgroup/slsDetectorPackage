#pragma once

#include "Multi.h"

#include <string>
using namespace std;

class Single {
public:
	Single(int id);
	~Single();
	int getID();

	int printNumber(int i);
	string printString(string s);
	char* printCharArray(char a[]);

private:
	int detID;

	int* someValue;

};

