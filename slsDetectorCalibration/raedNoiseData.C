#include "moenchReadData.C"



void raedNoiseData(char *tit, int sign=1){





  char fname[1000];
  char f[1000];
  TFile *fout;
  THStack *hs2N;

  sprintf(fname,"/data/moench_xbox_20140113/MoTarget_45kV_0_8mA_120V_%s_0.root",tit);
  fout=new TFile(fname,"RECREATE");

  sprintf(fname,"/data/moench_xbox_20140113/MoTarget_45kV_0_8mA_120V_%s_f00000%%04d000_0.raw",tit);

  hs2N=moenchReadData(fname,0,3000,1500,-500,2500,1,0.,1,159,1,159, 0,1); 
  hs2N->SetName(tit);
  hs2N->SetTitle(tit);
  (TH2F*)(hs2N->GetHists()->At(0))->Write();

 (TH2F*)(hs2N->GetHists()->At(1))->Write();
 (TH2F*)(hs2N->GetHists()->At(2))->Write();
 (TH2F*)(hs2N->GetHists()->At(3))->Write();
 (TH2F*)(hs2N->GetHists()->At(4))->Write();


  fout->Close();



}



void raedNoiseDataN(char *tit, int sign=1){



  char fname[1000];
  char f[1000];
  TFile *fout;
  THStack *hs2N;

  sprintf(fname,"/data/moench_xbox_20140116/noise_%s.root",tit);
  fout=new TFile(fname,"RECREATE");

  sprintf(fname,"/data/moench_xbox_20140116/noise_%s_f00000%%04d000_0.raw",tit);

  hs2N=moenchReadData(fname,tit,0,3000,1500,-500,2500,sign,0.,1,159,1,159, 0,0); 
  hs2N->SetName(tit);
  hs2N->SetTitle(tit);
  (TH2F*)(hs2N->GetHists()->At(0))->Write();

  // (TH2F*)(hs2N->GetHists()->At(1))->Write();
  // (TH2F*)(hs2N->GetHists()->At(2))->Write();
  // (TH2F*)(hs2N->GetHists()->At(3))->Write();
  // (TH2F*)(hs2N->GetHists()->At(4))->Write();


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

/*
clkdivider data
/data/moench_xbox_20140114/MoTarget_45kV_0_8mA_12us_120V_cds_g1_clkdiv17_f000000010000_0.raw
-rw-rw-r-- 1 l_msdetect l_msdetect 51440000 Jan 14 12:40 /data/moench_xbox_20140114/MoTarget_45kV_0_8mA_12us_120V_cds_g1_clkdiv25_f000000010000_0.raw
-rw-rw-r-- 1 l_msdetect l_msdetect 51440000 Jan 14 13:26 /data/moench_xbox_20140114/MoTarget_45kV_0_8mA_12us_120V_cds_g1_clkdiv35_f000000010000_0.raw
-rw-rw-r-- 1 l_msdetect l_msdetect 51440000 Jan 14 14:09 /data/moench_xbox_20140114/MoTarget_45kV_0_8mA_12us_120V_cds_g1_clkdiv50_f000000010000_0.raw
-rw-rw-r-- 1 l_msdetect l_msdetect 51440000 Jan 14 14:54 /data/moench_xbox_20140114/MoTarget_45kV_0_8mA_12us_120V_cds_g1_clkdiv70_f000000010000_0.raw
-rw-rw-r-- 1 l_msdetect l_msdetect 51440000 Jan 14 16:42 /data/moench_xbox_20140114/MoTarget_45kV_0_8mA_12us_120V_cds_g1_clkdiv110_f000000010000_0.raw
-rw-rw-r-- 1 l_msdetect l_msdetect 51440000 Jan 14 17:27 /data/moench_xbox_20140114/MoTarget_45kV_0_8mA_12us_120V_cds_g1_clkdiv170_f000000010000_0.raw
*/


/* oversampled data
-rw-rw-r-- 1 l_msdetect l_msdetect 51440000 Jan 14 18:12 /data/moench_xbox_20140114/MoTarget_45kV_0_8mA_12us_120V_os10_16rows_f000000010000_0.raw
-rw-rw-r-- 1 l_msdetect l_msdetect 51440000 Jan 14 18:47 /data/moench_xbox_20140114/MoTarget_45kV_0_8mA_12us_120V_os10_16rows_f000000010000_1.raw
-rw-rw-r-- 1 l_msdetect l_msdetect 51440000 Jan 14 19:22 /data/moench_xbox_20140114/MoTarget_45kV_0_8mA_12us_120V_os10_16rows_f000000010000_2.raw
-rw-rw-r-- 1 l_msdetect l_msdetect 51440000 Jan 14 20:02 /data/moench_xbox_20140114/MoTarget_45kV_0_8mA_12us_120V_os10_16rows_f000000010000_3.raw
-rw-rw-r-- 1 l_msdetect l_msdetect 51440000 Jan 14 20:41 /data/moench_xbox_20140114/MoTarget_45kV_0_8mA_12us_120V_os10_16rows_f000000010000_4.raw
-rw-rw-r-- 1 l_msdetect l_msdetect 51440000 Jan 14 21:16 /data/moench_xbox_20140114/MoTarget_45kV_0_8mA_12us_120V_os10_16rows_f000000010000_5.raw
-rw-rw-r-- 1 l_msdetect l_msdetect 51440000 Jan 14 21:56 /data/moench_xbox_20140114/MoTarget_45kV_0_8mA_12us_120V_os10_16rows_f000000010000_6.raw
-rw-rw-r-- 1 l_msdetect l_msdetect 51440000 Jan 14 22:35 /data/moench_xbox_20140114/MoTarget_45kV_0_8mA_12us_120V_os10_16rows_f000000010000_7.raw
-rw-rw-r-- 1 l_msdetect l_msdetect 51440000 Jan 14 23:11 /data/moench_xbox_20140114/MoTarget_45kV_0_8mA_12us_120V_os10_16rows_f000000010000_8.raw
-rw-rw-r-- 1 l_msdetect l_msdetect 51440000 Jan 14 23:50 /data/moench_xbox_20140114/MoTarget_45kV_0_8mA_12us_120V_os10_16rows_f000000010000_9.raw
*/

