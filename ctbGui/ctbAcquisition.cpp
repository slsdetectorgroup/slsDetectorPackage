// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
//#define TESTADC


#include <TGButtonGroup.h>
#include <TGButton.h>
#include <TGNumberEntry.h>
#include <TGComboBox.h>
#include <TGLabel.h>
#include <TList.h>
#include <TThread.h>
#include <TGraph.h>
#include <TMultiGraph.h>
#include <TCanvas.h>
#include <TROOT.h>
#include <THStack.h>
#include <TColor.h>
#include <TTimer.h>
#include <TH2F.h>
#include <TSystem.h>


#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>

#include "ctbAcquisition.h"
#include "ctbDefs.h"
#include "sls/Detector.h"
#include "sls/sls_detector_defs.h"
#include "ctbMain.h"
#include "moench03CtbData.h" 
//#include "moench03TCtbData.h" 
//#include "moench03T1CtbData.h" 
#include "moench03CommonMode.h" 
#include "moench03T1ZmqDataNew.h" 
#include "moench02CtbData.h" 
//#include "jungfrau10ModuleData.h" 
#include "moenchCommonMode.h" 
#include "singlePhotonDetector.h"
#include "Mythen3_01_jctbData.h"
#include "Mythen3_02_jctbData.h"
#include "adcSar2_jctbData.h"
#include "moench04CtbZmqData.h"
#include "moench04CtbZmq10GbData.h"
#include "deserializer.h"
#include "sls/detectorData.h"
#include "imageZmq16bit.h"
#include "imageZmq32bit.h"


using namespace std;




ctbAcquisition::ctbAcquisition(TGVerticalFrame *page, sls::Detector *det) : TGGroupFrame(page,"Acquisition",kVerticalFrame),  myDet(det), myCanvas(NULL), globalPlot(0), tenG(0), nAnalogSamples(1), nDigitalSamples(1), dataStructure(NULL), photonFinder(NULL), cmSub(0), dBitMask(0xffffffffffffffff), deserializer(0) {

  adcFit=NULL;
  bitPlot=NULL;
  countsFit=NULL;

    page->AddFrame(this,new TGLayoutHints(kLHintsTop | kLHintsExpandX , 10,10,10,10));
    MapWindow();

    TGHorizontalFrame *hframe=new TGHorizontalFrame(this, 800,50);
    AddFrame(hframe,new TGLayoutHints(kLHintsTop | kLHintsExpandX , 10,10,10,10));
    hframe->MapWindow();

    char tit[100];
    
    cout << "outfile "<< endl;

   cFileSave= new TGCheckButton(hframe, "Output file: ");
   hframe->AddFrame(cFileSave,new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 5, 5, 5, 5));
   cFileSave->MapWindow();
   cFileSave->SetTextJustify(kTextRight);
   cFileSave->Connect("Toggled(Bool_t)","ctbAcquisition",this,"setFsave(Bool_t)");

  std::string temp = "run";
  try {
    temp = myDet->getFileNamePrefix().tsquash("Different values");
  } CATCH_DISPLAY ("Could not get file name prefix.", "ctbAcquisition::ctbAcquisition")
  eFname = new TGTextEntry(hframe, temp.c_str());

   hframe->AddFrame(eFname,new TGLayoutHints(kLHintsTop |  kLHintsExpandX, 5, 5, 5, 5));
   eFname->MapWindow();
   eFname->Resize(150,30);

   eFname->Connect("ReturnPressed()","ctbAcquisition",this,"setFname()");


   TGLabel *label=new TGLabel(hframe,"index: ");
   hframe->AddFrame(label,new TGLayoutHints(kLHintsTop | kLHintsLeft| kLHintsExpandX, 5, 5, 5, 5));
   label->MapWindow();
   label->SetTextJustify(kTextRight);


   eFindex = new TGNumberEntry(hframe, 0, 9,999, TGNumberFormat::kNESInteger,
                                               TGNumberFormat::kNEANonNegative, 
					 TGNumberFormat::kNELNoLimits);
   hframe->AddFrame( eFindex,new TGLayoutHints(kLHintsTop |  kLHintsExpandX, 1, 1, 1, 1));
   eFindex->MapWindow();
   eFindex->Resize(150,30);
    TGTextEntry *e= eFindex->TGNumberEntry::GetNumberEntry();
    e->Connect("ReturnPressed()","ctbAcquisition",this,"setFindex()");




    cout << "outdir "<< endl;


   hframe=new TGHorizontalFrame(this, 800,50);
   AddFrame(hframe,new TGLayoutHints(kLHintsTop | kLHintsExpandX , 10,10,10,10));
   hframe->MapWindow();

   label=new TGLabel(hframe,"Output directory: ");
   hframe->AddFrame(label,new TGLayoutHints(kLHintsTop | kLHintsLeft| kLHintsExpandX, 5, 5, 5, 5));
   label->MapWindow();
   label->SetTextJustify(kTextRight);



  temp = "/tmp/";
  try {
    temp = myDet->getFilePath().tsquash("Different values");
  } CATCH_DISPLAY ("Could not get file path.", "ctbAcquisition::ctbAcquisition")
   eOutdir = new TGTextEntry(hframe, temp.c_str());

   hframe->AddFrame(eOutdir,new TGLayoutHints(kLHintsTop |  kLHintsExpandX, 5, 5, 5, 5));
   eOutdir->MapWindow();
   eOutdir->Resize(150,30);


   eOutdir->Connect("ReturnPressed()","ctbAcquisition",this,"setOutdir()");

   hframe=new TGHorizontalFrame(this, 800,50);
    AddFrame(hframe,new TGLayoutHints(kLHintsTop | kLHintsExpandX , 10,10,10,10));
    hframe->MapWindow();


    cout << "meas "<< endl;
label=new TGLabel(hframe,"Number of Measurements (fake): ");
   hframe->AddFrame(label,new TGLayoutHints(kLHintsTop | kLHintsLeft| kLHintsExpandX, 5, 5, 5, 5));
   label->MapWindow();
   label->SetTextJustify(kTextRight);


   eMeasurements = new TGNumberEntry(hframe, 0, 9,999, TGNumberFormat::kNESInteger,
                                               TGNumberFormat::kNEANonNegative, 
					 TGNumberFormat::kNELNoLimits);
   hframe->AddFrame( eMeasurements,new TGLayoutHints(kLHintsTop |  kLHintsExpandX, 1, 1, 1, 1));
   eMeasurements->MapWindow();
   eMeasurements->Resize(150,30);
   eMeasurements->SetNumber(1);
    e= eMeasurements->TGNumberEntry::GetNumberEntry();
    e->Connect("ReturnPressed()","ctbAcquisition",this,"setMeasurements()");




hframe=new TGHorizontalFrame(this, 800,50);
    AddFrame(hframe,new TGLayoutHints(kLHintsTop | kLHintsExpandX , 10,10,10,10));
    hframe->MapWindow();




    cout << "pattern "<< endl;




   cCompile= new TGCheckButton(hframe, "Compile");
   hframe->AddFrame(cCompile,new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 5, 5, 5, 5));
   cCompile->MapWindow();
   cCompile->SetOn();
   //  cCompile->Connect("Toggled(Bool_t)","ctbAcquisition",this,"setFsave(Bool_t)");


   cLoad= new TGTextButton(hframe, "Load");
   hframe->AddFrame(cLoad,new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 5, 5, 5, 5));
   cLoad->MapWindow();
   cLoad->Connect("Clicked()","ctbAcquisition",this,"loadPattern()");


