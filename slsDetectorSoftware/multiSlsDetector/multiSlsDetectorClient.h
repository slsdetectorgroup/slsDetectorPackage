#include <iostream>
#include <string>


#include "multiSlsDetector.h"
#include "multiSlsDetectorCommand.h"


#include <stdlib.h>
using namespace std;



class multiSlsDetectorClient  { 

 public:
  multiSlsDetectorClient(int argc, char *argv[], int action, multiSlsDetector *myDetector=NULL) {	\
    string answer;					      \
    multiSlsDetectorCommand *myCmd;			      \
    int del=0;						      \
    if (argc==0 && action==slsDetectorDefs::READOUT_ACTION) { \
      if (myDetector==NULL) {				      \
	myDetector=new multiSlsDetector();		      \
	del=1;						      \
      };
      myCmd=new multiSlsDetectorCommand(myDetector);	      \
      answer=myCmd->executeLine(argc, argv, action);	      \
      cout << answer<< endl;				      \
      delete myCmd;					      \
      if (del)      delete myDetector;			      \
      return;						      \
    };							      \
    int    id=-1, iv=0, pos=-1;				      \
    char *c;						      \
    char cmd[100];					      \
    if (action==slsDetectorDefs::PUT_ACTION && argc<2) {		\
      cout << "Wrong usage - should be: "<< argv[0] << "[id-][pos:]channel arg" << endl; \
      cout << endl;							\
      return;								\
    };
    if (action==slsDetectorDefs::GET_ACTION && argc<1) {		\
      cout << "Wrong usage - should be: "<< argv[0] << "[id-][pos:]channel arg" << endl; \
      cout << endl;							\

      return;								\
    };									\
    if (myDetector==NULL) {
      iv=sscanf(argv[0],"%d-%s",&id, cmd);				\
      if (iv==2 && id>=0) {						\
	myDetector=new multiSlsDetector(id);				\
	argv[0]=cmd;							\
	cout << id << "-" ;						\
      } else {								\
	myDetector=new multiSlsDetector();				\
      };
      del=1;
    }									\
    iv=sscanf(argv[0],"%d:%s",&pos, cmd);				\
    if (iv==2 && pos>=0) {						\
      argv[0]=cmd;							\
      cout << pos << ":" ;						\
    }	;								\
    myCmd=new multiSlsDetectorCommand(myDetector);			\
    answer=myCmd->executeLine(argc, argv, action, pos);			\
    cout << argv[0] << " " ;						\
    cout << answer<< endl;						\
    delete myCmd;							\
    if (del) delete myDetector;						\
  };									
  								
  


};











