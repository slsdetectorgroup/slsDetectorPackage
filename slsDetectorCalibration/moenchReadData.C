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
#include "RunningStat.h"
#include "MovingStat.h"
#include "moench02ModuleData.h"

using namespace std;


TH2F *moenchReadData(char *fformat, int runmin, int runmax, int nbins=1500, int hmin=-500, int hmax=1000, int sign=1) {

  moench02ModuleData *decoder=new moench02ModuleData();
  char *buff;
  char fname[10000];
  
  int nf=0;

  TH2F *h2=NULL;
  h2=new TH2F("h2",fformat,nbins,hmin-0.5,hmax-0.5,160*160,-0.5,160*160-0.5);
  
  int val, dum;
  double me, sig, tot;

  MovingStat stat[160][160];

  ifstream filebin;

  int nbg=1000;

  int ix, iy, ir, ic;


  for (ir=0; ir<160; ir++) {
    for (ic=0; ic<160; ic++) {
      stat[ir][ic].Clear();
      stat[ir][ic].SetN(nbg);
    }
  }

  for (int irun=runmin; irun<runmax; irun++) {
    sprintf(fname,fformat,irun);
    cout << "file name " << fname << endl;
    filebin.open((const char *)(fname), ios::in | ios::binary);
   
    while ((buff=decoder->readNextFrame(filebin))) {
	
	for (ix=0; ix<160; ix++)
	  for (iy=0; iy<160; iy++) {

	    dum=0; //no hit
	    tot=0;

	    if (nf>1000) {
	      me=stat[iy][ix].Mean();
	      sig=stat[iy][ix].StandardDeviation();
	      val=sign*decoder->getChannelShort(buff,ix,iy)-me;
	      h2->Fill(val,ix*160+iy);

	      dum=0; //no hit
	      tot=0;

	      for (ir=-1; ir<2; ir++)
		for (ic=-1; ic<2; ic++){
		  if ((ix+ic)>=0 && (ix+ic)<160 && (iy+ir)>=0 && (iy+ir)<160) {
		    if (decoder->getChannelShort(buff,ix+ic,iy+ir)>(stat[iy+ir][ix+ic].Mean()+3.*stat[iy+ir][ix+ic].StandardDeviation())) dum=1; //is a hit or neighbour is a hit!
		    tot+=decoder->getChannelShort(buff,ix+ic,iy+ir)-stat[iy+ir][ix+ic].Mean();
		  }
		}

	      if (tot>3.*sig) dum=3; //discard events where sum of the neighbours is too large.

	      if (val<(me-3.*sig)) dum=2; //discard negative events!
	    }
	    if (nf<1000 || dum==0) 
	      stat[iy][ix].Calc(sign*decoder->getChannelShort(buff,ix,iy));
	   
	  }
      delete [] buff;
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

