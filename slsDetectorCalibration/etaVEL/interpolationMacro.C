#include <TH1D.h>
#include <TH2D.h>
#include <TPad.h>
#include <TDirectory.h>
#include <TEntryList.h>
#include <TFile.h>
#include <TMath.h>
#include <TTree.h>
#include <TChain.h>
#include <TStyle.h>
#include <THStack.h>
#include <TCanvas.h>
#include <TROOT.h>
#include <TMultiGraph.h>
#include <TF1.h>
#include <TLegend.h>
#include <stdio.h>
#include <iostream>
#include <deque>
#include <list>
#include <queue>
#include <fstream>
#include <TPaletteAxis.h>
//#include "interpolation_EtaVEL.h"
#include "interpolation_etaVEL.cpp"
#include "../energyCalibration.cpp"

/*
Zum erstellen der correction map ist createGainAndEtaFile(...)  in EVELAlg.C der entry point.
Zum erstellen des HR images ist createImage(...) der entry point.
*/
#define YMIN 100
#define XMIN 0
#define NX 400
#define NY 200

// sets nice plotting settings
int rootlogon ()
{
  gStyle->SetDrawBorder(0);
  gStyle->SetCanvasColor(kWhite);     
  gStyle->SetCanvasDefH(800);
  gStyle->SetCanvasDefW(800);
  gStyle->SetCanvasBorderMode(0);     
  gStyle->SetPadBorderMode(0);
  gStyle->SetPaintTextFormat("5.2f"); 
  gStyle->SetLineWidth(2);
  gStyle->SetTextSize(1.1);
  gStyle->SetLabelSize(0.04,"xy");
  gStyle->SetTitleSize(0.05,"xy");
  gStyle->SetTitleOffset(1.0,"x");
  gStyle->SetTitleOffset(1.6,"y");
  gStyle->SetPadTopMargin(0.05);
  gStyle->SetPadRightMargin(0.05);
  gStyle->SetPadBottomMargin(0.15);
  gStyle->SetPadLeftMargin(0.15);
  gStyle->SetLegendBorderSize(1);
  gStyle->SetFrameBorderMode(0);
  gStyle->SetFrameFillColor(kWhite);
  gStyle->SetTitleFillColor(kWhite);
  gStyle->SetStatFontSize(0.03);
  gStyle->SetStatBorderSize(1);
  gStyle->SetStatFormat("6.4g");
  gStyle->SetStatX(0.95);
  gStyle->SetStatY(0.95);
  gStyle->SetStatW(0.2);
  gStyle->SetStatH(0.2);
  gStyle->SetStatColor(kWhite);
  gStyle->SetTitleX(0.3);
  gStyle->SetTitleY(0.98);
  gStyle->SetTitleBorderSize(1);
  gStyle->SetTitleFontSize(0.06);
  gStyle->SetLegendBorderSize(1);
  gROOT->SetStyle("Default");
  gROOT->ForceStyle();          
  return 0;

}


interpolation_EtaVEL *fillEta(TChain *ch, Long64_t nentries=-1, TH2F *gmap=NULL, THStack *hs=NULL){

  //  if (nentries<0)
  //  nentries=ch->GetEntries();

  //  cout << "Chain has " << nentries << " entries " << endl;



  Long64_t ie;

  Double_t dum[9],   data[9], gain=1;
  Int_t x, y, f, ix, iy;
  interpolation_EtaVEL *inte= new interpolation_EtaVEL(400, 200, 25, -0.05,1.05);
  int skip=0;
  TEntryList  *myelist=ch->GetEntryList();
  Long64_t listEntries = myelist->GetN();
  Double_t rr;
  Long64_t treeEntry, chainEntry;
  Int_t treenum;
  TCanvas *c1=new TCanvas();
  int corn;
  TH1D *h1=NULL, *h2=NULL, *h3=NULL;
  TH2F *heta=NULL, *hetaL=NULL, *heta3=NULL, *heta3X=NULL, *hcorn=NULL ;
  Double_t tot, totquad, sDum[2][2];
  Double_t etax, etay, etaxL, etayL, etax3, etay3,etax3x, etay3x,r ;
  heta=inte->setEta((TH2F*)NULL);

  if (hs) {    
    hetaL=(TH2F*)(heta->Clone("hetaL"));
    heta3=(TH2F*)(heta->Clone("heta3")); 
    heta3X=(TH2F*)(heta->Clone("heta3X")); 
    h1=new TH1D("h1","h1",1000,0,3);
    h2=new TH1D("h2","h2",1000,0,3);
    h3=new TH1D("h3","h3",1000,0,3);
    hcorn=new TH2F("hcorn","hcorn",2,-1,1,2,-1,1);
    hs->Add(heta);
    hs->Add(hetaL);
    hs->Add(heta3);
    hs->Add(heta3X);
    hs->Add(h1);
    hs->Add(h2);
    hs->Add(h3);
    hs->Add(hcorn);
    
  }

  cout << "List has " << listEntries*1E-6 << "M entries " << endl;

  ch->SetBranchAddress("iFrame",&f);
  ch->SetBranchAddress("x",&x);
  ch->SetBranchAddress("y",&y);
  ch->SetBranchAddress("data",dum);
  
  for (Long64_t el = 0; el < listEntries; el++) {
    treeEntry = myelist->GetEntryAndTree(el,treenum);
    chainEntry = treeEntry+ch->GetTreeOffset()[treenum];
   
    if (ch->GetEntry(chainEntry)) {
      if (el%1000000==0) {
	inte->DrawH();
	c1->Modified();
	c1->Update();
	cout << "+";
      }
      skip=0;
      for (ix=0; ix<3; ix++) {
	if (skip==0) {
	  for (iy=0; iy<3; iy++) {
	    if (gmap==NULL) {
	      data[ix+iy*3]=dum[ix+iy*3];
	    } else {
	      gain=gmap->GetBinContent(x+ix, y+iy);
	      if (gain>2000 && gain<3000) {
		data[ix+iy*3]=dum[ix+iy*3]/gain;
		//	cout << dum[ix+iy*3] << " " << gain << " " << data[ix+iy*3] << endl;
	      } else
		skip=1;
	    }
	  }
	}
      }
      if (skip==0) {
	if (heta==NULL)
	  inte->addToFlatField(data,etax,etay);
	else {
	  corn=slsInterpolation::calcQuad(data, tot, totquad, sDum);
	  if (tot>0) {
	    r=totquad/tot;
	    // cout << totquad<< " " << tot << endl;
	    if (r>0.8 && r<1.2) {
	      slsInterpolation::calcEta(totquad, sDum, etax, etay);
	      heta->Fill(etax,etay);
	      slsInterpolation::calcEtaL(totquad, corn, sDum, etaxL, etayL);
	      if (hetaL) hetaL->Fill(etaxL,etayL);
	      slsInterpolation::calcEta3(data, etax3, etay3, tot);
	      if (heta3) heta3->Fill(etax3,etay3);
	      slsInterpolation::calcEta3X(data, etax3x, etay3x, tot);
	      if (heta3X) heta3X->Fill(etax3x,etay3x);
	      
	      if (h1) h1->Fill(data[4]);
	      if (h2) h2->Fill(totquad);
	      if (h3) h3->Fill(tot);
	      if (hcorn) {
		switch(corn) {
		case TOP_LEFT:
		  hcorn->Fill(-0.5,0.5);
		  break;
		case TOP_RIGHT:
		  hcorn->Fill(0.5,0.5);
		  break;
		case BOTTOM_LEFT:
		  hcorn->Fill(-0.5,-0.5);
		  break;
		case BOTTOM_RIGHT:
		  hcorn->Fill(0.5,-0.5);
		  break;
		default:
		  break;
		}
	      
	      }
	    }
	  }
	}
      }
    }
  }

  // inte->DrawH();
  // c1->Modified();
  // c1->Update();
  // cout << "+";
  return inte;
}




TH2F *makeSpectrum(TChain *ch, Long64_t nentries=-1) {
  Long64_t ie;
  Double_t dum[9],  etax, etay;
  Int_t x, y, f;
  Double_t sum, totquad, sDum[2][2];
  // if (nentries<0)
  //   nentries=ch->GetEntries();
  //  cout << "Chain has " << nentries*1E-6 << "M entries " << endl;
  TH2F *h1=new TH2F("h1","h1",1000,0,5000,NY*NX,0,NY*NX);
 
  TCanvas *c1=new TCanvas();
  h1->Draw("colz");
  TEntryList  *myelist=ch->GetEntryList();
  Long64_t listEntries = myelist->GetN();

  ch->SetBranchAddress("iFrame",&f);
  ch->SetBranchAddress("x",&x);
  ch->SetBranchAddress("y",&y);
  ch->SetBranchAddress("data",dum);

  Long64_t treeEntry, chainEntry;
  Int_t treenum;

  cout << "List has " << listEntries*1E-6 << "M entries " << endl;

  for (Long64_t el = 0; el < listEntries; el++) {
    treeEntry = myelist->GetEntryAndTree(el,treenum);
    chainEntry = treeEntry+ch->GetTreeOffset()[treenum];
   
    if (ch->GetEntry(chainEntry)) {
      if (el%1000000==0) {
	c1->Modified();
	c1->Update();
	cout << "+";
      }
      h1->Fill(dum[4],(y-YMIN)*NX+x-XMIN); 
    }
  }
  return h1;
}

TChain *discardDoubles(TChain *ch, int xmin=0, int xmax=400, int ymin=0, int ymax=400, Double_t smin=2000, Double_t smax=3000, Long64_t nentries=-1) {

  TEntryList *elist=new TEntryList(ch);
  // TEntryList *elist1=new TEntryList(ch);
  elist->SetName("nodoubles");
  if (nentries<0)
    nentries=ch->GetEntries();
  Long64_t ie, ie0, iemax, i;
  Double_t dum[9], tot;
  Int_t x, y, f, ix, iy;

  ch->SetBranchAddress("iFrame",&f);
  ch->SetBranchAddress("x",&x);
  ch->SetBranchAddress("y",&y);
  ch->SetBranchAddress("data",dum);
  
  TH2I *hf=new TH2I("hf","frame",xmax-xmin, xmin, xmax, ymax-ymin, ymin, ymax);

  Int_t ff=-1, bad=0;
  Int_t ibad, igood, iph;
   TCanvas *c1=new TCanvas();

  for (ie=0; ie<nentries; ie++) {
    // cout << ie;
    hf->Reset();
    ie0=ie;
    iemax=ie0;
    iph=0;
    while (ff==-1 || f==ff) {
      // cout << f << " " << ff << endl;
      ch->GetEntry(iemax);
      iemax++;
      iph++;
      if (ff<0) ff=f;

      for (ix=-1; ix<2; ix++)
	for (iy=-1; iy<2; iy++)
	  hf->Fill(x+ix, y+iy);
      // if (iph>160000) break; 
    }
    //  cout <<"*************************************"<<  iemax<< " " << ie0 << " " << ff << " " << f << " " << iph<< endl;
      hf->Draw("colz");
    c1->Modified();
    c1->Update();
    
    ibad=0;
    igood=0;
    for (i=ie0; i<iemax; i++) {
      bad=0;
      ch->GetEntry(i);
      tot=0;
      for (ix=-1; ix<2; ix++) {
	for (iy=-1; iy<2; iy++) {
	  tot+=dum[ix+1+(iy+1)*3];
	  if (hf->GetBinContent(x+ix-xmin+1, y+iy-ymin+1)>1.1) {
	    // cout << "Bad " << x+ix<< " " << y+iy << endl;
	    bad=1;
	  }
	}
      }
       if (tot<=smin || tot>=smax) {
       	bad=1;
      // 	//	cout << "Bad Tot "<< tot << endl;
       }
      if (bad==0) {
	elist->Enter(i,ch); 
	igood++;
      } else {
	ibad++;
      }
      
    }
    if (f%10000==0)
      cout << "Frame " << f << " good " << igood << " bad " << ibad << endl;
    
    ff=-1;
    ie=i;
    //
  }

  //  delete hf;
  ch->SetEntryList(elist);
  return ch;
}

TChain *discardDoublesQuad(TChain *ch, int xmin=0, int xmax=400, int ymin=0, int ymax=400, Double_t smin=2000, Double_t smax=3000, Long64_t nentries=-1) {

  TEntryList *elist=new TEntryList(ch);
  // TEntryList *elist1=new TEntryList(ch);
  elist->SetName("nodoubles");
  if (nentries<0)
    nentries=ch->GetEntries();
  Long64_t ie, ie0, iemax, i;
  Double_t dum[9], tot, totquad, sDum[2][2];
  Int_t x, y, f, ix, iy, corn;

  ch->SetBranchAddress("iFrame",&f);
  ch->SetBranchAddress("x",&x);
  ch->SetBranchAddress("y",&y);
  ch->SetBranchAddress("data",dum);
  
  TH2I *hf=new TH2I("hf","frame",xmax-xmin, xmin, xmax, ymax-ymin, ymin, ymax);

  Int_t ff=-1, bad=0;
  Int_t ibad, igood, iph;
  //  TCanvas *c1=new TCanvas();

  for (ie=0; ie<nentries; ie++) {
    // cout << ie;
    hf->Reset();
    ie0=ie;
    iemax=ie0;
    iph=0;
    while (ff==-1 || f==ff) {
      // cout << f << " " << ff << endl;
      ch->GetEntry(iemax);
      iemax++;
      iph++;
      if (ff<0) ff=f;
      corn=slsInterpolation::calcQuad(dum, tot, totquad, sDum);
      // for (ix=-1; ix<2; ix++)
      //	for (iy=-1; iy<2; iy++)
      //	  hf->Fill(x+ix, y+iy);
      // if (iph>160000) break; 
      hf->Fill(x,y);
	switch(corn) {
	case TOP_LEFT:
	  hf->Fill(x-1,y);
	  hf->Fill(x-1,y+1);
	  hf->Fill(x,y+1);
	  break;
	case TOP_RIGHT:
	  hf->Fill(x+1,y);
	  hf->Fill(x+1,y+1);
	  hf->Fill(x,y+1);
	  break;
	case BOTTOM_LEFT:
	  hf->Fill(x-1,y);
	  hf->Fill(x-1,y-1);
	  hf->Fill(x,y-1);
	  break;
	case BOTTOM_RIGHT:
	  hf->Fill(x+1,y);
	  hf->Fill(x+1,y-1);
	  hf->Fill(x,y-1);
	  break;
	default:
	  break;
	}
    }
    //  cout <<"*************************************"<<  iemax<< " " << ie0 << " " << ff << " " << f << " " << iph<< endl;
    //     hf->Draw("colz");
    // c1->Modified();
    // c1->Update();
    
    ibad=0;
    igood=0;
    for (i=ie0; i<iemax; i++) {
      bad=0;
      ch->GetEntry(i);
      tot=0;
      for (ix=-1; ix<2; ix++) {
	for (iy=-1; iy<2; iy++) {
	  tot+=dum[ix+1+(iy+1)*3];
	  if (hf->GetBinContent(x+ix-xmin+1, y+iy-ymin+1)>1.1) {
	    // cout << "Bad " << x+ix<< " " << y+iy << endl;
	    bad=1;
	  }
	}
      }
       if (tot<=smin || tot>=smax) {
       	bad=1;
      // 	//	cout << "Bad Tot "<< tot << endl;
       }
      if (bad==0) {
	elist->Enter(i,ch); 
	igood++;
      } else {
	ibad++;
      }
      
    }
    if (f%10000==0)
      cout << "Frame " << f << " good " << igood << " bad " << ibad << endl;
    
    ff=-1;
    ie=i;
    //
  }

    delete hf;
  ch->SetEntryList(elist);
  return ch;
}

















