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
#include <TLegend.h>
#include <stdio.h>
#include <iostream>
#include <deque>
#include <list>
#include <queue>
#include <fstream>

#include "EtaVEL.h"
#include "EtaVEL.cpp"
/*
Zum erstellen der correction map ist createGainAndEtaFile(...) in EVELAlg.C der entry point.
Zum erstellen des HR images ist createImage(...) der entry point.
*/
int etabins = 25;
int nEtas = 25;
Double_t dum[3][3];
Int_t x,y,f,q;

int counter[5];
int remoteCounter[5];

//TH2D *sum = new TH2D("sum","sum",3,-0.1,2.1,3,-0.1,2.1);
//TH2F *subPos = new TH2F("subPos","subPos", 100, -1.,1. ,100, -1.,1.);
TH2D *subPosAEta = new TH2D("subPosAEta","subPosAEta", 50, -.5,1.5 ,50, -.5,1.5);
TH2D *subPosBEta = new TH2D("subPosBEta","subPosBEta", 50, -.5,1.5 ,50, -.5,1.5);



TH1D *cE = new TH1D("clusterEnergy","clusterEnergy",400, 0.,4000.);
//TH1D *cES = new TH1D("clusterEnergyS","clusterEnergyS",400, 0.,4000.);


TH2D *cES3vs2 = new TH2D("clusterEnergy3vs2","clusterEnergy3vs2",800, 0.,8000.,600,0.,6000.);
TH2D *cES3vs2S = new TH2D("clusterEnergy3vs2S","clusterEnergy3vs2S",800, 0.,8000.,600,0.,6000.);

double th = 0.99;
double sigmas = 1.0;

TH2D *imgRLR = new TH2D("imgRLR","imgRLR",160,0.0,160.0 ,160 ,0.0,160.0);
TH2D *imgLR = new TH2D("imgLR","imgLR",160*2,0.0,160.0 ,160*2 ,0.0,160.0);

TH2D *clusHist= new TH2D("clusHist","clusHist",3,-0.5,2.5,3,-0.5,2.5);
TH2D *clusHistC= new TH2D("clusHistC","clusHistC",3,-0.5,2.5,3,-0.5,2.5);

int **imgArray;

int findShape(Double_t cluster[3][3], double sDum[2][2]){
  int corner = -1;

  double sum = 	cluster[0][0] + cluster[1][0] + cluster[2][0] + cluster[0][1] + cluster[1][1] + cluster[2][1] + cluster[0][2] + cluster[1][2] + cluster[2][2];
	
  double sumTL = cluster[0][0] + cluster[1][0] + cluster[0][1] + cluster[1][1]; //2 ->BL
  double sumTR = cluster[1][0] + cluster[2][0] + cluster[2][1] + cluster[1][1]; //0 ->TL
  double sumBL = cluster[0][1] + cluster[0][2] + cluster[1][2] + cluster[1][1]; //3 ->BR
  double sumBR = cluster[1][2] + cluster[2][1] + cluster[2][2] + cluster[1][1]; //1 ->TR
  double sumMax = 0;


  //double **sDum = subCluster;
  Double_t ssDum[2][2];

  // if(sumTL  >= sumMax){
  sDum[0][0] = cluster[0][0]; sDum[1][0] = cluster[1][0];
    sDum[0][1] = cluster[0][1]; sDum[1][1] = cluster[1][1];

    ssDum[0][0] = cluster[0][0]; ssDum[1][0] = cluster[0][1];
    ssDum[0][1] = cluster[1][0]; ssDum[1][1] = cluster[1][1];

    corner = 2;
    sumMax=sumTL;
    // } 

  if(sumTR  >= sumMax){
    sDum[0][0] = cluster[1][0]; sDum[1][0] = cluster[2][0];
    sDum[0][1] = cluster[1][1]; sDum[1][1] = cluster[2][1];

    ssDum[0][0] = cluster[2][0]; ssDum[1][0] = cluster[2][1];
    ssDum[0][1] = cluster[1][0]; ssDum[1][1] = cluster[1][1];

    corner = 0;
    sumMax=sumTR;
  } 

  if(sumBL  >= sumMax){
    sDum[0][0] = cluster[0][1]; sDum[1][0] = cluster[1][1];
    sDum[0][1] = cluster[0][2]; sDum[1][1] = cluster[1][2];

    ssDum[0][0] = cluster[0][2]; ssDum[1][0] = cluster[0][1];
    ssDum[0][1] = cluster[1][2]; ssDum[1][1] = cluster[1][1];

    corner = 3;
    sumMax=sumBL;
  }

  if(sumBR  >= sumMax){
    sDum[0][0] = cluster[1][1]; sDum[1][0] = cluster[2][1];
    sDum[0][1] = cluster[1][2]; sDum[1][1] = cluster[2][2];

    ssDum[0][0] = cluster[2][2]; ssDum[1][0] = cluster[2][1];
    ssDum[0][1] = cluster[1][2]; ssDum[1][1] = cluster[1][1];

    corner = 1;
    sumMax=sumBR;
  }

  switch(corner){
  case 0:
    cES3vs2->Fill(sum,sumTR); break;
  case 1:
    cES3vs2->Fill(sum,sumBR); break;
  case 2:
    cES3vs2->Fill(sum,sumTL); break;
  case 3:
    cES3vs2->Fill(sum,sumBL); break;
  }
  
  counter[corner]++;
  remoteCounter[q]++;

  //  cout << "local corner is: " << corner << " remote corner is: " << q << endl;

  return corner;
}




