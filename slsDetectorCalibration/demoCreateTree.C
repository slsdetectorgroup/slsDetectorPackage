{

  //.L moenchReadData.C






  TFile *fout;
  THStack *hs2N;

  fout=new TFile("/scratch/outfile.root","RECREATE");

  hs2N=moenchReadData("/data/moench_xbox_20140113/MoTarget_45kV_0_8mA_12us_120V_cds_g4_f00000%04d000_0.raw","dum",0,20,1500,-500,2500,1,0.,1,159,1,159, 0,1); 
  hs2N->SetName("cds_g4");
  hs2N->SetTitle("cds_g4");
  (TH2F*)(hs2N->GetHists()->At(0))->Write();

 (TH2F*)(hs2N->GetHists()->At(1))->Write();
 (TH2F*)(hs2N->GetHists()->At(2))->Write();
 (TH2F*)(hs2N->GetHists()->At(3))->Write();
 (TH2F*)(hs2N->GetHists()->At(4))->Write();


  fout->Close();



}