TH2F *gainMap(TH2F *h1) {
  TH1D *hh;
  // fspixel->SetParNames("Background Pedestal","Background slope", "Peak position","Noise RMS", "Number of Photons","Charge Sharing Pedestal","Corner");
  Double_t par[7], epar[7];
  TH2F *gmap=new TH2F("gmap","gmap",NX,XMIN,XMIN+NX,NY,YMIN,YMIN+NY);
  int ix, iy;
  energyCalibration *ecal=new energyCalibration();
  ecal->fixParameter(0,0);
  ecal->fixParameter(1,0);
  ecal->setScanSign(1);
  ecal->setFitRange(2000,3000);
  ecal->setPlotFlag(0);
  for (int ich=0; ich<h1->GetNbinsY(); ich++) {
    hh=h1->ProjectionX("hh",ich+1,ich+1);
    if (hh->GetEntries()>10000) {
      par[0]=0;
      par[1]=0;
      par[2]=2500;
      par[3]=20;
      par[4]=1000;
      par[5]=0.1;
      par[6]-0.1;
      ecal->setStartParameters(par);
      if (ecal->fitSpectrumPixel(hh,par, epar)) {
	if (par[2]>2000 && par[2]<3000) {
	  ix=ich%NX+XMIN;
	  iy=ich/NX+YMIN;
	  gmap->SetBinContent(ix+1-XMIN,iy+1-YMIN,par[2]);
	}
      }
      
    }
    delete hh;
  }

  return gmap;

}
TChain *m1() {
  Long64_t nen=1000000000; //-1;
  rootlogon();
  TChain *ch=new TChain("blank");

  ch->Add("/local_zfs_raid/tomcat_20160528/trees/blank_*.root");
  discardDoublesQuad(ch, 0, 400, 100, 300, 2000, 3000, nen);
  TFile *fout=new TFile("/local_zfs_raid/tomcat_20160528/trees/elist.root", "RECREATE");
  TEntryList *elist=ch->GetEntryList();
  elist->Write("nodoubles",TObject::kOverwrite);
  fout->Close();

  return ch;

}


void mm() {

  int nen=1000000; //-1

  rootlogon();
  // TFile *fout=new TFile("/local_zfs_raid/tomcat_20160528/trees/gmap_eta_blank.root", "RECREATE"); 


  TFile *fL=new TFile("/local_zfs_raid/tomcat_20160528/trees/elist.root");
  TEntryList *elist =(TEntryList*)fL->Get("nodoubles");

  TFile *fg=new TFile("/local_zfs_raid/tomcat_20160528/trees/gmap_eta_blank.root");
  TH2F *gmap=(TH2F*)fg->Get("gmap");
  
  TChain *ch=new TChain("blank");

  ch->Add("/local_zfs_raid/tomcat_20160528/trees/blank_*.root");

  //  cout << "Chain has " << ch->GetEntries()*1E-6 << "M entries " << endl;
  cout << "creating entry list"<< endl;
  // ch->Draw(">>elist", "Sum$(data)>2000 && Sum$(data)<3000", "entrylist");
  //TEntryList *elist = (TEntryList*)gDirectory->Get("elist");

  //  TEntryList *elist =(TEntryList*)fg->Get("elist");

  ch->SetEntryList(elist);
  
  
   
  // fout->cd();
  // elist->Write("elist",TObject::kOverwrite);

  // cout << "creating spectrum"<< endl;
  //TH2F *h1=makeSpectrum(ch);  
  //fout->cd();
  //h1->Write("hg",TObject::kOverwrite);


  //cout << "creating gmap"<< endl;
  //TH2F *gmap=gainMap(h1);
  //new TCanvas();
  //gmap->Draw("colz");
  //fout->cd();
  //gmap->Write("gmap",TObject::kOverwrite);


  cout << "creating eta & c."<< endl; 
  //TFile *fout=new TFile("/local_zfs_raid/tomcat_20160528/trees/eta_dists.root", "RECREATE");  THStack *hs=new THStack();
  // TFile *fg=new TFile("/local_zfs_raid/tomcat_20160528/trees/gmap_blank.root");
  // TH2F *gmap=(TH2F*)fg->Get("gmap");
  // interpolation_EtaVEL *inte=fillEta(ch,nen, gmap, hs);
  // cout << "Writing to file "<< endl;
  // fout->cd();
  // inte->setEta((TH2F*)NULL)->Write();
  // inte->setEta((EtaVEL*)NULL)->Write();
  // int nh=-1;
  // if (hs->GetHists())nh=hs->GetHists()->GetEntries();
  // for (int ih=0; ih<nh; ih++) 
  //   (hs->GetHists()->At(ih))->Write();
  // //  fout->Close();
  // cout << "Finished "<< endl;
}

TH2F *fillFF(TChain *ch, interpolation_EtaVEL *eVel, Long64_t nentries=-1, TH2F *gmap=NULL, TH2F *hff=NULL) { 


  cout << "****************** Filling flat field " << nentries << endl;
  Double_t dum[9],   data[9], gain=1;
  Int_t px, py;
  Int_t x, y, f, ix, iy;
  int skip=0;
  Long64_t treeEntry, chainEntry;
  Int_t treenum;
  if (hff==NULL)
    hff=new TH2F("hff","Flat field",25,-12.5,12.5,25,-12.5,12.5);
  TAxis *ax=hff->GetXaxis();
  TAxis *ay=hff->GetYaxis();


  TEntryList  *myelist=ch->GetEntryList();
  Long64_t listEntries = myelist->GetN();

  cout << "List has " << listEntries*1E-6 << "M entries " << endl;

  ch->SetBranchAddress("iFrame",&f);
  ch->SetBranchAddress("x",&x);
  ch->SetBranchAddress("y",&y);
  ch->SetBranchAddress("data",dum);

  if (nentries<=0) nentries=listEntries;

  cout << "Nentries is " << nentries*1E-6 << "M entries " << endl;

  for (Long64_t el = 0; el < nentries; el++) {
    // if (el%1000==0) cout << (el*100.)/nentries<< "% " << nentries<< endl;
    skip=0;
    treeEntry = myelist->GetEntryAndTree(el,treenum);
    chainEntry = treeEntry+ch->GetTreeOffset()[treenum];
    
    ch->GetEntry(chainEntry);
      for (ix=0; ix<3; ix++) {
	if (skip==0) {
	  for (iy=0; iy<3; iy++) {
	    if (gmap==NULL) {
	      data[ix+iy*3]=dum[ix+iy*3];
	    } else {
	      gain=gmap->GetBinContent(x+ix, y+iy);
	      if (gain>2000 && gain<3000) {
		data[ix+iy*3]=dum[ix+iy*3]/gain;
		//	cout << dum[ix+iy*3] << " " << gain << " " << data[ix+iy*3] << endl;
	      } else
		skip=1;
	    }
	  }
	}
      }
  
      if (skip==0) {
	eVel->getInterpolatedBin(data, px, py);
	if (px>=0 && py>=0) hff->Fill(ax->GetBinCenter(px+1),ay->GetBinCenter(py+1));
      }
  }
  return hff;
}




void pos_dependent_etaX_etaY_normalization() {

  //  int ok;
  TFile *feta=new TFile("/local_zfs_raid/tomcat_20160528/trees/eta_dists.root");
  TH2F* heta=(TH2F*)feta->Get("heta");

  
  Double_t bsize=1./25.;



  cout << "Counts per bin required are "<< heta->Integral()/625. << endl;
  cout << "Counts per projection required are "<< heta->Integral()/25. << endl;
  
  Double_t meanC=heta->Integral()/625.;


  Double_t bx[26], by[26];

  TH2I *hhx=new TH2I("hhx","hhx",heta->GetNbinsX(),-0.05, 1.05, heta->GetNbinsY(),-0.05, 1.05); 
  TH2I *hhy=new TH2I("hhy","hhy",heta->GetNbinsX(),-0.05, 1.05, heta->GetNbinsY(),-0.05, 1.05); 

  TH2D *hff=new TH2D("hff","hff",25,0, 25, 25,0, 25); 
  
  Double_t cc[50];
  for (int i=0; i<50; i++) cc[i]=i+0.5;
  hhx->SetContour(50,cc);
  hhy->SetContour(50,cc);
  

  
  TH1D *hx;
  TH1D *hy;

  TH1D *hix;
  TH1D *hiy;
  int ii=0;
  cout << "X"<<endl;
  int nb=1;
  //  for (int ib=heta->GetYaxis()->FindBin(0.02); ib<heta->GetYaxis()->FindBin(1.02); ib++) {
     for (int ib=0; ib<heta->GetNbinsY(); ib++) {
       // if (ib<heta->GetYaxis()->FindBin(0.02) || ib>heta->GetYaxis()->FindBin(1.02))
      // else
      //nb=0;
    hx=heta->ProjectionX("px",ib+1,ib+1);
    // while (hx->Integral()<meanC) {
    //   // cout << ib << " " << nb << " " << hx->Integral() << endl;
    //   nb++;
    //   hx=heta->ProjectionX("px",ib+1,ib+1+nb);
    //   if ((ib+nb)>heta->GetNbinsY()) break;
    // }
    hix=(TH1D*)hx->Clone("hix");
    for (int ibx=0; ibx<hix->GetNbinsX(); ibx++)
      hix->SetBinContent(ibx+1,hix->GetBinContent(ibx)+hix->GetBinContent(ibx+1));
    hix->Scale(1./hx->Integral());
    ii=1;
    for (int ibx=0; ibx<hix->GetNbinsX(); ibx++) {
      //  for (int ibb=0; ibb<=nb; ibb++)
	hhx->SetBinContent(ibx+1, ib+1,ii-1);
      if (hix->GetBinContent(ibx+1)>=((double)ii)*bsize) {
	ii++;
      }
    }
    delete hx;
    delete hix;
    
    //  ib=ib+nb;
  
  }
  
  cout << "Y"<<endl;
  // for (int ib=heta->GetXaxis()->FindBin(0.02); ib<heta->GetXaxis()->FindBin(1.02); ib++) {
  for (int ib=0; ib<heta->GetNbinsX(); ib++) {
    // if (ib<heta->GetXaxis()->FindBin(0.02) || ib>heta->GetXaxis()->FindBin(1.02))
      // else
      // nb=0;
      
    hx=heta->ProjectionY("py",ib+1,ib+1);
    // while (hx->Integral()<meanC) {
    //   nb++;
    //   hx=heta->ProjectionY("px",ib+1,ib+1+nb);
    //   if ((ib+nb)>heta->GetNbinsX()) break;
    // }
    hix=(TH1D*)hx->Clone("hix");
    for (int ibx=0; ibx<hix->GetNbinsX(); ibx++)
      hix->SetBinContent(ibx+1,hix->GetBinContent(ibx)+hix->GetBinContent(ibx+1));
    hix->Scale(1./hx->Integral());
    ii=1;
    for (int ibx=0; ibx<hix->GetNbinsX(); ibx++) {
      //  for (int ibb=0; ibb<=nb; ibb++)
	hhy->SetBinContent( ib+1, ibx+1,ii-1);
      if (hix->GetBinContent(ibx+1)>=((double)ii)*bsize) {
	ii++;
      }
    }
    delete hx;
    delete hix;
    
    // ib=ib+nb;
  }

  new TCanvas();
  heta->Draw("colz");
  hhy->Draw("cont2 same");
  hhx->Draw("cont2 same");

  Double_t chi_sq=0;
  Double_t v;
  cout << "FF"<<endl;
       //  for (int ibx=heta->GetXaxis()->FindBin(0.02); ibx<heta->GetXaxis()->FindBin(1.02); ibx++) {
        for (int ibx=0; ibx<heta->GetNbinsX(); ibx++) { 
       for (int iby=0; iby<heta->GetNbinsY(); iby++) { 
       // for (int iby=heta->GetYaxis()->FindBin(0.02); iby<heta->GetYaxis()->FindBin(1.02); iby++) {
      hff->Fill(hhx->GetBinContent(ibx+1,iby+1)+0.5,hhy->GetBinContent(ibx+1,iby+1)+0.5,heta->GetBinContent(ibx+1,iby+1)); 
    }
  }
   for (int ibx=0; ibx<hff->GetNbinsX(); ibx++) { 
    for (int iby=0; iby<hff->GetNbinsY(); iby++) {
      v=hff->GetBinContent(ibx+1,iby+1);
      if (v>0)
	chi_sq+=(v-meanC)*(v-meanC)/v;
      else
	chi_sq+=(v-meanC)*(v-meanC);
    }
  }
   cout <<"*******chi square is " << chi_sq/625. << endl;


  new TCanvas();
  hff->Draw("colz");
  hff->Scale(1./meanC);

  
  TH2F *he2=new TH2F("he2","he2",heta->GetNbinsX(),-0.05, 1.05, heta->GetNbinsY(),-0.05, 1.05); 
  
  int ix, iy;

       // for (int ibx=heta->GetXaxis()->FindBin(0.02); ibx<heta->GetXaxis()->FindBin(1.02); ibx++) {
       // for (int iby=heta->GetYaxis()->FindBin(0.02); iby<heta->GetYaxis()->FindBin(1.02); iby++) {
       for (int ibx=0; ibx<he2->GetNbinsX(); ibx++) { 
       for (int iby=0; iby<he2->GetNbinsY(); iby++) { 
      ix=hhx->GetBinContent(ibx+1,iby+1);
      iy=hhy->GetBinContent(ibx+1,iby+1);
      he2->SetBinContent(ibx+1,iby+1,hff->GetBinContent(ix+1,iy+1));
    }
  }

  new TCanvas();
  he2->Draw("colz");

}


