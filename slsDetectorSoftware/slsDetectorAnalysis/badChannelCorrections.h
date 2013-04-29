#ifndef BAD_CHANNEL_CORRECTIONS_H
#define BAD_CHANNEL_CORRECTIONS_H


#include <iostream>
#include <fstream>
#include <sstream>
#include <string.h>

using namespace std;
class badChannelCorrections{

 public:

  static int readBadChannelCorrectionFile(string fname, int &nbad, int *badlist){ ifstream infile(fname.c_str()); int nb=-1; if (infile.is_open()) {nb=readBadChannelCorrectionFile(infile,nbad,badlist); infile.close();}; return nb;};
  
 

  static int readBadChannelCorrectionFile(ifstream &infile, int &nbad, int *badlist, int moff=0){ \
    int interrupt=0;							\
    int ich;							\
    int chmin,chmax;						\
    string str;							\
    nbad=0;							\
    while (infile.good() and interrupt==0) {			\
      getline(infile,str);					\
      istringstream ssstr;					\
      ssstr.str(str);						\
      if (ssstr.bad() || ssstr.fail() || infile.eof()) {	\
	interrupt=1;						\
	break;							\
      }								\
      if (str.find('-')!=string::npos) {			\
	ssstr >> chmin ;					\
	ssstr.str(str.substr(str.find('-')+1,str.size()));	\
	ssstr >> chmax;						\
	for (ich=chmin; ich<=chmax; ich++) {			\
	  badlist[nbad]=ich;					\
	  cout << badlist[nbad] << " " << moff << endl;			\
	  nbad++;							\
	}								\
      } else {								\
	ssstr >> ich;							\
	badlist[nbad]=ich;						\
	nbad++;								\
      }									\
    }									\
    return nbad;  };


  static int setBadChannelCorrection(ifstream &infile, int &nbad, int *badlist, int moff){ \
    int retval=readBadChannelCorrectionFile(infile,nbad,badlist);	\
    for (int ich=0; ich<nbad; ich++)    { badlist[ich]=badlist[ich]+moff; cout << badlist[ich] << " " << moff << endl; }; \
    return retval;							\
  };
  
 protected:
  

  char *badChanFile;
  int *nBadChans;
  int *badChansList;
  int *nBadFF;
  int *badFFList;




};
#endif
