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

THStack *gainMap(TH2F *h2, float g) {
  //  int npx, npy;
  int npx=160, npy=160;
  // det->getDetectorSize(npx, npy);

  THStack *hs=new THStack();

  TH2F *gMap=new TH2F("gmap",h2->GetTitle(), npx, -0.5, npx-0.5, npy, -0.5, npy-0.5);
  TH2F *nMap=new TH2F("nmap",h2->GetTitle(), npx, -0.5, npx-0.5, npy, -0.5, npy-0.5);

  hs->Add(gMap);
  hs->Add(nMap);

  Double_t ens[1]={20.}, eens[1]={20.};
  Double_t peaks[1], epeaks[1];

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

  int ix=20;
  int iy=40;


  for ( ix=1; ix<npx-1; ix++) {
   for ( iy=1; iy<npy-1; iy++) {
      // cout << ix << " " << iy << " " << ibin << endl;
      ibin=ix*npy+iy;
      px=h2->ProjectionX("px",ibin+1,ibin+1);
      enCal->setFitRange(50,3000);
      //enCal->setChargeSharing(0);
      if (px) {
      enCal->fixParameter(0,0);
      enCal->fixParameter(1,0);
      enCal->fixParameter(5,0);
      mypar[0]=0;
      mypar[1]=0;
      mypar[2]=px->GetBinCenter(px->GetMaximumBin());
      mypar[3]=10;
      mypar[4]=px->GetMaximum();
      mypar[5]=0;

      enCal->setStartParameters(mypar);
	enCal->fitSpectrum(px,mypar,emypar);

      cout << ix << " " << iy << " " << mypar[2] << endl;
      }

      if (mypar[2]>0) {
	gMap->SetBinContent(ix+1,iy+1,mypar[2]/ens[0]);
	gMap->SetBinError(ix+1,iy+1,emypar[2]/ens[0]);
	nMap->SetBinContent(ix+1,iy+1,mypar[3]);
	nMap->SetBinError(ix+1,iy+1,emypar[3]);
      }
   }
  }

  return hs;
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
      ibin=h2->GetYaxis()->FindBin(ix*npy+iy);
      px=h2->ProjectionX("px",ibin,ibin);
      px->Fit("gaus", FOPT,"",-100,100);
      if (px->GetFunction("gaus")) {
	if (px->GetFunction("gaus")->GetParameter(1)>-5 && px->GetFunction("gaus")->GetParameter(1)<5)
	  nMap->SetBinContent(ix+1,iy+1,px->GetFunction("gaus")->GetParameter(2));
      }
      // delete px;
    }
  }
 

  return nMap;
}

THStack *noiseHistos(TH2F *nmap, TH2F *gmap=NULL) {

  
  char tit[1000];


  if (nmap==NULL) {
    cout << "No noise map" << endl;

    return NULL;
  }

  if (gmap) {
    nmap->Divide(gmap);
    nmap->Scale(1000./3.6);
  }


  strcpy(tit,nmap->GetTitle());
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
      cout << " or " << h->GetFunction("gaus")->GetParameter(1) << "+-" << h->GetFunction("gaus")->GetParameter(2);
    }
    cout << endl;
  }

  //  cout << hs->GetHists()->GetEntries() << endl;

  return hs;


}