//    cRun= new TGCheckButton(hframe, "Run");
//    hframe->AddFrame(cRun,new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 5, 5, 5, 5));
//    cRun->MapWindow();
//    //  cCompile->Connect("Toggled(Bool_t)","ctbAcquisition",this,"setFsave(Bool_t)");






    bStatus=new TGTextButton(hframe, "Start");
    hframe->AddFrame(bStatus,new TGLayoutHints(kLHintsTop |  kLHintsExpandX, 5, 5, 5, 5));
    bStatus->MapWindow();
    bStatus->Connect("Clicked()","ctbAcquisition",this,"toggleAcquisition()");





    cout << "plot "<< endl;

    hframe=new TGHorizontalFrame(this, 800,50);
    AddFrame(hframe,new TGLayoutHints(kLHintsTop | kLHintsExpandX , 10,10,10,10));
    hframe->MapWindow();
    
    


    label=new TGLabel(hframe,"Plot: ");
   hframe->AddFrame(label,new TGLayoutHints(kLHintsTop | kLHintsLeft| kLHintsExpandX, 5, 5, 5, 5));
   label->MapWindow();
   label->SetTextJustify(kTextRight);






  TGButtonGroup *bgPlot = new TGButtonGroup(hframe);
  //  horizontal->SetTitlePos(TGGroupFrame::kCenter);
  rbPlotOff=new TGRadioButton(hframe, "No plot");
  rbWaveform=new TGRadioButton(hframe, "Waveform");
  rbDistribution=new TGRadioButton(hframe, "Distribution");
  rb2D=new TGRadioButton(hframe, "Image");
  // rbScan=new TGRadioButton(hframe, "Scan");

  cbDetType=new TGComboBox(hframe);
  //  enum {DESERIALIZER, MOENCH04, MOENCH02, ADCSAR2, MYTHEN301, MYTHEN302, MAXDET};
  cbDetType->AddEntry("Deserializer", DESERIALIZER);
  cbDetType->AddEntry("MOENCH02", MOENCH02);
  cbDetType->AddEntry("MOENCH04", MOENCH04);
  // cbDetType->AddEntry("JUNGFRAU1.0", 2);
  cbDetType->AddEntry("MOENCH03",MOENCH03);
  cbDetType->AddEntry("IMAGE32BIT",IMAGE32B);
  cbDetType->AddEntry("IMAGE16BIT",IMAGE16B);

  //cbDetType->AddEntry("MOENCH03", iiii++);
  // cbDetType->AddEntry("MYTHEN3 0.1", MYTHEN301);
  // cbDetType->AddEntry("ADCSAR2", ADCSAR2);
  // cbDetType->AddEntry("MYTHEN3 0.2", MYTHEN302);
 
  cbDetType->SetHeight(20);
  cbDetType->Select(0);
  
  bgPlot->Insert(rbPlotOff,0);
  bgPlot->Insert(rbWaveform,1);
  bgPlot->Insert(rbDistribution,2);
  bgPlot->Insert(rb2D,3);
  //  bgPlot->Insert(rbScan,4);
  
  bgPlot->Connect("Clicked(Int_t)", "ctbAcquisition", this, "changePlot(Int_t)");
  // hframe->AddFrame(bgPlot, new TGLayoutHints(kLHintsExpandX));

  cbDetType->Connect("Selected(Int_t)", "ctbAcquisition",this, "changeDetector(Int_t)");
  hframe->AddFrame(rbPlotOff, new TGLayoutHints(kLHintsTop | kLHintsExpandX));
  hframe->AddFrame(rbWaveform, new TGLayoutHints(kLHintsTop | kLHintsExpandX));
  hframe->AddFrame(rbDistribution, new TGLayoutHints(kLHintsTop | kLHintsExpandX));
  hframe->AddFrame(rb2D, new TGLayoutHints(kLHintsTop | kLHintsExpandX));
  //  hframe->AddFrame(rbScan, new TGLayoutHints(kLHintsTop | kLHintsExpandX));
  hframe->AddFrame(cbDetType, new TGLayoutHints(kLHintsTop | kLHintsExpandX| kLHintsExpandY));


  bgPlot->SetExclusive(kTRUE);
  rbWaveform->SetOn();
  rbPlotOff->MapWindow();
  rbWaveform->MapWindow();
  rbDistribution->MapWindow();
  rb2D->MapWindow();
  //  rbScan->MapWindow();
  cbDetType->MapWindow();




  //  cout << "off "<< endl;


  hframe=new TGHorizontalFrame(this, 800,50);
  AddFrame(hframe,new TGLayoutHints(kLHintsTop | kLHintsExpandX , 10,10,10,10));
  hframe->MapWindow();
    
 

  label=new TGLabel(hframe,"Serial offset:");
  hframe->AddFrame(label,new TGLayoutHints(kLHintsTop | kLHintsLeft| kLHintsExpandX, 5, 5, 5, 5));
   label->MapWindow();
   label->SetTextJustify(kTextRight);


   eSerOff=new TGNumberEntry(hframe, 0, 9,999, TGNumberFormat::kNESInteger,
                                               TGNumberFormat::kNEANonNegative, 
			     TGNumberFormat::kNELLimitMinMax,0,16535);
   hframe->AddFrame(eSerOff,new TGLayoutHints(kLHintsTop |  kLHintsExpandX, 1, 1, 1, 1));
   eSerOff->MapWindow();
   eSerOff->SetNumber(0);
   e= eSerOff->TGNumberEntry::GetNumberEntry();
   eSerOff->Connect("ValueSet(Long_t)","ctbAcquisition",this,"ChangeSerialOffset(Long_t)");
   e->Connect("ReturnPressed()","ctbAcquisition",this,"ChangeSerialOffset()");


   label=new TGLabel(hframe,"N counters:");
   hframe->AddFrame(label,new TGLayoutHints(kLHintsTop | kLHintsLeft| kLHintsExpandX, 5, 5, 5, 5));
   label->MapWindow();
   label->SetTextJustify(kTextRight);


   eNumCount=new TGNumberEntry(hframe, 0, 9,999, TGNumberFormat::kNESInteger,
                                               TGNumberFormat::kNEANonNegative, 
			     TGNumberFormat::kNELLimitMinMax,0,16535);
   hframe->AddFrame(eNumCount,new TGLayoutHints(kLHintsTop |  kLHintsExpandX, 1, 1, 1, 1));
   eNumCount->MapWindow();;
   eNumCount->SetNumber(128*3);
   e= eNumCount->TGNumberEntry::GetNumberEntry();
   eNumCount->Connect("ValueSet(Long_t)","ctbAcquisition",this,"ChangeNumberOfChannels(Long_t)");
   e->Connect("ReturnPressed()","ctbAcquisition",this,"ChangeNumberOfChannels()");

   
   
    cout << "dr "<< endl;

   label=new TGLabel(hframe,"Dynamic Range:");
   hframe->AddFrame(label,new TGLayoutHints(kLHintsTop | kLHintsLeft| kLHintsExpandX, 5, 5, 5, 5));
   label->MapWindow();
   label->SetTextJustify(kTextRight);


   eDynRange=new TGNumberEntry(hframe, 0, 9,999, TGNumberFormat::kNESInteger,
                                               TGNumberFormat::kNEANonNegative, 
			     TGNumberFormat::kNELLimitMinMax,0,16535);
   hframe->AddFrame(eDynRange,new TGLayoutHints(kLHintsTop |  kLHintsExpandX, 1, 1, 1, 1));
   eDynRange->MapWindow();;
   eDynRange->SetNumber(24);
   e= eDynRange->TGNumberEntry::GetNumberEntry();
   eDynRange->Connect("ValueSet(Long_t)","ctbAcquisition",this,"ChangeDynamicRange(Long_t)");
   e->Connect("ReturnPressed()","ctbAcquisition",this,"ChangeDynamicRange()");

   
   


  hframe=new TGHorizontalFrame(this, 800,50);
  AddFrame(hframe,new TGLayoutHints(kLHintsTop | kLHintsExpandX , 10,10,10,10));
  hframe->MapWindow();
    
 

  label=new TGLabel(hframe,"Image Pixels");
  hframe->AddFrame(label,new TGLayoutHints(kLHintsTop | kLHintsLeft| kLHintsExpandX, 5, 5, 5, 5));
  label->MapWindow();
  label->SetTextJustify(kTextLeft);


  label=new TGLabel(hframe,"X: ");
  hframe->AddFrame(label,new TGLayoutHints(kLHintsTop | kLHintsLeft| kLHintsExpandX, 5, 5, 5, 5));
  label->MapWindow();
  label->SetTextJustify(kTextRight);


  ePixX=new TGNumberEntry(hframe, 0, 9,999, TGNumberFormat::kNESInteger,
                                               TGNumberFormat::kNEANonNegative, 
			     TGNumberFormat::kNELLimitMinMax,0,16535);
   hframe->AddFrame(ePixX,new TGLayoutHints(kLHintsTop |  kLHintsExpandX, 1, 1, 1, 1));
   ePixX->MapWindow();
   ePixX->SetNumber(400);
   e= ePixX->TGNumberEntry::GetNumberEntry();
   ePixX->Connect("ValueSet(Long_t)","ctbAcquisition",this,"ChangeImagePixels(Long_t)");
   e->Connect("ReturnPressed()","ctbAcquisition",this,"ChangeImagePixels()");



  label=new TGLabel(hframe,"Y: ");
  hframe->AddFrame(label,new TGLayoutHints(kLHintsTop | kLHintsLeft| kLHintsExpandX, 5, 5, 5, 5));
  label->MapWindow();
  label->SetTextJustify(kTextRight);


  ePixY=new TGNumberEntry(hframe, 0, 9,999, TGNumberFormat::kNESInteger,
                                               TGNumberFormat::kNEANonNegative, 
			     TGNumberFormat::kNELLimitMinMax,0,16535);
   hframe->AddFrame(ePixY,new TGLayoutHints(kLHintsTop |  kLHintsExpandX, 1, 1, 1, 1));
   ePixY->MapWindow();
   ePixY->SetNumber(400);
   e= ePixY->TGNumberEntry::GetNumberEntry();
   ePixY->Connect("ValueSet(Long_t)","ctbAcquisition",this,"ChangeImagePixels(Long_t)");
   e->Connect("ReturnPressed()","ctbAcquisition",this,"ChangeImagePixels()");




  hframe=new TGHorizontalFrame(this, 800,50);
  AddFrame(hframe,new TGLayoutHints(kLHintsTop | kLHintsExpandX , 10,10,10,10));
  hframe->MapWindow();
    


    label=new TGLabel(hframe,"Pedestal ");
   hframe->AddFrame(label,new TGLayoutHints(kLHintsTop | kLHintsLeft| kLHintsExpandX, 5, 5, 5, 5));
   label->MapWindow();
   label->SetTextJustify(kTextRight);


  
  cbGetPedestal= new TGCheckButton(hframe, "Acquire");
   hframe->AddFrame(cbGetPedestal,new TGLayoutHints(kLHintsTop | kLHintsLeft| kLHintsExpandX, 5, 5, 5, 5));
   cbGetPedestal->MapWindow();

  cbSubtractPedestal= new TGCheckButton(hframe, "Subtract");
   hframe->AddFrame(cbSubtractPedestal,new TGLayoutHints(kLHintsTop | kLHintsLeft| kLHintsExpandX, 5, 5, 5, 5));
   cbSubtractPedestal->MapWindow();


   cbSubtractPedestal->Connect("Toggled(Bool_t)","ctbAcquisition",this,"TogglePedSub(Bool_t)");

  cbCommonMode= new TGCheckButton(hframe, "Common Mode");
   hframe->AddFrame(cbCommonMode,new TGLayoutHints(kLHintsTop | kLHintsLeft| kLHintsExpandX, 5, 5, 5, 5));
   cbCommonMode->MapWindow();


   cbCommonMode->Connect("Toggled(Bool_t)","ctbAcquisition",this,"ToggleCommonMode(Bool_t)");


   bResetPedestal= new TGTextButton(hframe, "Reset");
   hframe->AddFrame(bResetPedestal,new TGLayoutHints(kLHintsTop | kLHintsLeft| kLHintsExpandX, 5, 5, 5, 5));
   bResetPedestal->MapWindow();
  

    bResetPedestal->Connect("Clicked()","ctbAcquisition",this,"resetPedestal()");





  hframe=new TGHorizontalFrame(this, 800,50);
  AddFrame(hframe,new TGLayoutHints(kLHintsTop | kLHintsExpandX , 10,10,10,10));
  hframe->MapWindow();
    

  cMinMaxRaw=new TGCheckButton(hframe,"Raw data ");
   hframe->AddFrame(cMinMaxRaw,new TGLayoutHints(kLHintsTop | kLHintsLeft| kLHintsExpandX, 5, 5, 5, 5));
   cMinMaxRaw->MapWindow();
   cMinMaxRaw->Connect("Toggled(Bool_t)","ctbAcquisition",this,"ChangeHistoLimitsRaw(Bool_t)");




    label=new TGLabel(hframe,"Min: ");
   hframe->AddFrame(label,new TGLayoutHints(kLHintsTop | kLHintsLeft| kLHintsExpandX, 5, 5, 5, 5));
   label->MapWindow();
   label->SetTextJustify(kTextRight);


    eMinRaw=new TGNumberEntry(hframe, 0, 9,999, TGNumberFormat::kNESInteger,
                                               TGNumberFormat::kNEANonNegative, 
			     TGNumberFormat::kNELLimitMinMax,0,16535);
   hframe->AddFrame(eMinRaw,new TGLayoutHints(kLHintsTop |  kLHintsExpandX, 1, 1, 1, 1));
   eMinRaw->MapWindow();;
   eMinRaw->SetNumber(0);
    e= eMinRaw->TGNumberEntry::GetNumberEntry();
    eMinRaw->Connect("ValueSet(Long_t)","ctbAcquisition",this,"ChangeHistoLimitsRaw(Long_t)");
    e->Connect("ReturnPressed()","ctbAcquisition",this,"ChangeHistoLimitsRaw()");


    label=new TGLabel(hframe,"Max: ");
   hframe->AddFrame(label,new TGLayoutHints(kLHintsTop | kLHintsLeft| kLHintsExpandX, 5, 5, 5, 5));
   label->MapWindow();
   label->SetTextJustify(kTextRight);




    eMaxRaw=new TGNumberEntry(hframe, 0, 9,999, TGNumberFormat::kNESInteger,
                                               TGNumberFormat::kNEANonNegative, 
			     TGNumberFormat::kNELLimitMinMax,0,16535);
   hframe->AddFrame(eMaxRaw,new TGLayoutHints(kLHintsTop |  kLHintsExpandX, 1, 1, 1, 1));
   eMaxRaw->MapWindow();;
   eMaxRaw->SetNumber(16535);

    e= eMaxRaw->TGNumberEntry::GetNumberEntry();
    eMaxRaw->Connect("ValueSet(Long_t)","ctbAcquisition",this,"ChangeHistoLimitsRaw(Long_t)");
    e->Connect("ReturnPressed()","ctbAcquisition",this,"ChangeHistoLimitsRaw()");


  hframe=new TGHorizontalFrame(this, 800,50);
  AddFrame(hframe,new TGLayoutHints(kLHintsTop | kLHintsExpandX , 10,10,10,10));
  hframe->MapWindow();
    


  cMinMaxPedSub=new TGCheckButton(hframe,"Pedestal Subtracted ");
   hframe->AddFrame(cMinMaxPedSub,new TGLayoutHints(kLHintsTop | kLHintsLeft| kLHintsExpandX, 5, 5, 5, 5));
   cMinMaxPedSub->MapWindow();
   cMinMaxPedSub->Connect("Toggled(Bool_t)","ctbAcquisition",this,"ChangeHistoLimitsPedSub(Bool_t)");


    label=new TGLabel(hframe,"Min: ");
   hframe->AddFrame(label,new TGLayoutHints(kLHintsTop | kLHintsLeft| kLHintsExpandX, 5, 5, 5, 5));
   label->MapWindow();
   label->SetTextJustify(kTextRight);


    eMinPedSub=new TGNumberEntry(hframe, 0, 9,999, TGNumberFormat::kNESInteger,
                                               TGNumberFormat::kNEAAnyNumber, 
			     TGNumberFormat::kNELLimitMinMax,-16535,16535);
   hframe->AddFrame(eMinPedSub,new TGLayoutHints(kLHintsTop |  kLHintsExpandX, 1, 1, 1, 1));
   eMinPedSub->MapWindow();;
   eMinPedSub->SetNumber(-100);

    e= eMinPedSub->TGNumberEntry::GetNumberEntry();
    
    eMinPedSub->Connect("ValueSet(Long_t)","ctbAcquisition",this,"ChangeHistoLimitsPedSub(Long_t)");
    e->Connect("ReturnPressed()","ctbAcquisition",this,"ChangeHistoLimitsPedSub()");


    label=new TGLabel(hframe,"Max: ");
   hframe->AddFrame(label,new TGLayoutHints(kLHintsTop | kLHintsLeft| kLHintsExpandX, 5, 5, 5, 5));
   label->MapWindow();
   label->SetTextJustify(kTextRight);


    eMaxPedSub=new TGNumberEntry(hframe,0, 9,999, TGNumberFormat::kNESInteger,
                                               TGNumberFormat::kNEAAnyNumber, 
			     TGNumberFormat::kNELLimitMinMax,-16535,16535);
   hframe->AddFrame(eMaxPedSub,new TGLayoutHints(kLHintsTop |  kLHintsExpandX, 1, 1, 1, 1));
   eMaxPedSub->MapWindow();;
   eMaxPedSub->SetNumber(100);


    e= eMaxPedSub->TGNumberEntry::GetNumberEntry();
    eMaxPedSub->Connect("ValueSet(Long_t)","ctbAcquisition",this,"ChangeHistoLimitsPedSub(Long_t)");
    e->Connect("ReturnPressed()","ctbAcquisition",this,"ChangeHistoLimitsPedSub()");


  hframe=new TGHorizontalFrame(this, 800,50);
  AddFrame(hframe,new TGLayoutHints(kLHintsTop | kLHintsExpandX , 10,10,10,10));
  hframe->MapWindow();
    




   TGTextButton *b= new TGTextButton(hframe, "Fit Panel ADC:");
   hframe->AddFrame(b,new TGLayoutHints(kLHintsTop | kLHintsLeft| kLHintsExpandX, 5, 5, 5, 5));
   b->MapWindow();
  

    b->Connect("Clicked()","ctbAcquisition",this,"FitADC()");


    eFitADC=new TGNumberEntry(hframe, 0, 9,999, TGNumberFormat::kNESInteger,
                                               TGNumberFormat::kNEANonNegative, 
			     TGNumberFormat::kNELLimitMinMax,0,31);
   hframe->AddFrame( eFitADC,new TGLayoutHints(kLHintsTop |  kLHintsExpandX, 1, 1, 1, 1));
   eFitADC->MapWindow();;



  hframe=new TGHorizontalFrame(this, 800,50);
  AddFrame(hframe,new TGLayoutHints(kLHintsTop | kLHintsExpandX , 10,10,10,10));
  hframe->MapWindow();
    




   b= new TGTextButton(hframe, "Plot bit:");
   hframe->AddFrame(b,new TGLayoutHints(kLHintsTop | kLHintsLeft| kLHintsExpandX, 5, 5, 5, 5));
   b->MapWindow();
  

    b->Connect("Clicked()","ctbAcquisition",this,"plotBit()");


    eBitPlot=new TGNumberEntry(hframe, 0, 9,999, TGNumberFormat::kNESInteger,
                                               TGNumberFormat::kNEANonNegative, 
			     TGNumberFormat::kNELLimitMinMax,0,64);
   hframe->AddFrame( eBitPlot,new TGLayoutHints(kLHintsTop |  kLHintsExpandX, 1, 1, 1, 1));
   eBitPlot->MapWindow();;





  hframe=new TGHorizontalFrame(this, 800,50);
  AddFrame(hframe,new TGLayoutHints(kLHintsTop | kLHintsExpandX , 10,10,10,10));
  hframe->MapWindow();
    


    label=new TGLabel(hframe,"X ");
   hframe->AddFrame(label,new TGLayoutHints(kLHintsTop | kLHintsLeft| kLHintsExpandX, 5, 5, 5, 5));
   label->MapWindow();
   label->SetTextJustify(kTextRight);


    label=new TGLabel(hframe," ");
   hframe->AddFrame(label,new TGLayoutHints(kLHintsTop | kLHintsLeft| kLHintsExpandX, 5, 5, 5, 5));
   label->MapWindow();
   label->SetTextJustify(kTextLeft);
   lClickX=label;



    label=new TGLabel(hframe,"Y ");
   hframe->AddFrame(label,new TGLayoutHints(kLHintsTop | kLHintsLeft| kLHintsExpandX, 5, 5, 5, 5));
   label->MapWindow();
   label->SetTextJustify(kTextRight);



    label=new TGLabel(hframe," ");
   hframe->AddFrame(label,new TGLayoutHints(kLHintsTop | kLHintsLeft| kLHintsExpandX, 5, 5, 5, 5));
   label->MapWindow();
   label->SetTextJustify(kTextLeft);
   lClickY=label;


    label=new TGLabel(hframe,"Value ");
   hframe->AddFrame(label,new TGLayoutHints(kLHintsTop | kLHintsLeft| kLHintsExpandX, 5, 5, 5, 5));
   label->MapWindow();
   label->SetTextJustify(kTextRight);





    label=new TGLabel(hframe," ");
   hframe->AddFrame(label,new TGLayoutHints(kLHintsTop | kLHintsLeft| kLHintsExpandX, 5, 5, 5, 5));
   label->MapWindow();
   label->SetTextJustify(kTextLeft);
   lClickValue=label;







   b= new TGTextButton(hframe, "Refresh");
   hframe->AddFrame(b,new TGLayoutHints(kLHintsTop | kLHintsLeft| kLHintsExpandX, 5, 5, 5, 5));
   b->MapWindow();
  

    b->Connect("Clicked()","ctbAcquisition",this,"Draw()");










  acqThread = new TThread("acqThread",
			  ctbAcquisition::ThreadHandle,(void*)this);
  
  cout <<"Registering data callback" << endl;
  try {
    myDet->registerDataCallback(&dataCallback, (void*)this);
  } CATCH_DISPLAY ("Could not get register call back.", "ctbAcquisition::ctbAcquisition") 
      try {
	myDet->setRxZmqDataStream(true); 
      } CATCH_DISPLAY ("Could not get set RxZmqDataStream.", "ctbAcquisition::ctbAcquisition") 
  cout <<"Done" << endl;
  
    //  mgAdcs=new TMultiGraph();
    adcStack=new THStack();
    TH1F *h;
    int nSamples=nAnalogSamples;
     
    for (int i=0; i<NADCS; i++) {
      sprintf(tit,"adc%d",i);
      nSamples=nAnalogSamples;
      h=new TH1F(tit,tit,nSamples,0,nSamples);
      h->SetLineColor(i+1);
      h->SetLineWidth(2);
      adcStack->Add(h);
      adcHisto[i]=h;
      plotFlag[i]=0;
      // h->SetMinimum(-1);
      //  h->SetMaximum(16385);
    }


    //  mgAdcs=new TMultiGraph();
      bitStack=new THStack();
      // bitStack=adcStack;
    TH1F *hb;
    for (int i=0; i<NSIGNALS; i++) {
      sprintf(tit,"bit%d",i); 
      nSamples=nDigitalSamples;
      hb=new TH1F(tit,tit,nSamples,0,nSamples);
      hb->SetLineColor(i+1);
      hb->SetLineWidth(2);
      bitStack->Add(hb);
      bitHisto[i]=hb;
      bitOffset[i]=0;
      bitPlotFlag[i]=0;
      // h->SetMinimum(-1);
      //  h->SetMaximum(16385);
    }


    countsStack=new THStack();
    TH1F *h1;
    for (int i=0; i<NADCS; i++) {
      sprintf(tit,"Fadc%d",i);
      h1=new TH1F(tit,tit,2*16386,-16385,16385);
      h1->SetLineColor(i+1);
      h1->SetFillColor(i+1);
      h1->SetLineWidth(2);
      countsStack->Add(h1);
      countsHisto[i]=h1;
    }

    dataStructure=NULL;
    commonMode=NULL;
    photonFinder=NULL;
    h2DMapAn=NULL;
    h2DMapDig=NULL;
    //h2Scan=NULL;
    h1DMap=NULL;

    changeDetector(cbDetType->GetSelected());

  

  //  display could be updated with TTimer instead of with histogramfillthread:
  //   plotTimer= new TTimer("ctbAcquisition::Draw()",100); 
  

  //  plotTimer->Connect("TurnOff()", "ctbAcquisition", this, "Draw()");
}




