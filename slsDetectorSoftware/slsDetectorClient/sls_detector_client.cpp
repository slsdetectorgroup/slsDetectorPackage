#include "multiSlsDetectorClient.h"


#include <stdlib.h>
using namespace std;

int main(int argc, char *argv[])
  
{
#ifdef PUT
  int action=slsDetectorDefs::PUT_ACTION;
#endif
    
#ifdef GET
  int action=slsDetectorDefs::GET_ACTION;
#endif
    

#ifdef READOUT
  int action=slsDetectorDefs::READOUT_ACTION;
#endif
    

#ifdef HELP
  int action=slsDetectorDefs::HELP_ACTION;
#endif

  multiSlsDetectorClient *cl;
  if (argc>1)
    cl=new multiSlsDetectorClient(argc-1, argv+1, action);
  else
    cl=new multiSlsDetectorClient(argc-1, argv, action);

  delete cl;
}

  
