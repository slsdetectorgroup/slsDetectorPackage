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
#include <TF1.h>
#include <stdio.h>
//#include <deque>
//#include <list>
//#include <queue>
#include <map>
#include <fstream>
#include <sys/stat.h>
#include "moench03CtbData.h"
#include "moench03CommonMode.h"
#define MYROOT1
#include "singlePhotonDetector.h"

//#include "MovingStat.h"

using namespace std;

#define NC 400
#define NR 400


#define MY_DEBUG 1

#ifdef MY_DEBUG
#include <TCanvas.h>
#endif



TH2F *readImage(ifstream &filebin,   TH2F *h2=NULL, TH2F *hped=NULL) {
  moench03CtbData *decoder=new moench03CtbData();
  char *buff=decoder->readNextFrame(filebin);
  

//   TH1F *h1=new TH1F("h1","",400*400,0,400*400);
//   int ip=0;
  if (buff) {
    if (h2==NULL) {
      h2=new TH2F("h2","",400,0,400,400,0,400);
      h2->SetStats(kFALSE);
    }
    cout << "." << endl;
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
      cout << "="<< endl;
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
  moench03CtbData *decoder=new moench03CtbData();
  singlePhotonDetector<uint16_t> *filter=new singlePhotonDetector<uint16_t>(decoder, 3, 5, 1, NULL);
  char *buff;
  int ix,iy;
  int ii=0;
  TH2F* h2=NULL;

  for (int irun=runmin; irun<=runmax; irun++) {
    sprintf(fname,fformat,irun);
    

    filebin.open((const char *)(fname), ios::in | ios::binary);
    while ((buff=decoder->readNextFrame(filebin))) {
      for (ix=0; ix<400; ix++) {
	for (iy=0; iy<400; iy++) {
	  filter->addToPedestal(decoder->getValue(buff,ix,iy), ix, iy);
	}
      }
      delete [] buff;
      cout << "="<< endl;
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
/******************************************************************************/
TH2F * calcNoiseRMS(char *fformat, int runmin, int runmax){
  ifstream filebin;
  char fname[10000];
  moench03CtbData *decoder=new moench03CtbData();
  singlePhotonDetector<uint16_t> *filter=new singlePhotonDetector<uint16_t>(decoder, 3, 5, 1, NULL);
  char *buff;
  int ix,iy;
  int ii=0;
  TH2F* h2=NULL;

  for (int irun=runmin; irun<=runmax; irun++) {
    sprintf(fname,fformat,irun);
    

    filebin.open((const char *)(fname), ios::in | ios::binary);
    while ((buff=decoder->readNextFrame(filebin))) {
      for (ix=0; ix<400; ix++) {
	for (iy=0; iy<400; iy++) {
	  filter->addToPedestal(decoder->getValue(buff,ix,iy), ix, iy);
	}
      }
      delete [] buff;
      cout << "="<< endl;
      ii++;
    }
    if (filebin.is_open())
      filebin.close(); 
    
  }
  if (ii>0) {
    h2=new TH2F("hped","",400,0,400,400,0,400);
    
      for (ix=0; ix<400; ix++) {
	for (iy=0; iy<400; iy++) {
	  h2->SetBinContent(ix+1, iy+1,filter->getPedestalRMS(ix,iy));
	}
      }

  }
  return h2;

}

/**********************************************************************************/
THStack * calcNoise(char *fformat,std::string flag, int runmin, int runmax, int nfiles){
  std::map<int,TH1F *> pixelMap;
  cerr<<"Creating pedestal map for frames: "<<flag<<endl;
  cerr<<"/***************************************/"<<endl;


  THStack * hs = new THStack();

  int n(2);
  int r(0);

  if(flag=="even"){
    n=2;
    r=0;
  }else if(flag=="odd"){
    n=2;
    r=1;
  }else if(flag=="all"){
    n=1;
    r=0;
  }

  ifstream filebin;
  char fname[10000];
  moench03CtbData *decoder=new moench03CtbData();
  singlePhotonDetector<uint16_t> *filter=new singlePhotonDetector<uint16_t>(decoder, 3, 5, 1, NULL);
  char *buff;
  int ix,iy;
  int ii=0;
  TH2F* hped=NULL;
  TH2F* hnoise=NULL;
  int ibin(0);
  int iframe(0);
  Float_t sigma(0.);
  int filefound(0);
  pixelMap.clear();
  for (int irun=runmin; irun<=runmax; irun++) {
    sprintf(fname,fformat,irun);
    if( access(fname,F_OK) != -1){
      if(filefound<nfiles){
	filebin.open((const char *)(fname), ios::in | ios::binary);
	while ((buff=decoder->readNextFrame(filebin))) {
	  iframe=decoder->getFrameNumber(buff);
	  if(iframe%n==r){
	    for (ix=0; ix<400; ix++) {
	      for (iy=0; iy<400; iy++) {
		ibin = ix*NR+iy;
		if(pixelMap.find(ibin)!=pixelMap.end()){
		  pixelMap[ibin]->Fill(decoder->getValue(buff,ix,iy));
		}else{
		  pixelMap[ibin] = new TH1F(Form("h_%d",ibin),Form("h_%d",ibin),8000,-0.5,15999.5);
		}
	      }
	    }
	  }
	  delete [] buff;
	  if(ii%1000==0)cout << "="<<ii<< endl;
	  ii++;
	}
	if (filebin.is_open())
	  filebin.close(); 
	filefound++;
      }else{
	break;
      }
    }else{//end chek if file exists
      cerr<<fname<<" does not exist!"<<endl;
    }
    
  }
  if(ii>0){

    hped=new TH2F("hped","",400,-0.5,399.5,400,-0.5,399.5);
    hnoise=new TH2F("hnoise","",400,-0.5,399.5,400,-0.5,399.5);
    hs->Add(hped);
    hs->Add(hnoise);
    Float_t p(0.);
    Float_t perr(0.);
    Float_t amp(0.);
    Float_t sigmaerr(0.);
    for(ix=0; ix<400; ix++){
      for (iy=0; iy<400; iy++){
	sigma=0.;
	sigmaerr=0.;
	p=0.;
	perr=0.;
	ibin = ix*NR+iy;
	if(ibin%1000==0)cerr<<ibin<<endl;
	if(pixelMap.find(ibin)!=pixelMap.end()){
	  // cerr<<"/****************/"<<endl;
	  // cerr<<"("<<ix<<","<<iy<<")"<<endl;
	  p=pixelMap[ibin]->GetBinCenter(pixelMap[ibin]->GetMaximumBin());
	  // cerr<<"peak: "<<p<<endl;
	  amp=pixelMap[ibin]->GetBinContent(pixelMap[ibin]->GetMaximumBin());
	  sigma=pixelMap[ibin]->GetRMS();

	  TF1 * g = new TF1("g","gaus(0)",p-sigma,p+sigma);
	  g->SetParameters(amp,p,sigma);
	  g->SetParLimits(1,p-sigma,p+sigma);
	  g->SetParLimits(2,0.1*sigma,2.*sigma);

	  pixelMap[ibin]->Fit(g,"RQN0");
	  p=g->GetParameter(1);
	  perr=g->GetParError(1);
	  sigma=g->GetParameter(2);
	  sigmaerr=g->GetParError(2);
	  // cerr<<"sigma: "<<sigma<<endl;
	  delete g;
	  delete pixelMap[ibin];

	}
	hped->SetBinContent(ix+1,iy+1,p);
	hped->SetBinError(ix+1,iy+1,perr);
	hnoise->SetBinContent(ix+1,iy+1,sigma);
	hnoise->SetBinError(ix+1,iy+1,sigmaerr);

      }
    }

  }

  return hs;

}
/****************************************************/
THStack * DrawFrames(char *fformat,int framemin, int framemax){

  int nbins(400);
  int xmin(0);
  int xmax(400);

  THStack * hs = new THStack();
  ifstream filebin;
  char fname[10000];
  moench03CtbData *decoder=new moench03CtbData();
  char *buff;
  int ix,iy;
  int iframe(0);
  if(access(fformat,F_OK) != -1){
    filebin.open((const char *)(fformat), ios::in | ios::binary);
    while ((buff=decoder->readNextFrame(filebin))) {
      iframe=decoder->getFrameNumber(buff);
      if(iframe>=framemin && iframe<framemax){//check on frames
	ix=0;
	iy=0;
	TH2F * h = new TH2F(Form("h_%d",iframe),Form("h_%d",iframe),nbins,xmin,xmax,nbins,xmin,xmax);
	for(ix=xmin; ix<xmax; ix++){//loop on pixels
	  for(iy=xmin; iy<xmax; iy++){
	    h->SetBinContent(ix+1,iy+1,decoder->getValue(buff,ix,iy));
	  }
	}//end loop on pixels
	hs->Add(h);
      }//end check on frames
      delete [] buff;
    }
  }else{//check if file exists
    cerr<<fformat<<" does not exists!"<<endl;
  }

  return hs;
}
/****************************************************/
TH1F * SinglePixelHisto(char *fformat,int nbins, float xmin, float xmax, int runmin, int runmax, int x0, int y0){
  moench03CtbData *decoder=new moench03CtbData();
  cout << "decoder allocated " << endl;

  TH1F * _hout = new TH1F("hpix","",nbins,xmin,xmax);

  char *buff;
  char fname[10000];
  ifstream filebin;
  int iframe(0);
  for (int irun=runmin; irun<=runmax; irun++){//loop on files
    sprintf(fname,fformat,irun);
    iframe=0;
    if( access(fname,F_OK) != -1){//check if file exists
      filebin.open((const char *)(fname), ios::in | ios::binary);
      while ((buff=decoder->readNextFrame(filebin))){
	iframe=decoder->getFrameNumber(buff);
	if(iframe%1000==0) cerr<<iframe<<" frames"<<endl;
	_hout->Fill(decoder->getValue(buff,x0,y0));
	delete [] buff;

      }//end readout of frame
    }else{//end check on file
      cerr<<fname<<" does not exists!"<<endl;
    }
    if(filebin.is_open()) filebin.close();
  }//end loop on files

  return _hout;


}
/****************************************************/
void DrawAllPixelCorrelation(char *fformat, std::string frame,int runmin, int runmax, int xmin, int xmax, int ymin, int ymax, int sc_width,TH2F* hcorr){
  int cmsub(0);
  moench03CtbData *decoder=new moench03CtbData();
  cout << "decoder allocated " << endl;

  moench03CommonMode *cmSub=NULL;
  if (cmsub) {
    cmSub=new moench03CommonMode(100);
    cout << "common mode allocated " << endl;

  } else {
     
    cout << "non allocating common mode  " << endl;
  }

  int n(2);
  int r(0);
  if(frame=="even"){
    n=2;
    r=0;
  }else if(frame=="odd"){
    n=2;
    r=1;
  }else if(frame=="all"){
    n=1;
    r=0;
  }
  int sign(1);
  singlePhotonDetector<uint16_t> *filter=new singlePhotonDetector<uint16_t>(decoder, 3, 5, sign, cmSub, 1000, 100);
  char *buff;
  char fname[10000];

  float pix1(0.);
  float pix2(0.);
  ifstream filebin;
  int iframe(0);

  for (int irun=runmin; irun<=runmax; irun++){//loop on files
    sprintf(fname,fformat,irun);
    iframe=0;
    pix1=0.;
    pix2=0.;
    if( access(fname,F_OK) != -1){//check if file exists
      filebin.open((const char *)(fname), ios::in | ios::binary);
      while ((buff=decoder->readNextFrame(filebin))){//loop on frames
	iframe=decoder->getFrameNumber(buff);
	if(iframe%n==r){//check if frame is even or odd
	  for(int ix=xmin; ix<xmax; ix++){//loop on pixels
	    for(int iy=ymin; iy<ymax; iy++){

	    
	      if((ix+1)%sc_width != 0){//check that pixel is not at the edge of supercolumn

		pix1=decoder->getValue(buff,ix,iy);
		pix2=decoder->getValue(buff,ix+1,iy);

		hcorr->Fill(pix1,pix2);
	      }//end check on s_c edge

	    }
	  }//end loop on pixels
	}//edn check on frame number
	delete [] buff;

      }//end loop on frames

    }else{//end check if file exists
      cerr<<fname<<" does not exist!"<<endl;
    }
    if(filebin.is_open())filebin.close();
  }//end loop on files
  

  return;


}
/***************************************************/
void DrawPixelCorrelation(char *fformat, std::string frame,int runmin, int runmax, int px, int py,TH1F * h1,TH1F * h2, TH2F * hcorr){
  int cmsub(0);
  moench03CtbData *decoder=new moench03CtbData();
  cout << "decoder allocated " << endl;

  moench03CommonMode *cmSub=NULL;
  if (cmsub) {
    cmSub=new moench03CommonMode(100);
    cout << "common mode allocated " << endl;

  } else {
     
    cout << "non allocating common mode  " << endl;
  }

  int n(2);
  int r(0);
  if(frame=="even"){
    n=2;
    r=0;
  }else if(frame=="odd"){
    n=2;
    r=1;
  }else if(frame=="all"){
    n=1;
    r=0;
  }


  int sign(1);
  singlePhotonDetector<uint16_t> *filter=new singlePhotonDetector<uint16_t>(decoder, 3, 5, sign, cmSub, 1000, 100);
  char *buff;
  char fname[10000];

  float pix1(0.);
  float pix2(0.);
  ifstream filebin;
  int ii=0;
  int iframe(0);

  for (int irun=runmin; irun<=runmax; irun++){//loop on files
    sprintf(fname,fformat,irun);
    pix1=0.;
    pix2=0.;
    iframe=0;
    if( access(fname,F_OK) != -1){//check if file exists

      filebin.open((const char *)(fname), ios::in | ios::binary);
      while ((buff=decoder->readNextFrame(filebin))){//loop on frames
	iframe=decoder->getFrameNumber(buff);
	if(iframe%n==r){
	

	  pix1=decoder->getValue(buff,px,py);
	  pix2=decoder->getValue(buff,px+1,py);

	  h1->Fill(pix1);
	  h2->Fill(pix2);
	  hcorr->Fill(pix1,pix2);
	  delete [] buff;

	}
	if(ii%1000==0)cerr<<"reading frame "<<ii<<endl;
	ii++;
      }//end loop on frames
      if(filebin.is_open())filebin.close();

    }else{//end check on file
      cerr<<fname<<" does not exists!"<<endl;
    }
    if(filebin.is_open())filebin.close();
  }//end loop on files

  return;

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
  \param sign sign of the spectrum to find hits
  \param hc readout correlation coefficient with previous pixel
  \param xmin minimum x coordinate
  \param xmax maximum x coordinate
  \param ymin minimum y coordinate
  \param ymax maximum y coordinate
  \param cmsub  enable commonmode subtraction
  \returns pointer to histo stack with cluster spectra
*/

THStack *moench03ReadData(char *fformat, char *tit, int runmin, int runmax, int nbins=1500, int hmin=-500, int hmax=1000, int xmin=1, int xmax=NC-1, int ymin=1, int ymax=NR-1, int cmsub=0, int hitfinder=1) {
  
  double hc(0);
  int sign(1);
  moench03CtbData *decoder=new moench03CtbData();
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

  singlePhotonDetector<uint16_t> *filter=new singlePhotonDetector<uint16_t>(decoder, 3, 5, sign, cmSub, 1000, 100);
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

  // TH2F *h2=NULL;
  // TH2F *h3=NULL;
  if (hitfinder) {
    // h2=new TH2F("h2",tit,nbins,hmin-0.5,hmax-0.5,NC*NR,-0.5,NC*NR-0.5);
    // cout << "h2 allocated " << endl;
    // h3=new TH2F("h3",tit,nbins,hmin-0.5,hmax-0.5,NC*NR,-0.5,NC*NR-0.5);
    // cout << "h3 allocated " << endl;
    // // hetaX=new TH2F("hetaX",tit,nbins,-1,2,NC*NR,-0.5,NC*NR-0.5);
    // //  hetaY=new TH2F("hetaY",tit,nbins,-1,2,NC*NR,-0.5,NC*NR-0.5);
    // hs->Add(h2);
    // hs->Add(h3);
    // hs->Add(hetaX);
    // hs->Add(hetaY);
  }
  if (hs->GetHists()) {
    for (int i=0; i<1; i++)
      if (hs->GetHists()->At(i)) cout << i << " " ; 
    cout << " histos allocated " << endl;
  } else
    cout << "no hists in stack " << endl;
  
  
  


  int ix=20, iy=20, ir, ic;
  int adc(0);

  ifstream filebin;
  Int_t iFrame;
  TTree *tall;

  if (hitfinder){
    tall=filter->initEventTree(tit, &iFrame);

   }
  

#ifdef MY_DEBUG

  cout << "debug mode " << endl;
  
  TCanvas *myC;
  TH2F *he;
  TCanvas *cH1;
  // TCanvas *cH2;
  // TCanvas *cH3;
  cH1=new TCanvas("ch1");
  cH1->SetLogz();
  h1->Draw("colz");
  if (hitfinder) {
    myC=new TCanvas("myc");
    he=new TH2F("he","Event Mask",xmax-xmin, xmin, xmax, ymax-ymin, ymin, ymax);
    he->SetStats(kFALSE);
    he->Draw("colz");

    // cH2=new TCanvas("ch2");
    // cH2->SetLogz();
    // h2->Draw("colz");
    // cH3=new TCanvas("ch3");
    // cH3->SetLogz();
    // h3->Draw("colz");
  }
#endif

  filter->newDataSet();


  for (int irun=runmin; irun<runmax; irun++) {
    sprintf(fname,fformat,irun);
    cout << "file name " << fname << endl;

    if( access(fname,F_OK) != -1){//check if file exists
      filebin.open((const char *)(fname), ios::in | ios::binary);
      nph=0;
      while ( (buff=decoder->readNextFrame(filebin))) {//getting frames on the buffer
      

	if (hitfinder) {
	  filter->newFrame();

	  //calculate pedestals and common modes
	  if (cmsub) {
	    //	cout << "cm" << endl;
	    for (ix=xmin; ix<xmax+1; ix++)
	      for (iy=ymin; iy<ymax+1; iy++) {
		thisEvent=filter->getEventType(buff, ix, iy,0);	      
	      }//loop on pixels
	  }//end if cmsub
	}//end if hitfinder

	//   cout << "new frame " << endl;

	for (ix=xmin; ix<xmax+1; ix++)
	  for (iy=ymin; iy<ymax+1; iy++) {
	    //	  cout << ix << " " << iy << endl;
	    thisEvent=filter->getEventType(buff, ix, iy, cmsub);
	    iFrame=decoder->getFrameNumber(buff);

#ifdef MY_DEBUG
	    if (hitfinder) {
	      //  if (iev%10==0)
	      he->SetBinContent(ix+1-xmin, iy+1-ymin, (int)thisEvent);
	    }
#endif	  
	  
	    //  if (nf>10) {
	    h1->Fill(filter->getClusterTotal(1), iy+NR*ix);
	    if (hitfinder) {
	      
	      if (thisEvent==PHOTON_MAX ) {
		nph++;

		// h2->Fill(filter->getClusterTotal(2), iy+NR*ix);
		// h3->Fill(filter->getClusterTotal(3), iy+NR*ix);

		tall->Fill();
	      }//end if PHOTON_MAX
	    }//end if hitfinder
	  }//end loop on pixels
	//////////////////////////////////////////////////////////
    
#ifdef MY_DEBUG
	//  cout << iev << " " << h1->GetEntries() << " " << h2->GetEntries() << endl;
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
      
	cout << "=" ;
	delete [] buff;
      }//end frame
      cout << nph <<" Photons found"<< endl;
      if (filebin.is_open())
	filebin.close();	  
      else
	cout << "could not open file " << fname << endl;
    }else{
      cerr<<"File does not exist"<<endl;
    }//end check on file path



  }//end loop on files

  if (hitfinder){
    tall->Write(tall->GetName(),TObject::kOverwrite);
  }
   //////////////////////////////////////////////////////////
    
#ifdef MY_DEBUG 
  if(hitfinder){
    myC->Modified();
    myC->Update();
  }
  cH1->Modified();
  cH1->Update(); 
  // cH2->Modified();
  // cH2->Update();
  // cH3->Modified();
  // cH3->Update();
#endif	
  
  delete decoder;
  cout << "Read " << nf << " frames" << endl;
  return hs;
}
/*********************************************************************************************/ 
THStack *moench03ReadDataEvenOdd(char *fformat, char *tit, int runmin, int runmax, int nbins=1500, int hmin=-500, int hmax=1000, int xmin=1, int xmax=NC-1, int ymin=1, int ymax=NR-1, int cmsub=0, int hitfinder=1){
  
  double hc(0);
  int sign(1);
  moench03CtbData *decoder=new moench03CtbData();
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

  singlePhotonDetector<uint16_t> *filterEven=new singlePhotonDetector<uint16_t>(decoder, 3, 5, sign, cmSub, 1000, 100);
  cout << "filter for even frames allocated " << endl;
  singlePhotonDetector<uint16_t> *filterOdd=new singlePhotonDetector<uint16_t>(decoder, 3, 5, sign, cmSub, 1000, 100);
  cout << "filter for odd frames allocated " << endl;

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

  TH2F *h2=NULL;
  TH2F *h3=NULL;
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
      if (hs->GetHists()->At(i)) cout << i << " " ; 
    cout << " histos allocated " << endl;
  } else
    cout << "no hists in stack " << endl;
  
  
  ifstream filebin;


  int ix=20, iy=20, ir, ic;
 


  Int_t iFrame;
  TTree *tEven;
  TTree *tOdd;
  if (hitfinder){
    tEven=filterEven->initEventTree(Form("%s_Even",tit), &iFrame);
    cout<< "TTree for even frames initialized"<<endl;
    tOdd=filterOdd->initEventTree(Form("%s_Odd",tit), &iFrame);
    cout<< "TTree for odd frames initialized"<<endl;
  }
  

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

    cout<<"Canvas allocated"<<endl;

  }
#endif

  filterEven->newDataSet();
  filterOdd->newDataSet();
 


  for (int irun=runmin; irun<runmax; irun++) {
    sprintf(fname,fformat,irun);
    cout << "file name " << fname << endl;
    filebin.open((const char *)(fname), ios::in | ios::binary);
    nph=0;
    while ((buff=decoder->readNextFrame(filebin))) {
      iFrame=decoder->getFrameNumber(buff);

      if (hitfinder) {
	if(iFrame%2==0){
	  filterEven->newFrame();
	}else{
	  filterOdd->newFrame();
	}

	//calculate pedestals and common modes
	if (cmsub) {
	  //	cout << "cm" << endl;
	  for (ix=xmin-1; ix<xmax+1; ix++)
	    for (iy=ymin-1; iy<ymax+1; iy++) {
	      if(iFrame%2==0){
		thisEvent=filterEven->getEventType(buff, ix, iy,0);
	      }else{
		thisEvent=filterOdd->getEventType(buff, ix, iy,0);
	      }	      
	    }
	  }
	}

      //   cout << "new frame " << endl;

      for (ix=xmin-1; ix<xmax+1; ix++)
	for (iy=ymin-1; iy<ymax+1; iy++) {
	  if(iFrame%2==0){
	    thisEvent=filterEven->getEventType(buff, ix, iy, cmsub);
	  }else{
	    thisEvent=filterOdd->getEventType(buff, ix, iy,cmsub);
	  }




#ifdef MY_DEBUG
	  if (hitfinder) {

	    he->SetBinContent(ix+1-xmin, iy+1-ymin, (int)thisEvent);
	  }
#endif	  
	  
	  if(iFrame%2==0){
	    h1->Fill(filterEven->getClusterTotal(1), iy+NR*ix);
	  }else{
	    h1->Fill(filterOdd->getClusterTotal(1), iy+NR*ix);
	  }
	  if (hitfinder) {
	      
	    if (thisEvent==PHOTON_MAX ) {
	      nph++;
	      if(iFrame%2==0){
		h2->Fill(filterEven->getClusterTotal(2), iy+NR*ix);
		h3->Fill(filterEven->getClusterTotal(3), iy+NR*ix);
		tEven->Fill();
	      }else{
		h2->Fill(filterOdd->getClusterTotal(2), iy+NR*ix);
		h3->Fill(filterOdd->getClusterTotal(3), iy+NR*ix);
		tOdd->Fill();
	      }	      
       
	    }
	  }


	  // }
	}
      //////////////////////////////////////////////////////////
    
#ifdef MY_DEBUG
      //  cout << iev << " " << h1->GetEntries() << " " << h2->GetEntries() << endl;
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
      
      cout << "=" ;
      delete [] buff;
    }
    cout << nph << endl;
    if (filebin.is_open())
      filebin.close();	  
    else
      cout << "could not open file " << fname << endl;
  }

  if (hitfinder){
    tEven->Write(tEven->GetName(),TObject::kOverwrite);
    tOdd->Write(tOdd->GetName(),TObject::kOverwrite);
  }
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

/***********************************************************************************/
Int_t moench03DrawPedestals(char *fformat, char *tit, int runmin, int runmax, int nbins=1500, int hmin=-500, int hmax=1000, int xmin=1, int xmax=NC-1, int ymin=1, int ymax=NR-1, int cmsub=0, int hitfinder=1){

  double hc(0);
  int sign(1);
  moench03CtbData *decoder=new moench03CtbData();
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

  singlePhotonDetector<uint16_t> *filter=new singlePhotonDetector<uint16_t>(decoder, 3, 5, sign, cmSub, 1000, 100);
  cout << "filter allocated " << endl;

  char *buff;
  char fname[10000];
  int nf=0;
  
  eventType thisEvent=PEDESTAL;
  int iFrame(0);

  // int iframe;
  // double *data, ped, sigma;

  // data=decoder->getCluster();

  int nChx(xmax-xmin+2);
  int nChy(ymax-ymin+2);
  TH1F * HistoMap_All[nChx][nChy];
  TH1F * HistoMap_Even[nChx][nChy];
  TH1F * HistoMap_Odd[nChx][nChy];
  for(Int_t ix=0; ix<nChx; ix++){
    for(Int_t iy=0; iy<nChy; iy++){
      HistoMap_All[ix][iy] = new TH1F(Form("h_%d_%d_All",ix+xmin,iy+ymin),"",8000,0,16000);
      HistoMap_Even[ix][iy] = new TH1F(Form("h_%d_%d_Even",ix+xmin,iy+ymin),"",8000,0,16000);
      HistoMap_Odd[ix][iy] = new TH1F(Form("h_%d_%d_Odd",ix+xmin,iy+ymin),"",8000,0,16000);
    }
  }
   
  filter->newDataSet();
  float adc(0.);
  ifstream filebin;
  for (int irun=runmin; irun<runmax; irun++) {
    sprintf(fname,fformat,irun);
    cout << "file name " << fname << endl;
    filebin.open((const char *)(fname), ios::in | ios::binary);
    nph=0;
    while ((buff=decoder->readNextFrame(filebin))) {
      iFrame=decoder->getFrameNumber(buff);
      filter->newFrame();

      for(Int_t ix=0; ix<nChx; ix++){//loop on pixels
	for(Int_t iy=0; iy<nChy; iy++){

	  thisEvent=filter->getEventType(buff, ix+xmin, iy+ymin,cmsub);
	  adc=decoder->getValue(buff,ix+xmin,iy+ymin);
	  HistoMap_All[ix][iy]->Fill(adc);
	  if(iFrame%2==0){
	    HistoMap_Even[ix][iy]->Fill(adc);
	  }else{
	    HistoMap_Odd[ix][iy]->Fill(adc);
	  }
	}
      }//end loop on pixels

    }
    if (filebin.is_open())
      filebin.close();	  
    else
      cout << "could not open file " << fname << endl;



  }//end loop on files

  for(Int_t ix=0; ix<nChx; ix++){
    for(Int_t iy=0; iy<nChy; iy++){
      HistoMap_All[ix][iy]->Write();
      HistoMap_Even[ix][iy]->Write();
      HistoMap_Odd[ix][iy]->Write();
    }
  }

  delete decoder;
  return 1;

}