void ctbAcquisition::canvasClicked() {
   int event = gPad->GetEvent();
   if (event != 11) return; 
   TObject *select = gPad->GetSelected();
   if (!select) return;

    if (select->InheritsFrom("TH2")) {
      TH2* hh=(TH2*)select;
      



    int px = gPad->GetEventX();
    int py = gPad->GetEventY();
    Float_t upy = gPad->AbsPixeltoY(py);
    Float_t y = gPad->PadtoY(upy);
    Float_t upx = gPad->AbsPixeltoX(px);
    Float_t x = gPad->PadtoY(upx);


    //  cout << "x: " << x << " y: " << y << " " << hh->GetBinContent(hh->GetXaxis()->FindBin(x), hh->GetYaxis()->FindBin(y)) << endl;

    
    lClickX->SetText(hh->GetXaxis()->FindBin(x)-1);
    lClickY->SetText( hh->GetYaxis()->FindBin(y)-1);
    lClickValue->SetText(hh->GetBinContent(hh->GetXaxis()->FindBin(x), hh->GetYaxis()->FindBin(y)));
    





    } else if (select->InheritsFrom("TH1")) {

      TH1* h1=(TH1*)select;
    int px = gPad->GetEventX();
    Float_t upx = gPad->AbsPixeltoX(px);
    Float_t x = gPad->PadtoY(upx);


    //  cout << "x: " << x << " y: " << y << " " << hh->GetBinContent(hh->GetXaxis()->FindBin(x), hh->GetYaxis()->FindBin(y)) << endl;

    
    lClickX->SetText(h1->GetXaxis()->FindBin(x)-1);
    lClickY->SetText(" ");
    lClickValue->SetText(h1->GetBinContent(h1->GetXaxis()->FindBin(x)));
    




    } else// if  ((select->ClassName())=="THStack") {
      {

	
    int px = gPad->GetEventX();
    int py = gPad->GetEventY();
    Float_t upy = gPad->AbsPixeltoY(py);
    Float_t y = gPad->PadtoY(upy);
    Float_t upx = gPad->AbsPixeltoX(px);
    Float_t x = gPad->PadtoY(upx);


    lClickX->SetText(x);
    lClickY->SetText(y);
    lClickValue->SetText("");
    


    }

}






