#include "moenchReadData.C"


void raedNoiseData(char *tit, int sign=1){



  char fname[1000];
  char f[1000];
  TFile *fout;
  THStack *hs2N;

  sprintf(fname,"/data/moench_xbox_20140113/MoTarget_45kV_0_8mA_120V_%s.root",tit);
  fout=new TFile(fname,"RECREATE");

  sprintf(fname,"/data/moench_xbox_20140113/MoTarget_45kV_0_8mA_12us_120V_%s_f00000%%04d000_0.raw",tit);

  hs2N=moenchReadData(fname,tit,0,3000,1500,-500,2500,sign,0.,1,159,1,159, 0); 
  hs2N->SetName(tit);
  hs2N->SetTitle(tit);
  (TH2F*)(hs2N->GetHists()->At(0))->Write();

 (TH2F*)(hs2N->GetHists()->At(1))->Write();
 (TH2F*)(hs2N->GetHists()->At(2))->Write();
 (TH2F*)(hs2N->GetHists()->At(3))->Write();
 (TH2F*)(hs2N->GetHists()->At(4))->Write();


  fout->Close();



}



void g4() {

  raedNoiseData("cds_g4_low_gain");
  raedNoiseData("cds_g4_sto1_only");
  raedNoiseData("cds_g4_no sto");
  


}

void no_cds() {

  raedNoiseData("cds_disable_low_gain",-1);
  raedNoiseData("cds_disable_sto1_only",-1);
  raedNoiseData("cds_disable_no sto",-1);
  


}

void all_gains() {

  raedNoiseData("cds_g2");
  raedNoiseData("cds_g2HC");
  raedNoiseData("cds_g1_2");
  raedNoiseData("cds_g2_3");
  


}

void all_low_gains() {

  raedNoiseData("cds_g2_low_gain");
  raedNoiseData("cds_g2HC_low_gain");
  raedNoiseData("cds_g1_2_low_gain");
  raedNoiseData("cds_g2_3_low_gain");
}
