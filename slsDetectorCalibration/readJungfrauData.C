{

  gROOT->ProcessLine(".L jungfrauReadData.C+");






  TFile *fout;
  THStack *hs2N;

  fout=new TFile("/mnt/slitnas/datadir_jungfrau02/20140131_analysis/test/outfile.root","RECREATE");

  hs2N=jungfrauReadData("/mnt/slitnas/datadir_jungfrau02/20140131_BeamSLS/datadir_310114/18keV_10us_HIGHG0_CDS550_%d.bin","tit",30,40,1500,-500,2500,1,0.,1,47,1,47, 0,1); 
  hs2N->SetName("cds_g4");
  hs2N->SetTitle("cds_g4");
  (TH2F*)(hs2N->GetHists()->At(0))->Write();

 (TH2F*)(hs2N->GetHists()->At(1))->Write();
 (TH2F*)(hs2N->GetHists()->At(2))->Write();
 (TH2F*)(hs2N->GetHists()->At(3))->Write();
 (TH2F*)(hs2N->GetHists()->At(4))->Write();


 // fout->Close();



}