void ctbAcquisition::setCanvas(TCanvas* c) {
  myCanvas=c;  
  myCanvas->cd();
  myCanvas->AddExec("dynamic",Form("((ctbAcquisition*)%p)->canvasClicked()",this));
  // myCanvas->AddExec("ex","canvasClicked()");
}
void ctbAcquisition::dataCallback(sls::detectorData *data, long unsigned int index, unsigned int dum, void* pArgs) {

  // return 
  ((ctbAcquisition*)pArgs)->plotData(data,index);
}


int ctbAcquisition::plotData(sls::detectorData *data, int index) {

  /*
******************************************************************
When selecting dbit
amount of data is nadc * nasamples  * 16 bit + ndbitlist  * ndsamples (bits)
order of data
analog:
sample0 (adc0 + adc1 +...)
sample1 (adc0 + adc1 +...)
digital:
dbit0 (sample0 + sample1 ...)
dbit1 (sample0 + sample1..)

when setting dbit to all
amount of data: nadc * nasamples  * 16 bit + 8  * ndsamples * 64 bit
what you had before.. 
except analog first, then digital
analog:
sample0 (adc0 + adc1 +...)
sample1 (adc0 + adc1 +...)
digital:
sample0 (dbit0 + dbit1 +...)
sample1 (dbit0 + dbit1 +...)if (cmd == "rx_dbitlist") {

        if (action == PUT_ACTION) {
            std::vector <int> dbitlist;

            // if not all digital bits enabled
            if (std::string(args[1]) != "all") {
                for (int i = 1; i < narg; ++i) {
                    int temp = 0;
                    if (!sscanf(args[i], "%d", &temp))
                        return std::string("Could not scan dbitlist value ") +
                               std::string(args[i]);
                    if (temp < 0 || temp > 63)
                        return std::string("dbitlist value should be between 0 and 63 ") +
                               std::string(args[i]);
                    dbitlist.push_back(temp);
                }
                if (dbitlist.size() > 64) {
                    return std::string("Max number of values for dbitlist is 64 ");
                }
            }

            myDet->setReceiverDbitList(dbitlist, detPos); 
        }
        
******************************************************************
*/

  // cout << "plot data" << endl;


  //  cout <<"global plot is " << globalPlot << endl;
  // cout << "*******************************************" <<endl;
   cout <<"------Plot: "<<  index << " prog:" << data->progressIndex << " nx:" << data->nx << " ny: " << data->ny << " " << data->fileName << " bytes: " << data->databytes << " dr:"<< data->dynamicRange << " fi: " << data ->fileIndex <<  endl;
  if (globalPlot || cbGetPedestal->IsOn()) {
    //#ifdef TESTADC
    //  cout <<"------"<<  index << " " << ip << " " << data->npoints << endl;
    //#endif
  int ig=0;
  int i, ii, ib;
  // TList *l= adcStack->GetHists();
  // TList *l1= countsStack->GetHists();
  TH1F *h;
  TH1F *h1;
  TH1F *hb;
  int x;
  double ped=0;
  int dsize=-1;
  int  *val=NULL;
  int nx=1, ny=1;

  if (dataStructure) { 
    dataStructure->getDetectorSize(nx,ny);
    cout << "Data structure: " << dataStructure << " size " << nx << " " << ny <<  endl;
  }
  int  dr=24, soff=2;
  if (deserializer) {
    nx=eNumCount->GetIntNumber();
    dr=eDynRange->GetIntNumber();
    soff=eSerOff->GetIntNumber();
    // cout <<"deserializer: " << endl;
    // cout << "Number of chans:\t" << nx << endl;
    // cout << "Serial Offset:\t" << soff << endl;
    // cout << "Dynamic range:\t" << dr << endl;
    
  }

  i=0;
  int nadc;
  int ndbit;

  tenG = 0;
  


	
  if (adclist.empty()) 
    nadc=32;
  else
    nadc=adclist.size();
      
  std::vector <int> plotlist;
  if (dbitlist.empty()) {
    ndbit=64;
    dBitOffset=0;
    for (ib=0; ib<64; ib++){
      if (bitPlotFlag[ib]) {
	plotlist.push_back(ib);
      }
    }
  } else  
    ndbit=dbitlist.size();
  if (tenG){
   
    if (nDigitalSamples && nAnalogSamples){
      if (nDigitalSamples>nAnalogSamples)
	dsize=nDigitalSamples*(32*2+8);
      else
	dsize=nAnalogSamples*(32*2+8);
    } else
      dsize=32*2*nAnalogSamples+8*nDigitalSamples;
    
  } else
    dsize=nadc*2*nAnalogSamples+ndbit*(nDigitalSamples-dBitOffset/8)/8;
  
  cout << "dataBytes is " << data->databytes << " expected " << dsize << endl;

  cout << "*******************************************" <<endl;
  

  uint16_t aval;
  i=0;


  char *d_data;
  if (tenG)
    d_data=	data->data;
  else
    d_data =	data->data+2*nadc*nAnalogSamples;
  char dval;


  if (dataStructure) {
    
    
    for (int x=0; x<nx; x++) {
      for (int y=0; y<ny; y++) {
	ped=0;
	aval=dataStructure->getValue(data->data,x,y);
	//aval=dataStructure->getChannel(data->data,x,y);
	// cout << x << " " <<y << " "<< aval << endl;
	if (cbGetPedestal->IsOn()) {
	  if (photonFinder) {
	    photonFinder->addToPedestal(aval,x,y);
	  }
	}
	
      if (cbSubtractPedestal->IsOn()) {
	if (photonFinder) {
	  ped=photonFinder->getPedestal(x,y,cmSub);
	}
      }	
      if (h2DMapAn)
	h2DMapAn->SetBinContent(x+1,y+1,aval-ped);
  
      
      
      
      if (h2DMapDig)
	h2DMapDig->SetBinContent(x+1,y+1,dataStructure->getGain(data->data,x,y));
      
      
      }
    }
  } else 
  if (deserializer) {
    cout << "deserializer"<< endl;
    if (dbitlist.empty())
      val=deserializer::deserializeAll(d_data,plotlist,dr,nx,soff);//dataStructure->getData(d_data);
    else
      val=deserializer::deserializeList(d_data,dbitlist,dr,nx,soff);//dataStructure->getData(d_data);
      
    
    if (val) {
      if (h1DMap){
	for (x=0; x<nx; x++) {
	  h1DMap->SetBinContent(x+1,val[x]); 
	}
      }  
      delete [] val;
    } else
      cout << "get val did not succeed"<<endl;      
  } else { 
    cout << "analog histo " << endl;
    for (ip=0; ip<nAnalogSamples; ip++) { 
      for (ii=0; ii<nadc; ii++) {
	//for (ip=0; ip<adclist.size(); ip++) {	
	if (adclist.empty()) 
	  ig=ii;
	else
	  ig=adclist.at(ii);

	// if (tenG)
	//   aval=data->getChannel(i);
	// else
	  aval=data->getChannel(i);//*((uint16_t*)(data->cvalues+i*2));//
      
	if (plotFlag[ig]) {
	  
	  //if (enableFlag[ig]) {
	  h=adcHisto[ig];
	  h1=countsHisto[ig];
	  //}
	  
	//	cout << data->getChannel(i) << endl;
	  h->SetBinContent(ip+1,aval);	
	  h1->Fill(aval);
	} 
	
	i++;
      }
      if (tenG) i+=4;
      
    }
    
  
    cout << "bit histo"<< endl;

    if (dbitlist.empty())  {
      for (ip=0; ip<nDigitalSamples; ip++) {
	for (ig=0; ig<8; ig++) { 
	  if (tenG)
	    dval=*(d_data+ip*(8+32*2)+32*2+ig);
	  else
	    dval=*(d_data+ip*8+ig);
	  
	  for (ib=(ig)*8; ib<(ig+1)*8; ib++) {
	    if (bitPlotFlag[ib]) {
	      hb=bitHisto[ib];	
	    if (dval&(1<<(ib%8)))
	      hb->SetBinContent(ip+1,1+bitOffset[ib]);
	    else
	      hb->SetBinContent(ip+1,bitOffset[ib]); 
	    }
	  }
	
	}
      }
    } else {
      ii=0;
      int iii=0;
      for (const auto &value : dbitlist) {
	ib=value;
	hb=bitHisto[ib];  
	//	cout << dec <<endl <<  "Bit " << ib << " " << (nDigitalSamples-dBitOffset)/8 << endl;
	iii=0;
	for (ip=0; ip<(nDigitalSamples-dBitOffset)/8; ip++) {
	  if (bitPlotFlag[ib]) {
	    dval=*(d_data+ii*nDigitalSamples/8+ip);
	    
	    for (int jj=0; jj<8; jj++) {
	      if (dval&(1<<jj))
		hb->SetBinContent(iii,1+bitOffset[ib]);
	      else
		hb->SetBinContent(iii,bitOffset[ib]);
	      iii++;
	    }
	  }
	}
	ii++;
      }
    }
  }
  Draw();
  //  iScanStep++;
  if (photonFinder)
    photonFinder->newFrame();
  }
  return 0;

}



