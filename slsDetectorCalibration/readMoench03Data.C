#include "moench03ReadData.C"



void readMoench03Data(std::string path,char* tit, std::string phase, std::string wtime,int sign=1,int runmin=0, int runmax=100, int sc_num=0,int hitfinder=1){

  // int runmin(150);
  // int runmax(200);
  int nbins(2000);
  int hmin(-1000);
  int hmax(3000);
  int xmin(1);
  int xmax(399);
  int ymin(1);
  int ymax(399);
  int cmsub(0);
 
  


  char fname[1000];
  TFile *fout;
  // TFile *fouth1;
  THStack *hs;
  sprintf(fname,"%s/%s_%s_%s_%d_%d_Csub%d_HF%d.root",path.c_str(),tit,phase.c_str(),wtime.c_str(),runmin,runmax-1,cmsub,hitfinder);
  fout=new TFile(fname,"RECREATE");
  cerr<<"creating output file:"<<endl;
  cerr<<fname<<endl;
  cerr<<"/****************************/"<<endl;

   sprintf(fname,"%s/%s_%s_%s_f0_%%d.raw",path.c_str(),tit,phase.c_str(),wtime.c_str()); //  sprintf(fname,"%s/%s_phase%s_wtime%s_period0.075_OD%1.1f_f0_%%d.raw",path.c_str(),tit,phase.c_str(),wtime.c_str(),OD);

  cerr<<fname<<endl;

  // Int_t out = moench03DrawPedestals(fname,"Mo",runmin,runmax,nbins,hmin,hmax,xmin,xmax,ymin,ymax, cmsub,hitfinder); 
 
  fout->cd();
  hs=moench03ReadData(fname,tit,runmin,runmax,nbins,hmin,hmax,xmin,xmax,ymin,ymax, cmsub,hitfinder); 


  cout << "returned" << endl;
  hs->SetName(tit);
  hs->SetTitle(tit);
  cout << "name/title set" << endl;

  Int_t nH=1;

  TH2F * h=NULL;


  
  if (hs->GetHists()) {
    for (int i=0; i<nH; i++) {
      if (hs->GetHists()->At(i)) {
	(TH2F*)(hs->GetHists()->At(i))->Write();
	// h->SetName(Form("h%d",i+1));
	// h->SetTitle(Form("h%d",i+1));
	// cerr<<h->GetEntries()<<" entries"<<endl;
	// can->cd(i+1);
	// h->Draw("colz");
	// fout->cd();
	// h->Write();
	cout << i << " " ;
      }
    } 

    cout << " histos written " << endl;
  } else
    cout << "no hists in stack " << endl;
  if(fout->IsOpen())fout->Close();


  return;

}
/**********************************************************/
void readPixelCharge(std::string path,char* tit, std::string phase, std::string wtime, float OD, int sign,int runmin, int runmax, int x0, int y0){
  char fname[1000];
  sprintf(fname,"%s/%s_%s_%2.0fumSi_%s_f0_%%d.raw",path.c_str(),tit,phase.c_str(),OD,wtime.c_str());
  char fnameout[1000];
  sprintf(fnameout,"%s/%s_%s_%2.0fumSi_%s.root",path.c_str(),tit,phase.c_str(),OD,wtime.c_str());
  // sprintf(fnameout,"%s/%s_phase%s_wtime%s_period0.2_OD%1.1f.root",path.c_str(),tit,phase.c_str(),wtime.c_str(),OD);
  TFile * fout = new TFile(fnameout,"recreate");

  for(int i=0; i<12; i++){
    TH1F * hpix = SinglePixelHisto(fname,15000,-0.5,29999.5,runmin,runmax,x0+i,y0);
    hpix->SetName(Form("h_%d_%d",x0+i,y0));
    fout->cd();
    hpix->Write();
  }


  return;
}
/*************************************************/
void LoopOnPixelCharge(std::string path,char* tit, std::string phase, std::string wtime, int sign,int runmin, int runmax, int x0, int y0){
  float OD=700;

  for(int i=0; i<6; i++){
    readPixelCharge(path,tit,phase,wtime,OD,sign,runmin,runmax,x0,y0);
    OD += 200;
  }
  return;
}

