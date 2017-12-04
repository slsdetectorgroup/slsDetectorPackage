#ifndef ENCALLOGCLASS_H
#define ENCALLOGCLASS_H

#include <iostream>
#include <fstream>


#ifdef __CINT__
#define MYROOT
#endif

#ifndef MYROOT
#include "slsDetectorCommand.h"
#include "slsDetectorUtils.h"
#include "sls_detector_defs.h"
#endif

using namespace std;

class enCalLogClass {


 public:

#ifndef MYROOT
  enCalLogClass(slsDetectorUtils *det){					\
    char cmd[1000];							\
    char *argv[2];							\
    strcpy(vars[0],"settings");						\
    strcpy(vars[1],"type");						\
    strcpy(vars[2],"nmod");						\
    strcpy(vars[3],"modulenumber");					\
    argv[0]=cmd;							\
    sprintf(cmd,"_%d.encal",det->getFileIndex());			\
    outfile.open(string(det->getFilePath()+string("/")+det->getFileName()+string(cmd)).c_str()); \
    myDet=new slsDetectorCommand(det);					\
    strcpy(cmd,vars[0]);						\
    outfile << cmd << " " << myDet->executeLine(1,argv,slsDetectorDefs::GET_ACTION) << endl;	\
    strcpy(cmd,vars[1]);						\
    outfile << cmd << " " << myDet->executeLine(1,argv,slsDetectorDefs::GET_ACTION) << endl;		\
    strcpy(cmd,vars[2]);						\
    outfile << cmd << " " << myDet->executeLine(1,argv,slsDetectorDefs::GET_ACTION) << endl;		\
    for (int im=0; im<det->setNumberOfModules(); im++) {		\
      sprintf(cmd,"%s:%d",vars[3],im);					\
      outfile << cmd << " " << myDet->executeLine(1,argv,slsDetectorDefs::GET_ACTION) << endl;		\
    }									\
  };
  
  ~enCalLogClass(){delete myDet; outfile.close();};
#else
  enCalLogClass() {							\
    strcpy(vars[0],"settings");						\
    strcpy(vars[1],"type");						\
    strcpy(vars[2],"nmod");						\
    strcpy(vars[3],"modulenumber");					\
  };
  ~enCalLogClass(){};
#endif  
  

  int addStep(double threshold, string fname) {outfile << threshold << " " << fname << endl; return 0;};
  

  // 

  int readHeader(ifstream &infile, char *settings, int &nmod, int &chanspermod, int *mods ) {				\
    nmod=0; strcpy(settings,"unknown"); chanspermod=0;			\
    char line[1000],myarg[100];						\
    int dum;								\
    for (int iv=0; iv<3; iv++) {					\
      infile.getline(line,1000);					\
      switch (iv) {							\
      case 0:								\
	sscanf(line,"settings %s", settings);				\
	break;								\
      case 1:								\
	sscanf(line,"type %s", myarg);					\
	if (string(myarg).find("Mythen")!=string::npos)			\
	  chanspermod=1280;						\
	else if (string(myarg).find("Gotthard")!=string::npos)		\
	  chanspermod=1280;						\
	else								\
	  chanspermod=65535;						\
	break;								\
      case 2:								\
	sscanf(line,"nmod %d", &nmod);					\
	break;								\
      default:								\
	;								\
      };								\
      if  (infile.bad() || infile.eof()) { cout << "bad file "<< iv << endl; return -1;} \
    }									\
    for (int im=0; im<nmod; im++) {					\
      infile.getline(line,1000);					\
      sscanf(line,"modulenumber:%d %x",&dum,mods+im);			\
      if (dum!=im) cout << "read module number "<< dum << " does not match with " << im << " as expected" << endl;
      if (infile.bad() || infile.eof()) { cout << "bad file - module "<< im << endl; return -1;} \
    }

									\
    return 0;								\
  };

  int getStep(ifstream &infile, double &threshold, char *datafname){	\
      char line[1000];							\
      float v;
      infile.getline(line,1000);					\
      if (sscanf(line,"%g %s",&v, datafname)<2)	return -1;		\
      threshold=v;							\
      if  (infile.bad() || infile.eof())				\
	return -1;							\
      return 0;								\
  };
  


  

 private:
  

#ifndef MYROOT
  slsDetectorCommand *myDet;
#endif
  ofstream outfile;
  char vars[4][100];

};




#endif
