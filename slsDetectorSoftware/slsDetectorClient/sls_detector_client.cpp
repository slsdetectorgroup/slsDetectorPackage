#include <iostream>
#include <string>

#ifdef MYTHEN_DET
#include "mythenDetector.h"
#endif

#ifdef GOTTHARD_DET
#include "gotthardDetector.h"
#endif


#include <stdlib.h>
using namespace std;

int main(int argc, char *argv[])
  
{

  int    id=0;
  char *c;
  string answer;
  int action;
#ifdef MYTHEN_DET
  mythenDetector *myDetector;
#ifdef READOUT 
  action=mythenDetector::READOUT_ACTION;
#elif PUT   
  action=mythenDetector::PUT_ACTION;
#elif GET   
  action=mythenDetector::GET_ACTION;
#endif  
#endif 
  
#ifdef GOTTHARD_DET
  gotthardDetector *myDetector;
#ifdef READOUT 
  action=gotthardDetector::READOUT_ACTION;
#elif PUT   
  action=gotthardDetector::PUT_ACTION;
#elif GET   
  action=gotthardDetector::GET_ACTION;
#endif  

#endif

  if (argc>1 && sscanf(argv[1],"%d",&id)){

#ifdef MYTHEN_DET
#ifndef PICASSOD
      myDetector=new mythenDetector(id);
#else
      myDetector=new mythenDetector(id,PICASSO);
#endif
#endif 
#ifdef GOTTHARD_DET
      myDetector=new gotthardDetector(id);
#endif
  } else { 
      cout << "Wrong usage - should be: "<< argv[0] << " id";
      cout <<     "(:channel arg)" << endl;
#ifdef MYTHEN_DET
      cout << mythenDetector::helpLine(action);
#endif
#ifdef GOTTHARD_DET
      cout << gotthardDetector::helpLine(action);
#endif
      cout << endl;
      return -1;
    }
    
#ifndef READOUT
  string s(argv[1]);
#ifdef PUT
  if (argc<3) {
    cout << "Wrong usage - should be: "<< argv[0] << " id";
    cout <<     ":channel arg" << endl;
#ifdef MYTHEN_DET
    cout << mythenDetector::helpLine(action);
#endif
#ifdef GOTTHARD_DET
      cout << gotthardDetector::helpLine(action);
#endif
    cout << endl;
    return -1;
  }
#endif
  if ((c=strchr(argv[1],':')))
    argv[1]=c+1;
#endif
  answer=myDetector->executeLine(argc-1, argv+1, action);
#ifndef READOUT 
  cout << s << " " << answer<< endl;
#endif
  
  return 0;
}
  