/**********************************************************/
void readPixelCorrelation(std::string path,char* tit, std::string frame, std::string phase, std::string wtime,int sign,int runmin, int runmax,int supercolumn){

  int npx(400);
  int npy(400);

  char fname[1000];
  int sc_width(25);
  int sc_height(200);
  int sc_number=supercolumn;
  int xmin(0);
  int xmax(0);
  int ymin(0);
  int ymax(0);


  ADCMap * map = new ADCMap(npx,npy,sc_width,sc_height);
  int ret = map->Init();
  
  xmin=map->getXmin(sc_number);
  xmax=map->getXmax(sc_number);
  ymin=map->getYmin(sc_number);
  ymax=map->getYmax(sc_number);

  cerr<<"/**********************************/"<<endl;
  cerr<<"Checking super column "<<sc_number<<endl;
  cerr<<"x: "<<map->getXmin(sc_number)<<" - "<<map->getXmax(sc_number)<<endl;
  cerr<<"y: "<<map->getYmin(sc_number)<<" - "<<map->getYmax(sc_number)<<endl;
  cerr<<"/**********************************/"<<endl;




  char fnameout[1000];
  sprintf(fnameout,"%s/%s_wtime%s_%d_%d_Corr_SC%d.root",path.c_str(),tit,wtime.c_str(),runmin,runmax-1,sc_number);
  TFile * fout = new TFile(fnameout,"recreate");
  Int_t nbins(3000);
  Float_t xlow(6999.5);
  Float_t xup(9999.5);
  TH2F * hcorr=NULL;
  for(int i=0; i<25; i++){
    phase.clear();
    phase=(std::string)Form("%d",i*5);
    cerr<<"check ADC phase "<<phase.c_str()<<endl;

    sprintf(fname,"%s/%s_phase%s_wtime%s_period0.075_f0_%%d.raw",path.c_str(),tit,phase.c_str(),wtime.c_str());
  // TH1F * h1 = new TH1F("h1",Form("ch%d",nchan1),nbins,xlow,xup);
  // TH1F * h2 = new TH1F("h2",Form("ch%d",nchan2),nbins,xlow,xup);

    hcorr= new TH2F(Form("hcorr_ph%s",phase.c_str()),Form("hcorr_ph%s",phase.c_str()),nbins,xlow,xup,nbins,xlow,xup);

    DrawAllPixelCorrelation(fname,frame,runmin,runmax,xmin,xmax,ymin,ymax,sc_width,hcorr);
  // h1->GetXaxis()->SetTitle("ADC");
  // h2->GetXaxis()->SetTitle("ADC");
  


  fout->cd();
  // h1->Write();
  // h2->Write();
  hcorr->Write();
  }
  return;
  fout->Close();
}