void global_etaX_etaY_normalization() {

  //  int ok;
  TFile *feta=new TFile("/local_zfs_raid/tomcat_20160528/trees/eta_dists.root");
  TH2F* heta=(TH2F*)feta->Get("heta");

  
  Double_t bsize=1./25.;



  cout << "Counts per bin required are "<< heta->Integral()/625. << endl;
  cout << "Counts per projection required are "<< heta->Integral()/25. << endl;
  
  Double_t meanC=heta->Integral()/625.;


  Double_t bx[26], by[26];

  TH2I *hhx=new TH2I("hhx","hhx",heta->GetNbinsX(),-0.05, 1.05, heta->GetNbinsY(),-0.05, 1.05); 
  TH2I *hhy=new TH2I("hhy","hhy",heta->GetNbinsX(),-0.05, 1.05, heta->GetNbinsY(),-0.05, 1.05); 

  TH2D *hff=new TH2D("hff","hff",25,0, 25, 25,0, 25); 
  
  Double_t cc[50];
  for (int i=0; i<50; i++) cc[i]=i+0.5;
  hhx->SetContour(50,cc);
  hhy->SetContour(50,cc);
  

  
  TH1D *hx;
  TH1D *hy;

  TH1D *hix;
  TH1D *hiy;
  int ii=0;
  cout << "X"<<endl;
  int nb=1;

  hx=heta->ProjectionX();
    hix=(TH1D*)hx->Clone("hix");
    for (int ibx=0; ibx<hix->GetNbinsX(); ibx++)
      hix->SetBinContent(ibx+1,hix->GetBinContent(ibx)+hix->GetBinContent(ibx+1));
    hix->Scale(1./hx->Integral());
  //  for (int ib=heta->GetYaxis()->FindBin(0.02); ib<heta->GetYaxis()->FindBin(1.02); ib++) {
     for (int ib=0; ib<heta->GetNbinsY(); ib++) {
       // if (ib<heta->GetYaxis()->FindBin(0.02) || ib>heta->GetYaxis()->FindBin(1.02))
      // else
      //nb=0;
       //  hx=heta->ProjectionX("px",ib+1-0.5*nb,ib+1+0.5*nb);
    // while (hx->Integral()<meanC) {
    //   // cout << ib << " " << nb << " " << hx->Integral() << endl;
    //   nb++;
    //   hx=heta->ProjectionX("px",ib+1,ib+1+nb);
    //   if ((ib+nb)>heta->GetNbinsY()) break;
    // }
    ii=1;
    for (int ibx=0; ibx<hix->GetNbinsX(); ibx++) {
      //  for (int ibb=0; ibb<=nb; ibb++)
	hhx->SetBinContent(ibx+1, ib+1,ii-1);
      if (hix->GetBinContent(ibx+1)>=((double)ii)*bsize) {
	ii++;
      }
    }
    
    //  ib=ib+nb;
  
  }


    delete hx;
    delete hix;

  hx=heta->ProjectionY();

  hix=(TH1D*)hx->Clone("hix");
  for (int ibx=0; ibx<hix->GetNbinsX(); ibx++)
    hix->SetBinContent(ibx+1,hix->GetBinContent(ibx)+hix->GetBinContent(ibx+1));
  hix->Scale(1./hx->Integral());



  cout << "Y"<<endl;
  // for (int ib=heta->GetXaxis()->FindBin(0.02); ib<heta->GetXaxis()->FindBin(1.02); ib++) {
  for (int ib=0; ib<heta->GetNbinsX(); ib++) {
    // if (ib<heta->GetXaxis()->FindBin(0.02) || ib>heta->GetXaxis()->FindBin(1.02))
      // else
      // nb=0;
      
    ii=1;
    for (int ibx=0; ibx<hix->GetNbinsX(); ibx++) {
      //  for (int ibb=0; ibb<=nb; ibb++)
      hhy->SetBinContent( ib+1, ibx+1,ii-1);
      if (hix->GetBinContent(ibx+1)>=((double)ii)*bsize) {
	ii++;
      }
    }
    
    // ib=ib+nb;
  }

    delete hx;
    delete hix;

  new TCanvas();
  heta->Draw("colz");
  hhy->Draw("cont2 same");
  hhx->Draw("cont2 same");

  Double_t chi_sq=0;
  Double_t v;
  cout << "FF"<<endl;
       //  for (int ibx=heta->GetXaxis()->FindBin(0.02); ibx<heta->GetXaxis()->FindBin(1.02); ibx++) {
        for (int ibx=0; ibx<heta->GetNbinsX(); ibx++) { 
       for (int iby=0; iby<heta->GetNbinsY(); iby++) { 
       // for (int iby=heta->GetYaxis()->FindBin(0.02); iby<heta->GetYaxis()->FindBin(1.02); iby++) {
      hff->Fill(hhx->GetBinContent(ibx+1,iby+1)+0.5,hhy->GetBinContent(ibx+1,iby+1)+0.5,heta->GetBinContent(ibx+1,iby+1)); 
    }
  }
   for (int ibx=0; ibx<hff->GetNbinsX(); ibx++) { 
    for (int iby=0; iby<hff->GetNbinsY(); iby++) {
      v=hff->GetBinContent(ibx+1,iby+1);
      if (v>0)
	chi_sq+=(v-meanC)*(v-meanC)/v;
      else
	chi_sq+=(v-meanC)*(v-meanC);
    }
  }
   cout <<"*******chi square is " << chi_sq/625. << endl;


  new TCanvas();
  hff->Draw("colz");
  hff->Scale(1./meanC);

  
  TH2F *he2=new TH2F("he2","he2",heta->GetNbinsX(),-0.05, 1.05, heta->GetNbinsY(),-0.05, 1.05); 
  
  int ix, iy;

       // for (int ibx=heta->GetXaxis()->FindBin(0.02); ibx<heta->GetXaxis()->FindBin(1.02); ibx++) {
       // for (int iby=heta->GetYaxis()->FindBin(0.02); iby<heta->GetYaxis()->FindBin(1.02); iby++) {
       for (int ibx=0; ibx<he2->GetNbinsX(); ibx++) { 
       for (int iby=0; iby<he2->GetNbinsY(); iby++) { 
      ix=hhx->GetBinContent(ibx+1,iby+1);
      iy=hhy->GetBinContent(ibx+1,iby+1);
      he2->SetBinContent(ibx+1,iby+1,hff->GetBinContent(ix+1,iy+1));
    }
  }

  new TCanvas();
  he2->Draw("colz");

}
















void hh() {
  rootlogon();


  TFile *fL=new TFile("/local_zfs_raid/tomcat_20160528/trees/elist.root");
  TEntryList *elist =(TEntryList*)fL->Get("nodoubles");

  TFile *fg=new TFile("/local_zfs_raid/tomcat_20160528/trees/gmap_eta_blank.root");
  TH2F *gmap=(TH2F*)fg->Get("gmap");
  
  TChain *ch=new TChain("blank");

  ch->Add("/local_zfs_raid/tomcat_20160528/trees/blank_*.root");



  ch->SetEntryList(elist);




  int ok;
  TFile *feta=new TFile("/local_zfs_raid/tomcat_20160528/trees/eta_dists.root");
  // EtaVEL* *eVel=(TH2F*)feta->Get("EtaVEL");
  TH2F* heta=(TH2F*)feta->Get("heta");
  heta->SetName("hheta");
  interpolation_EtaVEL *eVel=new interpolation_EtaVEL(400, 200, 25, -0.05,1.05);
  int it=0;
  char tit[10000];
  eVel->setEta(heta); 
  TH2F *hff=new TH2F("hff","Flat field",25,-12.5,12.5,25,-12.5,12.5);
  hff->SetTitle("; #pi_{x} (#mum); #pi_{y} (#mum); Counts (A.U.)");
  hff->SetStats(kFALSE);
  hff->SetMinimum(0);
  // hff->SetMaximum(20E3);
  hff->GetZaxis()->SetTitleOffset(1.3);
  hff->GetZaxis()->SetLabelSize(0.02);

  // TH2F *hgrid=new TH2F("hgrid","",25,-12.5,12.5,25,-12.5,12.5);
  // hgrid->SetTitle("; #pi_{x} (#mum); #pi_{y} (#mum); Counts (A.U.)");
  // hgrid->GetXaxis()->SetNdivisions(25);
  // hgrid->GetYaxis()->SetNdivisions(25);
  // hgrid->GetYaxis()->SetLabelOffset(999.);
  // hgrid->GetXaxis()->SetLabelOffset(999.); 
  TGraph *g_chi=new TGraph();
  g_chi->SetMarkerStyle(20);
  TCanvas *c1=new TCanvas("c1","c1",1000,500);
  TCanvas *c2=new TCanvas("c2","c2",500,500);
  c2->SetLogy(kTRUE); 
  c1->Divide(2,1);
  
  c1->cd(1);
  TPad* pad = (TPad*)c1->GetPad(1);
  //pad->SetLeftMargin(0.15);
  // pad->SetBottomMargin(0.15);
  // pad->SetTopMargin(0.2);
  // pad->SetRightMargin(0.2);
  // pad->SetFillColor(0);
   pad->SetBorderMode(0);
  pad->SetLogz(kTRUE); 
  
  c1->cd(2);
  pad = (TPad*)c1->GetPad(2);
  pad->SetLeftMargin(0.1);
  // pad->SetBottomMargin(0.15);
  // pad->SetTopMargin(0.2);
 pad->SetRightMargin(0.15);
  // pad->SetFillColor(0);
 pad->SetBorderMode(0); 
 // pad->SetLogz(kTRUE);
  pad->SetGridx(kTRUE);
  pad->SetGridy(kTRUE);
  



  heta->SetStats(kFALSE);
  //  c1->cd(2);
  // hff->Fill(0.,0.);
  // hff->Draw("colz");
  // TPaletteAxis *palette1 =(TPaletteAxis*)hff->GetListOfFunctions()->FindObject("palette");
  // palette1->SetLabelSize(0.02);


  while (ok==0 && it<10000) {
     sprintf(tit,"Iteration %d; #eta_{x}; #eta_{y}; Counts (A.U.)",it);
    hff->Reset();
     fillFF(ch, eVel, 100000, gmap, hff);

    heta->SetTitle(tit);
     c1->cd(1);
     eVel->DrawH();

     c1->cd(2);
     //  hgrid->Draw();
     hff->Draw("colz");

     c1->cd();
     c1->Modified();
     c1->Update();
     c1->SaveAs("test1.gif+");


    eVel->prepareInterpolation(ok,1);
    cout << "**************************** Iteration number " << it << endl;
    if (eVel->getChiSq()<1E10)
      g_chi->SetPoint(it,it,eVel->getChiSq());
    else
      g_chi->SetPoint(it,it,1E10);
    it++;
    if (g_chi->GetN()>0) {
      c2->cd();
      g_chi->Draw("ALP");
      c2->Modified();
      c2->Update();
    }
  }

    sprintf(tit,"Iteration %d",it);
    hff->Reset();
    fillFF(ch, eVel, 100000, gmap, hff);

    heta->SetTitle(tit);

    c1->cd(1);
    eVel->DrawH();

    c1->cd(2);
    // hgrid->Draw();
    hff->Draw("colz");

    c1->Modified();
    c1->Update();
    c1->SaveAs("test1.gif+");


}
 
TH2D *interpGrating1D() {



  TFile *fg=new TFile("/local_zfs_raid/tomcat_20160528/trees/gmap_eta_blank.root");
  TH2F *gmap=NULL;//(TH2F*)fg->Get("gmap");
  TH2F *heta=(TH2F*)fg->Get("heta");
  
  TChain *ch=new TChain("grating_1d");
  ch->Add("/local_zfs_raid/tomcat_20160528/trees/grating_1d_t*_v4.root");

  // TFile *ft=new TFile("/local_zfs_raid/tomcat_20160528/trees/grating_1d_t0_v4.root");
  // TTree *ch=(TTree*)ft->Get("grating_1d");

  // ch->Draw("y:x>>hh(400,0,400,400,0,400)","","colz");
  // gPad->Modified();
  // gPad->Update();
  
  


  TH2F *hietaX=(TH2F*)heta->Clone("hietaX");
  TH2F *hietaY=(TH2F*)heta->Clone("hietaY");
  hietaX->SetTitle("hietaX");
  hietaY->SetTitle("hietaY");


  TH1D *hetaY=heta->ProjectionX();
  TH1D *hetaX=heta->ProjectionY();

  TH1D *h1etaX=(TH1D*)hetaX->Clone("h1etaX");
  TH1D *h1etaY=(TH1D*)hetaY->Clone("h1etaY");
  h1etaX->SetTitle("h1etaX");
  h1etaY->SetTitle("h1etaY");
  
  for (int ibx=0; ibx<heta->GetNbinsX(); ibx++) {
    for (int iby=0; iby<heta->GetNbinsY(); iby++) {
      if (iby==0)
	hietaY->SetBinContent(ibx+1, iby+1,heta->GetBinContent(ibx+1, iby+1)/hetaX->GetBinContent(ibx+1));
      else
	hietaY->SetBinContent(ibx+1, iby+1,hietaY->GetBinContent(ibx+1, iby)+heta->GetBinContent(ibx+1, iby+1)/hetaX->GetBinContent(ibx+1));
	
      if (ibx==0)
	hietaX->SetBinContent(ibx+1, iby+1,heta->GetBinContent(ibx+1, iby+1)/hetaY->GetBinContent(iby+1));
      else
	hietaX->SetBinContent(ibx+1, iby+1,hietaX->GetBinContent(ibx, iby+1)+heta->GetBinContent(ibx+1, iby+1)/hetaY->GetBinContent(iby+1));
	
      if (ibx==0) {
      if (iby==0)
	h1etaY->SetBinContent(iby+1,hetaY->GetBinContent(iby+1)/hetaY->Integral());
      else
	h1etaY->SetBinContent(iby+1,h1etaY->GetBinContent(iby)+hetaY->GetBinContent(iby+1)/hetaY->Integral());

	}
    }
    if (ibx==0)
      h1etaX->SetBinContent(ibx+1,hetaX->GetBinContent(ibx+1)/hetaX->Integral());
    else
      h1etaX->SetBinContent(ibx+1,h1etaX->GetBinContent(ibx)+hetaX->GetBinContent(ibx+1)/hetaX->Integral());
      
  }
 

   TFile *fout=new TFile("/local_zfs_raid/tomcat_20160528/trees/img_grating_1d_eta.root", "RECREATE");
  


  new TCanvas();
  hietaY->Draw("colz");
  gPad->Modified();
  gPad->Update();
  
  new TCanvas();
  hietaX->Draw("colz");
  gPad->Modified();
  gPad->Update();
  new TCanvas();
  h1etaX->Draw();
  gPad->Modified();
  gPad->Update();
  new TCanvas();
  h1etaY->Draw();
  gPad->Modified();
  gPad->Update();
  
  Double_t data[9], gain=1;
  Double_t dum[9], sDum[2][2], etax, etay, sum, totquad;
  Int_t x,y,f;
  int ix, iy, skip;

  Long64_t nentries=ch->GetEntries();
  ch->SetBranchAddress("iFrame",&f);
  ch->SetBranchAddress("x",&x);
  ch->SetBranchAddress("y",&y);
  ch->SetBranchAddress("data",dum);
  Long64_t ie;

  TH2D *imgLR=new TH2D("imgLR","imgLR",400,0,400,200,100,300);
  TH2D *imgHR=new TH2D("imgHR","imgHR",400*25,0,400,200*25,100,300);
  TH2D *imgHR1=new TH2D("imgHR1","imgHR1",400*25,0,400,200*25,100,300);
  

  TCanvas *c1=new TCanvas();
  imgLR->Draw("colz");
   TCanvas *c2=new TCanvas();
   imgHR->Draw("colz");
   TCanvas *c3=new TCanvas();
   imgHR1->Draw("colz");

  cout << "Chain has " << nentries*1E-6 << "M entries " << endl;
  Double_t etamax=hietaX->GetXaxis()->GetBinCenter(hietaX->GetNbinsX());
  Double_t etamin=hietaX->GetXaxis()->GetBinCenter(0);

  for (ie=0; ie<nentries; ie++) {
    skip=1;
    if (ch->GetEntry(ie)) {  
    skip=0;
      for (ix=0; ix<3; ix++) {
      	if (skip==0) {
      	  for (iy=0; iy<3; iy++) {
      	    if (gmap==NULL) {
      	      data[ix+iy*3]=dum[ix+iy*3];
      	    } else {
      	      gain=gmap->GetBinContent(gmap->GetXaxis()->FindBin(x), gmap->GetYaxis()->FindBin(y));
      	      if (gain>2000 && gain<3000) {
      		data[ix+iy*3]=dum[ix+iy*3]/gain;
      		//	cout << dum[ix+iy*3] << " " << gain << " " << data[ix+iy*3] << endl;
      	      } else
      		skip=1;
      	    }
      	  }
      	}
      }
   
    }  
     if (skip==0) {
      //slsInterpolation::calcEta(dum, etax, etay, sum, totquad, sDum);
      slsInterpolation::calcEta(data, etax, etay, sum, totquad, sDum);
       }
      if (etax<etamin || etay<etamin || etax>etamax || etay>etamax || totquad/sum<0.8 || totquad/sum>1.2) skip=1;
    
      if (skip==0) {
      imgLR->Fill(x,y);
      imgHR->Fill(x-0.5+hietaX->GetBinContent(hietaX->GetXaxis()->FindBin(etax),hietaX->GetYaxis()->FindBin(etay)),y-0.5+hietaY->GetBinContent(hietaY->GetXaxis()->FindBin(etax),hietaY->GetYaxis()->FindBin(etay)));
      imgHR1->Fill(x-0.5+h1etaX->GetBinContent(h1etaX->FindBin(etax)), y-0.5+h1etaY->GetBinContent(h1etaY->FindBin(etay)));
    }
     if (ie%1000000==0) {
    //   c1->Modified();
    //   c1->Update();
    //   c2->Modified();
    //   c2->Update();
    //   c3->Modified();
    //   c3->Update();
     } 
     if (ie%10000000==0) {
    //   c1->Modified();
    //   c1->Update();
    //   c2->Modified();export PATH=/afs/psi.ch/project/sls_det_firmware/jungfrau_software/uClinux-2010_64bit/bfin-uclinux/bin:$PATH
    //   c2->Update();
    //   c3->Modified();
    //   c3->Update();
       cout << " " << ((float)ie)*100./((float)nentries)<< "%" << endl;
     }
    
  }
  imgLR->Write("imgLR");
  imgHR->Write("imgHR");
  imgHR1->Write("imgHR1");
 
  c1->cd();
  imgLR->DrawCopy("colz");
  c2->cd();
  imgHR->DrawCopy("colz");
  c3->cd();
  imgHR1->DrawCopy("colz");
  
  fout->Close();

}









