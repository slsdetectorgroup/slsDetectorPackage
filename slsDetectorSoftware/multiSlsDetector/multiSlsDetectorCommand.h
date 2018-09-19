
#ifndef MULTI_SLS_DETECTOR_COMMAND_H
#define MULTI_SLS_DETECTOR_COMMAND_H


#include "slsDetector.h"
#include "multiSlsDetector.h"
#include "slsDetectorCommand.h"



/** @short This class handles the command line I/Os, help etc. of the text clients  */


class multiSlsDetectorCommand : public slsDetectorCommand { 

 public:


  multiSlsDetectorCommand(multiSlsDetector *det) : slsDetectorCommand(det) {myDet=det;};


  /*   /\** */
/*      executes a set of string arguments according to a given format. It is used to read/write configuration file, dump and retrieve detector settings and for the command line interface command parsing */
/*      \param narg number of arguments */
/*      \param args array of string arguments */
/*      \param action can be PUT_ACTION or GET_ACTION (from text client even READOUT_ACTION for acquisition) */
/*      \returns answer string  */
/*   *\/ */

    std::string executeLine(int narg, char *args[], int action, int id=-1) { \
      std::string s;								\
      if (id>=0) {
	slsDetector *d=myDet->getSlsDetector(id);			\
	if (d) {							\
	  slsDetectorCommand *cmd=new slsDetectorCommand(d);		\
	  s=cmd->executeLine(narg, args, action);			\
	  if(d->getErrorMask())					\
		  myDet->setErrorMask((myDet->getErrorMask())|(1<<id));		\
	  delete cmd;
	} else
	  s=std::string("detector does no exist");			\
      } else							\
	s=slsDetectorCommand::executeLine(narg,args,action);	\
      return s;
    };

    /**
     * calls executeLine with PUT_ACTION
     */
    std::string putCommand(int narg, char *args[], int pos=-1){\
    	return executeLine(narg, args,slsDetectorDefs::PUT_ACTION,pos);\
    };
    /**
     * calls executeLine with GET_ACTION
     */
    std::string getCommand(int narg, char *args[], int pos=-1){\
    	return executeLine(narg, args,slsDetectorDefs::GET_ACTION,pos);\
    };

 private:


  multiSlsDetector *myDet;


};



#endif

