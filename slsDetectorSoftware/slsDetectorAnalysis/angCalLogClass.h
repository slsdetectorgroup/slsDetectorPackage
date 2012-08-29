#ifndef ANGCALLOGCLASS_H
#define ANGCALLOGCLASS_H

#include <iostream>
#include <fstream>
#include "slsDetectorCommand.h"
#include "slsDetectorUtils.h"
#include "sls_detector_defs.h"

using namespace std;

class angCalLogClass {


 public:

  
  angCalLogClass(slsDetectorUtils *det){				\
    char cmd[1000];
    char *argv[2];							\
    argv[0]=cmd;							\
    sprintf(cmd,"_%d.angcal",det->getFileIndex());			\
    outfile.open(string(det->getFilePath()+string("/")+det->getFileName()+string(cmd)).c_str()); \
    myDet=new slsDetectorCommand(det);					\
    strcpy(cmd,"type");						\
    outfile << myDet->executeLine(1,argv,slsDetectorDefs::GET_ACTION) << endl;		\
    strcpy(cmd,"nmod");							\
    outfile << myDet->executeLine(1,argv,slsDetectorDefs::GET_ACTION) << endl; \
    strcpy(cmd,"angconv");							\
    outfile << myDet->executeLine(1,argv,slsDetectorDefs::GET_ACTION) << endl;		\
    strcpy(cmd,"globaloff");							\
    outfile << myDet->executeLine(1,argv,slsDetectorDefs::GET_ACTION) << endl;		\
    strcpy(cmd,"fineoff");							\
    outfile << myDet->executeLine(1,argv,slsDetectorDefs::GET_ACTION) << endl;		\
    strcpy(cmd,"angdir");							\
    outfile << myDet->executeLine(1,argv,slsDetectorDefs::GET_ACTION) << endl;		\
    strcpy(cmd,"ffdir");							\
    outfile << myDet->executeLine(1,argv,slsDetectorDefs::GET_ACTION) << endl;		\
    strcpy(cmd,"flatfield");							\
    outfile << myDet->executeLine(1,argv,slsDetectorDefs::GET_ACTION) << endl;		\
    strcpy(cmd,"badchannels");							\
    outfile << myDet->executeLine(1,argv,slsDetectorDefs::GET_ACTION) << endl;		\
  };
  
  ~angCalLogClass(){delete myDet; outfile.close();};
  
  int addStep(double pos, string fname) {outfile << pos << " " << fname << endl; return 0;};
  
  
  
 private:
  
  slsDetectorCommand *myDet;
  ofstream outfile;



};





#endif
