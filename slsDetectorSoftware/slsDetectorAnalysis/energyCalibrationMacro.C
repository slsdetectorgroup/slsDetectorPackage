{

  TH1F *h1[3];
  TH2F *h2=createScan("/scratch/stability_test/ag_source_S%d_10.raw",500,900,10,1280*2);
  h1[0]=getCh(h2,500);
  h1[1]=getCh(h2,300);
  h1[2]=getCh(h2,700);
  Double_t gain, offset, eg, eo;
  Double_t en[3]={20,22,24};
  TH1F *h;
  energyCalibration *e=new energyCalibration();
  e->setFitRange(500,700);
  Double_t mypar[6];
  mypar[0]=0; //pedestal
  mypar[1]=0; //pedestal slope
  mypar[2]=-1; //inflection point - must be free for all energies and will be set at half of the scan range
  mypar[3]=10; //noise rms
  mypar[4]=1000; //number of photons
  mypar[5]=0; //charge sharing slope
  e->setFitParameters(mypar);

  TGraphErrors *gr=e->calibrate(3,en,NULL,h1,gain,offset,eg,eo);

}
