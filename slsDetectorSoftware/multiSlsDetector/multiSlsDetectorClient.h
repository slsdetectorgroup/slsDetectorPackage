#include <iostream>
#include <string>


#include "multiSlsDetector.h"
#include "multiSlsDetectorCommand.h"
#include "sls_receiver_exceptions.h"


#include <stdlib.h>


int dummyCallback(detectorData* d, int p,void*) {
	cout << "got data "	<< p <<  endl;
	return 0;
};

class multiSlsDetectorClient  { 

public:
	multiSlsDetectorClient(int argc, char *argv[], int action, multiSlsDetector *myDetector=NULL) {	\
		std::string answer;					      									\
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

		if (action==slsDetectorDefs::READOUT_ACTION)  {							\
			id = 0;																\
			pos = -1;															\
			if (argc) {															\
				// multi id scanned
				if (strchr(argv[0],'-')) {										\
					iv=sscanf(argv[0],"%d-%s",&id, cmd);						\
					//%s needn't be there (if not 1:), so 1 or 2 arguments scanned
					if (iv >= 1 && id >= 0) { 									\
						argv[0] = cmd;                                      	\
						cout << id << "-" ;                                 	\
					} else {                                               		\
						id = 0;                                          		\
					}															\
				}																\
				// single id scanned
				if (strchr(argv[0],':')) {										\
					iv=sscanf(argv[0],"%d:",&pos);								\
					if (iv == 1 && pos >= 0) {									\
						cout << "pos " << pos << " is not allowed for readout!" << endl;		\
						return;		                  							\
					}															\
				}																\
			}																	\
		} else {																\
			// multi id scanned
			iv=sscanf(argv[0],"%d-%s",&id, cmd);								\
			// scan success
			if (iv == 2 && id >= 0) {                                       	\
				argv[0] = cmd;                                                  \
				cout << id << "-" ;                                             \
			} else {                                                            \
				id = 0;                                                         \
			}																	\
			// sls pos scanned
			iv=sscanf(argv[0],"%d:%s", &pos, cmd);                              \
			// scan success
			if (iv == 2 && pos >= 0) {                                     		\
				argv[0] = cmd;                                                  \
				cout << pos << ":" ;                                            \
			}                                                                   \
			if (iv != 2) {                                                  	\
				pos = -1;                                                       \
			}																	\
			// remove the %d- and %d:
			if (!strlen(cmd))	{												\
				strcpy(cmd, argv[0]);											\
			}																	\
			// special commands
			std::string scmd = cmd;													\
			// free without calling multiSlsDetector constructor
			if (scmd == "free") {												\
				if (pos != -1)													\
				slsDetector::freeSharedMemory(id, pos);							\
				else															\
				multiSlsDetector::freeSharedMemory(id);							\
				return;															\
			}																	\
			// get user details without verify sharedMultiSlsDetector version
			else if ((scmd == "user") &&  (action==slsDetectorDefs::GET_ACTION)) {	\
				verify = false;													\
				update = false;													\
				myDetector=NULL;												\
			}																	\
		}																		\



		//cout<<"id:"<<id<<" pos:"<<pos<<endl;
		// create multiSlsDetector class if required
		if (myDetector==NULL) {													\
			try {																\
				multiSlsDetector* m = new multiSlsDetector(id, verify, update);			\
				myDetector = m;													\
			} catch (const SlsDetectorPackageExceptions & e) {							\
				/*cout << e.GetMessage() << endl;*/									\
				return;															\
			} catch (...) {														\
				cout << " caught exception" << endl;							\
				return;															\
			}																	\
			del=1;																\
		}																		\

		// call multi detector command line
		myCmd=new multiSlsDetectorCommand(myDetector);							\
		try {																	\
			answer=myCmd->executeLine(argc, argv, action, pos);					\
		} catch (const SlsDetectorPackageExceptions & e) {								\
			/*cout << e.GetMessage() << endl;	*/									\
			delete myCmd;														\
			if (del) delete myDetector;											\
			return;																\
		} catch (...) {															\
			cout << " caught exception" << endl;								\
			delete myCmd;														\
			if (del) delete myDetector;											\
			return;																\
		}																		\
		if (action!=slsDetectorDefs::READOUT_ACTION) { 							\
			cout << argv[0] << " " ;											\
		}																		\
		cout << answer<< endl;													\
		delete myCmd;															\
		if (del) delete myDetector;												\
	};

};











