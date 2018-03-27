#include "multiSlsDetectorClient.h"
#include "gitInfoLib.h"


#include <stdlib.h>
using namespace std;

int main(int argc, char *argv[])
  
{
	for (int i = 1; i < argc; ++i ) {
		if (!(strcmp (argv[i],"--version")) || !(strcmp (argv[i],"-v"))) {
			int64_t tempval = GITDATE;
			cout << argv[0] << " " << GITBRANCH << " (0x" << hex << tempval << ")" << endl;
			return 0;
		}
	}

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

  