TH2D *interpBlank() {


  const Double_t Delta_eta=0.025;


  TFile *fg=new TFile("/local_zfs_raid/tomcat_20160528/trees/gmap_eta_blank.root");
  TH2F *gmap=NULL;//(TH2F*)fg->Get("gmap");
  TH2F *heta=(TH2F*)fg->Get("heta");
  
  TChain *ch=new TChain("blank");
  ch->Add("/local_zfs_raid/tomcat_20160528/trees/blank_t*.root");

  // TFile *ft=new TFile("/local_zfs_raid/tomcat_20160528/trees/grating_1d_t0_v4.root");
  // TTree *ch=(TTree*)ft->Get("grating_1d");

  // ch->Draw("y:x>>hh(400,0,400,400,0,400)","","colz");
  // gPad->Modified();
  // gPad->Update();
  

  
  


  TH2F *hietaX=(TH2F*)heta->Clone("hietaX");
  TH2F *hietaY=(TH2F*)heta->Clone("hietaY");
  hietaX->SetTitle("hietaX");
  hietaY->SetTitle("hietaY");


  TH1D *hetaY=heta->ProjectionX();
  TH1D *hetaX=heta->ProjectionY();

  TH1D *h1etaX=(TH1D*)hetaX->Clone("h1etaX");
  TH1D *h1etaY=(TH1D*)hetaY->Clone("h1etaY");
  h1etaX->SetTitle("h1etaX");
  h1etaY->SetTitle("h1etaY");
  
  int binminX=heta->GetXaxis()->FindBin(-Delta_eta);
  int binmaxX=heta->GetXaxis()->FindBin(1+Delta_eta);
  int binminY=heta->GetYaxis()->FindBin(-Delta_eta);
  int binmaxY=heta->GetYaxis()->FindBin(1+Delta_eta);
  

  for (int ibx=binminX; ibx<binmaxX; ibx++) {
    for (int iby=binminY; iby<binmaxY; iby++) {
      if (iby==0)
	hietaY->SetBinContent(ibx, iby,heta->GetBinContent(ibx, iby)/hetaX->GetBinContent(ibx));
      else
	hietaY->SetBinContent(ibx, iby,hietaY->GetBinContent(ibx, iby-1)+heta->GetBinContent(ibx, iby)/hetaX->GetBinContent(ibx));
	
      if (ibx==0)
	hietaX->SetBinContent(ibx, iby,heta->GetBinContent(ibx, iby)/hetaY->GetBinContent(iby));
      else
	hietaX->SetBinContent(ibx, iby,hietaX->GetBinContent(ibx-1, iby)+heta->GetBinContent(ibx, iby)/hetaY->GetBinContent(iby));
	
      if (ibx==binminX) {
      if (iby==0)
	h1etaY->SetBinContent(iby+1,hetaY->GetBinContent(iby)/hetaY->Integral(binminY,binmaxY));
      else
	h1etaY->SetBinContent(iby+1,h1etaY->GetBinContent(iby-1)+hetaY->GetBinContent(iby)/hetaY->Integral(binminY,binmaxY));
      
      }
    }
    if (ibx==0)
      h1etaX->SetBinContent(ibx+1,hetaX->GetBinContent(ibx+1)/hetaX->Integral(binminX,binmaxX));
    else
      h1etaX->SetBinContent(ibx+1,h1etaX->GetBinContent(ibx)+hetaX->GetBinContent(ibx+1)/hetaX->Integral(binminX,binmaxX));
      
  }
 

   TFile *fout=new TFile("/local_zfs_raid/tomcat_20160528/trees/img_blank_eta.root", "RECREATE");
  


  new TCanvas();
  hietaY->Draw("colz");
  gPad->Modified();
  gPad->Update();
  
  new TCanvas();
  hietaX->Draw("colz");
  gPad->Modified();
  gPad->Update();
  new TCanvas();
  h1etaX->Draw();
  gPad->Modified();
  gPad->Update();
  new TCanvas();
  h1etaY->Draw();
  gPad->Modified();
  gPad->Update();
  
  Double_t data[9], gain=1;
  Double_t dum[9], sDum[2][2], etax, etay, sum, totquad;
  Int_t x,y,f;
  int ix, iy, skip;

  Long64_t nentries=ch->GetEntries();
  ch->SetBranchAddress("iFrame",&f);
  ch->SetBranchAddress("x",&x);
  ch->SetBranchAddress("y",&y);
  ch->SetBranchAddress("data",dum);
  Long64_t ie;

  TH2D *imgLR=new TH2D("imgLR","imgLR",400,0,400,200,100,300);
  TH2D *imgHR=new TH2D("imgHR","imgHR",400*25,0,400,200*25,100,300);
  TH2D *imgHR1=new TH2D("imgHR1","imgHR1",400*25,0,400,200*25,100,300);
  

  TCanvas *c1=new TCanvas();
  imgLR->Draw("colz");
   TCanvas *c2=new TCanvas();
   imgHR->Draw("colz");
   TCanvas *c3=new TCanvas();
   imgHR1->Draw("colz");

  cout << "Chain has " << nentries*1E-6 << "M entries " << endl;
  Double_t etamax=hietaX->GetXaxis()->GetBinCenter(hietaX->GetNbinsX());
  Double_t etamin=hietaX->GetXaxis()->GetBinCenter(0);

  for (ie=0; ie<nentries; ie++) {
    skip=1;
    if (ch->GetEntry(ie)) {  
    skip=0;
      for (ix=0; ix<3; ix++) {
      	if (skip==0) {
      	  for (iy=0; iy<3; iy++) {
      	    if (gmap==NULL) {
      	      data[ix+iy*3]=dum[ix+iy*3];
      	    } else {
      	      gain=gmap->GetBinContent(gmap->GetXaxis()->FindBin(x), gmap->GetYaxis()->FindBin(y));
      	      if (gain>2000 && gain<3000) {
      		data[ix+iy*3]=dum[ix+iy*3]/gain;
      		//	cout << dum[ix+iy*3] << " " << gain << " " << data[ix+iy*3] << endl;
      	      } else
      		skip=1;
      	    }
      	  }
      	}
      }
   
    }  
     if (skip==0) {
      slsInterpolation::calcEta(data, etax, etay, sum, totquad, sDum);
       }
      if (etax<etamin || etay<etamin || etax>etamax || etay>etamax || totquad/sum<0.8 || totquad/sum>1.2) skip=1;
    
      if (skip==0) {
      imgLR->Fill(x,y);
      imgHR->Fill(x-0.5+hietaX->GetBinContent(hietaX->GetXaxis()->FindBin(etax),hietaX->GetYaxis()->FindBin(etay)),y-0.5+hietaY->GetBinContent(hietaY->GetXaxis()->FindBin(etax),hietaY->GetYaxis()->FindBin(etay)));
      imgHR1->Fill(x-0.5+h1etaX->GetBinContent(h1etaX->FindBin(etax)), y-0.5+h1etaY->GetBinContent(h1etaY->FindBin(etay)));
    }
     if (ie%1000000==0) {
    //   c1->Modified();
    //   c1->Update();
    //   c2->Modified();
    //   c2->Update();
    //   c3->Modified();
    //   c3->Update();
     } 
     if (ie%10000000==0) {
    //   c1->Modified();
    //   c1->Update();
    //   c2->Modified();export PATH=/afs/psi.ch/project/sls_det_firmware/jungfrau_software/uClinux-2010_64bit/bfin-uclinux/bin:$PATH
    //   c2->Update();
    //   c3->Modified();
    //   c3->Update();
       cout << " " << ((float)ie)*100./((float)nentries)<< "%" << endl;
       imgLR->Write("imgLR",TObject::kOverwrite);
       imgHR->Write("imgHR",TObject::kOverwrite);
       imgHR1->Write("imgHR1",TObject::kOverwrite);
     }
    
  }
  imgLR->Write("imgLR",TObject::kOverwrite);
  imgHR->Write("imgHR",TObject::kOverwrite);
  imgHR1->Write("imgHR1",TObject::kOverwrite);
 
  // c1->cd();
  // imgLR->DrawCopy("colz");
  // c2->cd();
  // imgHR->DrawCopy("colz");
  // c3->cd();
  // imgHR1->DrawCopy("colz");
  
  fout->Close();

}







