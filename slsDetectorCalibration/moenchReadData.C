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




THStack *moenchReadData(char *fformat, char *tit, int runmin, int runmax, int nbins=1500, int hmin=-500, int hmax=1000, int sign=1, double hc=0, double tc=0, int xmin=0, int xmax=NC, int ymin=0, int ymax=NR, int pedsub=1) {
  
  moench02ModuleData *decoder=new moench02ModuleData();
  char *buff;
  char *oldbuff=NULL;
  char fname[10000];
  double oldval;
  int nf=0;


  moench02ModuleData::eventType thisEvent=moench02ModuleData::PEDESTAL;

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

  if (pedsub) {

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


  Int_t iFrame, x, y;
  Double_t data[3][3],ped,sigma;

  TTree *tall;
  if (pedsub) {
    cout << "Subtracting pedestal!!!! " << endl;
    tall=decoder->initMoenchTree(tit, &iFrame, &x, &y, data, &ped, &sigma);
  }
  double tot, tl, tr, bl, br, v;


  // 6% x-talk from previous pixel
  // 12% x-talk from previous frame


  for (int irun=runmin; irun<runmax; irun++) {
    sprintf(fname,fformat,irun);
    cout << "file name " << fname << endl;
    filebin.open((const char *)(fname), ios::in | ios::binary);
   
    while ((buff=decoder->readNextFrame(filebin))) {
	
      for (ix=xmin-1; ix<xmax+1; ix++)
	for (iy=ymin-1; iy<ymax+1; iy++) {
	  ////////////////////////////////////////////////////////
	  if (pedsub) {
	  if (nf>100) {
	    thisEvent= decoder->getEventType(ix, iy, hc, tc, 1);
	  } 
		
	  
	  
	  if (thisEvent==moench02ModuleData::PEDESTAL) {	
	    decoder->addToPedestal( decoder->getChannelShort(ix, iy, hc, tc), ix, iy); 
	  }
	    
	      
	      /***********************************************************
Add  here the function that you want to call: fill histos, make trees etc.
	      ************************************************************/
	      // getClusterElement to access quickly the photon and the neighbours
	  if (nf>1000) {
	    tot=0;
	    tl=0;
	    tr=0;
	    bl=0;
	    br=0;
	    h1->Fill(decoder->getClusterElement(0,0), iy+NR*ix);

	    if (thisEvent==moench02ModuleData::PHOTON_MAX ) {


	      for (ir=-1; ir<2; ir++) {
		for (ic=-1; ic<2; ic++) {
		  v=decoder->getClusterElement(ic,ir);
		  data[ic+1][ir+1]=v;
		  tot+=v;
		  if (ir<1) {
		    
		    if (ic<1) {
		      bl+=v;

		    }
		    if (ic>-1) {
		      br+=v;
		    }
		  }
		  
		  if (ir>-1) {
		    if (ic<1) {
		      tl+=v;
		    }
		    if (ic>-1) {
		      tr+=v;
		    }
		  }
		}
	      }


	      if (bl>br && bl>tl && bl>tr) {
		  h2->Fill(bl, iy+NR*ix);
		if (bl>0) {
		  hetaX->Fill((decoder->getClusterElement(0,0)+decoder->getClusterElement(0,-1))/bl,iy+NR*ix);
		  hetaY->Fill((decoder->getClusterElement(0,0)+decoder->getClusterElement(-1,0))/bl,iy+NR*ix);
		  hetaX->Fill(1.-(decoder->getClusterElement(0,0)+decoder->getClusterElement(0,-1))/bl,iy+NR*(ix-1));
		  hetaY->Fill(1.-(decoder->getClusterElement(0,0)+decoder->getClusterElement(-1,0))/bl,(iy-1)+NR*ix);
		}
	      } else if (br>bl && br>tl && br>tr) {
		  h2->Fill(br, iy+NR*ix);
		if (br>0) {
		  hetaX->Fill((decoder->getClusterElement(0,0)+decoder->getClusterElement(0,-1))/br,iy+NR*ix);
		  hetaY->Fill((decoder->getClusterElement(0,0)+decoder->getClusterElement(1,0))/br,iy+NR*ix);
		  hetaX->Fill(1.-(decoder->getClusterElement(0,0)+decoder->getClusterElement(0,-1))/br,iy+NR*(ix+1));
		  hetaY->Fill(1.-(decoder->getClusterElement(0,0)+decoder->getClusterElement(1,0))/br,iy-1+NR*ix);
		}
	      } else if (tl>br && tl>bl && tl>tr) {
		  h2->Fill(tl, iy+NR*ix);
		if (tl>0) {
		  hetaX->Fill((decoder->getClusterElement(0,0)+decoder->getClusterElement(0,1))/tl,iy+NR*ix);
		  hetaY->Fill((decoder->getClusterElement(0,0)+decoder->getClusterElement(-1,0))/tl,iy+NR*ix);
		  hetaX->Fill(1.-(decoder->getClusterElement(0,0)+decoder->getClusterElement(0,1))/tl,iy+NR*(ix-1));
		  hetaY->Fill(1.-(decoder->getClusterElement(0,0)+decoder->getClusterElement(-1,0))/tl,iy+1+NR*ix);
		}
	      } else if (tr>bl && tr>tl && tr>br) {
		  h2->Fill(tr, iy+NR*ix);
		if (tr>0) {
		  hetaX->Fill((decoder->getClusterElement(0,0)+decoder->getClusterElement(0,1))/tr,iy+NR*ix);
		  hetaY->Fill((decoder->getClusterElement(0,0)+decoder->getClusterElement(1,0))/tr,iy+NR*ix);
		  hetaX->Fill(1.-(decoder->getClusterElement(0,0)+decoder->getClusterElement(0,1))/tr,iy+NR*(ix+1));
		  hetaY->Fill(1.-(decoder->getClusterElement(0,0)+decoder->getClusterElement(1,0))/tr,iy+1+NR*ix);
		}



	      }

		h3->Fill(tot, iy+NR*ix);
		iFrame=decoder->getFrameNumber(buff);
		ped=decoder->getPedestal(ix,iy);
		sigma=decoder->getPedestalRMS(ix,iy);
		x=ix;
		y=iy;

		tall->Fill();
	          
	    }
	    
	    
	    
	  }
	  } else {
	    //cout << ix << " " << iy << endl;
	    h1->Fill(decoder->getChannelShort(ix, iy), iy+NR*ix);
	  }
	  //////////////////////////////////////////////////////////
	  
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
  if (pedsub)
    tall->Write();

  
  
  delete decoder;
  cout << "Read " << nf << " frames" << endl;
  return hs;
}