int placePhoton( TH2D *img, double subCluster[2][2], int cX, int cY, int corner, double *sX, double *sY, double *scX, double *scY){
  double tot = subCluster[0][0] + subCluster[0][1] + subCluster[1][0] + subCluster[1][1];
  double t = subCluster[1][0] + subCluster[1][1];
  double r = subCluster[0][1] + subCluster[1][1];
  
  double xHitC = r/tot;
  double yHitC = t/tot;

  imgRLR->Fill(cX,cY);

  cE->Fill(tot);

  double dX, dY;

  //before looking at annas code
  /*  if(corner == 0){ dX=-1.; dY=-1.;   }
  if(corner == 1){ dX=-1.; dY=+1.;  }
  if(corner == 2){ dX=+1.; dY=-1.;  }
  if(corner == 3){ dX=+1.; dY=+1.;    }*/

  if(corner == 0){ dX=-1.; dY=+1.;   } //top left
  if(corner == 1){ dX=+1.; dY=+1.;  } //top right
  if(corner == 2){ dX=-1.; dY=-1.;  } //bottom left
  if(corner == 3){ dX=+1.; dY=-1.;    } //bottom right

  imgLR->Fill(cX+0.25*dX,cY+0.25*dY);

  double posX = ((double)cX) + 0.5*dX + xHitC;
  double posY = ((double)cY) + 0.5*dY + yHitC;

  subPosBEta->Fill(xHitC ,yHitC);
  if(img){
    img->Fill(posX,posY);
  }

  if(xHitC < 0.02&& yHitC < 0.02){

    cES3vs2S->Fill(dum[0][0]+dum[0][1]+dum[0][2]+dum[1][0]+dum[1][1]+dum[1][2]+dum[2][0]+dum[2][1]+dum[2][2],subCluster[0][0]+subCluster[0][1]+subCluster[1][0]+subCluster[1][1]);
  }
  

  if(sX && sY && scX && scY){
    *sX = xHitC; //0.5 + 0.5*dX + xHitC;
    *sY = yHitC; //0.5 + 0.5*dY + yHitC;
    *scX = ((double)cX) + 0.5*dX;
    *scY = ((double)cY) + 0.5*dY;
  }
  return 1;
}



void placePhotonCorr(TH2D *img, EtaVEL *e,double sX, double sY, double scX, double scY){
  int bin = e->findBin(sX,sY);
  if(bin <= 0) return;
  double subX = ((double)(e->getXBin(bin))+.5)/((double)e->getNPixels());
  double subY = ((double)(e->getYBin(bin))+.5)/((double)e->getNPixels());

  if(img!=NULL){
    img->Fill(scX+ subX , scY+ subY);
  }
  subPosAEta->Fill(subX,subY);

  int iscx = scX;
  int iscy = scY;
  if(iscx >=nx || iscx<0 || iscy >=ny || iscy<0) return;
  //cout << iscx*e->getNPixels()+e->getXBin(bin) << " " << iscy*e->getNPixels()+e->getXBin(bin) << endl;
  if(img==NULL) return;
  imgArray[iscx*e->getNPixels()+e->getXBin(bin)][iscy*e->getNPixels()+e->getYBin(bin)]++;
}

