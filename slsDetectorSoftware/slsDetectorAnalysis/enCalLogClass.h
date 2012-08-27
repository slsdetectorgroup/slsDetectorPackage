#ifndef ENCALLOGCLASS_H
#define ENCALLOGCLASS_H

#include <iostream>
#include <fstream>
#include "slsDetectorCommand.h"
#include "slsDetectorUtils.h"
#include "sls_detector_defs.h"

using namespace std;

class enCalLogClass {


 public:

  
  enCalLogClass(slsDetectorUtils *det){					\
    char cmd[1000];
    char *argv[2];							\
    argv[0]=cmd;							\
    sprintf(cmd,"_%d.encal",det->getFileIndex());			\
    outfile.open(string(det->getFilePath()+string("/")+det->getFileName()+string(cmd)).c_str()); \
    myDet=new slsDetectorCommand(det);					\
    strcpy(cmd,"settings");						\
    outfile << myDet->executeLine(1,argv,slsDetectorDefs::GET_ACTION) << endl;	\
    strcpy(cmd,"nmod");						\
    outfile << myDet->executeLine(1,argv,slsDetectorDefs::GET_ACTION) << endl;		\
    for (int im=0; im<det->setNumberOfModules(); im++) {		\
      sprintf(cmd,"modulenumber:%d",im);				\
      outfile << myDet->executeLine(1,argv,slsDetectorDefs::GET_ACTION) << endl;		\
    };									\
  };
  
  ~enCalLogClass(){delete myDet; outfile.close();};
  
  int addStep(double threshold, string fname) {outfile << threshold << " " << fname << endl; return 0;};
  
  
  
 private:
  
  slsDetectorCommand *myDet;
  ofstream outfile;



};





#endif