/****************************************************/
void readNoise(std::string path, char *tit, std::string flag, std::string phase, std::string wtime,float OD,int sign,int runmin, int runmax){
  TFile * fout = new TFile(Form("%s/%s_%s_%s_noiseMap%s.root",path.c_str(),tit,phase.c_str(),wtime.c_str(),flag.c_str()),"recreate");
  char fname[1000];
  int nfiles=runmax-runmin;
  // int runmin(382);
  // int runmax(442);
  // std::string target("Cu");
  


  // THStack * hsnoise = NULL;
  TH2F * hped=NULL;
  TH2F * hnoise=NULL;
  // for(int i=0; i<25; i++){
  //   phase.clear();
    // phase=(std::string)Form("%d",i*5);
  //    sprintf(fname,"%s/%s_phase%s_wtime%s_period0.2_OD%1.1f_f0_%%d.raw",path.c_str(),tit,phase.c_str(),wtime.c_str(),OD); 
    // sprintf(fname,"%s/%s_phase%s_wtime%s_period0.2_f0_%%d.raw",path.c_str(),tit,phase.c_str(),wtime.c_str()); 
  sprintf(fname,"%s/%s_%s_%s_f0_%%d.raw",path.c_str(),tit,phase.c_str(),wtime.c_str()); 
    THStack * hsnoise=calcNoise(fname,flag,runmin,runmax,nfiles);
    hsnoise->SetName(Form("%s_noiseMap_ph%s",tit,phase.c_str()));

    fout->cd();

    if (hsnoise->GetHists()) {
      hped=(TH2F*)(hsnoise->GetHists()->At(0));
      hped->SetName(Form("hped_ph%s",phase.c_str()));
      hped->Write();
      hnoise=(TH2F*)(hsnoise->GetHists()->At(1));
      hnoise->SetName(Form("hnoise_ph%s",phase.c_str()));
      hnoise->Write();
      cout << " histos written for ADC Phase " << phase.c_str()<<endl;
    } else
      cout << "no hists in stack " << endl;
    delete hsnoise;

  // }



  fout->Close();
  return;


}
/************************************************************************/
void readFrames(std::string path, char *tit, std::string phase, std::string wtime,int sign,int runnum, int framemin, int framemax){
  char fformat[1000];
  char fname[1000];
  sprintf(fformat,"%s/%s_phase%s_wtime%s_period0.075_f0_%d.raw",path.c_str(),tit,phase.c_str(),wtime.c_str(),runnum);
  THStack * hs = DrawFrames(fformat,framemin,framemax);
  int nframes=framemax-framemin;
  TFile * fout = new TFile(Form("%s/Frames_phase%s_wtime%s_period0.075_f0_%d_fr%d-%d.root",path.c_str(),phase.c_str(),wtime.c_str(),runnum,framemin,framemax),"recreate");

  TCanvas * c1= new TCanvas("c1","",800,600);
  c1->Print(Form("%s/Frames_phase%s_wtime%s_period0.075_f0_%d_fr%d-%d.pdf[",path.c_str(),phase.c_str(),wtime.c_str(),runnum,framemin,framemax));
  TH2F * h=NULL;

  for(int hnum=0; hnum<nframes; hnum++){
    h=(TH2F*)hs->GetHists()->At(hnum);
    h->SetName(Form("h_%d",hnum+framemin));
    h->SetTitle(Form("h_%d",hnum+framemin));
    h->GetZaxis()->SetRangeUser(5000.,10000.);
    h->Draw("colz");
    

    c1->Print(Form("%s/Frames_phase%s_wtime%s_period0.075_f0_%d_fr%d-%d.pdf",path.c_str(),phase.c_str(),wtime.c_str(),runnum,framemin,framemax));
    fout->cd();
    h->Write();


  }

  c1->Print(Form("%s/Frames_phase%s_wtime%s_period0.075_f0_%d_fr%d-%d.pdf]",path.c_str(),phase.c_str(),wtime.c_str(),runnum,framemin,framemax));

  fout->Close();

  return;

}
/************************************************************************************/
void PlotSinglePixelHisto(std::string path, char *tit, std::string flag, std::string phase, std::string wtime,float OD,int sign,int runmin, int runmax, int x0, int y0){
  char fname[1000];
  sprintf(fname,"%s/%s_%s_%s_f0_%%d.raw",path.c_str(),tit,phase.c_str(),wtime.c_str());

  int nbins(8000);
  float xmin(-0.5);
  float xmax(15999.5);
  
  TH1F * h1 = SinglePixelHisto(fname,nbins,xmin,xmax,runmin,runmax,x0,y0);
  h1->Draw("hist");


  return;

}
// void raedNoiseDataN(char *tit, int sign=1){



//   char fname[1000];
//   char f[1000];
//   TFile *fout;
//   THStack *hs2N;

//   sprintf(fname,"/data/moench_xbox_20140116/noise_%s.root",tit);
//   fout=new TFile(fname,"RECREATE");

//   sprintf(fname,"/data/moench_xbox_20140116/noise_%s_f00000%%04d000_0.raw",tit);

//   hs2N=moenchReadData(fname,tit,0,3000,1500,-500,2500,sign,0.,1,159,1,159, 0,0); 
//   hs2N->SetName(tit);
//   hs2N->SetTitle(tit);
//   (TH2F*)(hs2N->GetHists()->At(0))->Write();

//   // (TH2F*)(hs2N->GetHists()->At(1))->Write();
//   // (TH2F*)(hs2N->GetHists()->At(2))->Write();
//   // (TH2F*)(hs2N->GetHists()->At(3))->Write();
//   // (TH2F*)(hs2N->GetHists()->At(4))->Write();


//   fout->Close();



// }



// void g4() {

//   raedNoiseData("cds_g4_low_gain");
//   raedNoiseData("cds_g4_sto1_only");
//   raedNoiseData("cds_g4_no sto");
  


// }

// void no_cds() {

//   raedNoiseData("cds_disable_low_gain",-1);
//   raedNoiseData("cds_disable_sto1_only",-1);
//   raedNoiseData("cds_disable_no sto",-1);
  


// }

// void all_gains() {

//   raedNoiseData("cds_g2");
//   raedNoiseData("cds_g2HC");
//   raedNoiseData("cds_g1_2");
//   raedNoiseData("cds_g2_3");
  


