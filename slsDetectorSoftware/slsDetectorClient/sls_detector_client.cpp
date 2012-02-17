#include <iostream>
#include <string>

#include "slsDetector.h"


#include <stdlib.h>
using namespace std;

int main(int argc, char *argv[])
  
{


  int    id=0;
  char *c;
  string answer;
  int action;
 slsDetector *myDetector;



#ifdef READOUT 
  action=READOUT_ACTION;
#elif PUT   
  action=PUT_ACTION;
#elif GET   
  action=GET_ACTION;
#endif  



  detectorType t;
  
#ifdef MYTHEN_DET
#ifndef PICASSOD
  t=MYTHEN;
#else
  t=PICASSO;
#endif;
#elif GOTTHARD_DET
  t=GOTTHARD;
#else
  t=GENERIC;
#endif




  if (argc>1 && sscanf(argv[1],"%d",&id)){


    myDetector=new slsDetector(t,id);


  } else { 
      cout << "Wrong usage - should be: "<< argv[0] << " id";
      cout <<     "(:channel arg)" << endl;

      cout << slsDetector::helpLine(argc-1, argv+1, action);

      cout << endl;
      return -1;
    }
    
#ifndef READOUT
  string s(argv[1]);
#ifdef PUT
  if (argc<3) {
    cout << "Wrong usage - should be: "<< argv[0] << " id";
    cout <<     ":channel arg" << endl;

    cout << slsDetector::helpLine(argc-1, argv+1, action);

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
  
