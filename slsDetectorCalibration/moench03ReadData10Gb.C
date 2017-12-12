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
#include <TGraph.h>
#include <stdio.h>
//#include <deque>
//#include <list>
//#include <queue>
#include <fstream>
#include "moench03Ctb10GbData.h"
#include "moench03CommonMode.h"
#define MYROOT1
#include "singlePhotonDetector.h"

//#include "MovingStat.h"

using namespace std;

#define NC 400
#define NR 400


//#define MY_DEBUG 1

#ifdef MY_DEBUG
#include <TCanvas.h>
#endif



TH2F *readImage(ifstream &filebin,   TH2F *h2=NULL, TH2F *hped=NULL) {
   moench03Ctb10GbData *decoder=new moench03Ctb10GbData();
  char *buff=decoder->readNextFrame(filebin);
  

//   TH1F *h1=new TH1F("h1","",400*400,0,400*400);
//   int ip=0;
  if (buff) {
    if (h2==NULL) {
      h2=new TH2F("h2","",400,0,400,400,0,400);
      h2->SetStats(kFALSE);
    }
    //  cout << "." << endl;
    for (int ix=0; ix<400; ix++) {
      for (int iy=0; iy<400; iy++) {
	//	cout <<  decoder->getDataSize() << " " << decoder->getValue(buff,ix,iy)<< endl;
	h2->SetBinContent(ix+1,iy+1,decoder->getValue(buff,ix,iy));
	//	h1->SetBinContent(++ip,decoder->getValue(buff,ix,iy));
      }
    }
    if (hped) h2->Add(hped,-1);
    return h2;
  }
  return NULL;
}


TH2F *readImage(char *fname, int iframe=0, TH2F *hped=NULL) {
  ifstream filebin;
  filebin.open((const char *)(fname), ios::in | ios::binary);
  TH2F *h2=new TH2F("h2","",400,0,400,400,0,400);
  TH2F *hh;
  hh=readImage(filebin, h2, hped );
  if (hh==NULL) {

    delete h2;
    return NULL;
  }
  for (int i=0; i<iframe; i++) {
    if (hh==NULL) break;
    hh=readImage(filebin, h2, hped );
    if (hh)
      ;// cout << "="<< endl;
    else {
      delete h2;
      return NULL;
    }
  }
  if (filebin.is_open())
    filebin.close();
  if (hped!=NULL)
    h2->Add(hped,-1);

  return h2;
}


TH2F *calcPedestal(char *fformat, int runmin, int runmax){
  ifstream filebin;
  char fname[10000];
   moench03Ctb10GbData *decoder=new  moench03Ctb10GbData();
  singlePhotonDetector<uint16_t> *filter=new singlePhotonDetector<uint16_t>(decoder, 3, 5, 1, NULL);
  char *buff;
  int ix,iy;
  int ii=0;
  TH2F* h2=NULL;

  for (int irun=runmin; irun<=runmax; irun++) {
    sprintf(fname,fformat,irun);
    
    cout << fname << endl;

    filebin.open((const char *)(fname), ios::in | ios::binary);
    while ((buff=decoder->readNextFrame(filebin))) {
      for (ix=0; ix<400; ix++) {
	for (iy=0; iy<400; iy++) {
	  if (decoder->getValue(buff,ix,iy)>1000)
	    filter->addToPedestal(decoder->getValue(buff,ix,iy), ix, iy);
	}
      }
      delete [] buff;
      //cout << "="<< endl;
      ii++;
    }
    if (filebin.is_open())
      filebin.close(); 
    
  }
  if (ii>0) {
    h2=new TH2F("hped","",400,0,400,400,0,400);
    
      for (ix=0; ix<400; ix++) {
	for (iy=0; iy<400; iy++) {
	  h2->SetBinContent(ix+1, iy+1,filter->getPedestal(ix,iy));
	}
      }

  }
  return h2;

}