void ctbAcquisition::Draw(){
  if (globalPlot) {
    //   TThread::Lock();
    cout << "Draw" << endl;
    if (myCanvas) {
      if (adcPlot && dbitPlot) {

	myCanvas->cd(1);
	//	myCanvas->Modified();
	//	myCanvas->Update();
	gPad->Modified();
	gPad->Update();

	myCanvas->cd(2);
	//	myCanvas->Modified();
	//	myCanvas->Update();
	gPad->Modified();
	gPad->Update();

      } else {
	
	myCanvas->cd();
	myCanvas->Modified();
	myCanvas->Update();
	
      }
    }
    // TThread::UnLock();
  }
  
}


//here!!
void ctbAcquisition::changePlot(){
  if (rbPlotOff->IsOn()) {
    adcPlot=0;
    dbitPlot=0;   
    try {
      myDet->registerDataCallback(nullptr, this);  
    } CATCH_DISPLAY ("Could not get unregister call back.", "ctbAcquisition::ctbAcquisition")   
    try {
      myDet->setRxZmqDataStream(false); 
    } CATCH_DISPLAY ("Could not get unset RxZmqDataStream.", "ctbAcquisition::ctbAcquisition") 
  } else {
  try {
    myDet->registerDataCallback(&dataCallback, (void*)this);
  } CATCH_DISPLAY ("Could not get register call back.", "ctbAcquisition::ctbAcquisition")   
    try {
      myDet->setRxZmqDataStream(true); 
    } CATCH_DISPLAY ("Could not get set RxZmqDataStream.", "ctbAcquisition::ctbAcquisition") 
    adcPlot=0;
    dbitPlot=0;
    for (int ii=0; ii<NADCS; ii++)
      if (plotFlag[ii]==1) adcPlot=1;
    for (int ii=0; ii<NSIGNALS; ii++)
      if (bitPlotFlag[ii]==1) dbitPlot=1;
  }

  globalPlot=adcPlot || dbitPlot;

  if (globalPlot!=0 && myCanvas) {
    if (adcPlot && dbitPlot) {
      if (myCanvas->GetPad(1)==NULL || myCanvas->GetPad(2)==NULL) {
	myCanvas->Clear();
	myCanvas->Divide(1,2);
      } else
	cout << "Pad already there" << endl;
      myCanvas->cd(1);
    }    else {
      myCanvas->Clear();
	// myCanvas->Divide(1,1);
      myCanvas->cd();
    }

    if (adcPlot) {
      if (rbWaveform->IsOn())
	if (adcStack)
	  adcStack->Draw("NOSTACK");
	else
	  cout << "adcStack is NULL" << endl;
      else if (rbDistribution->IsOn())
	if (countsStack)
	  countsStack->Draw("NOSTACK");
	else
	  cout << "countsStack is NULL" << endl;
      else if (rb2D->IsOn()) {
	if (h2DMapAn)
	  h2DMapAn->Draw("colz");
	else if (h1DMap)
	  h1DMap->Draw();
	else
	  cout << "h2DMap  and h1DMap are NULL" << endl;
      } 
    }

    if (dbitPlot) {
      if (adcPlot)
	myCanvas->cd(2);
      if (rb2D->IsOn()) {
	if (h2DMapDig)
	  h2DMapDig->Draw("colz");
	else if (h1DMap)
	  h1DMap->Draw();
      } else if (bitStack)
	bitStack->Draw("NOSTACK");
      else
	cout << "bitStack is NULL" << endl;
    }
    
    
    // else if (rbScan->IsOn()) {
    //   if (h2Scan)
    // 	h2Scan->Draw("colz");
    //   else
    // 	cout << "h2Scan is NULL" << endl;
    // }
    
    Draw();
  
  }
}





