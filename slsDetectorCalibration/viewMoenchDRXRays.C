
#define MYROOT1

#include <TH1D.h>
#include <TF1.h>
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
#include <fstream>
#include "moench03CtbData.h"
//#include "moench03CommonMode.h"
//#include "singlePhotonDetector.h"


using namespace std;


THStack *viewMoenchDRXRays(int ix=70, int iy=88){

  TF1 *poiss = new TF1("poiss", "[0]*TMath::Power(([1]/[2]),(x/[2]))*(TMath::Exp( ([1]/[2])))/TMath::Gamma((x/[2])+1)", 0, 5);


  int thick[]={1700,1500,1300,1100,900,700};
  int nt=6;
  int nf=5;
  char fname[1000];
  char *data;
  char tit[100];
  TH1F *hh[nt], *hh3[nt], *hh5[nt];
  TH2F *h2[nt], *hpix[nt];
  Double_t val, val3, val5, val1;
  int it=0;
  Double_t ped[25];
  TH1D *p;

  THStack *hs=new THStack();
  cout << nt << endl;
  ifstream filebin;
  moench03CtbData *decoder=new moench03CtbData();

  sprintf(tit,"hpix_%dumSi_g1",thick[it]);
  cout << tit << endl;


  hpix[it]=new TH2F(tit,tit,2500,6000,16000,25,0,25);
    hs->Add(hpix[it]);
  
    sprintf(tit,"%dumSi_g1",thick[it]);
    cout << tit << endl;

    for (int iff=0; iff<nf; iff++) {
      // cout << tit << " " << iff << endl;
      sprintf(fname,"/mnt/moench/Moench03_MS_20150606/direct_beam_12.4keV_filter_scan/direct_beam_12.4keV_%s_400clk_f0_%d.raw",tit,iff);
      
      filebin.open((const char *)(fname), ios::in | ios::binary);
      if (filebin.is_open()) cout << "ok "<< fname << endl;
      else  cout << "could not open "<< fname << endl;
      
      while ((data=decoder->readNextFrame(filebin))) {
	
	for (int iiy=-2; iiy<3; iiy++)
	  for (int iix=-2; iix<3; iix++)
	    hpix[it]->Fill(decoder->getChannel(data,ix+iix,iy+iiy), (iiy+2)*5+iix+2);
	
	delete [] data;
      }
      filebin.close();
      cout << endl;
      
      
      
    }
   

	for (int iix=-2; iix<3; iix++) {
	for (int iiy=-2; iiy<3; iiy++) {
	  cout << iix << " " << iiy << " " ;// <<endl;
	  p=hpix[0]->ProjectionX("p",(iiy+2)*5+iix+2+1,(2+iiy)*5+iix+2+1);

	  ped[(iiy+2)*5+iix+2]=p->GetBinCenter(p->GetMaximumBin());

	   cout << ped[(iiy+2)*5+iix+2] <<endl; 
	}
	}



    for (it=0; it<nt; it++) {

    sprintf(tit,"hh_%dumSi_g1",thick[it]);
    cout << tit << endl;
    hh[it]=new TH1F(tit,tit,5000,0,10000);
    hs->Add(hh[it]);
    sprintf(tit,"hh3_%dumSi_g1",thick[it]);
    hh3[it]=new TH1F(tit,tit,5000,0,30000);
    hs->Add(hh3[it]);

    sprintf(tit,"hh5_%dumSi_g1",thick[it]);
    hh5[it]=new TH1F(tit,tit,5000,0,50000);
    hs->Add(hh5[it]);

    sprintf(tit,"%dumSi_g1",thick[it]);
    cout << tit << endl;
    hs->Add(hh[it]);
    for (int iff=0; iff<nf; iff++) {
      // cout << tit << " " << iff << endl;
      sprintf(fname,"/mnt/moench/Moench03_MS_20150606/direct_beam_12.4keV_filter_scan/direct_beam_12.4keV_%s_400clk_f0_%d.raw",tit,iff);
      
      filebin.open((const char *)(fname), ios::in | ios::binary);
      if (filebin.is_open()) cout << "ok "<< fname << endl;
      else  cout << "could not open "<< fname << endl;

      while ((data=decoder->readNextFrame(filebin))) {
	cout << "-" ;
	//	for (int iy=0; iy<40; iy++)
	//  for (int ix=0; ix<350; ix++){


	    

	val1=0;
	val3=0;
	val5=0;
	

	for (int iix=-2; iix<3; iix++) {
	for (int iiy=-2; iiy<3; iiy++) {
	  // cout << iix << " " << iiy << " " ;// <<endl;
	  // p=hpix[]->ProjectionX("p",(iiy+2)*5+iix+2+1,(2+iiy)*5+iix+2+1);

	  // ped[it]=p->GetBinCenter(p->GetMaximumBin());

	  // cout << ped[it] <<endl;
	  val=decoder->getChannel(data,ix+iix,iy+iiy)-ped[(iiy+2)*5+iix+2];
	    if ((iix<-1 || iix>1) || (iiy<-1 || iiy>1))
	      val5+=val;
	    else if (iix!=0 || iiy!=0) {
	      val5+=val;
	      val3+=val;
	    } else {
	      val5+=val;
	      val3+=val;
	      val1+=val;
	    }
	    // if (iiy==1 && iix==0)
	    //   h2[it]->Fill(val1,val);
	      
	}
	}
	      hh5[it]->Fill(val5);
	      hh3[it]->Fill(val3);
	      hh[it]->Fill(val1);

      

	delete [] data;
      }
      filebin.close();
      cout << endl;

    }


    }
  


  return hs;

}
