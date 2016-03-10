#include "Multi.h"
#include "Single.h"
#include "ThreadPool.h"



#include <iostream>
#include <cstring>
#include <stdio.h>
#include <stdlib.h>
using namespace std;

char ans[1000];
int threadflag = 1;

Multi::Multi() {
	numSingles = 1;
	threadpool = 0;
	for(int i=0;i<numSingles;i++)
		singles[i] = new Single(i);

	if(createThreadPool()== 0)
		exit(-1);

}

Multi::~Multi() {
	destroyThreadPool();
}

string Multi::executeCommand(int argc,char* argv[]){
	if(!strcmp(argv[1],"printnum")){
		int ival;
		char answer[100];
		if (!sscanf(argv[2],"%d",&ival))
			return string("Could not scan input ")+string(argv[2]);
		sprintf(answer,"%d",printNumber(ival));
		return string(answer);
	}
	else if(!strcmp(argv[1],"printstring")){
		return printString(argv[2]);
	}
	else if(!strcmp(argv[1],"printchararr")){
		return string(printCharArray(argv[2]));
	}


	else return string("unrecognized command");
}


int Multi::createThreadPool(){
	if(threadpool){
		threadpool->destroy_threadpool();
		threadpool=0;
	}
	if(numSingles > 0)
		threadpool = new ThreadPool(numSingles);
	switch(threadpool->initialize_threadpool()){
	case -1:
		cerr << "Failed to initialize thread pool!" << endl;
		return 0;
	case 0:
		cout << "Not initializing threads, only one detector" << endl;
		break;
	default:
		cout << "Initialized Threadpool" << endl;
		break;
	}
	return 1;
}

int Multi::destroyThreadPool(){
	if(threadpool){
		threadpool->destroy_threadpool();
		threadpool=0;
		cout<<"Destroyed Threadpool"<<endl;
	}
	return 1;
}


int Multi::printNumber(int inum){
	int ret=-100, ret1=-1;

	if(!threadpool){
		cout << "Error in creating threadpool. Exiting" << endl;
		return -1;
	}
	else{
		int* iret[numSingles];

		for(int i=0;i<numSingles;i++){
			iret[i]= new int(-1);
			//func_t <int,Single,int, int>* binder =
			//		new func_t<int, Single,int, int>(&Single::printNumber,singles[i],inum,iret[i]);
			Task* task = new Task(new func1_t<int, Single,int, int>(&Single::printNumber,singles[i],inum,iret[i]));
			threadpool->add_task(task);
		}
		threadpool->wait_for_tasks_to_complete();


		for(int i=0;i<numSingles;i++){
			if(iret[i] != NULL){
				ret1 = *iret[i];
				delete iret[i];
			}

			if(ret==-100)
				ret = ret1;
			else if (ret != ret1)
				ret = -1;
		}
	}

	return ret;
}

string Multi::printString(string s){
	string ret="error", ret1="sss";

	if(numSingles>1){
		string* sret[numSingles];


		for(int i=0;i<numSingles;i++){
			sret[i]= new string("sss");
			func1_t <string,Single,string,string>* binder =
					new func1_t<string,Single,string,string>(&Single::printString,singles[i],s,sret[i]);
			Task* task = new Task(binder);
			threadpool->add_task(task);
		}
		threadpool->wait_for_tasks_to_complete();
		for(int i=0;i<numSingles;i++){
			if(sret[i] != NULL){
				ret1 = *sret[i];
				delete sret[i];
			}
			if(ret=="error")
				ret = ret1;
			else if (ret != ret1)
				ret = "sss";
		}
	}

	else{
		for(int i=0;i<numSingles;i++){
			ret1=singles[i]->printString(s);
			if(ret=="error")
				ret = ret1;
			else if (ret != ret1)
				ret = "sss";
		}
	}
	return ret;
}

char* Multi::printCharArray(char a[]){
	string ret="error", ret1="sss";

	if(numSingles>1){
		string* sret[numSingles];

		for(int i=0;i<numSingles;i++){
			sret[i]= new string("sss");
			//std::fill_n(cret[i],1000,0);
			func1_t <char*,Single,char*,string>* binder =
					new func1_t <char*,Single,char*,string>(&Single::printCharArray,singles[i],a,sret[i]);
			Task* task = new Task(binder);
			threadpool->add_task(task);
		}
		threadpool->wait_for_tasks_to_complete();

		for(int i=0;i<numSingles;i++){
			if(sret[i] != NULL){
				ret1 = *sret[i];
				delete sret[i];
			}
			if(ret=="error")
				ret = ret1;
			else if (ret != ret1)
				ret = "sss";
		}
	}

	else{
		for(int i=0;i<numSingles;i++){
			ret1=singles[i]->printCharArray(a);
			if(ret=="error")
				ret = ret1;
			else if (ret != ret1)
				ret = "sss";
		}
	}
	strcpy(ans,ret.c_str());
	return ans;
}
