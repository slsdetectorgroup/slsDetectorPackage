#include <iostream>
#include <string>


#include "multiSlsDetector.h"
#include "slsDetector.h"
#include "slsDetectorCommand.h"


#include <stdlib.h>
using namespace std;

int main(int argc, char *argv[])
  
{


  int    id=-1, iv=0;
  char *c;
  string answer;
  char cmd[100];
  int action;
  slsDetectorBase *myDetector;
  slsDetectorCommand *myCmd;
  


#ifdef READOUT 
  action=READOUT_ACTION;
#elif PUT   
  action=PUT_ACTION;
#elif GET   
  action=GET_ACTION;
#elif HELP  
  action=HELP_ACTION;
#endif 

  if (argc>1){



    iv=sscanf(argv[1],"%d%s",&id, cmd);
    if (id>=0) {
      if (iv==2) {
	if (cmd[0]=='-') {
#ifdef VERBOSE
	  cout << "Using multiSlsDetector id=" << id << endl;
#endif
	  myDetector=new multiSlsDetector(id);
	  argv[1]=cmd+1;
	} else if (cmd[0]==':') {
#ifdef VERBOSE
	  cout << "Using slsDetector id=" << id << endl;
#endif
	  myDetector=new slsDetector(id);
	  argv[1]=cmd+1;
	} else {
	  cout << "Wrong syntax: no channels starts with integer number "<<id <<". Bhould be " << argv[0] << endl;
	  cout           << id << ":channel  for single detector" ;
	  cout << " or " << id << "-channel  for multiple detectors" << endl; 
	  return -1;
	}
      } else {
#ifdef VERBOSE
	cout << "Using slsDetector id=" << id << endl;
#endif
	myDetector=new slsDetector(id);
      }
    } else {
#ifdef VERBOSE
	  cout << "Using default multiSlsDetector" << id << endl;
#endif
      myDetector=new multiSlsDetector();
    }
  } else {
#ifdef GET
    cout << "Wrong usage - should be: "<< argv[0] << "[id:/id-]channel" << endl;
    cout << slsDetectorCommand::helpLine(argc-1, argv, action);
    cout << endl;
    return -1;
#endif

#ifdef PUT
  if (argc<3) {
    cout << "Wrong usage - should be: "<< argv[0] << "[id:/id-]channel arg" << endl;
    cout << slsDetectorCommand::helpLine(argc-1, argv+1, action);
    cout << endl;
    return -1;
  }  
#endif
#ifdef VERBOSE
	  cout << "Using default multiSlsDetector" << id << endl;
#endif
  myDetector=new multiSlsDetector();
  }


#ifdef PUT
  if (argc<3) {
    cout << "Wrong usage - should be: "<< argv[0] <<" " << argv[1]<< "  arg" << endl;
    cout << slsDetectorCommand::helpLine(argc-1, argv+1, action);
    cout << endl;
    return -1;
  }  
#endif
  myCmd=new slsDetectorCommand(myDetector);

  if (argc<2) {
    answer=myCmd->executeLine(argc-1, argv, action);
  } else {
    answer=myCmd->executeLine(argc-1, argv+1, action);
  }
  cout << answer<< endl;

  
  return 0;
}
  