void gainCorrection(Double_t corrected[3][3], TH2D *gainMap){

  for(int xx = 0; xx < 3; xx++)
    for(int yy = 0; yy < 3; yy++){
      if(gainMap && gainMap->GetBinContent(x+xx+2,y+yy+2) != 0){
	corrected[xx][yy] = dum[xx][yy] / gainMap->GetBinContent(x+xx+2,y+yy+2);
	clusHistC->Fill(xx,yy,corrected[xx][yy]);
      }
      else
	corrected[xx][yy] = dum[xx][yy];

      clusHist->Fill(xx,yy,dum[xx][yy]);
    }
}


EtaVEL *plotEtaDensity(TChain* tree2, TEntryList *el, EtaVEL *oldEta = NULL, TH2D **img = NULL, TH2D *gainMap=NULL, int nPixels=25) {


  
  EtaVEL *newEta = new EtaVEL(25,-0.02,1.02);

  Long64_t listEntries=el->GetN();
  Long64_t treeEntry;
  Long64_t chainEntry;
  
  Int_t treenum=0;
  tree2->SetEntryList(el);   

  double gainCorrC[3][3];
  double subCluster[2][2];
  double sX, sY, scX, scY;

  cout << "Events: " << listEntries << endl;
  if(oldEta == NULL){  cout << "Old Eta is NULL " <<  endl; }
  for(int i = 0; i<4; i++){ counter[i] = 0; remoteCounter[i] = 0; }

   for (Long64_t il =0; il<listEntries;il++) {
     treeEntry = el->GetEntryAndTree(il,treenum);
      
      chainEntry = treeEntry+tree2->GetTreeOffset()[treenum];
      if (tree2->GetEntry(chainEntry)) {

	gainCorrection(gainCorrC,gainMap);
	//cout << gainCorrC[1][1] << endl;

	//finds corner
	int corner = findShape(gainCorrC,subCluster);

	int validEvent;
	
	
	if(img){
	  validEvent = placePhoton(img[0],subCluster,x,y, corner, &sX, &sY, &scX, &scY);
	}else{
	  //calc etaX, etaY
	  validEvent = placePhoton(NULL,subCluster,x,y, corner, &sX, &sY, &scX, &scY);
	}

	//fill etavel
	newEta->fill(sX,sY);
	



	if(oldEta && img && img[1]){
	  placePhotonCorr(img[1],oldEta, sX,sY, scX, scY);
	}else{
	  placePhotonCorr(NULL,newEta,sX,sY,scX,scY);
	}
	
	
      }
      //cout << il << endl;
      int ssize = 500000;
      if(il % ssize == 0 && il != 0 && oldEta==NULL){

	cout << " -------------- "<< endl;
	newEta->updatePixelPos();
	

	//newEta->resolveSelfIntersect();
	char tit[1000];
	/*	TFile *ff = new TFile("/scratch/Spider.root","UPDATE");
	sprintf(tit,"subPosAEta%i",newEta->getIt()); subPosAEta->SetName(tit);
	subPosAEta->Write(); subPosAEta->Reset();
	sprintf(tit,"subPosBEta%i",newEta->getIt()); subPosBEta->SetName(tit);
	subPosBEta->Write(); subPosBEta->Reset();
	sprintf(tit,"Eta%i",newEta->getIt()); newEta->Write(tit);
	ff->Close(); */
	//il = 0;
      }
      
      if(il % ssize == ssize-1){
	double prog = (double)il/(double)listEntries*100.;
	cout << prog << "%" << endl;
	//if(prog > 19.) return newEta;
	if(newEta->converged == 1){ cout << "converged ... " << endl; return newEta; }
      }
      
   }

   cout << "local corners: " ;
   for(int i = 0; i<4; i++) cout << i << ": " << counter[i] << " || " ;
   cout << endl;

   //cout << "remote corners: " ;
   //for(int i = 0; i<4; i++) cout << i << ": " << remoteCounter[i] << " || " ;
   //cout << endl;
   
   return newEta;
}