TH2D *interpSampleGrating() {



  TFile *fg=new TFile("/local_zfs_raid/tomcat_20160528/trees/gmap_eta_blank.root");
  TH2F *gmap=NULL;//(TH2F*)fg->Get("gmap");
  TH2F *heta=(TH2F*)fg->Get("heta");
  
  TChain *ch=new TChain("sample_grating_1d");
  ch->Add("/local_zfs_raid/tomcat_20160528/trees/sample_grating_1d_t*.root");

  // TFile *ft=new TFile("/local_zfs_raid/tomcat_20160528/trees/grating_1d_t0_v4.root");
  // TTree *ch=(TTree*)ft->Get("grating_1d");

  // ch->Draw("y:x>>hh(400,0,400,400,0,400)","","colz");
  // gPad->Modified();
  // gPad->Update();
  
  


  TH2F *hietaX=(TH2F*)heta->Clone("hietaX");
  TH2F *hietaY=(TH2F*)heta->Clone("hietaY");
  hietaX->SetTitle("hietaX");
  hietaY->SetTitle("hietaY");


  TH1D *hetaY=heta->ProjectionX();
  TH1D *hetaX=heta->ProjectionY();

  TH1D *h1etaX=(TH1D*)hetaX->Clone("h1etaX");
  TH1D *h1etaY=(TH1D*)hetaY->Clone("h1etaY");
  h1etaX->SetTitle("h1etaX");
  h1etaY->SetTitle("h1etaY");
  
  for (int ibx=0; ibx<heta->GetNbinsX(); ibx++) {
    for (int iby=0; iby<heta->GetNbinsY(); iby++) {
      if (iby==0)
	hietaY->SetBinContent(ibx+1, iby+1,heta->GetBinContent(ibx+1, iby+1)/hetaX->GetBinContent(ibx+1));
      else
	hietaY->SetBinContent(ibx+1, iby+1,hietaY->GetBinContent(ibx+1, iby)+heta->GetBinContent(ibx+1, iby+1)/hetaX->GetBinContent(ibx+1));
	
      if (ibx==0)
	hietaX->SetBinContent(ibx+1, iby+1,heta->GetBinContent(ibx+1, iby+1)/hetaY->GetBinContent(iby+1));
      else
	hietaX->SetBinContent(ibx+1, iby+1,hietaX->GetBinContent(ibx, iby+1)+heta->GetBinContent(ibx+1, iby+1)/hetaY->GetBinContent(iby+1));
	
      if (ibx==0) {
      if (iby==0)
	h1etaY->SetBinContent(iby+1,hetaY->GetBinContent(iby+1)/hetaY->Integral());
      else
	h1etaY->SetBinContent(iby+1,h1etaY->GetBinContent(iby)+hetaY->GetBinContent(iby+1)/hetaY->Integral());

	}
    }
    if (ibx==0)
      h1etaX->SetBinContent(ibx+1,hetaX->GetBinContent(ibx+1)/hetaX->Integral());
    else
      h1etaX->SetBinContent(ibx+1,h1etaX->GetBinContent(ibx)+hetaX->GetBinContent(ibx+1)/hetaX->Integral());
      
  }
 

   TFile *fout=new TFile("/local_zfs_raid/tomcat_20160528/trees/img_sample_grating_1d_eta.root", "RECREATE");
  


  new TCanvas();
  hietaY->Draw("colz");
  gPad->Modified();
  gPad->Update();
  
  new TCanvas();
  hietaX->Draw("colz");
  gPad->Modified();
  gPad->Update();
  new TCanvas();
  h1etaX->Draw();
  gPad->Modified();
  gPad->Update();
  new TCanvas();
  h1etaY->Draw();
  gPad->Modified();
  gPad->Update();
  
  Double_t data[9], gain=1;
  Double_t dum[9], sDum[2][2], etax, etay, sum, totquad;
  Int_t x,y,f;
  int ix, iy, skip;

  Long64_t nentries=ch->GetEntries();
  ch->SetBranchAddress("iFrame",&f);
  ch->SetBranchAddress("x",&x);
  ch->SetBranchAddress("y",&y);
  ch->SetBranchAddress("data",dum);
  Long64_t ie;

  TH2D *imgLR=new TH2D("imgLR","imgLR",400,0,400,200,100,300);
  TH2D *imgHR=new TH2D("imgHR","imgHR",400*25,0,400,200*25,100,300);
  TH2D *imgHR1=new TH2D("imgHR1","imgHR1",400*25,0,400,200*25,100,300);
  

  TCanvas *c1=new TCanvas();
  imgLR->Draw("colz");
   TCanvas *c2=new TCanvas();
   imgHR->Draw("colz");
   TCanvas *c3=new TCanvas();
   imgHR1->Draw("colz");

  cout << "Chain has " << nentries*1E-6 << "M entries " << endl;
  Double_t etamax=hietaX->GetXaxis()->GetBinCenter(hietaX->GetNbinsX());
  Double_t etamin=hietaX->GetXaxis()->GetBinCenter(0);

  for (ie=0; ie<nentries; ie++) {
    skip=1;
    if (ch->GetEntry(ie)) {  
    skip=0;
      for (ix=0; ix<3; ix++) {
      	if (skip==0) {
      	  for (iy=0; iy<3; iy++) {
      	    if (gmap==NULL) {
      	      data[ix+iy*3]=dum[ix+iy*3];
      	    } else {
      	      gain=gmap->GetBinContent(gmap->GetXaxis()->FindBin(x), gmap->GetYaxis()->FindBin(y));
      	      if (gain>2000 && gain<3000) {
      		data[ix+iy*3]=dum[ix+iy*3]/gain;
      		//	cout << dum[ix+iy*3] << " " << gain << " " << data[ix+iy*3] << endl;
      	      } else
      		skip=1;
      	    }
      	  }
      	}
      }
   
    }  
     if (skip==0) {
      slsInterpolation::calcEta(data, etax, etay, sum, totquad, sDum);
       }
      if (etax<etamin || etay<etamin || etax>etamax || etay>etamax || totquad/sum<0.8 || totquad/sum>1.2) skip=1;
    
      if (skip==0) {
      imgLR->Fill(x,y);
      imgHR->Fill(x-0.5+hietaX->GetBinContent(hietaX->GetXaxis()->FindBin(etax),hietaX->GetYaxis()->FindBin(etay)),y-0.5+hietaY->GetBinContent(hietaY->GetXaxis()->FindBin(etax),hietaY->GetYaxis()->FindBin(etay)));
      imgHR1->Fill(x-0.5+h1etaX->GetBinContent(h1etaX->FindBin(etax)), y-0.5+h1etaY->GetBinContent(h1etaY->FindBin(etay)));
    }
     if (ie%1000000==0) {
    //   c1->Modified();
    //   c1->Update();
    //   c2->Modified();
    //   c2->Update();
    //   c3->Modified();
    //   c3->Update();
     } 
     if (ie%10000000==0) {
    //   c1->Modified();
    //   c1->Update();
    //   c2->Modified();export PATH=/afs/psi.ch/project/sls_det_firmware/jungfrau_software/uClinux-2010_64bit/bfin-uclinux/bin:$PATH
    //   c2->Update();
    //   c3->Modified();
    //   c3->Update();
       cout << " " << ((float)ie)*100./((float)nentries)<< "%" << endl;
       imgLR->Write("imgLR",TObject::kOverwrite);
       imgHR->Write("imgHR",TObject::kOverwrite);
       imgHR1->Write("imgHR1",TObject::kOverwrite);
     }
    
  }
  imgLR->Write("imgLR",TObject::kOverwrite);
  imgHR->Write("imgHR",TObject::kOverwrite);
  imgHR1->Write("imgHR1",TObject::kOverwrite);
 
  // c1->cd();
  // imgLR->DrawCopy("colz");
  // c2->cd();
  // imgHR->DrawCopy("colz");
  // c3->cd();
  // imgHR1->DrawCopy("colz");
  
  fout->Close();

}

TH2D *interp(char *name, int ip, int nb=25) {

  char chainName[1000];
  sprintf(chainName,"/local_zfs_raid/tomcat_20160528/trees/%s_t*.root",name);

  const Double_t Delta_eta=0.05;


  char gName[1000];
  //sprintf(gName,"/local_zfs_raid/tomcat_20160528/trees/gmap_eta_blank_%d.root",ip);
  sprintf(gName,"/local_zfs_raid/tomcat_20160528/trees/gmap_eta_gcorr_%d.root",ip);
  TFile *fg=new TFile(gName);
  TH2F *gmap=(TH2F*)fg->Get("gmap");
  TH2F *heta=(TH2F*)fg->Get("heta");
  
  TChain *ch=new TChain(name);
  ch->Add(chainName);

  // TFile *ft=new TFile("/local_zfs_raid/tomcat_20160528/trees/grating_1d_t0_v4.root");
  // TTree *ch=(TTree*)ft->Get("grating_1d");

  // ch->Draw("y:x>>hh(400,0,400,400,0,400)","","colz");
  // gPad->Modified();
  // gPad->Update();
  

  
  


  TH2F *hietaX=(TH2F*)heta->Clone("hietaX");
  TH2F *hietaY=(TH2F*)heta->Clone("hietaY");
  hietaX->SetTitle("hietaX");
  hietaY->SetTitle("hietaY");
  hietaX->Reset();
  hietaY->Reset();
  


  TH1D *hetaY=heta->ProjectionX();
  TH1D *hetaX=heta->ProjectionY();

  TH1D *h1etaX=(TH1D*)hetaX->Clone("h1etaX");
  TH1D *h1etaY=(TH1D*)hetaY->Clone("h1etaY");
  h1etaX->SetTitle("h1etaX");
  h1etaY->SetTitle("h1etaY");
  h1etaX->Reset();
  h1etaY->Reset();
  
  int binminX=heta->GetXaxis()->FindBin(-Delta_eta);
  int binmaxX=heta->GetXaxis()->FindBin(1+Delta_eta);
  int binminY=heta->GetYaxis()->FindBin(-Delta_eta);
  int binmaxY=heta->GetYaxis()->FindBin(1+Delta_eta);
  
  
  TH1D *hetaYp=heta->ProjectionX("py",binminY,binmaxY);
  TH1D *hetaXp=heta->ProjectionY("px",binminX,binmaxX);

  for (int ibx=binminX; ibx<binmaxX; ibx++) {
      for (int iby=binminY; iby<binmaxY; iby++) {
	
	  hietaY->SetBinContent(ibx, iby,hietaY->GetBinContent(ibx, iby-1)+heta->GetBinContent(ibx, iby));
	  
	  hietaX->SetBinContent(ibx, iby,hietaX->GetBinContent(ibx-1, iby)+heta->GetBinContent(ibx, iby));
	 
	  
	  
	}
	   
	     }  


  for (int ibx=binminX; ibx<binmaxX; ibx++) {
      for (int iby=binminY; iby<binmaxY; iby++) {

	  hietaY->SetBinContent(ibx,iby,hietaY->GetBinContent(ibx,iby)/hietaY->GetBinContent(ibx,binmaxY-1));
	  hietaX->SetBinContent(ibx,iby,hietaX->GetBinContent(ibx,iby)/hietaX->GetBinContent(binmaxX-1,iby));
	  
	}

	     }
 
      for (int iby=binminY; iby<binmaxY; iby++) 
	     h1etaY->SetBinContent(iby+1,h1etaY->GetBinContent(iby)+hetaY->GetBinContent(iby+1)/hetaY->Integral(binminY,binmaxY));

	   for (int ibx=binminX; ibx<binmaxX; ibx++)
		  h1etaX->SetBinContent(ibx+1,h1etaX->GetBinContent(ibx)+hetaX->GetBinContent(ibx+1)/hetaX->Integral(binminX,binmaxX));

		for (int ibx=binmaxX; ibx<heta->GetNbinsX()+1; ibx++)
		  h1etaX->SetBinContent(ibx,1);
		for (int iby=binmaxX; iby<heta->GetNbinsY()+1; iby++)
		  h1etaY->SetBinContent(iby,1);


      for (int ibx=binmaxX-1; ibx<heta->GetNbinsX(); ibx++) {
	  
      for (int iby=0; iby<heta->GetNbinsY(); iby++) {
	  hietaX->SetBinContent(ibx+1,iby+1,1);
	}
	     }
	  

      for (int iby=0; iby<binminY+1; iby++) {
	  for (int ibx=0; ibx<heta->GetNbinsX(); ibx++) {
	      hietaX->SetBinContent(ibx+1,iby+1,hietaX->GetBinContent(ibx+1,binminY+1));
	    }
	}


      for (int iby=binmaxY-1; iby<heta->GetNbinsY(); iby++) {
	  for (int ibx=0; ibx<heta->GetNbinsX(); ibx++) {
	      hietaX->SetBinContent(ibx+1,iby+1,hietaX->GetBinContent(ibx+1,binmaxY-1));
	    }
	}



      for (int iby=binmaxX-1; iby<heta->GetNbinsY(); iby++) {
	  
      for (int ibx=0; ibx<heta->GetNbinsX(); ibx++) {
	  hietaY->SetBinContent(ibx+1,iby+1,1);
	}
	     }
	  


      for (int ibx=0; ibx<binminX+1; ibx++) {
	  for (int iby=0; iby<heta->GetNbinsY(); iby++) {
	      hietaY->SetBinContent(ibx+1,iby+1,hietaY->GetBinContent(binminX+1, iby+1));
	    }
	}


      for (int ibx=binmaxX-1; ibx<heta->GetNbinsX(); ibx++) {
	  for (int iby=0; iby<heta->GetNbinsY(); iby++) {
	      hietaY->SetBinContent(ibx+1,iby+1,hietaY->GetBinContent(binmaxX-1, iby+1));
	    }
	}

  


  // new TCanvas();
  // hietaY->Draw("colz");
  // gPad->Modified();
  // gPad->Update();
  
  // new TCanvas();
  // hietaX->Draw("colz");
  // gPad->Modified();
  // gPad->Update();
  // new TCanvas();
  // h1etaX->Draw();
  // gPad->Modified();
  // gPad->Update();
  // new TCanvas();
  // h1etaY->Draw();
  // gPad->Modified();
  // gPad->Update();
  

  char foutName[1000];
  sprintf(foutName,"/local_zfs_raid/tomcat_20160528/trees/img_%s_eta_gcorr_nb%d.root",name,nb);
    

   TFile *fout=new TFile(foutName, "RECREATE");
  Double_t data[9], gain=1;
  Double_t dum[9], sDum[2][2], etax, etay, sum, totquad;
  Int_t x,y,f;
  int ix, iy, skip;

  Long64_t nentries=ch->GetEntries();
  ch->SetBranchAddress("iFrame",&f);
  ch->SetBranchAddress("x",&x);
  ch->SetBranchAddress("y",&y);
  ch->SetBranchAddress("data",dum);
  Long64_t ie;

  TH2D *imgLR=new TH2D("imgLR","imgLR",400,0,400,200,100,300);
  TH2D *imgHR=new TH2D("imgHR","imgHR",400*nb,0,400,200*nb,100,300);
  // TH2D *imgHR1=new TH2D("imgHR1","imgHR1",400*25,0,400,200*25,100,300);
  

  // TCanvas *c1=new TCanvas();
  // imgLR->Draw("colz");
  //  TCanvas *c2=new TCanvas();
  //  imgHR->Draw("colz");
  //  TCanvas *c3=new TCanvas();
  //  imgHR1->Draw("colz");

  cout << "Chain has " << nentries*1E-6 << "M entries " << endl;
  Double_t etamax=hietaX->GetXaxis()->GetBinCenter(hietaX->GetNbinsX());
  Double_t etamin=hietaX->GetXaxis()->GetBinCenter(0);

  char name1[100],name2[100],name3[100];
  sprintf(name1,"%sLR",name);
  sprintf(name2,"%sHR",name);
  sprintf(name3,"%sHR1",name);

  for (ie=0; ie<nentries; ie++) {
    skip=1;
    if (ch->GetEntry(ie)) {  
    skip=0;
      for (ix=0; ix<3; ix++) {
      	if (skip==0) {
      	  for (iy=0; iy<3; iy++) {
      	    if (gmap==NULL) {
      	      data[ix+iy*3]=dum[ix+iy*3];
      	    } else {
      	      gain=gmap->GetBinContent(gmap->GetXaxis()->FindBin(x), gmap->GetYaxis()->FindBin(y));
      	      if (gain>2000 && gain<3000) {
      		data[ix+iy*3]=dum[ix+iy*3]/gain;
      		//	cout << dum[ix+iy*3] << " " << gain << " " << data[ix+iy*3] << endl;
      	      } else
      		skip=1;
      	    }
      	  }
      	}
      }
   
    }  
     if (skip==0) {
      slsInterpolation::calcEta(data, etax, etay, sum, totquad, sDum);
       }
      if (etax<etamin || etay<etamin || etax>etamax || etay>etamax || totquad/sum<0.8 || totquad/sum>1.2) skip=1;
    
      if (skip==0) {
	imgLR->Fill(x,y);
	imgHR->Fill(x-0.5+hietaX->GetBinContent(hietaX->GetXaxis()->FindBin(etax),hietaX->GetYaxis()->FindBin(etay)),y-0.5+hietaY->GetBinContent(hietaY->GetXaxis()->FindBin(etax),hietaY->GetYaxis()->FindBin(etay)));
	//	imgHR1->Fill(x-0.5+h1etaX->GetBinContent(h1etaX->FindBin(etax)), y-0.5+h1etaY->GetBinContent(h1etaY->FindBin(etay)));
    }
     if (ie%1000000==0) {
    //   c1->Modified();
    //   c1->Update();
    //   c2->Modified();
    //   c2->Update();
    //   c3->Modified();
    //   c3->Update();
     } 
     if (ie%10000000==0) {
    //   c1->Modified();
    //   c1->Update();
    //   c2->Modified();export PATH=/afs/psi.ch/project/sls_det_firmware/jungfrau_software/uClinux-2010_64bit/bfin-uclinux/bin:$PATH
    //   c2->Update();
    //   c3->Modified();
    //   c3->Update();
       cout << " " << ((float)ie)*100./((float)nentries)<< "%" << endl;
       imgLR->Write(name1,TObject::kOverwrite);
       imgHR->Write(name2,TObject::kOverwrite);
       //    imgHR1->Write(name3,TObject::kOverwrite);
     }
    
  }
  imgLR->Write(name1,TObject::kOverwrite);
  imgHR->Write(name2,TObject::kOverwrite);
  // imgHR1->Write(name3,TObject::kOverwrite);
 
  // c1->cd();
  // imgLR->DrawCopy("colz");
  // c2->cd();
  // imgHR->DrawCopy("colz");
  // c3->cd();
  // imgHR1->DrawCopy("colz");
  
  fout->Close();

}