void ctbAcquisition::changeDetector(){
  //  cout << "change detector " << i << " old " << cbDetType->GetSelected() << endl;
  
  if (dataStructure) delete dataStructure;
  if (commonMode) delete commonMode; 
  if (photonFinder)   delete photonFinder;
  if (h2DMapAn)  delete h2DMapAn; 
  if (h2DMapDig)  delete h2DMapDig; 
  if (h1DMap) delete h1DMap;
  //  if (h2Scan) delete h2Scan;
  h2DMapAn=NULL;
  h2DMapDig=NULL;
  h1DMap=NULL;
  // h2Scan=NULL;
  photonFinder=NULL;
  dataStructure=NULL;
  commonMode=NULL;

  // TH2F *h2ScanOld=h2Scan;


  int nx,ny;
  int csize=3;
  int nsigma=5;
  commonModeSubtraction* cm=0;
   eNumCount->SetState(kFALSE);
  eDynRange->SetState(kFALSE);
  eSerOff->SetState(kFALSE);
  ePixX->SetState(kFALSE);
  ePixY->SetState(kFALSE);
    
  deserializer=0;
  if (rb2D->IsOn() ) {//|| rbScan->IsOn()
    switch  (cbDetType->GetSelected()) {
    case DESERIALIZER:
      deserializer=1;
       cout << "DESERIALIZER!" << endl;
      // dataStructure=new moench03T1CtbData(); 
      // commonMode=new moench03CommonMode();
      break;
     case MOENCH04:
      try {
        auto retval = myDet->getTenGiga().tsquash("Different values");
        if (retval) {
          dataStructure=new moench04CtbZmq10GbData(nAnalogSamples, nDigitalSamples); 
        } else {
          dataStructure=new moench04CtbZmqData(nAnalogSamples, nDigitalSamples); 
        }
      } CATCH_DISPLAY ("Could not get ten giga enable.", "ctbAcquisition::changeDetector")

       cout << "MOENCH 0.4!" << endl;
       commonMode=new moench03CommonMode();
      break;
     case MOENCH03:
       //try {
	// auto retval = myDet->getTenGiga().tsquash("Different values");
	// if (retval) {
          dataStructure=new moench03T1ZmqDataNew(nAnalogSamples); 
        // } else {
        //   dataStructure=new moench04CtbZmqData(nAnalogSamples, nDigitalSamples); 
        // }
	  //} CATCH_DISPLAY ("Could not get ten giga enable.", "ctbAcquisition::changeDetector")

       cout << "MOENCH 0.3! USE JUNGFRAU MODULE!" << endl;
       commonMode=new moench03CommonMode();
      break;
       case IMAGE32B:
       //try {
       	// auto retval = myDet->getTenGiga().tsquash("Different values");
       	// if (retval) {
       	// if (deserializer) {
       	   ePixX->SetState(kTRUE);
       	   ePixY->SetState(kTRUE);
       	   // }
       	 dataStructure=new imageZmq32bit(ePixX->GetIntNumber(),ePixY->GetIntNumber()); 
        // } else {
        //   dataStructure=new moench04CtbZmqData(nAnalogSamples, nDigitalSamples); 
        // }
       	  //} CATCH_DISPLAY ("Could not get ten giga enable.", "ctbAcquisition::changeDetector")

       	 cout << "Image 32bit, no channel shuffling" << endl;
       commonMode=NULL;
       break;
   
       case IMAGE16B:
       //try {
	// auto retval = myDet->getTenGiga().tsquash("Different values");
	// if (retval) {
	// if (deserializer) {
	   ePixX->SetState(kTRUE);
	   ePixY->SetState(kTRUE);
	   // }
	 dataStructure=new imageZmq16bit(ePixX->GetIntNumber(),ePixY->GetIntNumber()); 
        // } else {
        //   dataStructure=new moench04CtbZmqData(nAnalogSamples, nDigitalSamples); 
        // }
	  //} CATCH_DISPLAY ("Could not get ten giga enable.", "ctbAcquisition::changeDetector")

	 cout << "Image 16bit, no channel shuffling" << endl;
       commonMode=NULL;
       break;
   
    // case 1:
    //   cout << "************** T!!!!!!!!!!" << endl;
    //   dataStructure=new moench03TCtbData(); 
    //   commonMode=new moench03CommonMode();
    //   break;
    case MOENCH02:
      cout << "MOENCH 0.2" << endl;
      dataStructure=new moench02CtbData(); 
      commonMode=new moenchCommonMode();
      break;  
    // case 2:
    //   dataStructure=new jungfrau10ModuleData(); 
    //   commonMode=new commonModeSubtraction();
    //  break;
    // case 3:
    //   cout << "************** Flat!!!!!!!!!!" << endl;
    //   dataStructure=new moench03CtbData(); 
    //   commonMode=new moench03CommonMode();
    //   break;
    // case MYTHEN301:
    //   deserializer=1;
    //   cout << "MYTHEN 3 0.1" << endl;
    //   dataStructure=new mythen3_01_jctbData(eNumCount->GetIntNumber(),eDynRange->GetIntNumber(),eSerOff->GetIntNumber());
    //   //( int nch=64*3,int dr=24, int off=5)
    //   eNumCount->SetState(kTRUE);
    //   eDynRange->SetState(kTRUE);
    //   eSerOff->SetState(kTRUE);
    //   commonMode=NULL;
    //   dim=1; 
    //   break;
    // case ADCSAR2:
    //   deserializer=1;
    //   //adcsar2
    //   dataStructure=new adcSar2_jctbData();
    //   //need to use configurable number of counters, offset or dynamic range?
    //   commonMode=NULL;
    //   dim=1;
    //   break;
      
    // case MYTHEN302:
    //   deserializer=1;
    //   cout << "MYTHEN 3 0.2" << endl;
    //   dataStructure=new mythen3_02_jctbData(eNumCount->GetIntNumber(),eDynRange->GetIntNumber(),eSerOff->GetIntNumber());
    //   //( int nch=64*3,int dr=24, int off=5)
    //   eNumCount->SetState(kTRUE);
    //   eDynRange->SetState(kTRUE);
    //   eSerOff->SetState(kTRUE);
    //   commonMode=NULL;
    //   dim=1; 
    //   break;
    default:
      dataStructure=NULL;
       commonMode=NULL;
    }
    if (cbCommonMode->IsOn()) cm=commonMode;
  }
    
    if (dataStructure || deserializer) {
      if (dataStructure) {
	photonFinder=new singlePhotonDetector(dataStructure,csize,nsigma,1,cm); //sign is positive - should correct with ADC mask, no common mode 
	//photonFinder=new singlePhotonDetector(dataStructure,csize,nsigma,1,cm); //sign is positive - should correct with ADC mask, no common mode
	dataStructure->getDetectorSize(nx,ny);
	
      }
      if (deserializer) {
	ny=1;
	nx=eNumCount->GetIntNumber();
	eNumCount->SetState(kTRUE);
	eDynRange->SetState(kTRUE);
	eSerOff->SetState(kTRUE);
      }
      //  cout << "h size is " << nx << " " << ny << endl;
      int ymax=ny, xmax=nx;
      // if (ny>500) {ny=ny/2;}
      // if (nx>500) {nx=nx/2;}
      cout << "*** " << nx << " " << ny << endl;
      if (rb2D->IsOn()) {
	if (ny>1) {
	  h2DMapAn=new TH2F("h2dmapAn","",nx,0,xmax,ny,0,ymax);
	  h2DMapAn->SetStats(kFALSE);
	  cout << "Created h2DMapAn"<< endl; 
	  if (dbitPlot && adcPlot){
	    h2DMapDig=new TH2F("h2dmapDig","",nx,0,xmax,ny,0,ymax);
	    h2DMapDig->SetStats(kFALSE);
	    cout << "Created h2DMapDig"<< endl;
	  }
	} else  {
	  h1DMap=new TH1F("h1dmap","",nx,0,xmax);
	  h1DMap->SetStats(kFALSE);
	  cout << "Created h1DMap"<< endl;
	}
      } // else if  (rbScan->IsOn()) {
      // 	int nsteps=0;//myDet->getScanSteps(0);	
      // 	double stepmin=0, stepmax=1;
      // 	if (nsteps>0) {
      // 	  stepmin=myDet->getScanStep(0,0);
      // 	  stepmax=myDet->getScanStep(0,nsteps-1);
      // 	}
      // 	cout << "************ creating scan histogram " << nx*ny << " " << nsteps << " " << stepmin << " " << stepmax << endl;
      // 	if (nsteps<1) nsteps=1;
      // 	double hmin=stepmin, hmax=stepmax;
      // 	if (stepmin>stepmax) {
      // 	  hmin=stepmax;
      // 	  hmax=stepmin;
      // 	}
      // 	h2Scan=new TH2F("h2scan","",nx*ny,0,nx*ny,nsteps,hmin,hmax);
      // }

    }

     
    cout << "done " << endl;
}



void ctbAcquisition::changeDetector(int i){
  changePlot();
  changeDetector();
}

void ctbAcquisition::changePlot(int i){
  changePlot();
  changeDetector();
}



void ctbAcquisition::setGraph(int i ,int en, Pixel_t col) {
  char name[100];
  //  TList *l= mgAdcs->GetListOfGraphs();
  sprintf(name,"adc%d",i);
 
  // TList *l= adcStack->GetHists();
  TH1F *h=adcHisto[i];//(TH1F*)l->At(i);;
  TH1F *h1=countsHisto[i];//(TH1F*)(countsStack->GetHists()->At(i));
  if (en) {
    plotFlag[i]=1;
    h->SetLineColor(TColor::GetColor(col));
    h1->SetLineColor(TColor::GetColor(col));
    h1->SetFillColor(TColor::GetColor(col));

    if (adcStack->GetHists()) 
      // if (adcStack->GetHists()->GetEntries())
	if (adcStack->GetHists()->Contains(h)==0)
	  adcStack->Add(h);

    if (countsStack->GetHists()) 
      if (countsStack->GetHists()->Contains(h1)==0)
	countsStack->Add(h1);
    
    cout << "Enable plot " << i << " color " << col << endl;
  } else {
    cout << "Disable plot " << i << endl; 
    plotFlag[i]=0;
    if (adcStack->GetHists()) 
      // if (adcStack->GetHists()->GetEntries())
      if (adcStack->GetHists()->Contains(h))
	adcStack->RecursiveRemove(h);
    if (countsStack->GetHists()) 
      if (countsStack->GetHists()->Contains(h1))
	countsStack->RecursiveRemove(h1);
  }
  cout << countsStack->GetHists()->GetEntries() << endl;

  cout << "Number of histos " << adcStack->GetHists()->GetEntries() << endl;

  changePlot();

  // globalPlot=0;
  // for (int ii=0; ii<NADCS; ii++)
  //   if (plotFlag[ii]==1) globalPlot=1;
  // // if (globalPlot)  Draw();


}


void ctbAcquisition::setBitGraph(int i ,int en, Pixel_t col) {
  char name[100];
  //  TList *l= mgAdcs->GetListOfGraphs();
  sprintf(name,"bit%d",i);
  // TList *l= adcStack->GetHists();
  TH1F *h=bitHisto[i];//(TH1F*)l->At(i);;
  if (en) {
    //cout<< "enabling plot of bit "<<i << endl;
    bitPlotFlag[i]=1;
    h->SetLineColor(TColor::GetColor(col));
    if (bitStack->GetHists()) 
      //if (bitStack->GetHists()->GetEntries()) 
	if (bitStack->GetHists()->Contains(h)==0)
	  bitStack->Add(h);

    
    cout << "Enable bit plot " << i << " color " << col << endl;
  } else {
    cout << "Disable bit plot " << i << endl; 
    bitPlotFlag[i]=0;
    if (bitStack->GetHists()) 
      // if (bitStack->GetHists()->GetEntries())
	if (bitStack->GetHists()->Contains(h))
	  bitStack->RecursiveRemove(h);
  }
  cout << "Number of histos " << bitStack->GetHists()->GetEntries() << endl;

  changePlot();

  float off=0;
  for (int ii=0; ii<NSIGNALS; ii++) {
    if (bitPlotFlag[ii]) {bitOffset[ii]=off;
      off+=static_cast<float>(1.5);
      cout << "bit " << ii << " offset " << bitOffset[ii] << endl;
    }
  }

  // globalPlot=0;
  // for (int ii=0; ii<NADCS; ii++)
  //   if (plotFlag[ii]==1) globalPlot=1;
  // // if (globalPlot)  Draw();


}

void ctbAcquisition::setOutdir() {
  try {
    myDet->setFilePath(eOutdir->GetText());
  } CATCH_DISPLAY ("Could not set file path", "ctbAcquisition::setOutdir")
}

void ctbAcquisition::setFname() {
  try {
    myDet->setFileNamePrefix(eFname->GetText());
  } CATCH_DISPLAY ("Could not set file name prefix", "ctbAcquisition::setFname")
}

void ctbAcquisition::setFindex() {
  try {
    myDet->setAcquisitionIndex(eFindex->GetNumber());
  } CATCH_DISPLAY ("Could not set acquisition index", "ctbAcquisition::setFindex")
}

void ctbAcquisition::setFsave(Bool_t b) {
  try {
    myDet->setFileWrite(b);
    eFname->SetState(b);
    eOutdir->SetState(b);
  } CATCH_DISPLAY ("Could not set file write", "ctbAcquisition::setFsave")
}