TH1D *calcSpectrum(char *fformat, int runmin, int runmax, TH2F *hped=NULL){
  ifstream filebin;
  char fname[10000];
  moench03Ctb10GbData *decoder=new  moench03Ctb10GbData();
  TH1D *hspectrum=new TH1D("hsp","hsp",2500,-500,10000);
  char *buff;
  int ix,iy;
  int ii=0;
  TH2F* h2=NULL;
  int ich=0;
  Double_t ped=0;
  
  for (int irun=runmin; irun<=runmax; irun++) {
    sprintf(fname,fformat,irun);
    
    cout << fname << endl;
    filebin.open((const char *)(fname), ios::in | ios::binary);
    while ((buff=decoder->readNextFrame(filebin))) {
      for (ix=0; ix<200; ix++) {
	for (iy=200; iy<400; iy++) {
	  if (decoder->getValue(buff,ix,iy)>1000) {
	    if(hped) ped=hped->GetBinContent(ix+1,iy+1);
	    hspectrum->Fill(decoder->getValue(buff,ix,iy)-ped);
	  }
	  ich++;
	}
      }
      delete [] buff;
      //cout << "="<< endl;
      ii++;
    }
    if (filebin.is_open())
      filebin.close(); 
    
  }
  return hspectrum;

}
TH2F *drawImage(char *fformat, int runmin, int runmax, TH2F *hped=NULL){
  ifstream filebin;
  char fname[10000];
  moench03Ctb10GbData *decoder=new  moench03Ctb10GbData();
  TH2F *hspectrum=new TH2F("hsp","hsp",400,0,400,400,0,400);
  char *buff;
  int ix,iy;
  int ii=0;
  TH2F* h2=NULL;
  int ich=0;
  Double_t ped=0;
  
  for (int irun=runmin; irun<=runmax; irun++) {
    sprintf(fname,fformat,irun);
    
    cout << fname << endl;
    filebin.open((const char *)(fname), ios::in | ios::binary);
    while ((buff=decoder->readNextFrame(filebin))) {
      for (ix=0; ix<400; ix++) {
	for (iy=0; iy<400; iy++) {
	  if (decoder->getValue(buff,ix,iy)>1000) {
	    if(hped) ped=hped->GetBinContent(ix+1,iy+1);
	    hspectrum->Fill(ix, iy, decoder->getValue(buff,ix,iy)-ped);
	  }
	  ich++;
	}
      }
      delete [] buff;
      //cout << "="<< endl;
      ii++;
    }
    if (filebin.is_open())
      filebin.close(); 
    
  }
  return hspectrum;

}





/**

Loops over data file to find single photons, fills the tree (and writes it to file, althoug the root file should be opened before) and creates 1x1, 2x2, 3x3 cluster histograms with ADCu on the x axis, channel number (160*x+y) on the y axis.

  \param fformat file name format
  \param tit title of the tree etc.
  \param runmin minimum run number
  \param runmax max run number
  \param nbins  number of bins for spectrum hists
  \param hmin histo minimum for spectrum hists
  \param hmax histo maximum for spectrum hists
  \param xmin minimum x coordinate
  \param xmax maximum x coordinate
  \param ymin minimum y coordinate
  \param ymax maximum y coordinate
  \param cmsub  enable commonmode subtraction
  \returns pointer to histo stack with cluster spectra
*/