TH2D *interpPos(char *name, int ip) {

  char chainName[1000];
  sprintf(chainName,"/local_zfs_raid/tomcat_20160528/trees/%s_t*.root",name);

  const Double_t Delta_eta=0.015;


  char gName[1000];
  sprintf(gName,"/local_zfs_raid/tomcat_20160528/trees/gmap_eta_blank_%d.root",ip);
  TFile *fg=new TFile(gName);
  TH2F *gmap=(TH2F*)fg->Get("gmap");
  // TH2F *heta=(TH2F*)fg->Get("heta");
  
  sprintf(gName,"/local_zfs_raid/tomcat_20160528/trees/hpos_anna_%d.root",ip);
  TFile *fp=new TFile(gName);
  TH2F *hpos=(TH2F*)fp->Get("hpos");

  TChain *ch=new TChain(name);
  ch->Add(chainName);

  // TFile *ft=new TFile("/local_zfs_raid/tomcat_20160528/trees/grating_1d_t0_v4.root");
  // TTree *ch=(TTree*)ft->Get("grating_1d");

  // ch->Draw("y:x>>hh(400,0,400,400,0,400)","","colz");
  // gPad->Modified();
  // gPad->Update();
  

  
  char foutName[1000];
  sprintf(foutName,"/local_zfs_raid/tomcat_20160528/trees/img_%s_pos_gmap_v2.root",name);
    

  TFile *fout=new TFile(foutName, "RECREATE");
  Double_t data[9], gain=1;
  Double_t dum[9], sDum[2][2], etax, etay, sum, totquad;
  Int_t x,y,f;
  int ix, iy, skip;

  Long64_t nentries=ch->GetEntries();
  ch->SetBranchAddress("iFrame",&f);
  ch->SetBranchAddress("x",&x);
  ch->SetBranchAddress("y",&y);
  ch->SetBranchAddress("data",dum);
  Long64_t ie;

  TH2D *imgLR=new TH2D("imgLR","imgLR",400,0,400,200,100,300);
  TH2D *imgHR=new TH2D("imgHR","imgHR",400*25,0,400,200*25,100,300);
  TH2D *imgHR1=new TH2D("imgHR1","imgHR1",400*25,0,400,200*25,100,300);
  

  TCanvas *c1=new TCanvas();
  imgLR->Draw("colz");
   TCanvas *c2=new TCanvas();
   imgHR->Draw("colz");
   TCanvas *c3=new TCanvas();
   imgHR1->Draw("colz");

  cout << "Chain has " << nentries*1E-6 << "M entries " << endl;
  int ppos, xpos, ypos;
  char name1[100],name2[100],name3[100];
  sprintf(name1,"%sLR",name);
  sprintf(name2,"%sHR",name);
  sprintf(name3,"%sHR1",name);

  for (ie=0; ie<nentries; ie++) {
    skip=1;
    if (ch->GetEntry(ie)) {  
    skip=0;
      for (ix=0; ix<3; ix++) {
      	if (skip==0) {
      	  for (iy=0; iy<3; iy++) {
      	    if (gmap==NULL) {
      	      data[ix+iy*3]=dum[ix+iy*3];
      	    } else {
      	      gain=gmap->GetBinContent(gmap->GetXaxis()->FindBin(x), gmap->GetYaxis()->FindBin(y));
      	      if (gain>2000 && gain<3000) {
      		data[ix+iy*3]=dum[ix+iy*3]/gain;
      		//	cout << dum[ix+iy*3] << " " << gain << " " << data[ix+iy*3] << endl;
      	      } else
      		skip=1;
      	    }
      	  }
      	}
      }
   
    }  
     if (skip==0) {
       slsInterpolation::calcEta(data, etax, etay, sum, totquad, sDum);
     }
     if (totquad/sum<0.8 || totquad/sum>1.2) skip=1;
     
     if (skip==0) {
       imgLR->Fill(x,y);
       ppos=hpos->GetBinContent(hpos->GetXaxis()->FindBin(etax),hpos->GetYaxis()->FindBin(etay));
       xpos=ppos/25;
       ypos=ppos%25;
       imgHR->Fill(x-0.5+((Double_t)xpos+0.1)/25.,y-0.5+((Double_t)ypos+0.1)/25.);
     }
     if (ie%1000000==0) {
    //   c1->Modified();
    //   c1->Update();
    //   c2->Modified();
    //   c2->Update();
    //   c3->Modified();
    //   c3->Update();
     } 
     if (ie%10000000==0) {
    //   c1->Modified();
    //   c1->Update();
    //   c2->Modified();export PATH=/afs/psi.ch/project/sls_det_firmware/jungfrau_software/uClinux-2010_64bit/bfin-uclinux/bin:$PATH
    //   c2->Update();
    //   c3->Modified();
    //   c3->Update();
       cout << " " << ((float)ie)*100./((float)nentries)<< "%" << endl;
       imgLR->Write(name1,TObject::kOverwrite);
       imgHR->Write(name2,TObject::kOverwrite);
       imgHR1->Write(name3,TObject::kOverwrite);
     }
    
  }
  imgLR->Write(name1,TObject::kOverwrite);
  imgHR->Write(name2,TObject::kOverwrite);
  imgHR1->Write(name3,TObject::kOverwrite);
 
  // c1->cd();
  // imgLR->DrawCopy("colz");
  // c2->cd();
  // imgHR->DrawCopy("colz");
  // c3->cd();
  // imgHR1->DrawCopy("colz");
  
  fout->Close();

}





TH2F *iterate(TH2F *heta, TH2F *hpos, int i) {
  int ipx,ipy;


  TH2F *hpos2=(TH2F*)heta->Clone("hpos");
  hpos2->Reset();
  
  if (i%2==0) {

  TH1F *hx=new TH1F("hx","hx",heta->GetNbinsX(),heta->GetXaxis()->GetXmin(),heta->GetXaxis()->GetXmax());
  TH1F *hix=(TH1F*)hx->Clone("hix");
  for ( ipy=0; ipy<25; ipy++) {
    hx->Reset();
    hix->Reset();
    for (int ibx=0; ibx<heta->GetNbinsX(); ibx++) {
      for (int iby=0; iby<heta->GetNbinsY(); iby++) {
	if ((((int)(hpos->GetBinContent(ibx+1,iby+1)))%25)==ipy) {
	  hx->SetBinContent(ibx+1,hx->GetBinContent(ibx+1)+heta->GetBinContent(ibx+1,iby+1));
	}
      }
    }
    ipx=0;
    for (int ibx=1; ibx<heta->GetNbinsX(); ibx++) {
      hix->SetBinContent(ibx+1, hix->GetBinContent(ibx)+hx->GetBinContent(ibx+1));
    }
    // new TCanvas();
    //hix->Draw();

    hix->Scale(1./hix->GetBinContent(hix->GetNbinsX()));
    for (int ibx=0; ibx<heta->GetNbinsX(); ibx++) {
      if (hix->GetBinContent(ibx+1)>((float)(ipx+1))/25.) ipx++;
      for (int iby=0; iby<heta->GetNbinsY(); iby++) {
	if ((((int)hpos->GetBinContent(ibx+1,iby+1))%25)==ipy) {
	  hpos2->SetBinContent(ibx+1,iby+1,25*ipx+ipy);
	
	}
      }
    }
  }

  delete hx;
  delete hix;
  
  
  } else {
  TH1F *hy=new TH1F("hy","hy",heta->GetNbinsY(),heta->GetYaxis()->GetXmin(),heta->GetYaxis()->GetXmax());
  TH1F *hiy=(TH1F*)hy->Clone("hiy");
  for ( ipx=0; ipx<25; ipx++) {
    hy->Reset();
    hiy->Reset();
    for (int iby=0; iby<heta->GetNbinsY(); iby++) {
      for (int ibx=0; ibx<heta->GetNbinsX(); ibx++) {
  	if ((((int)hpos->GetBinContent(ibx+1,iby+1))/25)==ipx) {
  	  hy->SetBinContent(iby+1,hy->GetBinContent(iby+1)+heta->GetBinContent(ibx+1,iby+1));
  	}
      }
    }
    ipy=0;
    for (int iby=1; iby<heta->GetNbinsY(); iby++) {
      hiy->SetBinContent(iby+1, hiy->GetBinContent(iby)+hy->GetBinContent(iby+1));
    }
    hiy->Scale(1./hiy->GetBinContent(hiy->GetNbinsX()));
    for (int iby=0; iby<heta->GetNbinsY(); iby++) {
  	      if (hiy->GetBinContent(iby+1)>((float)(ipy+1))/25.) ipy++;
  	      for (int ibx=0; ibx<heta->GetNbinsX(); ibx++) {
  		if ((((int)hpos->GetBinContent(ibx+1,iby+1))/25)==ipx) {
  		  hpos2->SetBinContent(ibx+1,iby+1,ipx*25+ipy);//hpos2->GetBinContent(ibx+1,iby+1)+ipy);
  		}
  	      }
    }
  }
  delete hy;
  delete hiy;
  }



  // new TCanvas();
  // heta->Draw("colz");
  // hnet->Draw("same");
  
  
  // new TCanvas();
  // hpos2->Draw("colz");
  


  






  
  return hpos2;

  }



TH2F *etaTest() {

  TMultiGraph *mg=new TMultiGraph();
  TGraph *g=new TGraph();
  TGraph *g1=new TGraph();

  mg->Add(g);
  mg->Add(g1);

  const Double_t Delta_eta=0.1;


  char gName[1000];
  sprintf(gName,"/local_zfs_raid/tomcat_20160528/trees/gmap_eta_blank.root");
  TFile *fg=new TFile(gName);
  TH2F *gmap=(TH2F*)fg->Get("gmap");
  TH2F *heta=(TH2F*)fg->Get("heta");
  


  
  


  TH2F *hietaX=(TH2F*)heta->Clone("hietaX");
  TH2F *hietaY=(TH2F*)heta->Clone("hietaY");
  hietaX->SetTitle("hietaX");
  hietaY->SetTitle("hietaY");
  hietaX->Reset();
  hietaY->Reset();
  
  TH2F *hnet=(TH2F*)heta->Clone("hnet");
  hnet->Reset();
  TH2F *hpos=(TH2F*)heta->Clone("hpos");
  hpos->Reset();

  TH1D *hetaY=heta->ProjectionX();
  TH1D *hetaX=heta->ProjectionY();

  TH1D *h1etaX=(TH1D*)hetaX->Clone("h1etaX");
  TH1D *h1etaY=(TH1D*)hetaY->Clone("h1etaY");
  h1etaX->SetTitle("h1etaX");
  h1etaY->SetTitle("h1etaY");
  h1etaX->Reset();
  h1etaY->Reset();
  
  int binminX=heta->GetXaxis()->FindBin(-Delta_eta);
  int binmaxX=heta->GetXaxis()->FindBin(1+Delta_eta);
  int binminY=heta->GetYaxis()->FindBin(-Delta_eta);
  int binmaxY=heta->GetYaxis()->FindBin(1+Delta_eta);
  
  
  TH1D *hetaYp=heta->ProjectionX("py",binminY,binmaxY);
  TH1D *hetaXp=heta->ProjectionY("px",binminX,binmaxX);

  for (int ibx=binminX; ibx<binmaxX; ibx++) {
      for (int iby=binminY; iby<binmaxY; iby++) {
	
	  hietaY->SetBinContent(ibx, iby,hietaY->GetBinContent(ibx, iby-1)+heta->GetBinContent(ibx, iby));
	  
	  hietaX->SetBinContent(ibx, iby,hietaX->GetBinContent(ibx-1, iby)+heta->GetBinContent(ibx, iby));
	 
	  
	  
	}
	   
	     }  


  for (int ibx=binminX; ibx<binmaxX; ibx++) {
      for (int iby=binminY; iby<binmaxY; iby++) {

	  hietaY->SetBinContent(ibx,iby,hietaY->GetBinContent(ibx,iby)/hietaY->GetBinContent(ibx,binmaxY-1));
	  hietaX->SetBinContent(ibx,iby,hietaX->GetBinContent(ibx,iby)/hietaX->GetBinContent(binmaxX-1,iby));
	  
	}

	     }
 
      for (int iby=binminY; iby<binmaxY; iby++) 
	     h1etaY->SetBinContent(iby+1,h1etaY->GetBinContent(iby)+hetaY->GetBinContent(iby+1)/hetaY->Integral(binminY,binmaxY));

	   for (int ibx=binminX; ibx<binmaxX; ibx++)
		  h1etaX->SetBinContent(ibx+1,h1etaX->GetBinContent(ibx)+hetaX->GetBinContent(ibx+1)/hetaX->Integral(binminX,binmaxX));

		for (int ibx=binmaxX; ibx<heta->GetNbinsX()+1; ibx++)
		  h1etaX->SetBinContent(ibx,1);
		for (int iby=binmaxX; iby<heta->GetNbinsY()+1; iby++)
		  h1etaY->SetBinContent(iby,1);


      for (int ibx=binmaxX-1; ibx<heta->GetNbinsX(); ibx++) {
	  
      for (int iby=0; iby<heta->GetNbinsY(); iby++) {
	  hietaX->SetBinContent(ibx+1,iby+1,1);
	}
	     }
	  

      for (int iby=0; iby<binminY+1; iby++) {
	  for (int ibx=0; ibx<heta->GetNbinsX(); ibx++) {
	      hietaX->SetBinContent(ibx+1,iby+1,hietaX->GetBinContent(ibx+1,binminY+1));
	    }
	}


      for (int iby=binmaxY-1; iby<heta->GetNbinsY(); iby++) {
	  for (int ibx=0; ibx<heta->GetNbinsX(); ibx++) {
	      hietaX->SetBinContent(ibx+1,iby+1,hietaX->GetBinContent(ibx+1,binmaxY-1));
	    }
	}



      for (int iby=binmaxX-1; iby<heta->GetNbinsY(); iby++) {
	  
      for (int ibx=0; ibx<heta->GetNbinsX(); ibx++) {
	  hietaY->SetBinContent(ibx+1,iby+1,1);
	}
	     }
	  


      for (int ibx=0; ibx<binminX+1; ibx++) {
	  for (int iby=0; iby<heta->GetNbinsY(); iby++) {
	      hietaY->SetBinContent(ibx+1,iby+1,hietaY->GetBinContent(binminX+1, iby+1));
	    }
	}


      for (int ibx=binmaxX-1; ibx<heta->GetNbinsX(); ibx++) {
	  for (int iby=0; iby<heta->GetNbinsY(); iby++) {
	      hietaY->SetBinContent(ibx+1,iby+1,hietaY->GetBinContent(binmaxX-1, iby+1));
	    }
	}

  
      int ipx=0;
      int ipy=0;

      for (int ibx=0; ibx<heta->GetNbinsX(); ibx++) {
	ipy=0;
	for (int iby=0; iby<heta->GetNbinsY(); iby++) {
	  if (hietaY->GetBinContent(ibx+1,iby+1)>((float)ipy+1)/25.) {
	    hnet->SetBinContent(ibx+1,iby+1,1);
	    ipy++;
	  }
	  hpos->SetBinContent(ibx+1,iby+1,ipy);
	}
      } 
      
      for (int iby=0; iby<heta->GetNbinsY(); iby++) {
	ipx=0;
	for (int ibx=0; ibx<heta->GetNbinsX(); ibx++) {
	  if (hietaX->GetBinContent(ibx+1,iby+1)>((float)ipx+1)/25.)	    ipx++;
	  hpos->SetBinContent(ibx+1,iby+1,hpos->GetBinContent(ibx+1,iby+1)+25*ipx);
	}
      }
      new TCanvas();
      heta->Draw("colz");


      for (int iby=1; iby<heta->GetNbinsY(); iby++) {
	for (int ibx=1; ibx<heta->GetNbinsX(); ibx++) {
	  if (hpos->GetBinContent(ibx+1,iby+1)!=hpos->GetBinContent(ibx,iby+1)) hnet->SetBinContent(ibx+1,iby+1,1);
	  if (hpos->GetBinContent(ibx+1,iby+1)!=hpos->GetBinContent(ibx+1,iby)) hnet->SetBinContent(ibx+1,iby+1,1);
	  else hnet->SetBinContent(ibx+1,iby+1,0);
	}
      }
      

      hnet->DrawCopy("same");


      TH2F *hff=new TH2F("hff","hff",25,0,25,25,0,25);
      Double_t mm=heta->Integral()/(hff->GetNbinsX()*hff->GetNbinsY());
      
      int ibp;
      for (int iby=0; iby<heta->GetNbinsY(); iby++) {
	for (int ibx=0; ibx<heta->GetNbinsX(); ibx++) {
	  ibp=hpos->GetBinContent(ibx+1,iby+1);
	  hff->Fill(ibp/25,ibp%25,heta->GetBinContent(ibx+1,iby+1));
	}
      }
      
      Double_t chi2=0;
      Double_t maxS=0;
      for (int ibx=0; ibx<25; ibx++)
	for (int iby=0; iby<25; iby++) {
	  chi2+=(hff->GetBinContent(ibx+1,iby+1)-mm)*(hff->GetBinContent(ibx+1,iby+1)-mm)/mm;
	  if ((hff->GetBinContent(ibx+1,iby+1)-mm)*(hff->GetBinContent(ibx+1,iby+1))>maxS) maxS=(hff->GetBinContent(ibx+1,iby+1)-mm)*(hff->GetBinContent(ibx+1,iby+1)-mm);
	}										  
      chi2/=625.;
      maxS=TMath::Sqrt(maxS/mm);
      
	  cout << "Start Chi2 "<< chi2 << " maxS "<< maxS << endl;
      
      g->SetPoint(0,0,chi2);
    g1->SetPoint(0,0,maxS);

      new TCanvas();
      char tit[1000];
      sprintf(tit,"Start Chi2=%d",(int)chi2);
      hff->SetTitle(tit);
      hff->DrawCopy("colz");
      

  TH2F *hpos2;
  int i;
  new TCanvas();
  for (i=0; i<500; i++) {
    hpos2=iterate(heta,hpos,i);
    delete hpos;
    hpos=hpos2;
    
    
    hff->Reset();

    for (int iby=0; iby<heta->GetNbinsY(); iby++) {
      for (int ibx=0; ibx<heta->GetNbinsX(); ibx++) {
	ibp=hpos->GetBinContent(ibx+1,iby+1);
	hff->Fill(ibp/25,ibp%25,heta->GetBinContent(ibx+1,iby+1));
      }
    }
    
    chi2=0;
    maxS=0;
    for (int ibx=0; ibx<25; ibx++)
      for (int iby=0; iby<25; iby++) {
	chi2+=(hff->GetBinContent(ibx+1,iby+1)-mm)*(hff->GetBinContent(ibx+1,iby+1)-mm)/mm;
	if ((hff->GetBinContent(ibx+1,iby+1)-mm)*(hff->GetBinContent(ibx+1,iby+1))>maxS) maxS=(hff->GetBinContent(ibx+1,iby+1)-mm)*(hff->GetBinContent(ibx+1,iby+1)-mm);

      }
    chi2/=625.;
    maxS=TMath::Sqrt(maxS/mm);
  
    cout << "Iteration " << i << " Chi2 "<< chi2 << " maxS "<< maxS<< endl;
 
    g->SetPoint(i+1,i+1,chi2);
    g1->SetPoint(i+1,i+1,maxS);
    if (i%100==0) {

      g->Draw("ALP");
      g1->Draw("LP");
      gPad->Modified();
      gPad->Update();
    }
    
  }

  new TCanvas();
  heta->Draw("colz");


  for (int iby=1; iby<heta->GetNbinsY(); iby++) {
    for (int ibx=1; ibx<heta->GetNbinsX(); ibx++) {
      if (hpos->GetBinContent(ibx+1,iby+1)!=hpos->GetBinContent(ibx,iby+1)) hnet->SetBinContent(ibx+1,iby+1,1);
      else if (hpos->GetBinContent(ibx+1,iby+1)!=hpos->GetBinContent(ibx+1,iby)) hnet->SetBinContent(ibx+1,iby+1,1);
      else hnet->SetBinContent(ibx+1,iby+1,0);
    }
  }
      

  hnet->DrawCopy("same");

  new TCanvas();
  sprintf(tit,"Iteration %d Chi2=%d",i,(int)chi2);
  hff->SetTitle(tit);
  hff->DrawCopy("colz");

  TFile *fffff=new TFile("/local_zfs_raid/tomcat_20160528/trees/hpos_anna.root","RECREATE");
  hpos->Write("hpos");
  fffff->Close();



  return hpos;
  
  
}


