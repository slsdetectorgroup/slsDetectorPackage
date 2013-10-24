#include <TH1D.h>
#include <TH2D.h>
#include <TPad.h>
#include <TDirectory.h>
#include <TEntryList.h>
#include <TFile.h>
#include <TMath.h>
#include <TTree.h>
#include <TChain.h>
#include <THStack.h>
#include <TCanvas.h>
#include <stdio.h>
#include <deque>
#include <list>
#include <queue>
#include <fstream>
#include "moench02ModuleData.h"

using namespace std;

#define NC 160
#define NR 160



TH2F *moenchReadData(char *fformat, int runmin, int runmax, int nbins=1500, int hmin=-500, int hmax=1000, int sign=1, int nsigma=5, double hc=0, double tc=0, int xmin=0, int xmax=NC, int ymin=0, int ymax=NR) {
  
  moench02ModuleData *decoder=new moench02ModuleData();
  char *buff;
  char *oldbuff=NULL;
  char fname[10000];
  double oldval;
  int nf=0;


  moench02ModuleData::eventType thisEvent=moench02ModuleData::PEDESTAL;


  TH2F *h2=NULL;
  h2=new TH2F("h2",fformat,nbins,hmin-0.5,hmax-0.5,NC*NR,-0.5,NC*NR-0.5);

  int val, dum;
  double me=0, sig, tot, vv;

  MovingStat stat[160][160];

  ifstream filebin;

  int nbg=1000;

  int ix=20, iy=20, ir, ic;

  // 6% x-talk from previous pixel
  // 12% x-talk from previous frame


  for (int irun=runmin; irun<runmax; irun++) {
    sprintf(fname,fformat,irun);
    cout << "file name " << fname << endl;
    filebin.open((const char *)(fname), ios::in | ios::binary);
   
    while ((buff=decoder->readNextFrame(filebin))) {
	
      for (ix=xmin-1; ix<xmax+1; ix++)
	for (iy=ymin-1; iy<ymax+1; iy++) {

	  
	  if (nf>100) {
	    thisEvent= decoder->getEventType(ix, iy, hc, tc, 1);
	  } 
		
	  if (thisEvent==moench02ModuleData::PEDESTAL) {	
	    decoder->addToPedestal(decoder->getClusterElement(0,0), ix, iy);
	  }
	    
	      
	      /***********************************************************
Add  here the function that you want to call: fill histos, make trees etc.
	      ************************************************************/
	      // getClusterElement to access quickly the photon and the neighbours
	  
	}
      cout << "=" ;
      nf++;
    }
    cout << endl;
    if (filebin.is_open())
      filebin.close();	  
    else
      cout << "could not open file " << fname << endl;
  }
  
  

  
  
  delete decoder;
  cout << "Read " << nf << " frames" << endl;
  return h2;
}

