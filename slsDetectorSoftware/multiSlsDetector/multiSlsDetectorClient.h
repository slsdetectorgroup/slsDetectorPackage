#include <iostream>
#include <string>


#include "multiSlsDetector.h"
#include "multiSlsDetectorCommand.h"


#include <stdlib.h>
using namespace std;

int dummyCallback(detectorData* d, int p,void*) {
	cout << "got data "	<< p <<  endl;
	return 0;
};

class multiSlsDetectorClient  { 

 public:
  multiSlsDetectorClient(int argc, char *argv[], int action, multiSlsDetector *myDetector=NULL) {	\
    string answer;					      \
    multiSlsDetectorCommand *myCmd;					      \
    int    id=-1, iv=0, pos=-1;				      \
    int del=0;						      \
    char cmd[100];							\
    if (action==slsDetectorDefs::READOUT_ACTION) { \

      if (argc!=0) {
	iv=sscanf(argv[0],"%d-%s",&id,cmd);		\
	if (iv>0 && id>=0 && strchr(argv[0],'-')) {
	  cout << "id " << id << endl;		\
	  if (iv>1)
	    argv[0]=cmd;
	}
	iv=sscanf(argv[0],"%d:",&pos);			\
	if (iv>0 && pos>=0 && strchr(argv[0],':'))
	  cout << "pos " << pos << "is not allowed!" << endl;	\
      } 
      if (id<0)
	id=0;

      if (myDetector==NULL) {				      \
	myDetector=new multiSlsDetector(id);				\
	//myDetector->registerDataCallback(&dummyCallback,  NULL);
	del=1;						      \
      };
      //  cout << "noid" <<endl;
      myCmd=new multiSlsDetectorCommand(myDetector);	      \
      answer=myCmd->executeLine(argc, argv, action);	      \
      cout << answer<< endl;				      \
      delete myCmd;					      \
      if (del)      delete myDetector;			      \
      return;						      \
    };							      \
    if (action==slsDetectorDefs::PUT_ACTION && argc<2) {		\
      cout << "Wrong usage - should be: "<< argv[0] <<			\
	"[id-][pos:]channel arg" << endl;				\
      cout << endl;							\
      return;								\
      if (del) delete myDetector;					\
    };
    if (action==slsDetectorDefs::GET_ACTION && argc<1) {		\
      cout << "Wrong usage - should be: "<< argv[0] <<			\
	"[id-][pos:]channel arg" << endl;				\
      cout << endl;							\
      if (del) delete myDetector;					\
      return;								\
    };									\
    if (myDetector==NULL) {						\
      iv=sscanf(argv[0],"%d-%s",&id, cmd);					\
      if (iv==2 && id>=0) {						\
	myDetector=new multiSlsDetector(id);				\
	argv[0]=cmd;						\
	cout << id << "-" ;						\
      } else {								\
	myDetector=new multiSlsDetector();				\
      };								\
      del=1;								\
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











