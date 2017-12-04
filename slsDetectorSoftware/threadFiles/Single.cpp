#include "Single.h"

#include <iostream>
#include <cstring>
using namespace std;

char local_ans[1000];

Single::Single(int id) {
  detID = id;
 // cout<<"detID:"<<detID<<endl;
}

Single::~Single() {
}

int Single::getID(){
	return detID;
}

int Single::printNumber(int i){
	cout << detID << " - Number:"<< i << endl;
	//usleep(detID*10000);
	//usleep(100000 - detID* 10000);
	return (i+1);
}

string Single::printString(string s){
	cout << detID << " - String:"<< s << endl;
	return string("string done");
}

char* Single::printCharArray(char a[]){
	cout << detID << " - Char array:"<< a << endl;
	strcpy(local_ans,"char done");
	return local_ans;
}
