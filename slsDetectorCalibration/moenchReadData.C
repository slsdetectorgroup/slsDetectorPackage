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
//#include <deque>
//#include <list>
//#include <queue>
#include <fstream>
#include "moench02ModuleData.h"
#include "moenchCommonMode.h"
#include "singlePhotonDetector.h"

//#include "MovingStat.h"

using namespace std;

#define NC 160
#define NR 160


#define MY_DEBUG 1
#ifdef MY_DEBUG
#include <TCanvas.h>
#endif

/**

Loops over data file to find single photons, fills the tree (and writes it to file, althoug the root file should be opened before) and creates 1x1, 2x2, 3x3 cluster histograms with ADCu on the x axis, channel number (160*x+y) on the y axis.

  \param fformat file name format
  \param tit title of the tree etc.
  \param runmin minimum run number
  \param runmax max run number
  \param nbins  number of bins for spectrum hists
  \param hmin histo minimum for spectrum hists
  \param hmax histo maximum for spectrum hists
  \param sign sign of the spectrum to find hits
  \param hc readout correlation coefficient with previous pixel
  \param xmin minimum x coordinate
  \param xmax maximum x coordinate
  \param ymin minimum y coordinate
  \param ymax maximum y coordinate
  \param cmsub  enable commonmode subtraction
  \returns pointer to histo stack with cluster spectra
*/


THStack *moenchReadData(char *fformat, char *tit, int runmin, int runmax, int nbins=1500, int hmin=-500, int hmax=1000, int sign=1, double hc=0, int xmin=1, int xmax=NC-1, int ymin=1, int ymax=NR-1, int cmsub=0, int hitfinder=1) {
  
  moench02ModuleData *decoder=new moench02ModuleData(hc);
  moenchCommonMode *cmSub=NULL;
  if (cmsub)
    cmSub=new moenchCommonMode();

  int nph=0;

  singlePhotonDetector<uint16_t> *filter=new singlePhotonDetector<uint16_t>(decoder, 3, 5, sign, cmSub);

  char *buff;
  char fname[10000];
  int nf=0;
  
  eventType thisEvent=PEDESTAL;

  // int iframe;
  // double *data, ped, sigma;

  // data=decoder->getCluster();

  TH2F *h2;
  TH2F *h3;
  TH2F *hetaX;
  TH2F *hetaY;

  THStack *hs=new THStack("hs",fformat);



  TH2F *h1=new TH2F("h1",tit,nbins,hmin-0.5,hmax-0.5,NC*NR,-0.5,NC*NR-0.5);
  hs->Add(h1);

  if (hitfinder) {
    h2=new TH2F("h2",tit,nbins,hmin-0.5,hmax-0.5,NC*NR,-0.5,NC*NR-0.5);
    h3=new TH2F("h3",tit,nbins,hmin-0.5,hmax-0.5,NC*NR,-0.5,NC*NR-0.5);
    hetaX=new TH2F("hetaX",tit,nbins,-1,2,NC*NR,-0.5,NC*NR-0.5);
    hetaY=new TH2F("hetaY",tit,nbins,-1,2,NC*NR,-0.5,NC*NR-0.5);
    hs->Add(h2);
    hs->Add(h3);
    hs->Add(hetaX);
    hs->Add(hetaY);
  }

  
  
  ifstream filebin;


  int ix=20, iy=20, ir, ic;


  Int_t iFrame;
  TTree *tall;
  if (hitfinder)
    tall=filter->initEventTree(tit, &iFrame);


  

#ifdef MY_DEBUG
  
  TCanvas *myC;
  TH2F *he;
  TCanvas *cH1;
  TCanvas *cH2;
  TCanvas *cH3;

  if (hitfinder) {
    myC=new TCanvas();
    he=new TH2F("he","Event Mask",xmax-xmin, xmin, xmax, ymax-ymin, ymin, ymax);
    he->SetStats(kFALSE);
    he->Draw("colz");
    cH1=new TCanvas();
    cH1->SetLogz();
    h1->Draw("colz");
    cH2=new TCanvas();
    cH2->SetLogz();
    h2->Draw("colz");
    cH3=new TCanvas();
    cH3->SetLogz();
    h3->Draw("colz");
  }
#endif
  filter->newDataSet();


  for (int irun=runmin; irun<runmax; irun++) {
    sprintf(fname,fformat,irun);
    cout << "file name " << fname << endl;
    filebin.open((const char *)(fname), ios::in | ios::binary);
    nph=0;
    while ((buff=decoder->readNextFrame(filebin))) {
      

      if (hitfinder) {
	filter->newFrame();

	//calculate pedestals and common modes
	if (cmsub) {
	//	cout << "cm" << endl;
	  for (ix=xmin-1; ix<xmax+1; ix++)
	    for (iy=ymin-1; iy<ymax+1; iy++) {
	      thisEvent=filter->getEventType(buff, ix, iy,0);
	    }
	}
      }

      //   cout << "new frame " << endl;

      for (ix=xmin-1; ix<xmax+1; ix++)
	for (iy=ymin-1; iy<ymax+1; iy++) {
	  //	  cout << ix << " " << iy << endl;
	  thisEvent=filter->getEventType(buff, ix, iy, cmsub);

#ifdef MY_DEBUG
	  if (hitfinder) {
	    if (iev%1000==0)
	      he->SetBinContent(ix+1-xmin, iy+1-ymin, (int)thisEvent);
	  }
#endif	  
	  
	  if (nf>1000) {
	    h1->Fill(filter->getClusterTotal(1), iy+NR*ix);
	    if (hitfinder) {
	      
	      if (thisEvent==PHOTON_MAX ) {
		nph++;

		h2->Fill(filter->getClusterTotal(2), iy+NR*ix);
		h3->Fill(filter->getClusterTotal(3), iy+NR*ix);
		iFrame=decoder->getFrameNumber(buff);

		tall->Fill();
	   
       
	      }
	  
	    
	    }
	    

	  }
	}
	  //////////////////////////////////////////////////////////
    
#ifdef MY_DEBUG
      if (iev%1000==0) {
	myC->Modified();
	myC->Update();
	cH1->Modified();
	cH1->Update();
	cH2->Modified();
	cH2->Update();
	cH3->Modified();
	cH3->Update();
      }
      iev++;
#endif		    
      nf++;
      
      cout << "=" ;
      delete [] buff;
    }
    cout << nph << endl;
    if (filebin.is_open())
    filebin.close();	  
    else
      cout << "could not open file " << fname << endl;
  } 
  if (hitfinder)
    tall->Write(tall->GetName(),TObject::kOverwrite);
  
    
  
  delete decoder;
  cout << "Read " << nf << " frames" << endl;
  return hs;
}
  