void ctbAcquisition::update() {
  try {
    auto retval = myDet->getFileNamePrefix().tsquash("Different values");
    eFname->SetText(retval.c_str());
  } CATCH_DISPLAY ("Could not get file name prefix", "ctbAcquisition::update")
  
  try {
    auto retval = myDet->getAcquisitionIndex().tsquash("Different values");
    eFindex->SetNumber(retval);
  } CATCH_DISPLAY ("Could not get acquisition index", "ctbAcquisition::update")
  
  try {
    auto retval = myDet->getFileWrite().tsquash("Different values");
    cFileSave->SetOn(retval);
  } CATCH_DISPLAY ("Could not get file write", "ctbAcquisition::update")
  
  eFname->SetState(cFileSave->IsOn());
  eOutdir->SetState(cFileSave->IsOn());
  eFindex->SetState(cFileSave->IsOn());
  
  try {
    auto retval = myDet->getNumberOfAnalogSamples().tsquash("Different values");
    setAnalogSamples(retval);
  } CATCH_DISPLAY ("Could not get number of analog samples", "ctbAcquisition::update")
  
  try {
    auto retval = myDet->getNumberOfDigitalSamples().tsquash("Different values");
    setDigitalSamples(retval);
  } CATCH_DISPLAY ("Could not get number of digital samples", "ctbAcquisition::update")
  
  try {
    roMode = static_cast<int>(myDet->getReadoutMode().tsquash("Different values"));
    setReadoutMode(roMode);
  } CATCH_DISPLAY ("Could not get readout mode", "ctbAcquisition::update")

  updateChans();

   if (dataStructure) {
     cout << cbDetType->GetSelected()<< endl;
    // if (cbDetType->GetSelected()==MYTHEN301 || cbDetType->GetSelected()==MYTHEN302){
    //   cout << "settings deserialiation parameters for MYTHEN" << endl;
    //   mythen3_01_jctbData* ms=(mythen3_01_jctbData*)dataStructure;
    //   eSerOff->SetNumber( ms->setSerialOffset(-1));
    //   eDynRange->SetNumber( ms->setDynamicRange(-1));
    //   eNumCount->SetNumber( ms->setNumberOfCounters(-1));
    // }
    
   }

  if (myDet->getDetectorType().squash() == slsDetectorDefs::MOENCH) {
    dBitOffset = 0;
  } else {
    try {
      dBitOffset = myDet->getRxDbitOffset().tsquash("Different values");
    } CATCH_DISPLAY ("Could not get receiver dbit offset", "ctbAcquisition::update")
  }
  try {
    tenG = myDet->getTenGiga().tsquash("Different values");
  } CATCH_DISPLAY ("Could not get ten giga enable", "ctbAcquisition::update")

  // char aargs[10][100];
  // char *args[10];
  // for (int i=0; i<10; i++)
  //   args[i]=aargs[i];

  // string retval;
  // sprintf(args[0],"adcdisable");
  // slsDetectorCommand *cmd=new slsDetectorCommand(myDet);
  // retval=cmd->executeLine(1,args,slsDetectorDefs::GET_ACTION);
  // delete cmd;
  // int mask;
  // sscanf(retval.c_str(),"adcdisable %d",&mask);
  //   for (int i=0; i<NADCS; i++){
  //     if (mask&(1<<i))
  // 	enableFlag[i]=0;
  //     else
  // 	enableFlag[i]=1;
  //   }


}


void ctbAcquisition::loadPattern() {


  char fname[10000];
  char currdir[10000];
  char cdir[10000];
  

  cout << "Load Pattern " << endl;

  if (acqThread->GetState()==1 || acqThread->GetState()==6) {


    if (cCompile->IsOn()) {
      sprintf(fname,"%s %s",patternCompiler,patternFile);
      cout << "Compile: " << fname << endl;
      strcpy(currdir,gSystem->pwd());
      
      std::size_t found = string(patternCompiler).rfind('/');
      if (found!=std::string::npos)
	gSystem->cd(string(patternCompiler).substr(0,found).c_str());

      gSystem->cd(cdir);
      system(fname);
      gSystem->cd(currdir);
    }
 
    if (string(patternCompiler).rfind(".pat")!=std::string::npos) 
      strcpy(fname,patternFile);
    else if (string(patternCompiler).rfind(".npat")!=std::string::npos) 
      strcpy(fname,patternFile);
    else
      sprintf(fname,"%sat",patternFile);

    cout << "Load: " << fname << endl;
    try {
      myDet->loadParameters(fname);
    } CATCH_DISPLAY ("Could not load parameters", "ctbAcquisition::loadPattern")
  }
}


void ctbAcquisition::toggleAcquisition() {


  if (acqThread->GetState()==1 || acqThread->GetState()==6) {
    /** update all infos useful for the acquisition! */

  try {
    auto retval = myDet->getNumberOfAnalogSamples().tsquash("Different values");
    setAnalogSamples(retval);
  } CATCH_DISPLAY ("Could not get number of analog samples", "ctbAcquisition::toggleAcquisition")
  
  try {
    auto retval = myDet->getNumberOfDigitalSamples().tsquash("Different values");
    setDigitalSamples(retval);
  } CATCH_DISPLAY ("Could not get number of digital samples", "ctbAcquisition::toggleAcquisition")
  
  if (myDet->getDetectorType().squash() == slsDetectorDefs::MOENCH) {
    dBitOffset = 0;
  } else {
  try {
      dBitOffset = myDet->getRxDbitOffset().tsquash("Different values");
    } CATCH_DISPLAY ("Could not get receiver dbit offset", "ctbAcquisition::toggleAcquisition")
  }

  try {
    roMode = static_cast<int>(myDet->getReadoutMode().tsquash("Different values"));
    setReadoutMode(roMode);
  } CATCH_DISPLAY ("Could not get readout mode", "ctbAcquisition::toggleAcquisition")

  
      cout << "Run" << endl;
      bStatus->SetText("Stop");
      ip=0;    
      for (int i=0; i<NADCS; i++) {
	//	cout << "reset " << i << endl;
	if (adcHisto[i]->GetListOfFunctions())
	  adcHisto[i]->GetListOfFunctions()->Delete();

	adcHisto[i]->Reset();

	if (countsHisto[i]->GetListOfFunctions())
	  countsHisto[i]->GetListOfFunctions()->Delete();
	countsHisto[i]->Reset();
	// 	((TH1F*)adcStack->GetHists()->At(i))->Reset();
	// 	((TH1F*)countsStack->GetHists()->At(i))->Reset();
      }
      for (int i=0; i<NSIGNALS; i++) {
	bitHisto[i]->Reset();
      }
	cout << "reset 2d an" << endl;;
	if (h2DMapAn)	h2DMapAn->Reset();
	cout << "reset 2d dig" << endl;;
	if (h2DMapDig)	h2DMapDig->Reset();
	cout << "reset 1d" << endl;;
	if (h1DMap)	h1DMap->Reset();
	cout << "done" << endl;;
	//	if (h2Scan)	h2Scan->Reset();
	//	cout << "reset 1d" << endl;;
      //   if (rbWaveform->IsOn())
//  	adcStack->Draw("NOSTACK");
//       else if  (rbDistribution->IsOn())
// 	countsStack->Draw("NOSTACK");
//       else if (rb2D->IsOn())
// 	h2DMap->Draw("colz");

      //  cout << "timer" << endl;
      changePlot();

      // plotTimer->TurnOn();
      //   cout << "thread" << endl;
      acqThread->Run();
      StopFlag=0;





  } else {
    StopFlag=1; 
    try{
      myDet->stopDetector();
    } CATCH_DISPLAY ("Could not stop acquisition", "ctbAcquisition::toggleAcquisition")
    stop=1;
    bStatus->SetText("Start");
    //  acqThread->Kill();
  }
}

void ctbAcquisition::acquisitionFinished() {
    bStatus->SetText("Start");
    cout << "finished " << endl;
    //   plotTimer->TurnOff();
    Draw();
}

void ctbAcquisition::startAcquisition(){
  cout << "Detector started " <<eMeasurements->GetNumber()<< endl;
  stop=0;

  try {
    tenG = myDet->getTenGiga().tsquash("Different values");
  } CATCH_DISPLAY ("Could not get ten giga enable", "ctbAcquisition::startAcquisition")

  for (int im=0; im<eMeasurements->GetNumber(); im++) {
    try {
      myDet->acquire();
    } CATCH_DISPLAY ("Could not acquire", "ctbAcquisition::startAcquisition")
    
    cout << im << endl;
    if (stop) 
      break;
  }
}

void* ctbAcquisition::ThreadHandle(void *arg)
{
   ctbAcquisition *acq = static_cast<ctbAcquisition*>(arg);

   acq->startAcquisition();
   acq->acquisitionFinished();

  return nullptr;
}
 
 void ctbAcquisition::progressCallback(double f,void* arg) {


   // ctbAcquisition *acq = static_cast<ctbAcquisition*>(arg);


   cout << "*********" << f << "*******" << endl;




 }

void ctbAcquisition::setPatternFile(const char* t) {


  cout << "New pattern is " << t << endl;

  strcpy(patternFile,t);
 }

void ctbAcquisition::setPatternCompiler(const char* t) {


  cout << "New compiler is " << t << endl;
  strcpy(patternCompiler,t);

 }
void ctbAcquisition::setMeasurements() {

}

void ctbAcquisition::setAnalogSamples(int n) {


  cout<< "Set number of analog samples to " << dec<< n << endl;
  if (n>0 && n<8192)
    nAnalogSamples=n;

  // TList *l= adcStack->GetHists();
  TH1 *h;
  //  if (l) {
    for (int i=0; i<NADCS; i++) {
      h=adcHisto[i];//(TH1F*)l->At(i);
      if (h) {
	
	h->SetBins(nAnalogSamples,0,nAnalogSamples);
      }
    }

  h=adcStack->GetHistogram();
  if (h)
    h->SetBins(nAnalogSamples,0,nAnalogSamples);
}



void ctbAcquisition::setDigitalSamples(int n) {


  cout<< "Set number of digital samples to " << dec<< n << endl;
  if (n>0 && n<8192)
    nDigitalSamples=n;

  TH1 *h;
    for (int i=0; i<NSIGNALS; i++) {
      h=bitHisto[i];//(TH1F*)l->At(i);
      if (h) {
	
	h->SetBins(nDigitalSamples,0,nDigitalSamples);
      }
    
    }
    //  cout<< "histos resized " << dec<< h->GetNbinsX() << endl;
    
    h=bitStack->GetHistogram();
    if (h)
      h->SetBins(nDigitalSamples,0,nDigitalSamples);
    
}