TChain *openTree(char *tname, char *fname,double lEc, double hEc, double rms=5., char *chainName=">>thischan"){
  TChain *tree2;
  //  TH1D **etaDI;
  char cut[1000];

  tree2=new TChain(tname);
  tree2->Add(fname);
  tree2->Print();

  //sprintf(cut,"(x<=40) && (data[%d][%d]>%f*rms) && Sum$(data)<%f && Sum$(data)>%f",1,1,rms, hEc, lEc);
  //  sprintf(cut,"(x<=40) && (data[%d][%d]>%f*rms)",1,1,rms);// && Sum$(data)<%f && Sum$(data)>%f",1,1,rms, hEc, lEc);
  sprintf(cut,"(x<=40) && Sum$(data)<%f && Sum$(data)>%f", hEc, lEc);
  //    sprintf(cut,"");
  cout << cut << endl;

  tree2->Draw(chainName, cut, "entrylist");


  tree2->SetBranchAddress("iFrame",&f);
  tree2->SetBranchAddress("x",&x);
  tree2->SetBranchAddress("y",&y);
  tree2->SetBranchAddress("data",dum);
  //tree2->SetBranchAddress("q",&q);
  
  cout << "openTree : end" << endl;
  return tree2;
}

EtaVEL *etaDensity(char *tname, char *fname, double lEc = 1000, double hEc=3000, TH2D *gainMap=NULL, int nPixels=25) {
  /** open tree and make selection */
  TChain *tree2 = openTree(tname,fname,lEc,hEc);
  TEntryList *elist = (TEntryList*)gDirectory->Get("thischan");
  if(elist == NULL) { cout << "could not open tree " << endl; return NULL; }
  
  EtaVEL *etaDen = plotEtaDensity(tree2,elist,NULL,NULL,gainMap,nPixels);

  
  //etaDen->Draw("colz");
  cout << "done" << endl;

  return etaDen;
}

void interpolate(char *tname, char *fname, EtaVEL *etaDI, double lEc = 1000, double hEc=3000, TH2D *gainMap=NULL) {

  TChain *tree2 = openTree(tname,fname,lEc,hEc,5.,">>intChain");
  TEntryList *elist = (TEntryList*)gDirectory->Get("intChain");
  if(elist == NULL) { cout << "could not open tree " << endl; return; }

  double nPixels = (double)etaDI->getNPixels();

  TH2D **img = new TH2D*[3];
  img[0] = new TH2D("img","img",nPixels*160,0.0,160.0 ,nPixels*160 ,0.0,160.0);
  img[1] = new TH2D("imgE","imgE",nPixels*160,0.0,160.0 ,nPixels*160 ,0.0,160.0);

  int inPixels = etaDI->getNPixels();

  imgArray = new int*[inPixels*160];
  for(int i = 0; i < inPixels*160; i++){
    imgArray[i] = new int[inPixels*160];
    for(int j = 0; j < inPixels*160; j++){
      imgArray[i][j] = 0;
    }
  }

  cout << "starting" << endl;
  plotEtaDensity(tree2,elist, etaDI,img,gainMap);

  //img->Draw("colz");
}


TH2D *createGainMap(char *tname, char *fname, double lEc = 0,double hEc=10000){
  char name[100];
  TH1D *avgSpec3 = new TH1D("avgSpec3", "avgSpec3",hEc/20,0,hEc); 
  TH1D ***specs3 = new TH1D**[160];
  TH1D ***specs1 = new TH1D**[160];
  for(int xx = 0; xx < 160; xx++){
    specs3[xx] = new TH1D*[160];
    specs1[xx] = new TH1D*[160];
    for(int yy = 0; yy < 160; yy++){
      sprintf(name,"S3x%iy%i",xx,yy);
      specs3[xx][yy] = new TH1D(name,name,hEc/20,0,hEc);
      sprintf(name,"S1x%iy%i",xx,yy);
      specs1[xx][yy] = new TH1D(name,name,hEc/20,0,hEc);
    }
  }
  

  TChain *tree2 = openTree(tname,fname,0,hEc,5.,">>gainChan");
  TEntryList *elist = (TEntryList*)gDirectory->Get("gainChan");
  if(elist == NULL) { cout << "could not open tree " << endl; return NULL; }

  Long64_t listEntries=elist->GetN();
  Long64_t treeEntry;
  Long64_t chainEntry;
  
  Int_t treenum=0;
  tree2->SetEntryList(elist);   

  cout << "Events: " << listEntries << endl;
  for(int i = 0; i<4; i++) counter[i] = 0;
   for (Long64_t il =0; il<listEntries;il++) {
     treeEntry = elist->GetEntryAndTree(il,treenum);
     chainEntry = treeEntry+tree2->GetTreeOffset()[treenum];
     
     if (tree2->GetEntry(chainEntry)) {
       double sum = 0;
       for(int xx = 0; xx < 3; xx++)
	 for(int yy = 0; yy < 3; yy++)
	   sum += dum[xx][yy];
       specs3[x][y]->Fill(sum);
       specs1[x][y]->Fill(dum[1][1]);
       avgSpec3->Fill(sum);
     }
   }  

   TH2D *gainMap3 = new TH2D("gainMap3","gainMap3",160,-0.5,160.-0.5,160,-.5,160.-.5);
   TH2D *gainMap1 = new TH2D("gainMap1","gainMap1",160,-0.5,160.-0.5,160,-.5,160.-.5);
   for(int xx = 0; xx < 160; xx++){
     for(int yy = 0; yy < 160; yy++){
        TF1 *gf3 = new TF1("gf3","gaus", lEc, hEc);
	specs3[xx][yy]->Fit(gf3,"Q");
	double e3 = gf3->GetParameter(1);
	gainMap3->Fill(xx,yy,e3);

        TF1 *gf1 = new TF1("gf1","gaus", lEc, hEc);
	specs1[xx][yy]->Fit(gf1,"Q");
	double e1 = gf1->GetParameter(1);
	gainMap1->Fill(xx,yy,e1);

     }
   }

   return gainMap3;
}