THStack *moench03ReadData(char *fformat, char *tit, int runmin, int runmax, int nbins=1500, int hmin=-500, int hmax=1000,  int xmin=1, int xmax=NC-1, int ymin=1, int ymax=NR-1, int cmsub=0, int hitfinder=1) {
  double hc=0;
  int sign=1;

   moench03Ctb10GbData *decoder=new  moench03Ctb10GbData();
  cout << "decoder allocated " << endl;

   moench03CommonMode *cmSub=NULL;
   if (cmsub) {
     cmSub=new moench03CommonMode(100);
     cout << "common mode allocated " << endl;

   } else {
     
     cout << "non allocating common mode  " << endl;
   }
   int iev=0;
  int nph=0;

  singlePhotonDetector<uint16_t> *filter=new singlePhotonDetector<uint16_t>(decoder, 3, 5, sign, cmSub, 100, 10);
  cout << "filter allocated " << endl;

  char *buff;
  char fname[10000];
  int nf=0;
  
  eventType thisEvent=PEDESTAL;

  // int iframe;
  // double *data, ped, sigma;

  // data=decoder->getCluster();


  THStack *hs=new THStack("hs",fformat);

  cout << "hstack allocated " << endl;


  TH2F *h1=new TH2F("h1",tit,nbins,hmin-0.5,hmax-0.5,NC*NR,-0.5,NC*NR-0.5);
  hs->Add(h1);
  cout << "h1 allocated " << endl;

  TH2F *h2;
  TH2F *h3;
  if (hitfinder) {
    h2=new TH2F("h2",tit,nbins,hmin-0.5,hmax-0.5,NC*NR,-0.5,NC*NR-0.5);
  cout << "h2 allocated " << endl;
    h3=new TH2F("h3",tit,nbins,hmin-0.5,hmax-0.5,NC*NR,-0.5,NC*NR-0.5);
  cout << "h3 allocated " << endl;
    // hetaX=new TH2F("hetaX",tit,nbins,-1,2,NC*NR,-0.5,NC*NR-0.5);
    //  hetaY=new TH2F("hetaY",tit,nbins,-1,2,NC*NR,-0.5,NC*NR-0.5);
    hs->Add(h2);
    hs->Add(h3);
    // hs->Add(hetaX);
    // hs->Add(hetaY);
  }
  if (hs->GetHists()) {
    for (int i=0; i<3; i++)
      if (hs->GetHists()->At(1)) cout << i << " " ; 
    cout << " histos allocated " << endl;
  } else
    cout << "no hists in stack " << endl;
  
  
  ifstream filebin;


  int ix=20, iy=20, ir, ic;


  Int_t iFrame;
  TTree *tall;
  if (hitfinder)
    tall=filter->initEventTree(tit, &iFrame);


  

#ifdef MY_DEBUG

  cout << "debug mode " << endl;
  
  TCanvas *myC;
  TH2F *he;
  TCanvas *cH1;
  TCanvas *cH2;
  TCanvas *cH3;

  if (hitfinder) {
    myC=new TCanvas("myc");
    he=new TH2F("he","Event Mask",xmax-xmin, xmin, xmax, ymax-ymin, ymin, ymax);
    he->SetStats(kFALSE);
    he->Draw("colz");
    cH1=new TCanvas("ch1");
    cH1->SetLogz();
    h1->Draw("colz");
    cH2=new TCanvas("ch2");
    cH2->SetLogz();
    h2->Draw("colz");
    cH3=new TCanvas("ch3");
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
      
      filter->newFrame();

	if (cmsub) {
	//	cout << "cm" << endl;
	  for (ix=xmin-1; ix<xmax+1; ix++)
	    for (iy=ymin-1; iy<ymax+1; iy++) {
	      thisEvent=filter->getEventType(buff, ix, iy,0);
	    }
	}
      // if (hitfinder) {

      // 	//calculate pedestals and common modes
      // }

      //   cout << "new frame " << endl;

      for (ix=xmin-1; ix<xmax+1; ix++)
	for (iy=ymin-1; iy<ymax+1; iy++) {
	  //	  cout << ix << " " << iy << endl;
	    thisEvent=filter->getEventType(buff, ix, iy, cmsub);
	  //  if (nf>10) {
	    h1->Fill(filter->getClusterTotal(1), iy+NR*ix);

#ifdef MY_DEBUG
	    //  if (iev%10==0)
	    he->SetBinContent(ix+1-xmin, iy+1-ymin, (int)thisEvent);
#endif	  
	  
	  if (hitfinder) {
	      
	      if (thisEvent==PHOTON_MAX ) {
		nph++;

		h2->Fill(filter->getClusterTotal(2), iy+NR*ix);
		h3->Fill(filter->getClusterTotal(3), iy+NR*ix);
		iFrame=decoder->getFrameNumber(buff);

		tall->Fill();
	   
       
	      }
	  
	    
	  } // else {
	  //   filter->addToPedestal(decoder->getValue(buff,ix,iy, cmsub));
	    
	  // }
	    

	    // }
	}
	  //////////////////////////////////////////////////////////
    
#ifdef MY_DEBUG
      //    cout << iev << " " << h1->GetEntries() << " " << h2->GetEntries() << endl;
//       if (iev%10==0) {
// 	myC->Modified();
// 	myC->Update();
// 	cH1->Modified();
// 	cH1->Update();
// 	cH2->Modified();
// 	cH2->Update();
// 	cH3->Modified();
// 	cH3->Update();
//       }
      iev++;
#endif		    
      nf++;
      
      // cout << "=" ;
      delete [] buff;
    }
    //  cout << nph << endl;
    if (filebin.is_open())
    filebin.close();	  
    else
      cout << "could not open file " << fname << endl;
  } 
  if (hitfinder)
    tall->Write(tall->GetName(),TObject::kOverwrite);
  
   //////////////////////////////////////////////////////////
    
#ifdef MY_DEBUG  
	myC->Modified();
	myC->Update();
	cH1->Modified();
	cH1->Update();
	cH2->Modified();
	cH2->Update();
	cH3->Modified();
	cH3->Update();
#endif	
  
  delete decoder;
  cout << "Read " << nf << " frames" << endl;
  return hs;
}
  
TGraph* checkFrameNumber(char *fformat, int runmin, int runmax, int ix, int iy){
  ifstream filebin;
  char fname[10000];
  moench03Ctb10GbData *decoder=new  moench03Ctb10GbData();
  char *buff;
  int ii=0;
 
  TGraph *g=new TGraph();

  

  for (int irun=runmin; irun<=runmax; irun++) {
    sprintf(fname,fformat,irun);
    
    cout << fname << endl;

    filebin.open((const char *)(fname), ios::in | ios::binary);
    
    if (filebin.is_open()) {
    while ((buff=decoder->readNextFrame(filebin))) {
      g->SetPoint(ii,decoder->getFrameNumber(buff),decoder->getValue(buff,ix,iy));   
      ii++;
      delete [] buff;
    }
      //cout << "="<< endl;
    filebin.close(); 
    } else
      cout << "Could not open file " << fname << endl;
    
  }
 
  return g;

}
