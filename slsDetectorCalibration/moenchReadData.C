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

//#include "MovingStat.h"

using namespace std;

#define NC 160
#define NR 160


#define MY_DEBUG 1
#ifdef MY_DEBUG
#include <TCanvas.h>
#endif

/**

  char *fformat, file name format
  char *tit, title of the tree etc.
  int runmin, minimum run number
  int runmax, max run number
  int nbins=1500, number of bins for spectrum hists
  int hmin=-500, minimum for spectrum hists
  int hmax=1000, maximum for spectrum hists
  int sign=1, sign of the spectrum to find hits
  double hc=0, readout correlation coefficient with previous pixel
  double tc=0, time correlation coefficient with previous frame (case of bad reset)
  int xmin=0, minimum x coordinate
  int xmax=NC, maximum x coordinate
  int ymin=0, minimum y coordinate
  int ymax=NR, maximum y coordinate
  int pedsub=1, enable pedestal subtraction
  int cmsub=0  enable commonmode subtraction

*/


THStack *moenchReadData(char *fformat, char *tit, int runmin, int runmax, int nbins=1500, int hmin=-500, int hmax=1000, int sign=1, double hc=0, double tc=0, int xmin=0, int xmax=NC, int ymin=0, int ymax=NR, int pedsub=1, int cmsub=0) {
  
  moench02ModuleData *decoder=new moench02ModuleData();
  char *buff;
  char *oldbuff=NULL;
  char fname[10000];
  double oldval;
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


  int iev=0;

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
  

  int scmin=xmin/40, scmax=(xmax-1)/40+1;

  // 6% x-talk from previous pixel
  // 12% x-talk from previous frame

#ifdef MY_DEBUG
  TCanvas *myC=new TCanvas();
  TH2F *he=new TH2F("he","Event",3,-1.5,1.5,3,-1.5,1.5);
  he->SetStats(kFALSE);
  he->Draw("colz");
  he->SetMinimum(0);
  he->SetMaximum(0.5*hmax);
#endif

  for (int irun=runmin; irun<runmax; irun++) {
    sprintf(fname,fformat,irun);
    cout << "file name " << fname << endl;
    filebin.open((const char *)(fname), ios::in | ios::binary);
   
    while ((buff=decoder->readNextFrame(filebin))) {



      if (nf>100) {
	if (cmsub) {
	  for (int isc=scmin; isc<scmax; isc++) {
	    decoder->calculateCommonMode(3+isc*40, 40*(isc+1)-3, 3, NR-3, hc, tc);
#ifdef MY_DEBUG
	    if (nf%1000==0) cout << "sc=" << isc << " CM="<< decoder->getCommonMode(3+isc*40, NR/2)<< endl;
#endif
	  }
	}
	
      }
      



      for (ix=xmin-1; ix<xmax+1; ix++)
	for (iy=ymin-1; iy<ymax+1; iy++) {
	  ////////////////////////////////////////////////////////
	  if (pedsub) {


	    if (nf>100) {
	      thisEvent= decoder->getEventType(ix, iy, hc, tc, 1,1);
	    } 
	    
	  


	    if (thisEvent==PEDESTAL) {
	      if (cmsub && nf>1000)
		decoder->addToPedestal(decoder->getChannelShort(ix, iy, hc, tc)-decoder->getCommonMode(ix,iy), ix, iy); 
	      else
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
	    
	    //  if (nf%1000==0 && ix==20 && iy==20) cout <<  " val="<< decoder->getClusterElement(0,0)<< endl;

	    if (thisEvent==PHOTON_MAX ) {
	      #ifdef MY_DEBUG
		  if (iev%100000==0) {
		  cout << "Event " << iev << " Frame "<< nf << endl;
		  }
#endif		  
	
	      
	      for (ir=-1; ir<2; ir++) {
		for (ic=-1; ic<2; ic++) {
		  v=decoder->getClusterElement(ic,ir,cmsub);
		  data[ic+1][ir+1]=v;
#ifdef MY_DEBUG
		  if (iev%100000==0) {
		    he->SetBinContent(ic+2,ir+2,v);
		    cout << "Histo("<< ix+ic << ","<< iy+ir <<")" << v << endl;
		  }
#endif		  
		  

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
		  hetaX->Fill((decoder->getClusterElement(0,0,cmsub)+decoder->getClusterElement(0,-1,cmsub))/bl,iy+NR*ix);
		  hetaY->Fill((decoder->getClusterElement(0,0,cmsub)+decoder->getClusterElement(-1,0,cmsub))/bl,iy+NR*ix);
		  hetaX->Fill(1.-(decoder->getClusterElement(0,0,cmsub)+decoder->getClusterElement(0,-1,cmsub))/bl,iy+NR*(ix-1));
		  hetaY->Fill(1.-(decoder->getClusterElement(0,0,cmsub)+decoder->getClusterElement(-1,0,cmsub))/bl,(iy-1)+NR*ix);
		}
	      } else if (br>bl && br>tl && br>tr) {
		  h2->Fill(br, iy+NR*ix);
		if (br>0) {
		  hetaX->Fill((decoder->getClusterElement(0,0,cmsub)+decoder->getClusterElement(0,-1,cmsub))/br,iy+NR*ix);
		  hetaY->Fill((decoder->getClusterElement(0,0,cmsub)+decoder->getClusterElement(1,0,cmsub))/br,iy+NR*ix);
		  hetaX->Fill(1.-(decoder->getClusterElement(0,0,cmsub)+decoder->getClusterElement(0,-1,cmsub))/br,iy+NR*(ix+1));
		  hetaY->Fill(1.-(decoder->getClusterElement(0,0,cmsub)+decoder->getClusterElement(1,0,cmsub))/br,iy-1+NR*ix);
		}
	      } else if (tl>br && tl>bl && tl>tr) {
		  h2->Fill(tl, iy+NR*ix);
		if (tl>0) {
		  hetaX->Fill((decoder->getClusterElement(0,0,cmsub)+decoder->getClusterElement(0,1,cmsub))/tl,iy+NR*ix);
		  hetaY->Fill((decoder->getClusterElement(0,0,cmsub)+decoder->getClusterElement(-1,0,cmsub))/tl,iy+NR*ix);
		  hetaX->Fill(1.-(decoder->getClusterElement(0,0,cmsub)+decoder->getClusterElement(0,1,cmsub))/tl,iy+NR*(ix-1));
		  hetaY->Fill(1.-(decoder->getClusterElement(0,0,cmsub)+decoder->getClusterElement(-1,0,cmsub))/tl,iy+1+NR*ix);
		}
	      } else if (tr>bl && tr>tl && tr>br) {
		  h2->Fill(tr, iy+NR*ix);
		if (tr>0) {
		  hetaX->Fill((decoder->getClusterElement(0,0,cmsub)+decoder->getClusterElement(0,1,cmsub))/tr,iy+NR*ix);
		  hetaY->Fill((decoder->getClusterElement(0,0,cmsub)+decoder->getClusterElement(1,0,cmsub))/tr,iy+NR*ix);
		  hetaX->Fill(1.-(decoder->getClusterElement(0,0,cmsub)+decoder->getClusterElement(0,1,cmsub))/tr,iy+NR*(ix+1));
		  hetaY->Fill(1.-(decoder->getClusterElement(0,0,cmsub)+decoder->getClusterElement(1,0,cmsub))/tr,iy+1+NR*ix);
		}



	      }

		h3->Fill(tot, iy+NR*ix);
		iFrame=decoder->getFrameNumber(buff);
		ped=decoder->getPedestal(ix,iy);
		sigma=decoder->getPedestalRMS(ix,iy);
		x=ix;
		y=iy;

		tall->Fill();
	   
#ifdef MY_DEBUG
		  if (iev%100000==0) {
		    myC->Modified();
		    myC->Update();
		  }
#endif		  

		iev++;
       
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
    tall->Write(tall->GetName(),TObject::kOverwrite);

  
  
  delete decoder;
  cout << "Read " << nf << " frames" << endl;
  return hs;
  }