void ctbAcquisition::setReadoutMode(int f) {
  
  roMode=f;
  slsDetectorDefs::readoutMode flag=(slsDetectorDefs::readoutMode)f;
  if (flag == slsDetectorDefs::DIGITAL_ONLY) {
    nAnalogSamples=0;
    adclist.clear();
  }  else if (flag ==slsDetectorDefs::ANALOG_AND_DIGITAL) {
    ;
  }
  else {
    nDigitalSamples=0;
    dbitlist.clear();
  }
  
  // for (int i=0; i<NADCS; i++) {
  //   if (flags&slsDetectorDefs::DIGITAL_ONLY) enableFlag[i]=0;
  //   //else enableFlag[i+NADCS]=1;
  // }

  // for (int i=0; i<4; i++) {
  //   if (flags&slsDetectorDefs::ANALOG_AND_DIGITAL) enableFlag[i+NADCS]=1;
  //   else enableFlag[i+NADCS]=0;
  // }
  
  updateChans();
   
}



void ctbAcquisition::setADCEnable(Int_t reg){
  
  updateChans();
  // int chanEnable; 


}


void ctbAcquisition::setDbitEnable(Int_t reg){
  
 
  updateChans();
  // int chanEnable; 


}



void ctbAcquisition::updateChans() {

  // dbit list
  if (myDet->getDetectorType().squash() == slsDetectorDefs::MOENCH) {
    dbitlist.clear();
  } else {
    try {
      auto retval = myDet->getRxDbitList().tsquash("Different values");
      dbitlist.clear();
      if (!retval.empty()) {
        for (const auto &value : retval) 
          dbitlist.push_back(value);
      }
    } CATCH_DISPLAY ("Could not get receiver dbit list.", "ctbAcquisition::updateChans")
  }

  // adc mask
  try { 
    auto retval = myDet->getADCEnableMask().tsquash("Different values");
    adclist.clear();
    if (retval!=0xffffffff) {
      for (int i=0; i<NADCS; i++) {
        if (retval&(1<<i)) {
	        adclist.push_back(i);
        }
      }
    }
  } CATCH_DISPLAY ("Could not get adc enable mask.", "ctbAcquisition::updateChans")
}



void ctbAcquisition::resetPedestal() {
  if (photonFinder) {
    photonFinder->newDataSet();
  };

}

void ctbAcquisition::ToggleCommonMode(Bool_t b) {
  if (photonFinder) {
    if (b) {
    photonFinder->setCommonModeSubtraction(commonMode);
    cmSub=1;
    cout << "Enable common mode" << endl;
    } else {
      photonFinder->setCommonModeSubtraction(NULL);
      cmSub=0;
      cout << "Disable common mode" << endl;
    }
  }

}


void ctbAcquisition::TogglePedSub(Bool_t b) {
    if (b) {
      ChangeHistoLimitsPedSub();
    } else {
      ChangeHistoLimitsRaw();
    }

}


void ctbAcquisition::FitADC() {
  int iadc=eFitADC->GetNumber();
  if (iadc<0 || iadc>=NADCS) return;
  cout << "fit panel for adc " << eFitADC->GetNumber() << endl;
  if (rbWaveform->IsOn()) {
    if (adcHisto[iadc]==NULL) return;
    new TCanvas("Cadcfit");
    if (adcFit) {
      delete adcFit;
      adcFit=NULL;
    }
    adcFit=(TH1F*)(adcHisto[iadc]->Clone("adcfit"));
    adcFit->Draw();
    adcFit->FitPanel();

  } else if (rbDistribution->IsOn()) {
    if (countsHisto[iadc]==NULL) return;
    new TCanvas("Ccountsfit");

    if (countsFit) {
      delete countsFit;
      countsFit=NULL;
    }
    
    countsFit=(TH1F*)(countsHisto[iadc]->Clone("countsfit"));
    countsFit->Draw();
    countsFit->FitPanel();
  }
}


void ctbAcquisition::plotBit() {
  int iadc=eBitPlot->GetNumber();
  if (iadc<0 || iadc>=NSIGNALS) return;
  cout << "plot panel for bit " << eBitPlot->GetNumber() << endl;
    if (bitHisto[iadc]==NULL) return;
    new TCanvas("Cbitplot");
    if (bitPlot) {
      delete bitPlot;
      bitPlot=NULL;
    }
    bitPlot=(TH1F*)(bitHisto[iadc]->Clone("bitplot"));
    bitPlot->Draw();
}










void ctbAcquisition::ChangeSerialOffset(Long_t a){
  ChangeSerialOffset();
};


void ctbAcquisition::ChangeDynamicRange(Long_t a){
  ChangeDynamicRange();
};

void ctbAcquisition::ChangeNumberOfChannels(Long_t a){
  ChangeNumberOfChannels();
};



void ctbAcquisition::ChangeSerialOffset(){
  changeDetector();
  // if (dataStructure) {

  //    cout << cbDetType->GetSelected()<< endl;
  //   if (cbDetType->GetSelected()==MYTHEN301 || cbDetType->GetSelected()==MYTHEN302 ){
  //     cout << "settings offsets for MYTHEN" << endl;
  //     mythen3_01_jctbData* ms=(mythen3_01_jctbData*)dataStructure;
  //     ms->setSerialOffset(eSerOff->GetIntNumber());
      
  //   }
  // }
};


void ctbAcquisition::ChangeDynamicRange(){
  changeDetector();
  // if (dataStructure) {

  //    cout << cbDetType->GetSelected()<< endl;
  //    if (cbDetType->GetSelected()==MYTHEN301 || cbDetType->GetSelected()==MYTHEN302){
  //     cout << "settings dynamic range for MYTHEN" << endl;
  //     mythen3_01_jctbData* ms=(mythen3_01_jctbData*)dataStructure;
  //     ms->setDynamicRange(eDynRange->GetIntNumber());
      
  //   }
  // }
};

void ctbAcquisition::ChangeNumberOfChannels(){
  changeDetector();
  // if (dataStructure) {
  //    cout << cbDetType->GetSelected()<< endl;
  //   if (cbDetType->GetSelected()==MYTHEN301 || cbDetType->GetSelected()==MYTHEN302){
  //     cout << "settings number of channels for MYTHEN" << endl;
  //     mythen3_01_jctbData* ms=(mythen3_01_jctbData*)dataStructure;
  //     ms->setNumberOfCounters(eNumCount->GetIntNumber());
      
  //   }
  // }
  if (deserializer)
    changePlot();
};

void ctbAcquisition::ChangeImagePixels(Long_t a){
  ChangeImagePixels();
};

void ctbAcquisition::ChangeImagePixels(){
  changeDetector();
  // if (dataStructure) {
  //    cout << cbDetType->GetSelected()<< endl;
  //   if (cbDetType->GetSelected()==MYTHEN301 || cbDetType->GetSelected()==MYTHEN302){
  //     cout << "settings number of channels for MYTHEN" << endl;
  //     mythen3_01_jctbData* ms=(mythen3_01_jctbData*)dataStructure;
  //     ms->setNumberOfCounters(eNumCount->GetIntNumber());
      
  //   }
  // }
  // if (deserializer)
  //  changePlot();
};


void ctbAcquisition::ChangeHistoLimitsPedSub(Long_t a){
  ChangeHistoLimitsPedSub();
};


void ctbAcquisition::ChangeHistoLimitsRaw(Long_t a){
  ChangeHistoLimitsRaw();
}

void ctbAcquisition::ChangeHistoLimitsPedSub(Bool_t a){
  ChangeHistoLimitsPedSub();
};


void ctbAcquisition::ChangeHistoLimitsRaw(Bool_t a){
  ChangeHistoLimitsRaw();
}


void ctbAcquisition::ChangeHistoLimitsPedSub(){

  cout << "set Limits ped sub hist " << eMinPedSub->GetNumber() << " " << eMaxPedSub->GetNumber() << endl;

  if  (eMinPedSub->GetNumber()>eMaxPedSub->GetNumber())
    return;

  if (cbSubtractPedestal->IsOn()) {
    if (cMinMaxPedSub->IsOn()) {
      adcStack->SetMaximum( eMaxPedSub->GetNumber());
      adcStack->SetMinimum( eMinPedSub->GetNumber());
      if (h2DMapAn) {
	h2DMapAn->SetMaximum( eMaxPedSub->GetNumber());
	h2DMapAn->SetMinimum( eMinPedSub->GetNumber());
      }
      if (h1DMap) {
	h1DMap->SetMaximum( eMaxPedSub->GetNumber());
	h1DMap->SetMinimum( eMinPedSub->GetNumber());
      }
      if (countsStack->GetHistogram())
	countsStack->GetHistogram()->GetXaxis()->SetRangeUser(eMinPedSub->GetNumber(), eMaxPedSub->GetNumber());
    } else {
      if (adcStack->GetHistogram())
	adcStack->GetHistogram()->GetYaxis()->UnZoom();
      if (h2DMapAn) {
	h2DMapAn->GetZaxis()->UnZoom();
      }
      if (h1DMap) {
	h1DMap->GetYaxis()->UnZoom();
      }
    if (countsStack->GetHistogram())
      countsStack->GetHistogram()->GetXaxis()->UnZoom();
    }
  }


};


void ctbAcquisition::ChangeHistoLimitsRaw(){
  
  cout << "set Limits raw hist " << eMinRaw->GetNumber() << " " << eMaxRaw->GetNumber() << endl;
  if  (eMinRaw->GetNumber()>eMaxRaw->GetNumber())
    return;

  if (cbSubtractPedestal->IsOn()==0) {
    if (cMinMaxRaw->IsOn()) {
      adcStack->SetMaximum( eMaxRaw->GetNumber());
      adcStack->SetMinimum( eMinRaw->GetNumber());
      if (h2DMapAn) {
	h2DMapAn->SetMaximum( eMaxRaw->GetNumber());
	h2DMapAn->SetMinimum( eMinRaw->GetNumber());
      }
      if (h1DMap) {
	h1DMap->SetMaximum( eMaxRaw->GetNumber());
	h1DMap->SetMinimum( eMinRaw->GetNumber());
      }
      if (countsStack->GetHistogram())
	countsStack->GetHistogram()->GetXaxis()->SetRangeUser(eMinRaw->GetNumber(), eMaxRaw->GetNumber());
    } else {

      if (adcStack->GetHistogram())
	adcStack->GetHistogram()->GetYaxis()->UnZoom(); 
      if (h2DMapAn) {
	h2DMapAn->GetZaxis()->UnZoom();
      } 
      
      if (h1DMap) {
	h1DMap->GetYaxis()->UnZoom();
      } 
    if (countsStack->GetHistogram())
      countsStack->GetHistogram()->GetXaxis()->UnZoom();

    }
  }

}