void writeMatlab2DHisto(int xx, int yy,char *outFileName){
  ofstream outFile;
  outFile.open (outFileName);

  cout << "create matlab file with " << xx << " xbins and " << yy << " ybins" << endl;
  
  for(int y = 0; y < yy; y++){
      for(int x = 0; x < xx; x++){
	outFile << imgArray[x][y] << "\t";
      }
      outFile << endl;
  }
  
  outFile.close();
}

//COMPLETE STUFF

void createImage(char *tdir, char *tname, char *ftname, char *ifname = NULL, int useGM=0, double lEth=-1., double hEth=-1.){
  imgRLR->Reset();
  imgLR->Reset();

  char fname[1000];
  char inFName[1000];
  char outFName[1000];
  char moutFName[1000];
  if(ifname == NULL){
    sprintf(fname,"%s/%s_*.root",tdir,tname);
  }else{
    sprintf(fname,"%s",ifname);
  }

  if(useGM)  sprintf(inFName,"%s/%s-PlotsWGMVEL.root",tdir,ftname);
  else  sprintf(inFName,"%s/%s-PlotsVEL.root",tdir,ftname);

  sprintf(outFName,"%s/%s-ImgVEL.root",tdir,tname);
  sprintf(moutFName,"%s/%s-ImgVEL.mf",tdir,tname);

  TFile *inFile = new TFile(inFName,"READ");
  
  cout << "Image Tree File Name: " << fname << endl;
  cout << "Eta File Name: " << inFName << endl;
  cout << "Out File Name: " << outFName << endl;
  cout << "Matlab Out File Name: " << moutFName << endl;

  TH2D *gm = NULL;
  if(useGM){
    cout << "Load gain map" << endl;
    gm = (TH2D *)gDirectory->Get("gainMap");
    if(gm == NULL){ cout << "can not find gainMap in file" << endl; return; }
  }

  cout << "Load eta" << endl;
  EtaVEL *ee = (EtaVEL *)gDirectory->Get("etaDist");

  cout << "Select Energy BW" << endl;
  TH1D *spec = (TH1D *)gDirectory->Get("avgSpec3");
  if(spec == NULL){ cout << "can not find avgSpec3" << endl; return; }

  TF1 *gf3 = new TF1("gf3","gaus", 0, 10000);
  spec->Fit(gf3,"Q");
  double avgE = gf3->GetParameter(1);
  double sigE = gf3->GetParameter(2);
  cout << "avgE: " << avgE << " sigE: " << sigE << endl;
  cout << endl;

  if(lEth == -1.) lEth = avgE-5.*sigE;
  if(hEth == -1.) hEth = avgE+5.*sigE;   
  cout << lEth << " < E < " << hEth << " (eV)" << endl;

  cout << "start with interpolation" << endl;
  interpolate( tname, fname, ee,lEth,hEth ,gm);  

    
  TH2D *img = (TH2D *)gDirectory->Get("img");
  if(img == NULL){ cout << "could not find 2d-histogram: img " << endl; return; }


  TH2D *imgE = (TH2D *)gDirectory->Get("imgE");
  if(imgE == NULL){ cout << "could not find 2d-histogram: imgE " << endl; return; }


  //TH2D *imgEOM = (TH2D *)gDirectory->Get("imgEOM");
  //if(imgEOM == NULL){ cout << "could not find 2d-histogram: imgEOM " << endl; return; }

  TFile *outFile = new TFile(outFName,"UPDATE");
  imgLR->Write();
  imgRLR->Write();
  imgE->Write();
  //imgEOM->Write();
  img->Write();
  outFile->Close();
  inFile->Close(); 
  cout << "writing matlab file: " << moutFName << endl;
  writeMatlab2DHisto(160*ee->getNPixels(),160*ee->getNPixels(),moutFName);
  cout << "Done : " << outFName << endl;
  
}

