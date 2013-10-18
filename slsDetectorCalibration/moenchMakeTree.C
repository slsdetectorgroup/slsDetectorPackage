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
#include <TThread.h>

using namespace std;


//tree variables 
int xC,yC,iFrameC;
double meC,sigC;
Double_t dataC[3][3];
TTree* tall;

typedef struct task_s{
  char *fformat;
  char *tname;
  int runmin;
  int runmax;
  int sign;
} Task;

void setUpTree(char *tname){
  tall=new TTree(tname,tname);
  tall->Branch("iFrame",&iFrameC,"iframe/I");
  tall->Branch("x",&xC,"x/I");
  tall->Branch("y",&yC,"y/I");
  tall->Branch("data",dataC,"data[3][3]/D");
  tall->Branch("pedestal",&meC,"pedestal/D");
  tall->Branch("rms",&sigC,"rms/D");
}

inline void storeEvent(int iF,int x,int y, Double_t data[][3], double me, double sig){
  TThread::Lock();
  xC = x; yC = y; iFrameC = iF;
  memcpy(dataC,data,sizeof(Double_t)*3*3);
  meC = me; sigC = sig;
  tall->Fill(); 
  TThread::UnLock();
}

void moenchMakeTree(char *fformat, char *tname, int runmin, int runmax, int sign=1) {

  moench02ModuleData *decoder=new moench02ModuleData();
  char *buff;
  char fname[10000];
  
  int nf=0;
  
  int dum, nPhotons;
  double me, sig, tot, maxNei, val, valNei;

  MovingStat stat[160][160];
  MovingStat nPhotonsStat;

  ifstream filebin;

  int nbg=1000;

  int ix, iy, ir, ic;
  Double_t data[3][3];

  nPhotonsStat.Clear();
  nPhotonsStat.SetN(1000);

  for (ir=0; ir<160; ir++) {
    for (ic=0; ic<160; ic++) {
      stat[ir][ic].Clear();
      stat[ir][ic].SetN(nbg);
    }
  }

  for (int irun=runmin; irun<runmax; irun++) {
    sprintf(fname,fformat,irun);
    //cout << "process file " << fname; // cout.flush();
    filebin.open((const char *)(fname), ios::in | ios::binary);
   
    while ((buff=decoder->readNextFrame(filebin))) {
      nPhotons=0;
      for (ix=0; ix<160; ix++){
	  for (iy=0; iy<160; iy++) {

	    dum=0; //no hit
	    tot=0;

	    if (nf>nbg) {
	      me=stat[iy][ix].Mean();
	      sig=stat[iy][ix].StandardDeviation();
	      val=sign*decoder->getChannelShort(buff,ix,iy)-me;

	      dum=0; //no hit
	      tot=0;
	      maxNei = 0;

	      for (ir=-1; ir<2; ir++){
		for (ic=-1; ic<2; ic++){
		  if ((ix+ic)>=0 && (ix+ic)<160 && (iy+ir)>=0 && (iy+ir)<160) {
		    valNei = sign*decoder->getChannelShort(buff,ix+ic,iy+ir)-stat[iy+ir][ix+ic].Mean();
		    if (sign*decoder->getChannelShort(buff,ix+ic,iy+ir)>(stat[iy+ir][ix+ic].Mean()+3.*stat[iy+ir][ix+ic].StandardDeviation())) dum=1; //is a hit or neighbour is a hit!
		    tot+=valNei;
		    data[ir+1][ic+1] = valNei;
		    maxNei = max(maxNei,valNei);		    
		  }
		}
	      }

	      if (val<(me-3.*sig)) dum=2; //discard negative events!

	      if(maxNei == val && dum == 1){ // this is an event and we are in the center
		storeEvent(nf,ix,iy,data,me,sig);
		nPhotons++;
		//cout << "X: " << ix << " Y: " << iy << " val: " << val << " tot: " << tot << endl; 
	      }else{
		dum = 3; //discard events (for pedestal) where sum of the neighbours is too large.
	      }

	      //if (tot>3.*sig && dum == 1){ dum=3;} 	      
	      //cout << dum;
	    }
	    if (nf<nbg || dum==0) 
	      stat[iy][ix].Calc(sign*decoder->getChannelShort(buff,ix,iy));
	   
	  }
      }
      delete [] buff;
      //cout << "="; cout.flush();
      if(nf>nbg) nPhotonsStat.Calc((double)nPhotons);
      nf++;
    }
    //cout << endl;
    cout << "processed File " << fname << "  done. Avg. Photons/Frame: " << nPhotonsStat.Mean() << " sig: " <<  nPhotonsStat.StandardDeviation() << endl;
    if (filebin.is_open())
      filebin.close();	  
    else
      cout << "could not open file " << fname << endl;
  }

  delete decoder;
  cout << "Read " << nf << " frames" << endl;

 
}

void *moenchMakeTreeTask(void *p){
  Task *t = (Task *)p;
  moenchMakeTree(t->fformat,t->tname,t->runmin,t->runmax,t->sign);
  return 0;
}


//to compile: g++ -DMYROOT -g `root-config --cflags --glibs` -o moenchMakeTree moenchMakeTree.C
int main(int argc, char **argv){
  if(argc != 6){ cout << "Usage: inFile outdir tname fileNrStart fileNrEnd" << endl; exit(-1); }
  
  int nThreads = 15;
  TThread *threads[nThreads];

  char *inFile = argv[1];
  char *outDir = argv[2];
  char *tName = argv[3];
  int start = atoi(argv[4]);
  int end = atoi(argv[5]);

  TFile *f;
  char outfname[1000];
  char threadName[1000];

  sprintf(outfname,"%s/%s.root",outDir,tName);
  f=new TFile(outfname,"RECREATE");

  cout << "outputfile: " << outfname << endl;
  setUpTree(tName);
  
  for(int i = 0; i < nThreads; i++){
    sprintf(threadName,"t%i",i);
    Task *t = (Task *)malloc(sizeof(Task));
    t->fformat = inFile;
    t->tname = tName;
    t->sign = -1;
    t->runmin = start + (end-start)/(nThreads-1)*i;
    t->runmax = start + (end-start)/(nThreads-1)*(i+1);
    if(i == nThreads - 1) t->runmax = end;
    cout << "start thread " << i << " start: " << t->runmin << " end " << t->runmax << endl;
    threads[i] = new TThread(threadName, moenchMakeTreeTask, t);
    threads[i]->Run();
  }
  

  //TThread::Ps();

  for(int i = 0; i < nThreads; i++){
    threads[i]->Join();
  }


  tall->Write();
  tall->GetCurrentFile()->Close();
}

