#ifndef ANGCALLOGCLASS_H
#define ANGCALLOGCLASS_H

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

class angCalLogClass {


 public:

  
#ifndef MYROOT
  angCalLogClass(slsDetectorUtils *det){ createVars();			
    char cmd[1000];							\
    char *argv[2];							\
    argv[0]=cmd;							\
    sprintf(cmd,"_%d.angcal",det->getFileIndex());			\
    outfile.open(string(det->getFilePath()+string("/")+det->getFileName()+string(cmd)).c_str()); \
    outfile.precision(8);
    myDet=new slsDetectorCommand(det);					\
    if (outfile.is_open()) {						\
      for (int iv=0; iv<nvars; iv++) {					\
	strcpy(cmd,vars[iv]);						\
	outfile << cmd << " "<< myDet->executeLine(1,argv,slsDetectorDefs::GET_ACTION) << endl;	\
      };								\
    };									\
  };
  ~angCalLogClass(){delete myDet;  outfile.close();};
#else
  angCalLogClass() { createVars();  };
  ~angCalLogClass(){};
#endif  
  
  
  int addStep(double pos, string fname) {std::cout.precision(5); outfile << pos << " " << fname << endl; return 0;};
  
  
    // 

  int readHeader(ifstream &infile, int &maxmod, int &nmod, int &chanspermod, char *angconvfile, double &globaloff, double &fineoff, int &angdir, char *ffdir, char *fffile, char *badfile ) { \
    nmod=0; chanspermod=0; globaloff=0; fineoff=0; angdir=1;		\
    strcpy(angconvfile,"none");	 strcpy(ffdir,"none"); strcpy(fffile,"none"); strcpy(badfile,"none"); \
    char line[1000], myvar[100], myarg[100];				\
    float v;								\
    for (int iv=0; iv<nvars; iv++) {					\
      infile.getline(line,1000);					\
      sscanf(line,"%s %s", myvar, myarg);				\
      if (string(myvar)!=string(vars[iv]))
	cout << "Found variable " << myvar << " instead of " << vars[iv] << endl;
      else
	switch (iv) {							\
	case 0:								\
	  if (string(myarg).find("Mythen")!=string::npos)		\
	    chanspermod=1280;						\
	  else if (string(myarg).find("Gotthard")!=string::npos)	\
	    chanspermod=1280;						\
	  else								\
	    chanspermod=65535;						\
	  break;							\
	case 1:								\
	  sscanf(myarg,"%d", &maxmod);					\
	  break;							\
	case 2:								\
	  sscanf(myarg,"%d", &nmod);					\
	break;								\
	case 3:								\
	  strcpy(angconvfile,myarg);					\
	  break;							\
	case 4:								\
	  sscanf(myarg,"%f", &v);				\
	  globaloff=v;							\
	  break;							\
	case 5:								\
	  sscanf(myarg,"%f", &v);				\
	  fineoff=v;							\
	  break;							\
	case 6:								\
	  sscanf(myarg,"%d", &angdir);					\
	  break;							\
	case 7:								\
	  strcpy(ffdir,myarg);						\
	  break;							\
	case 8:								\
	  strcpy(fffile,myarg);						\
	  break;							\
	case 9:								\
	  strcpy(badfile,myarg);					\
	  break;							\
	default:							\
	  ;								\
	};								\
      if  (infile.bad() || infile.eof()) { cout << "bad file "<< iv << endl; return -1;} \
    }									\
    return 0;								\
  };

  int getStep(ifstream &infile, double &threshold, char *datafname){	\
      char line[1000];							\
      float v;
      infile.getline(line,1000);					\
      if (sscanf(line,"%g %s",&v, datafname)<2)	return -1;		\
      printf("scanned %s to %f %s",line,v,datafname);
      threshold=v;							\
      if  (infile.bad() || infile.eof())				\
	return -1;							\
      return 0;								\
  };
  


 private:
  

  void createVars(){							\
    strcpy(vars[0],"type");						\
    strcpy(vars[1],"maxmod");						\
    strcpy(vars[2],"nmod");						\
    strcpy(vars[3],"angconv");						\
    strcpy(vars[4],"globaloff");					\
    strcpy(vars[5],"fineoff");						\
    strcpy(vars[6],"angdir");						\
    strcpy(vars[7],"ffdir");						\
    strcpy(vars[8],"flatfield");					\
    strcpy(vars[9],"badchannels");					\
    nvars=10;								\
  };

#ifndef MYROOT
  slsDetectorCommand *myDet;
#endif
  ofstream outfile;

  char vars[100][100];
  int nvars;
};





#endif