// }

// void all_low_gains() {

//   raedNoiseData("cds_g2_low_gain");
//   raedNoiseData("cds_g2HC_low_gain");
//   raedNoiseData("cds_g1_2_low_gain");
//   raedNoiseData("cds_g2_3_low_gain");
// }

// /*
// clkdivider data
// /data/moench_xbox_20140114/MoTarget_45kV_0_8mA_12us_120V_cds_g1_clkdiv17_f000000010000_0.raw
// -rw-rw-r-- 1 l_msdetect l_msdetect 51440000 Jan 14 12:40 /data/moench_xbox_20140114/MoTarget_45kV_0_8mA_12us_120V_cds_g1_clkdiv25_f000000010000_0.raw
// -rw-rw-r-- 1 l_msdetect l_msdetect 51440000 Jan 14 13:26 /data/moench_xbox_20140114/MoTarget_45kV_0_8mA_12us_120V_cds_g1_clkdiv35_f000000010000_0.raw
// -rw-rw-r-- 1 l_msdetect l_msdetect 51440000 Jan 14 14:09 /data/moench_xbox_20140114/MoTarget_45kV_0_8mA_12us_120V_cds_g1_clkdiv50_f000000010000_0.raw
// -rw-rw-r-- 1 l_msdetect l_msdetect 51440000 Jan 14 14:54 /data/moench_xbox_20140114/MoTarget_45kV_0_8mA_12us_120V_cds_g1_clkdiv70_f000000010000_0.raw
// -rw-rw-r-- 1 l_msdetect l_msdetect 51440000 Jan 14 16:42 /data/moench_xbox_20140114/MoTarget_45kV_0_8mA_12us_120V_cds_g1_clkdiv110_f000000010000_0.raw
// -rw-rw-r-- 1 l_msdetect l_msdetect 51440000 Jan 14 17:27 /data/moench_xbox_20140114/MoTarget_45kV_0_8mA_12us_120V_cds_g1_clkdiv170_f000000010000_0.raw
// */


// /* oversampled data
// -rw-rw-r-- 1 l_msdetect l_msdetect 51440000 Jan 14 18:12 /data/moench_xbox_20140114/MoTarget_45kV_0_8mA_12us_120V_os10_16rows_f000000010000_0.raw
// -rw-rw-r-- 1 l_msdetect l_msdetect 51440000 Jan 14 18:47 /data/moench_xbox_20140114/MoTarget_45kV_0_8mA_12us_120V_os10_16rows_f000000010000_1.raw
// -rw-rw-r-- 1 l_msdetect l_msdetect 51440000 Jan 14 19:22 /data/moench_xbox_20140114/MoTarget_45kV_0_8mA_12us_120V_os10_16rows_f000000010000_2.raw
// -rw-rw-r-- 1 l_msdetect l_msdetect 51440000 Jan 14 20:02 /data/moench_xbox_20140114/MoTarget_45kV_0_8mA_12us_120V_os10_16rows_f000000010000_3.raw
// -rw-rw-r-- 1 l_msdetect l_msdetect 51440000 Jan 14 20:41 /data/moench_xbox_20140114/MoTarget_45kV_0_8mA_12us_120V_os10_16rows_f000000010000_4.raw
// -rw-rw-r-- 1 l_msdetect l_msdetect 51440000 Jan 14 21:16 /data/moench_xbox_20140114/MoTarget_45kV_0_8mA_12us_120V_os10_16rows_f000000010000_5.raw
// -rw-rw-r-- 1 l_msdetect l_msdetect 51440000 Jan 14 21:56 /data/moench_xbox_20140114/MoTarget_45kV_0_8mA_12us_120V_os10_16rows_f000000010000_6.raw
// -rw-rw-r-- 1 l_msdetect l_msdetect 51440000 Jan 14 22:35 /data/moench_xbox_20140114/MoTarget_45kV_0_8mA_12us_120V_os10_16rows_f000000010000_7.raw
// -rw-rw-r-- 1 l_msdetect l_msdetect 51440000 Jan 14 23:11 /data/moench_xbox_20140114/MoTarget_45kV_0_8mA_12us_120V_os10_16rows_f000000010000_8.raw
// -rw-rw-r-- 1 l_msdetect l_msdetect 51440000 Jan 14 23:50 /data/moench_xbox_20140114/MoTarget_45kV_0_8mA_12us_120V_os10_16rows_f000000010000_9.raw
// */

