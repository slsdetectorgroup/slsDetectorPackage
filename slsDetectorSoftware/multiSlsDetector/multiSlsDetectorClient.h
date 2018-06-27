#include <iostream>
#include <string>


#include "multiSlsDetector.h"
#include "multiSlsDetectorCommand.h"
#include "sls_detector_exceptions.h"


#include <stdlib.h>


int dummyCallback(detectorData* d, int p,void*) {
	cout << "got data "	<< p <<  endl;
	return 0;
};

class multiSlsDetectorClient  { 

public:
	multiSlsDetectorClient(int argc, char *argv[], int action, multiSlsDetector *myDetector=NULL) {	\
		string answer;					      									\
		multiSlsDetectorCommand *myCmd;					      					\
		int id = -1, pos = -1, iv = 0;			      							\
		bool verify = true, update = true;										\
		int del = 0;					      									\
		char cmd[100] = "";														\

		if (action==slsDetectorDefs::PUT_ACTION && argc<2) {					\
			cout << "Wrong usage - should be: "<< argv[0] <<					\
			"[id-][pos:]channel arg" << endl;									\
			cout << endl;														\
			return;																\
			if (del) delete myDetector;											\
		};																		\
		if (action==slsDetectorDefs::GET_ACTION && argc<1) {					\
			cout << "Wrong usage - should be: "<< argv[0] <<					\
			"[id-][pos:]channel arg" << endl;									\
			cout << endl;														\
			if (del) delete myDetector;											\
			return;																\
		};																		\



		// multi id scanned
		iv=sscanf(argv[0],"%d-%s",&id, cmd);									\
		if (iv != 2 || id < 0)                                                  \
			id = 0;                                                             \
		if (iv == 2 && id >= 0) {                                               \
			argv[0] = cmd;                                                      \
			cout << id << "-" ;                                                 \
		}                                                                       \

		// sls pos scanned
		iv=sscanf(argv[0],"%d:%s", &pos, cmd);                                  \
		if (iv != 2 )                                                           \
			pos = -1;                                                            \
		if (iv == 2 && pos >= 0) {                                              \
			argv[0] = cmd;                                                      \
			cout << pos << ":" ;                                                \
		}                                                                       \

		if ((action==slsDetectorDefs::READOUT_ACTION) && (pos != -1) ) {		\
			cout << "pos " << pos << "is not allowed for readout!" << endl;		\
			return;																\
		}																		\

		if (!strlen(cmd))														\
			strcpy(cmd, argv[0]);												\

		// special commands
		string scmd = cmd;														\
		// free without calling multiSlsDetector constructor
		if (scmd == "free") {													\
			if (pos != -1)														\
				slsDetector::freeSharedMemory(id, pos);							\
			else																\
				multiSlsDetector::freeSharedMemory(id);							\
			return;																\
		}																		\
		// (sls level): give error message
		// (multi level): free before calling multiSlsDetector constructor
		else if (scmd == "hostname") {											\
			if (pos != -1)	{													\
				cout << "pos " << pos << "not allowed for hostname. "			\
				"Only from multi detector level." << endl;						\
				return;															\
			}																	\
			else																\
				multiSlsDetector::freeSharedMemory(id);							\
		}																		\
		// get user details without verify sharedMultiSlsDetector version
		else if ((scmd == "userdetails") &&  (action==slsDetectorDefs::GET_ACTION)) {\
			verify = false;														\
			update = false;														\
			myDetector=NULL;													\
		}																		\


		// create multiSlsDetector class if required
		if (myDetector==NULL) {													\
			try {																\
				myDetector = new multiSlsDetector(id, verify, update);			\
			} catch (const SharedMemoryException & e) {							\
				cout << e.GetMessage() << endl;									\
				return;															\
			} catch (...) {														\
				cout << " caught exception" << endl;							\
				return;															\
			}																	\
			del=1;																\
		}																		\


		// readout
		if (action==slsDetectorDefs::READOUT_ACTION) { 							\
			myCmd=new multiSlsDetectorCommand(myDetector);	    				\
			answer=myCmd->executeLine(argc, argv, action);	      				\
			cout << answer<< endl;				      							\
			delete myCmd;					      								\
			if (del)      delete myDetector;			      					\
			return;						      									\
		};							      										\

		cout<<"id:"<<id<<" pos:"<<pos<<endl;
		// call multi detector command line
		myCmd=new multiSlsDetectorCommand(myDetector);							\
		answer=myCmd->executeLine(argc, argv, action, pos);						\
		cout << argv[0] << " " ;												\
		cout << answer<< endl;													\
		delete myCmd;															\
		if (del) delete myDetector;												\
	};

};











