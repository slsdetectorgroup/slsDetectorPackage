#include "moench03ReadData.C"



/************************************************************************/
TH2F *readExactImage(char *fname, int iframe=0, int frperfile,TH2F *hped=NULL) {
  ifstream filebin;
  filebin.open((const char *)(fname), ios::in | ios::binary);
  TH2F *h2=new TH2F("h2","",400,0,400,400,0,400);
  int framen(0);
  moench03CtbData *decoder=new moench03CtbData();
  char *buff=decoder->readNextFrame(filebin);
  framen=decoder->getFrameNumber(buff);

  int counter(0);

  while(framen<iframe && counter<frperfile){
   buff=decoder->readNextFrame(filebin);
   framen=decoder->getFrameNumber(buff);
   cerr<<"...";
   if(framen%1000==0) cerr<<framen;
   counter++;
  }
  if(counter<frperfile){
    h2->SetName(Form("frame_%d",framen));
    h2->SetTitle(Form("frame_%d",framen));
    cout << "==" << endl;
    for (int ix=0; ix<400; ix++) {
      for (int iy=0; iy<400; iy++) {
	//	cout <<  decoder->getDataSize() << " " << decoder->getValue(buff,ix,iy)<< endl;
	h2->SetBinContent(ix+1,iy+1,decoder->getValue(buff,ix,iy));
	//	h1->SetBinContent(++ip,decoder->getValue(buff,ix,iy));
      }
    }
    if (hped) h2->Add(hped,-1);

  }else{
    cerr<<"frame number not found"<<endl;
  }

  return h2;


}

/************************************************************/
//fnamein filename: ..._f0_%d.raw
//runmin, runmax to calculate pedestals
//framen0, framen1 first and last frame you want to add to the gif
//frperfile number of frames per file
void PlotRawFrameGif(char * fnamein, int runmin, int runmax, int framen0,int framen1, int frperfile){
  cerr<<"/***********************************/"<<endl;
  cerr<<"calculating pedestals"<<endl;

  TH2F * hp = calcPedestal(fnamein,runmin,runmax);
  int filen = (int)(framen0/frperfile);
  char  fname[1000];
  sprintf(fname,fnamein,filen);
  cerr<<"/***********************************/"<<endl;
  cerr<<"retrieving frame from"<<fname<<endl;
  
  int fileframe0 = framen0%frperfile;
  int fileframe1 = framen1%frperfile;

  TImage * img = NULL;
  TH2F * hf = NULL;
  TCanvas * c1 =NULL; 
  for(int fileframe=fileframe0; fileframe<fileframe1; fileframe++){
    hf=readExactImage(fname, fileframe,frperfile,hp);
    delete img;
    delete c1;
    c1 = new TCanvas("c1","",800,600); 
    c1->cd();
    
    hf->SetTitle(Form("Frame_%d",fileframe+framen0));
    hf->SetName(Form("Frame_%d",fileframe+framen0));
    hf->GetXaxis()->SetRangeUser(0,50);
    hf->GetXaxis()->SetTitle("Column");
    hf->GetYaxis()->SetRangeUser(240,290);
    hf->GetYaxis()->SetTitle("Row");
    hf->GetZaxis()->SetRangeUser(-50.,1300.);
    hf->SetStats(kFALSE);
    // c1->SetLogz();
    hf->Draw("colz");
    c1->Print(Form("/afs/psi/project/mythen/Marco/Pics/Fe_Raw3_%d.png",fileframe));
    img = TImage::Open(Form("/afs/psi/project/mythen/Marco/Pics/Fe_Raw3_%d.png",fileframe));
    if(fileframe<fileframe1-1){
      img->WriteImage(Form("/afs/psi/project/mythen/Marco/Pics/Fe_Raw3_%d.gif+200",fileframe1-fileframe0));
    }else{
      img->WriteImage(Form("/afs/psi/project/mythen/Marco/Pics/Fe_Raw3_%d.gif++200++",fileframe1-fileframe0));
    }
  }
  return;

}
/*********************************************************************/
TProfile2D * GetHitMap(TTree * treein, int framen,double zlow, double zup){
  TProfile2D* map = new TProfile2D("map","",400,-0.5,399.5,400,-0.5,399.5,zlow,zup);


  int x, y, iFrame;
  double data[9];

  TBranch * b_x = (TBranch*)treein->GetBranch("x");
  TBranch * b_y = (TBranch*)treein->GetBranch("y");
  TBranch * b_data = (TBranch*)treein->GetBranch("data");
  TBranch * b_iFrame = (TBranch*)treein->GetBranch("iFrame");

  b_x->SetAddress(&x);
  b_y->SetAddress(&y);
  b_data->SetAddress(data);
  b_iFrame->SetAddress(&iFrame);

  Int_t nEnt=treein->GetEntries();

  for(Int_t i=0; i<nEnt; i++){
    b_iFrame->GetEntry(i);
    if(iFrame==framen){
      b_x->GetEntry(i);
      b_y->GetEntry(i);
      b_data->GetEntry(i);
      map->SetBinEntries(map->FindFixBin(x-1,y-1),1); 
      map->SetBinEntries(map->FindFixBin(x,y-1),1); 
      map->SetBinEntries(map->FindFixBin(x+1,y-1),1); 

      map->SetBinEntries(map->FindFixBin(x-1,y),1); 
      map->SetBinEntries(map->FindFixBin(x,y),1);
      map->SetBinEntries(map->FindFixBin(x+1,y),1); 

      map->SetBinEntries(map->FindFixBin(x-1,y+1),1); 
      map->SetBinEntries(map->FindFixBin(x,y+1),1); 
      map->SetBinEntries(map->FindFixBin(x+1,y+1),1);


      map->SetBinContent(map->FindFixBin(x-1,y-1),data[0]); 
      map->SetBinContent(map->FindFixBin(x,y-1),data[1]); 
      map->SetBinContent(map->FindFixBin(x+1,y-1),data[2]);
 
      map->SetBinContent(map->FindFixBin(x-1,y),data[3]); 
      map->SetBinContent(map->FindFixBin(x,y),data[4]);
      map->SetBinContent(map->FindFixBin(x+1,y),data[5]); 

      map->SetBinContent(map->FindFixBin(x-1,y+1),data[6]); 
      map->SetBinContent(map->FindFixBin(x,y+1),data[7]); 
      map->SetBinContent(map->FindFixBin(x+1,y+1),data[8]);
    }
  }


  return map;


}

