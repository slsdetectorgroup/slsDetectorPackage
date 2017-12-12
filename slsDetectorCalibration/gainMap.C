#define MYROOT
#include <TH2F.h>
#include <TSpectrum.h>
#include <TH1D.h>
#include <TFile.h>
#include <TList.h>
#include <TPolyMarker.h>
#include <THStack.h>
#include <TF1.h>
#include <TGraphErrors.h>
#include <iostream>
#include <energyCalibration.h>

using namespace std;


#define FOPT "0"

TH2F *gainMap(TH2F *h2, float g) {
  //  int npx, npy;
  int npx=160, npy=160;
  // det->getDetectorSize(npx, npy);

  TH2F *gMap=new TH2F("gmap",h2->GetTitle(), npx, -0.5, npx-0.5, npy, -0.5, npy-0.5);

  Double_t ens[3]={0,8,17.5}, eens[3]={0.,0.1,0.1};
  Double_t peaks[3], epeaks[3];



  int ibin;
  TH1D *px;


  energyCalibration *enCal=new energyCalibration();
  enCal->setPlotFlag(0);
  // enCal->setChargeSharing(0);
  enCal->setScanSign(1);

  Double_t gain, off, egain, eoff;


  TList *functions;
  TPolyMarker *pm ;
  Double_t *peakX, *peakY;
  TSpectrum *s=new TSpectrum();
  Double_t mypar[10], emypar[10];
  Double_t prms, np;
  int iit=0;
  TGraph *glin;
  Double_t peakdum, hpeakdum;

  for (int ix=1; ix<npx-1; ix++) {
    for (int iy=1; iy<npy-1; iy++) {
      // cout << ix << " " << iy << " " << ibin << endl;
      ibin=ix*npy+iy;
      px=h2->ProjectionX("px",ibin+1,ibin+1);
      prms=10*g;
      iit=0;
      np=s->Search(px,prms,"",0.2);
      while (np !=2) {
	if (np>2)
	  prms+=0.5*prms;
	else
	  prms-=0.5*prms;
	iit++;
	if (iit>=10)
	  break;
	np=s->Search(px,prms,"",0.2);
      }
      if (np!=2)
	cout << "peak search could not converge " << ibin << endl;
      if (np==2) {
	pm=NULL;
	functions=px->GetListOfFunctions();
	if (functions)
	  pm = (TPolyMarker*)functions->FindObject("TPolyMarker");
	if (pm) {
	  peakX=pm->GetX();
	  peakY=pm->GetY();


	  if (peakX[0]>peakX[1]) {
	    peakdum=peakX[0];
	    hpeakdum=peakY[0];
	    peakX[0]= peakX[1];
	    peakY[0]= peakY[1];
	    peakX[1]= peakdum;
	    peakY[1]= hpeakdum;
	    
	  }

	  cout << "("<< ix << "," << iy << ") "  << endl;
	  for (int ip=0; ip<np; ip++) {
	    //  cout << ip << " " << peakX[ip] << " " << peakY[ip] << endl;
	    
	    enCal->setFitRange(peakX[ip]-10*g,peakX[ip]+10*g);
	    mypar[0]=0;
	    mypar[1]=0;
	    mypar[2]=peakX[ip];
	    mypar[3]=g*10;
	    mypar[4]=peakY[ip];
	    mypar[5]=0;
	    


	    enCal->setStartParameters(mypar);
	    enCal->fitSpectrum(px,mypar,emypar);

	    
	    peaks[ip+1]=mypar[2];
	    epeaks[ip+1]=emypar[2];
	  }
	  
	  peaks[0]=0;
	  epeaks[0]=1;

	  //	  for (int i=0; i<3; i++) cout << i << " " << ens[i] << " " << eens[i]<< " "<< peaks[i]<< " " << epeaks[i] << endl;

	  glin= enCal->linearCalibration(3,ens,eens,peaks,epeaks,gain,off,egain,eoff);

	  //  cout << "Gain " << gain << " off " << off << endl;
	  if (off>-10 && off<10) {
	    gMap->SetBinContent(ix+1,iy+1,gain);
	    gMap->SetBinError(ix+1,iy+1,egain);
	  }
	  if (glin)
	    delete glin;
	}
      }


    }
  }

  return gMap;
}


TH2F *noiseMap(TH2F *h2) {
  //  int npx, npy;
  int npx=160, npy=160;
  // det->getDetectorSize(npx, npy);

  TH2F *nMap=new TH2F("nmap",h2->GetTitle(), npx, -0.5, npx-0.5, npy, -0.5, npy-0.5);

  int ibin;
  TH1D *px;

  for (int ix=0; ix<npx; ix++) {
    for (int iy=0; iy<npy; iy++) {
      cout << ix << " " << iy << " " << ibin << endl;
      ibin=ix*npy+iy;
      px=h2->ProjectionX("px",ibin+1,ibin+1);
      px->Fit("gaus", FOPT);
      if (px->GetFunction("gaus")) {
	nMap->SetBinContent(ix+1,iy+1,px->GetFunction("gaus")->GetParameter(2));
      }
      // delete px;
    }
  }

  return nMap;
}


THStack *noiseHistos(char *tit) {
  char fname[10000];

  sprintf(fname,"/data/moench_xbox_20140116/noise_map_%s.root",tit);
  TFile *fn=new TFile(fname);
  TH2F *nmap=(TH2F*)fn->Get("nmap");

  if (nmap==NULL) {
    cout << "No noise map in file " << fname << endl;

    return NULL;
  }

  sprintf(fname,"/data/moench_xbox_20140113/gain_map_%s.root",tit);
  TFile *fg=new TFile(fname);
  TH2F *gmap=(TH2F*)fg->Get("gmap");

  if (gmap==NULL) {
    cout << "No gain map in file " << fname << endl;

    return NULL;
  }

  nmap->Divide(gmap);
  nmap->Scale(1000./3.6);

  THStack *hs=new THStack(tit,tit);
  hs->SetTitle(tit);

  TH1F *h;
  char hname[100];

    cout << tit << endl;
  for (int is=0; is<4; is++) {
    sprintf(hname,"h%ds",is+1);

    h=new TH1F(hname,tit,500,0,500);
    hs->Add(h);
    //  cout << hs->GetHists()->GetEntries() << endl;
    for (int ix=40*is+2; ix<40*(is+1)-2; ix++) {

      for (int iy=2; iy<158; iy++) {
	if (ix<100 || ix>120)
	  h->Fill(nmap->GetBinContent(ix+1,iy+1));
      }
    } 
    cout << is+1 << "SC: " <<  "" << h->GetMean() << "+-" << h->GetRMS();
    h->Fit("gaus","0Q");
    h->SetLineColor(is+1);
    if (h->GetFunction("gaus")) {
      h->GetFunction("gaus")->SetLineColor(is+1);
      cout << " or " << h->GetFunction("gaus")->GetParameter(1) << "+-" << h->GetFunction("gaus")->GetParError(1);
    }
    cout << endl;
  }

  //  cout << hs->GetHists()->GetEntries() << endl;

  return hs;


}