/**
   \par tdir input tree directory
   \par tname input tree name
   \par ifname input file name if different than tdir/tname_*.root
   \par useGM use gain map
   \par maxExpEinEv spectrum maximum
   \par nPixels sub-pixels bins
   \par lEth low threshold
   \par hEth high threshold

 */


EtaVEL *createGainAndEtaFile(char *tdir, char *tname, char *ifname=NULL, int useGM=0, double maxExpEinEv=25000., int nPixels =25, double lEth=-1., double hEth=-1.){
  char fname[1000];
  char outFName[1000];


  if(ifname == NULL){
    sprintf(fname,"%s/%s_*.root",tdir,tname);
  }else{
    sprintf(fname,"%s",ifname);
  }

  if(useGM)  sprintf(outFName,"%s/%s-PlotsWGVEL.root",tdir,tname);
  else  sprintf(outFName,"%s/%s-PlotsVEL.root",tdir,tname);

  
  cout << "Tree File Name: " << fname << endl;
  cout << "Output File Name: " << outFName << endl;

  /** creates gain map and 3x3 spectrum */
  cout << "Creating gain map: " << endl;
  TH2D *gm = createGainMap(tname,fname,0,maxExpEinEv/10.);
  gm->SetName("gainMap");


  /** gets average 3x3 spectrum and fits it with a gaus */
  TH1D *spec = (TH1D *)gDirectory->Get("avgSpec3");
  if(spec == NULL){ cout << "can not find avgSpec3" << endl; return NULL; }
  TF1 *gf3 = new TF1("gf3","gaus", 0, maxExpEinEv/10.);
  spec->Fit(gf3,"Q");
  double avgE = gf3->GetParameter(1);
  double sigE = gf3->GetParameter(2);  
  cout << "avgE: " << avgE << " sigE: " << sigE << endl;
  cout << endl;


  /** sets high and low threshold if not given by the user */
  if(lEth == -1.) lEth = avgE-5.*sigE;
  if(hEth == -1.) hEth = avgE+5.*sigE;
  cout << lEth << " < E < " << hEth << " (eV)" << endl;




  cout << "calculating eta stuff" << endl;

  EtaVEL *newEta;
  if(useGM) newEta = etaDensity(tname,fname,lEth,hEth,gm,nPixels);
  else newEta = etaDensity(tname,fname,lEth,hEth,NULL,nPixels);

  cout << "writing to file " << outFName << endl;

  TFile *outFile = new TFile(outFName,"UPDATE");

  newEta->Write("etaDist");
  
  gm->Write();
  spec->Write();
  subPosAEta->Write();
  cES3vs2->Write();

  outFile->Close();
  cout << "Done : " << outFName << endl;
  return newEta;
}

void exportSpec(char *tdir, char *tname){
  char tfname[1000];
  char ofname[1000];
  char cleanName[1000];

  for(int p = 0; p < strlen(tname);p++){
    cleanName[p+1] = '\0';
    cleanName[p] = tname[p];

    if(tname[p] == '-') cleanName[p] = '_';
  }
  
  sprintf(tfname,"%s/%s-PlotsVEL.root",tdir,tname);
  sprintf(ofname,"%s/%s_SpecVEL.m",tdir,cleanName);
  TFile *tf = new TFile(tfname);
  TH1D *spec = (TH1D *)gDirectory->Get("avgSpec3");
  
  ofstream outFile;
  outFile.open (ofname);

  if(outFile.fail()){
    cout << "Could not open file : " << ofname << endl;
    return;
  }

  cout << "create matlab file with with spec " << ofname << endl;
  

  outFile << cleanName << " = [ " << endl;
  for(int i = 0; i < spec->GetNbinsX(); i++){ 
    outFile << i << " " << spec->GetBinCenter(i) << " " << spec->GetBinContent(i) << " ; " << endl; 
  }
  
  outFile << " ] ; " << endl;

  outFile.close();
}