TH2D *interpV2(char *name, int ip, double sc=30./25.) {

  char chainName[1000];
  sprintf(chainName,"/local_zfs_raid/tomcat_20160528/trees/%s_t*.root",name);

  const Double_t Delta_eta=0.05;


  char gName[1000];
  sprintf(gName,"/local_zfs_raid/tomcat_20160528/trees/gmap_eta_blank_%d.root",ip);
  TFile *fg=new TFile(gName);
  TH2F *gmap=(TH2F*)fg->Get("gmap");
  TH2F *heta=(TH2F*)fg->Get("heta");
  
  TChain *ch=new TChain(name);
  ch->Add(chainName);

  // TFile *ft=new TFile("/local_zfs_raid/tomcat_20160528/trees/grating_1d_t0_v4.root");
  // TTree *ch=(TTree*)ft->Get("grating_1d");

  // ch->Draw("y:x>>hh(400,0,400,400,0,400)","","colz");
  // gPad->Modified();
  // gPad->Update();
  

  
  


  TH2F *hietaX=(TH2F*)heta->Clone("hietaX");
  TH2F *hietaY=(TH2F*)heta->Clone("hietaY");
  hietaX->SetTitle("hietaX");
  hietaY->SetTitle("hietaY");
  hietaX->Reset();
  hietaY->Reset();
  


  TH1D *hetaY=heta->ProjectionX();
  TH1D *hetaX=heta->ProjectionY();

  TH1D *h1etaX=(TH1D*)hetaX->Clone("h1etaX");
  TH1D *h1etaY=(TH1D*)hetaY->Clone("h1etaY");
  h1etaX->SetTitle("h1etaX");
  h1etaY->SetTitle("h1etaY");
  h1etaX->Reset();
  h1etaY->Reset();
  
  int binminX=heta->GetXaxis()->FindBin(-Delta_eta);
  int binmaxX=heta->GetXaxis()->FindBin(1+Delta_eta);
  int binminY=heta->GetYaxis()->FindBin(-Delta_eta);
  int binmaxY=heta->GetYaxis()->FindBin(1+Delta_eta);
  
  
  TH1D *hetaYp=heta->ProjectionX("py",binminY,binmaxY);
  TH1D *hetaXp=heta->ProjectionY("px",binminX,binmaxX);

  for (int ibx=binminX; ibx<binmaxX; ibx++) {
      for (int iby=binminY; iby<binmaxY; iby++) {
	
	  hietaY->SetBinContent(ibx, iby,hietaY->GetBinContent(ibx, iby-1)+heta->GetBinContent(ibx, iby));
	  
	  hietaX->SetBinContent(ibx, iby,hietaX->GetBinContent(ibx-1, iby)+heta->GetBinContent(ibx, iby));
	 
	  
	  
	}
	   
	     }  


  for (int ibx=binminX; ibx<binmaxX; ibx++) {
      for (int iby=binminY; iby<binmaxY; iby++) {

	  hietaY->SetBinContent(ibx,iby,hietaY->GetBinContent(ibx,iby)/hietaY->GetBinContent(ibx,binmaxY-1));
	  hietaX->SetBinContent(ibx,iby,hietaX->GetBinContent(ibx,iby)/hietaX->GetBinContent(binmaxX-1,iby));
	  
	}

	     }
 
      for (int iby=binminY; iby<binmaxY; iby++) 
	     h1etaY->SetBinContent(iby+1,h1etaY->GetBinContent(iby)+hetaY->GetBinContent(iby+1)/hetaY->Integral(binminY,binmaxY));

	   for (int ibx=binminX; ibx<binmaxX; ibx++)
		  h1etaX->SetBinContent(ibx+1,h1etaX->GetBinContent(ibx)+hetaX->GetBinContent(ibx+1)/hetaX->Integral(binminX,binmaxX));

		for (int ibx=binmaxX; ibx<heta->GetNbinsX()+1; ibx++)
		  h1etaX->SetBinContent(ibx,1);
		for (int iby=binmaxX; iby<heta->GetNbinsY()+1; iby++)
		  h1etaY->SetBinContent(iby,1);


      for (int ibx=binmaxX-1; ibx<heta->GetNbinsX(); ibx++) {
	  
      for (int iby=0; iby<heta->GetNbinsY(); iby++) {
	  hietaX->SetBinContent(ibx+1,iby+1,1);
	}
	     }
	  

      for (int iby=0; iby<binminY+1; iby++) {
	  for (int ibx=0; ibx<heta->GetNbinsX(); ibx++) {
	      hietaX->SetBinContent(ibx+1,iby+1,hietaX->GetBinContent(ibx+1,binminY+1));
	    }
	}


      for (int iby=binmaxY-1; iby<heta->GetNbinsY(); iby++) {
	  for (int ibx=0; ibx<heta->GetNbinsX(); ibx++) {
	      hietaX->SetBinContent(ibx+1,iby+1,hietaX->GetBinContent(ibx+1,binmaxY-1));
	    }
	}



      for (int iby=binmaxX-1; iby<heta->GetNbinsY(); iby++) {
	  
      for (int ibx=0; ibx<heta->GetNbinsX(); ibx++) {
	  hietaY->SetBinContent(ibx+1,iby+1,1);
	}
	     }
	  


      for (int ibx=0; ibx<binminX+1; ibx++) {
	  for (int iby=0; iby<heta->GetNbinsY(); iby++) {
	      hietaY->SetBinContent(ibx+1,iby+1,hietaY->GetBinContent(binminX+1, iby+1));
	    }
	}


      for (int ibx=binmaxX-1; ibx<heta->GetNbinsX(); ibx++) {
	  for (int iby=0; iby<heta->GetNbinsY(); iby++) {
	      hietaY->SetBinContent(ibx+1,iby+1,hietaY->GetBinContent(binmaxX-1, iby+1));
	    }
	}

  


  new TCanvas();
  hietaY->Draw("colz");
  gPad->Modified();
  gPad->Update();
  
  new TCanvas();
  hietaX->Draw("colz");
  gPad->Modified();
  gPad->Update();
  new TCanvas();
  h1etaX->Draw();
  gPad->Modified();
  gPad->Update();
  new TCanvas();
  h1etaY->Draw();
  gPad->Modified();
  gPad->Update();
  

  char foutName[1000];
  sprintf(foutName,"/local_zfs_raid/tomcat_20160528/trees/img_%s_eta_gmap_v2.root",name);
    

   TFile *fout=new TFile(foutName, "RECREATE");
  Double_t data[9], gain=1;
  Double_t dum[9], sDum[2][2], etax, etay, sum, totquad;
  Int_t x,y,f;
  int ix, iy, skip;

  Long64_t nentries=ch->GetEntries();
  ch->SetBranchAddress("iFrame",&f);
  ch->SetBranchAddress("x",&x);
  ch->SetBranchAddress("y",&y);
  ch->SetBranchAddress("data",dum);
  Long64_t ie;

  TH2D *imgLR=new TH2D("imgLR","imgLR",400,0,400,200,100,300);
  TH2D *imgHR=new TH2D("imgHR","imgHR",400*25,0,400,200*25,100,300);
  TH2D *imgHR1=new TH2D("imgHR1","imgHR1",400*25,0,400,200*25,100,300);
  

  TCanvas *c1=new TCanvas();
  imgLR->Draw("colz");
   TCanvas *c2=new TCanvas();
   imgHR->Draw("colz");
   TCanvas *c3=new TCanvas();
   imgHR1->Draw("colz");

  cout << "Chain has " << nentries*1E-6 << "M entries " << endl;
  Double_t etamax=hietaX->GetXaxis()->GetBinCenter(hietaX->GetNbinsX());
  Double_t etamin=hietaX->GetXaxis()->GetBinCenter(0);

  char name1[100],name2[100],name3[100];
  sprintf(name1,"%sLR",name);
  sprintf(name2,"%sHR",name);
  sprintf(name3,"%sHR1",name);

  for (ie=0; ie<nentries; ie++) {
    skip=1;
    if (ch->GetEntry(ie)) {  
    skip=0;
      for (ix=0; ix<3; ix++) {
      	if (skip==0) {
      	  for (iy=0; iy<3; iy++) {
      	    if (gmap==NULL) {
      	      data[ix+iy*3]=dum[ix+iy*3];
      	    } else {
      	      gain=gmap->GetBinContent(gmap->GetXaxis()->FindBin(x), gmap->GetYaxis()->FindBin(y));
      	      if (gain>2000 && gain<3000) {
      		data[ix+iy*3]=dum[ix+iy*3]/gain;
      		//	cout << dum[ix+iy*3] << " " << gain << " " << data[ix+iy*3] << endl;
      	      } else
      		skip=1;
      	    }
      	  }
      	}
      }
   
    }  
     if (skip==0) {
      slsInterpolation::calcEta(data, etax, etay, sum, totquad, sDum);
       }
      if (etax<etamin || etay<etamin || etax>etamax || etay>etamax || totquad/sum<0.8 || totquad/sum>1.2) skip=1;
    
      if (skip==0) {
	imgLR->Fill(x,y);
	imgHR->Fill(x-0.5+hietaX->GetBinContent(hietaX->GetXaxis()->FindBin(etax),hietaX->GetYaxis()->FindBin(etay))*sc-0.5*sc,y-0.5+hietaY->GetBinContent(hietaY->GetXaxis()->FindBin(etax),hietaY->GetYaxis()->FindBin(etay))*sc-0.5*sc);
	imgHR1->Fill(x-0.5+h1etaX->GetBinContent(h1etaX->FindBin(etax))*sc-0.5*sc, y-0.5+h1etaY->GetBinContent(h1etaY->FindBin(etay)*sc-0.5*sc));
    }
     if (ie%1000000==0) {
    //   c1->Modified();
    //   c1->Update();
    //   c2->Modified();
    //   c2->Update();
    //   c3->Modified();
    //   c3->Update();
     } 
     if (ie%10000000==0) {
    //   c1->Modified();
    //   c1->Update();
    //   c2->Modified();
    //   c2->Update();
    //   c3->Modified();
    //   c3->Update();
       cout << " " << ((float)ie)*100./((float)nentries)<< "%" << endl;
       imgLR->Write(name1,TObject::kOverwrite);
       imgHR->Write(name2,TObject::kOverwrite);
       imgHR1->Write(name3,TObject::kOverwrite);
     }
    
  }
  imgLR->Write(name1,TObject::kOverwrite);
  imgHR->Write(name2,TObject::kOverwrite);
  imgHR1->Write(name3,TObject::kOverwrite);
 
  // c1->cd();
  // imgLR->DrawCopy("colz");
  // c2->cd();
  // imgHR->DrawCopy("colz");
  // c3->cd();
  // imgHR1->DrawCopy("colz");
  
  fout->Close();

}