/*********************************************************/
/** creates an infinitely looping gif from clustered data
 **/
void PlotClusterHitMapGif(std::string filename, std::string treename,int framen0,int framen1){

  TFile * fin = new TFile(filename.c_str(),"read");
  TTree * treein = (TTree*)fin->Get(treename.c_str());

  TCanvas * c1 = NULL;


  TProfile2D* hmap = NULL;
  TImage * img = NULL;
  for(int framen=framen0; framen<framen1; framen++){
    delete c1;
    c1 = new TCanvas("c1","",800,600);
    c1->cd();
    // c1->SetLogz();
    hmap=GetHitMap(treein,framen,-50.,1300.);
    hmap->SetName(Form("Frame_%d",framen));
    hmap->SetTitle(Form("Frame_%d",framen));
    hmap->GetXaxis()->SetRangeUser(0,50);
    hmap->GetXaxis()->SetTitle("Column");
    hmap->GetYaxis()->SetRangeUser(240,290);
    hmap->GetYaxis()->SetTitle("Row");
    hmap->GetZaxis()->SetRangeUser(-50.,1300.);
    hmap->SetStats(kFALSE);
    hmap->Draw("colz");
    c1->Print(Form("/afs/psi/project/mythen/Marco/Pics/Fe_Cluster3_%d.png",framen));
    img = TImage::Open(Form("/afs/psi/project/mythen/Marco/Pics/Fe_Cluster3_%d.png",framen));
    if(framen<framen1-1){
      img->WriteImage(Form("/afs/psi/project/mythen/Marco/Pics/Fe_Cluster3_%d.gif+200",framen1-framen0));
    }else{
      img->WriteImage(Form("/afs/psi/project/mythen/Marco/Pics/Fe_Cluster3_%d.gif++200++",framen1-framen0));
    }
  }
  return;


}
