{ // script to run Jungfrau data analysis

  gROOT->ProcessLine(".L jungfrauReadData.C++");//");

  TFile *fout; 		// output file
  THStack *hs2N; 	// collection of objects

  fout=new TFile("/mnt/slitnas/datadir_jungfrau02/jungfrau_Trieste_2014/TriesteBeam2014/Analysis/D4_M1_HG_2000clk_CD986_CMsub.root","RECREATE"); // 
    
  //hs2N=jungfrauReadData("/mnt/slitnas/datadir_jungfrau02/20140131_BeamSLS/datadir_310114/21keV_10us_HIGHG0_CDS550_%d.bin","tit",0,75,1500,-500,2500,1,0.,1,47,1,47,1,1); //1,1); 
  
 //hs2N=jungfrauReadData("/mnt/slitnas/datadir_jungfrau02/20140228_GainBits/GSdata_laser_%d.bin","tit",0,5,1500,-500,2500,1,0.,1,47,1,47,0,1);//,"/mnt/slitnas/datadir_jungfrau02/analysis_tests/CalibrationParametersTest_fake.txt"); //1,1); // data set test GB -> set (3,0,0) in junfrauReadData.C

  hs2N=jungfrauReadData("/mnt/slitnas/datadir_jungfrau02/jungfrau_Trieste_2014/Trieste_20140314/HG_20keV_CDS986_2000clk_%d.bin","tit",0,201,3000,-499.5,2500.5,1,0.,1,47,1,47,1,1);
//,"/mnt/slitnas/datadir_jungfrau02/analysis_tests/CalibrationParametersTest_fake.txt"); //1,1); // data set test GB -> set (3,0,0) in junfrauReadData.C

  //hs2N->SetName("cds_g4");
  //hs2N->SetTitle("cds_g4");

 (TH2F*)(hs2N->GetHists()->At(0))->Write(); // write hists 
 (TH2F*)(hs2N->GetHists()->At(1))->Write();
 (TH2F*)(hs2N->GetHists()->At(2))->Write();
 (TH2F*)(hs2N->GetHists()->At(3))->Write();
 (TH2F*)(hs2N->GetHists()->At(4))->Write();
 //(TH2F*)(hs2N->GetHists()->At(5))->Write();
 //(TH2F*)(hs2N->GetHists()->At(6))->Write();

  //fout->Close(); // uncomment

}

