{
  //.L energyCalibration.cpp+
  //.L gainMap.C+
  TFile fin("/data/moench_xbox_20140113/MoTarget_45kV_0_8mA_120V_cds_g4.root");
  TH2F *h2=fin.Get("h2");
  TH2F *gMap=gainMap(h2,4);
  gMap->Draw("colz");



}
