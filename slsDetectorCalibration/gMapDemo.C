void gMap(char *tit, float g=1) {
  //.L energyCalibration.cpp+
  //.L gainMap.C+

  char fname[1000];

  sprintf(fname,"/data/moench_xbox_20140113/MoTarget_45kV_0_8mA_120V_%s.root",tit);

  TFile fin(fname);
  TH2F *h2=fin.Get("h2");
  TH2F *gMap=(TH2F*)gainMap(h2,g);
  gMap->Draw("colz");
  sprintf(fname,"/data/moench_xbox_20140113/gain_map_%s.root",tit);
  TFile fout(fname,"RECREATE");
  gMap->Write();
 
  fout.Close();

}