TH2D *interpV3(char *name, int ip, double sc=26./25.) {

  char chainName[1000];
  sprintf(chainName,"/local_zfs_raid/tomcat_20160528/trees/%s_t*.root",name);

  const Double_t Delta_eta=0.05;


  char gName[1000];
  sprintf(gName,"/local_zfs_raid/tomcat_20160528/trees/gmap_eta_blank_%d.root",ip);
  TFile *fg=new TFile(gName);
  TH2F *gmap=(TH2F*)fg->Get("gmap");
  TH2F *heta=(TH2F*)fg->Get("heta");
  
  TChain *ch=new TChain(name);
  ch->Add(chainName);

  // TFile *ft=new TFile("/local_zfs_raid/tomcat_20160528/trees/grating_1d_t0_v4.root");
  // TTree *ch=(TTree*)ft->Get("grating_1d");

  // ch->Draw("y:x>>hh(400,0,400,400,0,400)","","colz");
  // gPad->Modified();
  // gPad->Update();
  

  
  


  TH2F *hietaX=(TH2F*)heta->Clone("hietaX");
  TH2F *hietaY=(TH2F*)heta->Clone("hietaY");
  hietaX->SetTitle("hietaX");
  hietaY->SetTitle("hietaY");
  hietaX->Reset();
  hietaY->Reset();
  


  TH1D *hetaY=heta->ProjectionX();
  TH1D *hetaX=heta->ProjectionY();

  TH1D *h1etaX=(TH1D*)hetaX->Clone("h1etaX");
  TH1D *h1etaY=(TH1D*)hetaY->Clone("h1etaY");
  h1etaX->SetTitle("h1etaX");
  h1etaY->SetTitle("h1etaY");
  h1etaX->Reset();
  h1etaY->Reset();
  
  int binminX=heta->GetXaxis()->FindBin(-Delta_eta);
  int binmaxX=heta->GetXaxis()->FindBin(1+Delta_eta);
  int binminY=heta->GetYaxis()->FindBin(-Delta_eta);
  int binmaxY=heta->GetYaxis()->FindBin(1+Delta_eta);
  
  
  TH1D *hetaYp=heta->ProjectionX("py",binminY,binmaxY);
  TH1D *hetaXp=heta->ProjectionY("px",binminX,binmaxX);

  for (int ibx=binminX; ibx<binmaxX; ibx++) {
      for (int iby=binminY; iby<binmaxY; iby++) {
	
	  hietaY->SetBinContent(ibx, iby,hietaY->GetBinContent(ibx, iby-1)+heta->GetBinContent(ibx, iby));
	  
	  hietaX->SetBinContent(ibx, iby,hietaX->GetBinContent(ibx-1, iby)+heta->GetBinContent(ibx, iby));
	 
	  
	  
	}
	   
	     }  


  for (int ibx=binminX; ibx<binmaxX; ibx++) {
      for (int iby=binminY; iby<binmaxY; iby++) {

	  hietaY->SetBinContent(ibx,iby,hietaY->GetBinContent(ibx,iby)/hietaY->GetBinContent(ibx,binmaxY-1));
	  hietaX->SetBinContent(ibx,iby,hietaX->GetBinContent(ibx,iby)/hietaX->GetBinContent(binmaxX-1,iby));
	  
	}

	     }
 
      for (int iby=binminY; iby<binmaxY; iby++) 
	     h1etaY->SetBinContent(iby+1,h1etaY->GetBinContent(iby)+hetaY->GetBinContent(iby+1)/hetaY->Integral(binminY,binmaxY));

	   for (int ibx=binminX; ibx<binmaxX; ibx++)
		  h1etaX->SetBinContent(ibx+1,h1etaX->GetBinContent(ibx)+hetaX->GetBinContent(ibx+1)/hetaX->Integral(binminX,binmaxX));

		for (int ibx=binmaxX; ibx<heta->GetNbinsX()+1; ibx++)
		  h1etaX->SetBinContent(ibx,1);
		for (int iby=binmaxX; iby<heta->GetNbinsY()+1; iby++)
		  h1etaY->SetBinContent(iby,1);


      for (int ibx=binmaxX-1; ibx<heta->GetNbinsX(); ibx++) {
	  
      for (int iby=0; iby<heta->GetNbinsY(); iby++) {
	  hietaX->SetBinContent(ibx+1,iby+1,1);
	}
	     }
	  

      for (int iby=0; iby<binminY+1; iby++) {
	  for (int ibx=0; ibx<heta->GetNbinsX(); ibx++) {
	      hietaX->SetBinContent(ibx+1,iby+1,hietaX->GetBinContent(ibx+1,binminY+1));
	    }
	}


      for (int iby=binmaxY-1; iby<heta->GetNbinsY(); iby++) {
	  for (int ibx=0; ibx<heta->GetNbinsX(); ibx++) {
	      hietaX->SetBinContent(ibx+1,iby+1,hietaX->GetBinContent(ibx+1,binmaxY-1));
	    }
	}



      for (int iby=binmaxX-1; iby<heta->GetNbinsY(); iby++) {
	  
      for (int ibx=0; ibx<heta->GetNbinsX(); ibx++) {
	  hietaY->SetBinContent(ibx+1,iby+1,1);
	}
	     }
	  


      for (int ibx=0; ibx<binminX+1; ibx++) {
	  for (int iby=0; iby<heta->GetNbinsY(); iby++) {
	      hietaY->SetBinContent(ibx+1,iby+1,hietaY->GetBinContent(binminX+1, iby+1));
	    }
	}


      for (int ibx=binmaxX-1; ibx<heta->GetNbinsX(); ibx++) {
	  for (int iby=0; iby<heta->GetNbinsY(); iby++) {
	      hietaY->SetBinContent(ibx+1,iby+1,hietaY->GetBinContent(binmaxX-1, iby+1));
	    }
	}

  


  new TCanvas();
  hietaY->Draw("colz");
  gPad->Modified();
  gPad->Update();
  
  new TCanvas();
  hietaX->Draw("colz");
  gPad->Modified();
  gPad->Update();
  new TCanvas();
  h1etaX->Draw();
  gPad->Modified();
  gPad->Update();
  new TCanvas();
  h1etaY->Draw();
  gPad->Modified();
  gPad->Update();
  

  char foutName[1000];
  sprintf(foutName,"/local_zfs_raid/tomcat_20160528/trees/img_%s_eta_gmap_v2.root",name);
    

   TFile *fout=new TFile(foutName, "RECREATE");
  Double_t data[9], gain=1;
  Double_t dum[9], sDum[2][2], etax, etay, sum, totquad;
  Int_t x,y,f;
  int ix, iy, skip;

  Long64_t nentries=ch->GetEntries();
  ch->SetBranchAddress("iFrame",&f);
  ch->SetBranchAddress("x",&x);
  ch->SetBranchAddress("y",&y);
  ch->SetBranchAddress("data",dum);
  Long64_t ie;

  TH2D *imgLR=new TH2D("imgLR","imgLR",400,0,400,200,100,300);
  TH2D *imgHR=new TH2D("imgHR","imgHR",400*25,0,400,200*25,100,300);
  TH2D *imgHR1=new TH2D("imgHR1","imgHR1",400*25,0,400,200*25,100,300);
  

  TCanvas *c1=new TCanvas();
  imgLR->Draw("colz");
   TCanvas *c2=new TCanvas();
   imgHR->Draw("colz");
   TCanvas *c3=new TCanvas();
   imgHR1->Draw("colz");

  cout << "Chain has " << nentries*1E-6 << "M entries " << endl;
  Double_t etamax=hietaX->GetXaxis()->GetBinCenter(hietaX->GetNbinsX());
  Double_t etamin=hietaX->GetXaxis()->GetBinCenter(0);

  char name1[100],name2[100],name3[100];
  sprintf(name1,"%sLR",name);
  sprintf(name2,"%sHR",name);
  sprintf(name3,"%sHR1",name);

  for (ie=0; ie<nentries; ie++) {
    skip=1;
    if (ch->GetEntry(ie)) {  
    skip=0;
      for (ix=0; ix<3; ix++) {
      	if (skip==0) {
      	  for (iy=0; iy<3; iy++) {
      	    if (gmap==NULL) {
      	      data[ix+iy*3]=dum[ix+iy*3];
      	    } else {
      	      gain=gmap->GetBinContent(gmap->GetXaxis()->FindBin(x), gmap->GetYaxis()->FindBin(y));
      	      if (gain>2000 && gain<3000) {
      		data[ix+iy*3]=dum[ix+iy*3]/gain;
      		//	cout << dum[ix+iy*3] << " " << gain << " " << data[ix+iy*3] << endl;
      	      } else
      		skip=1;
      	    }
      	  }
      	}
      }
   
    }  
     if (skip==0) {
      slsInterpolation::calcEta(data, etax, etay, sum, totquad, sDum);
       }
       if (etax<etamin || etay<etamin || etax>etamax || etay>etamax || totquad/sum<0.8 || totquad/sum>1.2) skip=1;
    
      if (skip==0) {
	imgLR->Fill(x,y);
	imgHR->Fill(x-0.5+hietaX->GetBinContent(hietaX->GetXaxis()->FindBin(etax),hietaX->GetYaxis()->FindBin(etay))*sc,y-0.5+hietaY->GetBinContent(hietaY->GetXaxis()->FindBin(etax),hietaY->GetYaxis()->FindBin(etay))*sc);
	imgHR1->Fill(x-0.5+h1etaX->GetBinContent(h1etaX->FindBin(etax))*sc, y-0.5+h1etaY->GetBinContent(h1etaY->FindBin(etay)*sc));
    }
     if (ie%1000000==0) {
    //   c1->Modified();
    //   c1->Update();
    //   c2->Modified();
    //   c2->Update();
    //   c3->Modified();
    //   c3->Update();
     } 
     if (ie%10000000==0) {
    //   c1->Modified();
    //   c1->Update();
    //   c2->Modified();
    //   c2->Update();
    //   c3->Modified();
    //   c3->Update();
       cout << " " << ((float)ie)*100./((float)nentries)<< "%" << endl;
       imgLR->Write(name1,TObject::kOverwrite);
       imgHR->Write(name2,TObject::kOverwrite);
       imgHR1->Write(name3,TObject::kOverwrite);
     }
    
  }
  imgLR->Write(name1,TObject::kOverwrite);
  imgHR->Write(name2,TObject::kOverwrite);
  imgHR1->Write(name3,TObject::kOverwrite);
 
  // c1->cd();
  // imgLR->DrawCopy("colz");
  // c2->cd();
  // imgHR->DrawCopy("colz");
  // c3->cd();
  // imgHR1->DrawCopy("colz");
  
  fout->Close();

}

TH2D *calcEtaGcorr(int ip) {
  char name[1000];
  strcpy(name,"blank");
  char chainName[1000];
  sprintf(chainName,"/local_zfs_raid/tomcat_20160528/trees/%s_t*.root",name);

  const Double_t Delta_eta=0.05;


  char gName[1000];
  sprintf(gName,"/local_zfs_raid/tomcat_20160528/trees/gmap_eta_blank_%d.root",ip);
  TFile *fg=new TFile(gName);
  TH2F *gmap=(TH2F*)fg->Get("gmap");

  TH2F *heta=(TH2F*)fg->Get("heta");

  TH2F *heta_new=(TH2F*)heta->Clone("heta_gmap");

  heta_new->Reset();
  
  TChain *ch=new TChain(name);
  ch->Add(chainName);

  // TFile *ft=new TFile("/local_zfs_raid/tomcat_20160528/trees/grating_1d_t0_v4.root");
  // TTree *ch=(TTree*)ft->Get("grating_1d");

  // ch->Draw("y:x>>hh(400,0,400,400,0,400)","","colz");
  // gPad->Modified();
  // gPad->Update();
  

  
  char foutName[1000];
  sprintf(foutName,"/local_zfs_raid/tomcat_20160528/trees/gmap_eta_gcorr.root");
    

   TFile *fout=new TFile(foutName, "RECREATE");
  Double_t data[9], gain=1;
  Double_t dum[9], sDum[2][2], etax, etay, sum, totquad;
  Int_t x,y,f;
  int ix, iy, skip;

  Long64_t nentries=ch->GetEntries();
  ch->SetBranchAddress("iFrame",&f);
  ch->SetBranchAddress("x",&x);
  ch->SetBranchAddress("y",&y);
  ch->SetBranchAddress("data",dum);
  Long64_t ie;


  cout << "Chain has " << nentries*1E-6 << "M entries " << endl;



  for (ie=0; ie<nentries; ie++) {
    skip=1;
    if (ch->GetEntry(ie)) {  
    skip=0;
      for (ix=0; ix<3; ix++) {
      	if (skip==0) {
      	  for (iy=0; iy<3; iy++) {
      	    if (gmap==NULL) {
      	      data[ix+iy*3]=dum[ix+iy*3];
      	    } else {
      	      gain=gmap->GetBinContent(gmap->GetXaxis()->FindBin(x), gmap->GetYaxis()->FindBin(y));
      	      if (gain>2000 && gain<3000) {
      		data[ix+iy*3]=dum[ix+iy*3]/gain;
      		//	cout << dum[ix+iy*3] << " " << gain << " " << data[ix+iy*3] << endl;
      	      } else
      		skip=1;
      	    }
      	  }
      	}
      }
   
    }  
     if (skip==0) {
      slsInterpolation::calcEta(data, etax, etay, sum, totquad, sDum);
      if (totquad/sum<0.8 || totquad/sum>1.2) skip=1;
    
      if (skip==0) {
	heta_new->Fill(etax,etay);
      }
     }
     if (ie%10000000==0) {
       cout << " " << ((float)ie)*100./((float)nentries)<< "%" << endl;
       gmap->Write("gmap",TObject::kOverwrite);
       heta_new->Write("heta",TObject::kOverwrite);
       //    imgHR1->Write(name3,TObject::kOverwrite);
     }
    
  }
  gmap->Write("gmap",TObject::kOverwrite);
  heta_new->Write("heta",TObject::kOverwrite);
  heta_new->DrawCopy("colz");
  fout->Close();

}
