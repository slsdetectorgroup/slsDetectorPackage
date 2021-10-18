// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include <TApplication.h>
#include <TGClient.h>
#include <TCanvas.h>
#include <TF1.h>
#include <TRandom.h>
#include <TGButton.h>
#include <TRootEmbeddedCanvas.h>
#include <TGButtonGroup.h>
#include <TGNumberEntry.h>
#include <TGLabel.h>
#include <TList.h>
#include <TGFileDialog.h>
#include <TGComboBox.h>
#include <TH2F.h>
#include <TColor.h>
#include <TH1F.h>
#include <TGraphErrors.h>
#include <THStack.h>
#include <TGTab.h>
#include <TGTextEntry.h>

#include <stdio.h>
#include <iostream>
#include <fstream>

#include "ctbPattern.h"
#include "ctbDefs.h"
#include "sls/Detector.h"
#include <chrono>
using namespace std;





ctbLoop::ctbLoop(TGGroupFrame *page, int i, sls::Detector *det) : TGHorizontalFrame(page, 800,800), id(i), myDet(det) {

  TGHorizontalFrame *hframe=this;
  
  char tit[100];

  page->AddFrame(hframe,new TGLayoutHints(kLHintsTop | kLHintsExpandX , 1,1,1,1));
  MapWindow();


   

  
  sprintf(tit, "Loop %d Repetitions: ", id);
  
  TGLabel *label= new TGLabel(hframe, tit);
  hframe->AddFrame(label,new TGLayoutHints(kLHintsTop | kLHintsLeft| kLHintsExpandX, 1, 1, 1, 1));
  label->MapWindow();
  label->SetTextJustify(kTextLeft);



  
   eLoopNumber = new TGNumberEntry(hframe, 0, 9,999, TGNumberFormat::kNESInteger,
                                               TGNumberFormat::kNEANonNegative, 
					 TGNumberFormat::kNELNoLimits);
   hframe->AddFrame( eLoopNumber,new TGLayoutHints(kLHintsTop |  kLHintsExpandX, 1, 1, 1, 1));
   eLoopNumber->MapWindow();
   eLoopNumber->Resize(150,30);
    TGTextEntry *e= eLoopNumber->TGNumberEntry::GetNumberEntry();
   e->Connect("ReturnPressed()","ctbLoop",this,"setNLoops()");




   sprintf(tit, "Start Address: ");

   label= new TGLabel(hframe, tit);
   hframe->AddFrame(label,new TGLayoutHints(kLHintsTop | kLHintsLeft| kLHintsExpandX, 1, 1, 1, 1));
   label->MapWindow();
   label->SetTextJustify(kTextLeft);



  
   eLoopStartAddr = new TGNumberEntry(hframe, 0, 9,999, TGNumberFormat::kNESHex,
                                               TGNumberFormat::kNEANonNegative, 
                                               TGNumberFormat::kNELLimitMinMax,
                                               0, 1024);
   hframe->AddFrame( eLoopStartAddr,new TGLayoutHints(kLHintsTop |  kLHintsExpandX, 1, 1, 1, 1));
   eLoopStartAddr->MapWindow();
   eLoopStartAddr->Resize(150,30);

   // eLoopStartAddr->SetState(kFALSE);
   
   label= new TGLabel(hframe, "Stop Address: ");
   hframe->AddFrame(label,new TGLayoutHints(kLHintsTop | kLHintsLeft| kLHintsExpandX, 1, 1, 1, 1));
   label->MapWindow();
   label->SetTextJustify(kTextLeft);




  
   eLoopStopAddr = new TGNumberEntry(hframe, 0, 9,999, TGNumberFormat::kNESHex,
                                               TGNumberFormat::kNEANonNegative, 
                                               TGNumberFormat::kNELLimitMinMax,
                                               0, 1024);
   hframe->AddFrame( eLoopStopAddr,new TGLayoutHints(kLHintsTop |  kLHintsExpandX, 1, 1, 1, 1));
   eLoopStopAddr->MapWindow();
   eLoopStopAddr->Resize(150,30);



   //   eLoopStopAddr->SetState(kFALSE);


  



}

void ctbLoop::setNLoops() {
  try{
    myDet->setPatternLoopCycles(id, eLoopNumber->GetNumber());
  } CATCH_DISPLAY ("Could not set number of pattern loops for level " + to_string(id) + ".", "ctbLoop::setNLoops")
}



void ctbLoop::update() {
  try{

    auto loop = myDet->getPatternLoopCycles(id).tsquash("Different values");
    eLoopNumber->SetNumber(loop);
    auto loopaddr = myDet->getPatternLoopAddresses(id).tsquash("Different values");
    eLoopStartAddr->SetHexNumber(loopaddr[0]);
    eLoopStopAddr->SetHexNumber(loopaddr[1]);


  } CATCH_DISPLAY ("Could not get pattern loops for level " + to_string(id) + ".", "ctbLoop::update")
}



ctbWait::ctbWait(TGGroupFrame *page, int i, sls::Detector *det) : TGHorizontalFrame(page, 800,800), id(i), myDet(det) {

  char tit[100];
  TGHorizontalFrame *hframe=this;
  page->AddFrame(hframe,new TGLayoutHints(kLHintsTop | kLHintsExpandX , 1,1,1,1));
  MapWindow();



   sprintf(tit, "Wait %d (run clk): ", id);

   TGLabel *label= new TGLabel(hframe, tit);
   hframe->AddFrame(label,new TGLayoutHints(kLHintsTop | kLHintsLeft| kLHintsExpandX, 1, 1, 1, 1));
   label->MapWindow();
   label->SetTextJustify(kTextLeft);



  
   eWaitTime = new TGNumberEntry(hframe, 0, 9,999, TGNumberFormat::kNESInteger,
                                               TGNumberFormat::kNEANonNegative, 
					 TGNumberFormat::kNELNoLimits);
   hframe->AddFrame( eWaitTime,new TGLayoutHints(kLHintsTop |  kLHintsExpandX, 1, 1, 1, 1));
   eWaitTime->MapWindow();
   eWaitTime->Resize(150,30);
    TGTextEntry *e= eWaitTime->TGNumberEntry::GetNumberEntry();
   e->Connect("ReturnPressed()","ctbWait",this,"setWaitTime()");



   sprintf(tit, "Wait Address: ");

   label= new TGLabel(hframe, tit);
   hframe->AddFrame(label,new TGLayoutHints(kLHintsTop | kLHintsLeft| kLHintsExpandX, 1, 1, 1, 1));
   label->MapWindow();
   label->SetTextJustify(kTextLeft);



  
   eWaitAddr = new TGNumberEntry(hframe, 0, 9,999, TGNumberFormat::kNESHex,
                                               TGNumberFormat::kNEANonNegative, 
                                               TGNumberFormat::kNELLimitMinMax,
                                               0, 1024);
   hframe->AddFrame( eWaitAddr,new TGLayoutHints(kLHintsTop |  kLHintsExpandX, 1, 1, 1, 1));
   eWaitAddr->MapWindow();
   eWaitAddr->Resize(150,30);

   //  eWaitAddr->SetState(kFALSE);
  
}



void ctbWait::setWaitTime() {
  try{

    myDet->setPatternWaitTime(id, eWaitTime->GetNumber());

  } CATCH_DISPLAY ("Could not set pattern wait time for level " + to_string(id) + ".", "ctbWait::setWaitTime")
}



void ctbWait::update() {
  try{

    auto time = myDet->getPatternWaitTime(id).tsquash("Different values");
    auto addr = myDet->getPatternWaitAddr(id).tsquash("Different values");

    eWaitAddr->SetHexNumber(addr);
    eWaitTime->SetNumber(time);

  } CATCH_DISPLAY ("Could not get pattern loops for level " + to_string(id) + ".", "ctbWait::update")
}









ctbPattern::ctbPattern(TGVerticalFrame *page, sls::Detector *det)
  : TGGroupFrame(page,"Pattern",kVerticalFrame), myDet(det) {


  SetTitlePos(TGGroupFrame::kLeft);
  page->AddFrame(this,new TGLayoutHints( kLHintsTop | kLHintsExpandX , 10,10,10,10));
  MapWindow();
  

  char tit[100];

 
  TGHorizontalFrame* hframe=new TGHorizontalFrame(this, 800,800);
  AddFrame(hframe,new TGLayoutHints(kLHintsTop | kLHintsExpandX , 1,1,1,1));
  hframe->MapWindow();


   sprintf(tit, "Run Clock Frequency (MHz): ");

   TGLabel *label= new TGLabel(hframe, tit);
   hframe->AddFrame(label,new TGLayoutHints(kLHintsTop | kLHintsLeft| kLHintsExpandX, 1, 1, 1, 1));
   label->MapWindow();
   label->SetTextJustify(kTextLeft);



  
   eRunClkFreq = new TGNumberEntry(hframe, 0, 9,999, TGNumberFormat::kNESInteger,
                                               TGNumberFormat::kNEANonNegative, 
                                               TGNumberFormat::kNELLimitMinMax,
                                               0, 400);
   hframe->AddFrame( eRunClkFreq,new TGLayoutHints(kLHintsTop |  kLHintsExpandX, 1, 1, 1, 1));
   eRunClkFreq->MapWindow();
   eRunClkFreq->Resize(150,30);
   TGTextEntry *e= eRunClkFreq->TGNumberEntry::GetNumberEntry();
   e->Connect("ReturnPressed()","ctbPattern",this,"setRunFreq()");
 




   hframe=new TGHorizontalFrame(this, 800,800);
   AddFrame(hframe,new TGLayoutHints(kLHintsTop | kLHintsExpandX , 1,1,1,1));
   hframe->MapWindow();




   sprintf(tit, "ADC Clock Frequency (MHz): ");

   label= new TGLabel(hframe, tit);
   hframe->AddFrame(label,new TGLayoutHints(kLHintsTop | kLHintsLeft| kLHintsExpandX, 1, 1, 1, 1));
   label->MapWindow();
   label->SetTextJustify(kTextLeft);



  
   eAdcClkFreq = new TGNumberEntry(hframe, 0, 9,999, TGNumberFormat::kNESInteger,
                                               TGNumberFormat::kNEANonNegative, 
                                               TGNumberFormat::kNELLimitMinMax,
                                               0, 40);
   hframe->AddFrame( eAdcClkFreq,new TGLayoutHints(kLHintsTop |  kLHintsExpandX, 1, 1, 1, 1));
   eAdcClkFreq->MapWindow();
   eAdcClkFreq->Resize(150,30);   
   e= eAdcClkFreq->TGNumberEntry::GetNumberEntry();
   e->Connect("ReturnPressed()","ctbPattern",this,"setAdcFreq()");
 
   


   sprintf(tit, "DBIT Clock Frequency (MHz): ");

   label= new TGLabel(hframe, tit);
   hframe->AddFrame(label,new TGLayoutHints(kLHintsTop | kLHintsLeft| kLHintsExpandX, 1, 1, 1, 1));
   label->MapWindow();
   label->SetTextJustify(kTextLeft);



  
   eDBitClkFreq = new TGNumberEntry(hframe, 0, 9,999, TGNumberFormat::kNESInteger,
                                               TGNumberFormat::kNEANonNegative, 
                                               TGNumberFormat::kNELLimitMinMax,
                                               0, 400);
   hframe->AddFrame( eDBitClkFreq,new TGLayoutHints(kLHintsTop |  kLHintsExpandX, 1, 1, 1, 1));
   eDBitClkFreq->MapWindow();
   eDBitClkFreq->Resize(150,30);   
   e= eDBitClkFreq->TGNumberEntry::GetNumberEntry();
   e->Connect("ReturnPressed()","ctbPattern",this,"setDBitFreq()");
 
   








   hframe=new TGHorizontalFrame(this, 800,800);
   AddFrame(hframe,new TGLayoutHints(kLHintsTop | kLHintsExpandX , 1,1,1,1));
   hframe->MapWindow();




   label= new TGLabel(hframe, "ADC Clock Phase (a.u.): ");
   hframe->AddFrame(label,new TGLayoutHints(kLHintsTop | kLHintsLeft| kLHintsExpandX, 1, 1, 1, 1));
   label->MapWindow();
   label->SetTextJustify(kTextLeft);




  
   eAdcClkPhase = new TGNumberEntry(hframe, 0, 9,999, TGNumberFormat::kNESInteger,
                                               TGNumberFormat::kNEAAnyNumber, 
                                               TGNumberFormat::kNELLimitMinMax,
                                               -255, 255);
   hframe->AddFrame( eAdcClkPhase,new TGLayoutHints(kLHintsTop |  kLHintsExpandX, 1, 1, 1, 1));
   eAdcClkPhase->MapWindow();
   eAdcClkPhase->Resize(150,30);
   e= eAdcClkPhase->TGNumberEntry::GetNumberEntry();
   e->Connect("ReturnPressed()","ctbPattern",this,"setAdcPhase()");
 

   

   label= new TGLabel(hframe, "DBit Clock Phase (a.u.): ");
   hframe->AddFrame(label,new TGLayoutHints(kLHintsTop | kLHintsLeft| kLHintsExpandX, 1, 1, 1, 1));
   label->MapWindow();
   label->SetTextJustify(kTextLeft);




  
   eDBitClkPhase = new TGNumberEntry(hframe, 0, 9,999, TGNumberFormat::kNESInteger,
                                               TGNumberFormat::kNEAAnyNumber, 
                                               TGNumberFormat::kNELLimitMinMax,
                                               -255, 255);
   hframe->AddFrame( eDBitClkPhase,new TGLayoutHints(kLHintsTop |  kLHintsExpandX, 1, 1, 1, 1));
   eDBitClkPhase->MapWindow();
   eDBitClkPhase->Resize(150,30);
   e= eDBitClkPhase->TGNumberEntry::GetNumberEntry();
   e->Connect("ReturnPressed()","ctbPattern",this,"setDBitPhase()");
 

//    label= new TGLabel(hframe, " Phase (0.15ns step): ");
//    hframe->AddFrame(label,new TGLayoutHints(kLHintsTop | kLHintsLeft| kLHintsExpandX, 1, 1, 1, 1));
//    label->MapWindow();
//    label->SetTextJustify(kTextLeft);




  
//    eRunClkPhase = new TGNumberEntry(hframe, 0, 9,999, TGNumberFormat::kNESInteger,
//                                                TGNumberFormat::kNEANonNegative, 
//                                                TGNumberFormat::kNELLimitMinMax,
//                                                0, 200);
//    hframe->AddFrame( eRunClkPhase,new TGLayoutHints(kLHintsTop |  kLHintsExpandX, 1, 1, 1, 1));
//    eRunClkPhase->MapWindow();
//    eRunClkPhase->Resize(150,30);
//    e= eRunClkPhase->TGNumberEntry::GetNumberEntry();
//    e->Connect("ReturnPressed()","ctbPattern",this,"setRunPhase()");





   hframe=new TGHorizontalFrame(this, 800,800);
   AddFrame(hframe,new TGLayoutHints(kLHintsTop | kLHintsExpandX , 1,1,1,1));
   hframe->MapWindow();


   
   
    label= new TGLabel(hframe, "Adc pipeline: ");
    hframe->AddFrame(label,new TGLayoutHints(kLHintsTop | kLHintsLeft| kLHintsExpandX, 1, 1, 1, 1));
   label->MapWindow();
    label->SetTextJustify(kTextLeft);




  
    eAdcPipeline = new TGNumberEntry(hframe, 0, 9,999, TGNumberFormat::kNESInteger,
                                                TGNumberFormat::kNEANonNegative, 
                                                TGNumberFormat::kNELLimitMinMax,
                                                0, 64);
   hframe->AddFrame( eAdcPipeline,new TGLayoutHints(kLHintsTop |  kLHintsExpandX, 1, 1, 1, 1));
    eAdcPipeline->MapWindow();
    eAdcPipeline->Resize(150,30);
    e= eAdcPipeline->TGNumberEntry::GetNumberEntry();
    e->Connect("ReturnPressed()","ctbPattern",this,"setAdcPipeline()");


   


   
    label= new TGLabel(hframe, "DBIT pipeline: ");
    hframe->AddFrame(label,new TGLayoutHints(kLHintsTop | kLHintsLeft| kLHintsExpandX, 1, 1, 1, 1));
   label->MapWindow();
    label->SetTextJustify(kTextLeft);




  
    eDBitPipeline = new TGNumberEntry(hframe, 0, 9,999, TGNumberFormat::kNESInteger,
                                                TGNumberFormat::kNEANonNegative, 
                                                TGNumberFormat::kNELLimitMinMax,
                                                0, 64);
   hframe->AddFrame( eDBitPipeline,new TGLayoutHints(kLHintsTop |  kLHintsExpandX, 1, 1, 1, 1));
    eDBitPipeline->MapWindow();
    eDBitPipeline->Resize(150,30);
    e= eDBitPipeline->TGNumberEntry::GetNumberEntry();
    e->Connect("ReturnPressed()","ctbPattern",this,"setDBitPipeline()");


   





  hframe=new TGHorizontalFrame(this, 800,800);
   AddFrame(hframe,new TGLayoutHints(kLHintsTop | kLHintsExpandX , 1,1,1,1));
   hframe->MapWindow();


   sprintf(tit, "Number of triggers: ");

   label= new TGLabel(hframe, tit);
   hframe->AddFrame(label,new TGLayoutHints(kLHintsTop | kLHintsLeft| kLHintsExpandX, 1, 1, 1, 1));
   label->MapWindow();
   label->SetTextJustify(kTextLeft);



  
   eTriggers = new TGNumberEntry(hframe, 0, 9,999, TGNumberFormat::kNESInteger,
                                               TGNumberFormat::kNEANonNegative, 
                                               TGNumberFormat::kNELNoLimits);
   hframe->AddFrame( eTriggers,new TGLayoutHints(kLHintsTop |  kLHintsExpandX, 1, 1, 1, 1));
   eTriggers->MapWindow();
   eTriggers->Resize(150,30);
   e= eTriggers->TGNumberEntry::GetNumberEntry();
   e->Connect("ReturnPressed()","ctbPattern",this,"setTriggers()");


   // sprintf(tit, "Number of measurements: ");

   // label= new TGLabel(hframe, tit);
   // hframe->AddFrame(label,new TGLayoutHints(kLHintsTop | kLHintsLeft| kLHintsExpandX, 1, 1, 1, 1));
   // label->MapWindow();
   // label->SetTextJustify(kTextLeft);



  
   // eMeasurements = new TGNumberEntry(hframe, 0, 9,999, TGNumberFormat::kNESInteger,
   //                                             TGNumberFormat::kNEANonNegative, 
   //                                             TGNumberFormat::kNELNoLimits);
   // hframe->AddFrame( eMeasurements,new TGLayoutHints(kLHintsTop |  kLHintsExpandX, 1, 1, 1, 1));
   // eMeasurements->MapWindow();
   // eMeasurements->Resize(150,30);
   // e= eMeasurements->TGNumberEntry::GetNumberEntry();
   // e->Connect("ReturnPressed()","ctbPattern",this,"setMeasurements()");


  hframe=new TGHorizontalFrame(this, 800,800);
   AddFrame(hframe,new TGLayoutHints(kLHintsTop | kLHintsExpandX , 1,1,1,1));
   hframe->MapWindow();


   sprintf(tit, "Number of frames: ");

   label= new TGLabel(hframe, tit);
   hframe->AddFrame(label,new TGLayoutHints(kLHintsTop | kLHintsLeft| kLHintsExpandX, 1, 1, 1, 1));
   label->MapWindow();
   label->SetTextJustify(kTextLeft);



  
   eFrames = new TGNumberEntry(hframe, 0, 9,999, TGNumberFormat::kNESInteger,
                                               TGNumberFormat::kNEANonNegative, 
                                               TGNumberFormat::kNELNoLimits);
   hframe->AddFrame( eFrames,new TGLayoutHints(kLHintsTop |  kLHintsExpandX, 1, 1, 1, 1));
   eFrames->MapWindow();
   eFrames->Resize(150,30);
   e= eFrames->TGNumberEntry::GetNumberEntry();
   e->Connect("ReturnPressed()","ctbPattern",this,"setFrames()");

   
   label= new TGLabel(hframe, " Period (s): ");
   hframe->AddFrame(label,new TGLayoutHints(kLHintsTop | kLHintsLeft| kLHintsExpandX, 1, 1, 1, 1));
   label->MapWindow();
   label->SetTextJustify(kTextLeft);




  
   ePeriod = new TGNumberEntry(hframe, 0, 9,999, TGNumberFormat::kNESReal,
                                               TGNumberFormat::kNEANonNegative, 
                                               TGNumberFormat::kNELNoLimits);
   hframe->AddFrame( ePeriod,new TGLayoutHints(kLHintsTop |  kLHintsExpandX, 1, 1, 1, 1));
   ePeriod->MapWindow();
   ePeriod->Resize(150,30);
   e= ePeriod->TGNumberEntry::GetNumberEntry();
   e->Connect("ReturnPressed()","ctbPattern",this,"setPeriod()");






   hframe=new TGHorizontalFrame(this, 800,800);
   AddFrame(hframe,new TGLayoutHints(kLHintsTop | kLHintsExpandX , 1,1,1,1));
   hframe->MapWindow();


   sprintf(tit, "Start Address: ");

   label= new TGLabel(hframe, tit);
   hframe->AddFrame(label,new TGLayoutHints(kLHintsTop | kLHintsLeft| kLHintsExpandX, 1, 1, 1, 1));
   label->MapWindow();
   label->SetTextJustify(kTextLeft);



  
   eStartAddr = new TGNumberEntry(hframe, 0, 9,999, TGNumberFormat::kNESHex,
                                               TGNumberFormat::kNEANonNegative, 
                                               TGNumberFormat::kNELLimitMinMax,
                                               0, 1024);
   hframe->AddFrame( eStartAddr,new TGLayoutHints(kLHintsTop |  kLHintsExpandX, 1, 1, 1, 1));
   eStartAddr->MapWindow();
   eStartAddr->Resize(150,30);

   eStartAddr->SetState(kFALSE);
   
   label= new TGLabel(hframe, "Stop Address: ");
   hframe->AddFrame(label,new TGLayoutHints(kLHintsTop | kLHintsLeft| kLHintsExpandX, 1, 1, 1, 1));
   label->MapWindow();
   label->SetTextJustify(kTextLeft);




  
   eStopAddr = new TGNumberEntry(hframe, 0, 9,999, TGNumberFormat::kNESHex,
                                               TGNumberFormat::kNEANonNegative, 
                                               TGNumberFormat::kNELLimitMinMax,
                                               0, 1024);
   hframe->AddFrame( eStopAddr,new TGLayoutHints(kLHintsTop |  kLHintsExpandX, 1, 1, 1, 1));
   eStopAddr->MapWindow();
   eStopAddr->Resize(150,30);

  


    eStopAddr->SetState(kFALSE);






  int idac=0;
  for (idac=0; idac<NLOOPS; idac++) {

    eLoop[idac]=new ctbLoop(this,idac,myDet);


 

  }
















  for (idac=0; idac<NWAITS; idac++) {
 

   eWait[idac]=new ctbWait(this,idac,myDet);

   

  }



  

   hframe=new TGHorizontalFrame(this, 800,800);
   AddFrame(hframe,new TGLayoutHints(kLHintsTop | kLHintsExpandX , 1,1,1,1));
   hframe->MapWindow();



   label= new TGLabel(hframe, "Compiler: ");
   hframe->AddFrame(label,new TGLayoutHints(kLHintsTop | kLHintsLeft| kLHintsExpandX, 1, 1, 1, 1));
   label->MapWindow();
   label->SetTextJustify(kTextLeft);




   patternCompiler=new TGTextEntry(hframe,"generate.sh");
   hframe->AddFrame(patternCompiler,new TGLayoutHints(kLHintsTop | kLHintsLeft| kLHintsExpandX, 1, 1, 1, 1));
   patternCompiler->MapWindow();
   // patternCompiler->SetTextJustify(kTextLeft);
   patternCompiler->Connect("ReturnPressed()","ctbPattern",this,"setCompiler()");


   browseCompiler=new TGTextButton(hframe,"Browse");
   hframe->AddFrame(browseCompiler,new TGLayoutHints(kLHintsTop | kLHintsLeft| kLHintsExpandX, 1, 1, 1, 1));
   browseCompiler->MapWindow();
   // patternCompiler->SetTextJustify(kTextLeft);
   browseCompiler->Connect("Clicked()","ctbPattern",this,"chooseCompiler()");




   hframe=new TGHorizontalFrame(this, 800,800);
   AddFrame(hframe,new TGLayoutHints(kLHintsTop | kLHintsExpandX , 1,1,1,1));
   hframe->MapWindow();

   
   label= new TGLabel(hframe, "Pattern: ");
   hframe->AddFrame(label,new TGLayoutHints(kLHintsTop | kLHintsLeft| kLHintsExpandX, 1, 1, 1, 1));
   label->MapWindow();
   label->SetTextJustify(kTextLeft);


   patternFile=new TGTextEntry(hframe,"file.p");
   hframe->AddFrame(patternFile,new TGLayoutHints(kLHintsTop | kLHintsLeft| kLHintsExpandX, 1, 1, 1, 1));
   patternFile->MapWindow();
   patternFile->Connect("ReturnPressed()","ctbPattern",this,"setFile()");
   // patternFile->SetTextJustify(kTextLeft);

   

   browseFile=new TGTextButton(hframe,"Browse");
   hframe->AddFrame(browseFile,new TGLayoutHints(kLHintsTop | kLHintsLeft| kLHintsExpandX, 1, 1, 1, 1));
   browseFile->MapWindow();
   // patternCompiler->SetTextJustify(kTextLeft);
   browseFile->Connect("Clicked()","ctbPattern",this,"choosePattern()");




     hframe=new TGHorizontalFrame(this, 800,800);
   AddFrame(hframe,new TGLayoutHints(kLHintsTop | kLHintsExpandX , 1,1,1,1));
   hframe->MapWindow();


   sprintf(tit, "Samples per frame - ");

   label= new TGLabel(hframe, tit);
   hframe->AddFrame(label,new TGLayoutHints(kLHintsTop | kLHintsLeft| kLHintsExpandX, 1, 1, 1, 1));
   label->MapWindow();
   label->SetTextJustify(kTextLeft);


   sprintf(tit, "Analog: ");

   label= new TGLabel(hframe, tit);
   hframe->AddFrame(label,new TGLayoutHints(kLHintsTop | kLHintsLeft| kLHintsExpandX, 1, 1, 1, 1));
   label->MapWindow();
   label->SetTextJustify(kTextLeft);


  
   eAnalogSamples = new TGNumberEntry(hframe, 0, 9,999, TGNumberFormat::kNESInteger,
                                               TGNumberFormat::kNEANonNegative, 
                                               TGNumberFormat::kNELLimitMinMax,
                                               1, 8192);
   hframe->AddFrame( eAnalogSamples,new TGLayoutHints(kLHintsTop |  kLHintsExpandX, 1, 1, 1, 1));
   eAnalogSamples->MapWindow();
   eAnalogSamples->Resize(150,30);
   e= eAnalogSamples->TGNumberEntry::GetNumberEntry();
   e->Connect("ReturnPressed()","ctbPattern",this,"setAnalogSamples()");
 
   sprintf(tit, "Digital: ");

   label= new TGLabel(hframe, tit);
   hframe->AddFrame(label,new TGLayoutHints(kLHintsTop | kLHintsLeft| kLHintsExpandX, 1, 1, 1, 1));
   label->MapWindow();
   label->SetTextJustify(kTextLeft);


  
   eDigitalSamples = new TGNumberEntry(hframe, 0, 9,999, TGNumberFormat::kNESInteger,
                                               TGNumberFormat::kNEANonNegative, 
                                               TGNumberFormat::kNELLimitMinMax,
                                               1, 8192);
   hframe->AddFrame( eDigitalSamples,new TGLayoutHints(kLHintsTop |  kLHintsExpandX, 1, 1, 1, 1));
   eDigitalSamples->MapWindow();
   eDigitalSamples->Resize(150,30);
   e= eDigitalSamples->TGNumberEntry::GetNumberEntry();
   e->Connect("ReturnPressed()","ctbPattern",this,"setDigitalSamples()");
 
   
   hframe=new TGHorizontalFrame(this, 800,800);
   AddFrame(hframe,new TGLayoutHints(kLHintsTop | kLHintsExpandX , 1,1,1,1));
   hframe->MapWindow();


   sprintf(tit, "Read Out Mode: ");

   label= new TGLabel(hframe, tit);
   hframe->AddFrame(label,new TGLayoutHints(kLHintsTop | kLHintsLeft| kLHintsExpandX, 1, 1, 1, 1));
   label->MapWindow();
   label->SetTextJustify(kTextLeft);


   cbAnalog= new TGCheckButton(hframe, "Analog");
   hframe->AddFrame(cbAnalog,new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 5, 5, 5, 5));
   cbAnalog->MapWindow();
   cbAnalog->SetTextJustify(kTextRight);
   cbAnalog->Connect("Toggled(Bool_t)","ctbPattern",this,"setReadoutMode(Bool_t)");

   cbDigital= new TGCheckButton(hframe, "Digital");
   hframe->AddFrame(cbDigital,new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 5, 5, 5, 5));
   cbDigital->MapWindow();
   cbDigital->SetTextJustify(kTextRight);
   cbDigital->Connect("Toggled(Bool_t)","ctbPattern",this,"setReadoutMode(Bool_t)");

  


}

void ctbPattern::update() {
  try{
    auto retval = myDet->getRUNClock().tsquash("Different values");
    eRunClkFreq->SetNumber(retval);
  } CATCH_DISPLAY ("Could not get run clock.", "ctbPattern::update")

  try{
    auto retval = myDet->getADCClock().tsquash("Different values");
    eAdcClkFreq->SetNumber(retval);
  } CATCH_DISPLAY ("Could not get adc clock.", "ctbPattern::update")

  try{
    auto retval = myDet->getADCPhase().tsquash("Different values");
    eAdcClkPhase->SetNumber(retval);
  } CATCH_DISPLAY ("Could not get adc phase shift.", "ctbPattern::update")

  try{
    auto retval = myDet->getADCPipeline().tsquash("Different values");
    eAdcPipeline->SetNumber(retval);
  } CATCH_DISPLAY ("Could not get adc pipeline.", "ctbPattern::update")

  try{
    auto retval = myDet->getDBITClock().tsquash("Different values");
    eDBitClkFreq->SetNumber(retval);
  } CATCH_DISPLAY ("Could not get dbit clock.", "ctbPattern::update")

  try{
    auto retval = myDet->getDBITPhase().tsquash("Different values");
    eDBitClkPhase->SetNumber(retval);
  } CATCH_DISPLAY ("Could not get dbit phase shift.", "ctbPattern::update")

  try{
    auto retval = myDet->getDBITPipeline().tsquash("Different values");
    eDBitPipeline->SetNumber(retval);
  } CATCH_DISPLAY ("Could not get dbit pipeline.", "ctbPattern::update")

  try{
    auto retval = myDet->getNumberOfFrames().tsquash("Different values");
    eFrames->SetNumber(retval);
  } CATCH_DISPLAY ("Could not get number of frames.", "ctbPattern::update")

  try{
    auto timeNs = myDet->getPeriod().tsquash("Different values");
    ePeriod->SetNumber(ctbDefs::ConvertChronoNStoDoubleS(timeNs));
  } CATCH_DISPLAY ("Could not get period.", "ctbPattern::update")

  try{
    auto retval = myDet->getNumberOfTriggers().tsquash("Different values");
    eTriggers->SetNumber(retval);
  } CATCH_DISPLAY ("Could not get number of triggers.", "ctbPattern::update")

  try{
    auto retval = myDet->getPatternLoopAddresses(-1).tsquash("Different values");
    eStartAddr->SetHexNumber(retval[0]);
    eStopAddr->SetHexNumber(retval[1]);
  } CATCH_DISPLAY ("Could not get dbit phase shift.", "ctbPattern::update")

  for (int iloop=0; iloop<NLOOPS; iloop++) {
    eLoop[iloop]->update();  
  }

  for (int iwait=0; iwait<NWAITS; iwait++) {
    eWait[iwait]->update();
  }

  getAnalogSamples();
  getDigitalSamples();
  getReadoutMode();
}


void ctbPattern::setFile() {
  patternFileChanged(patternFile->GetText());

}


void ctbPattern::setCompiler() {
  patternCompilerChanged(patternCompiler->GetText());
}


void ctbPattern::patternFileChanged(const char* t){
  Emit("patternFileChanged(const char*)", t);
}


void ctbPattern::patternCompilerChanged(const char* t){
  Emit("patternCompilerChanged(const char*)", t);

}


void ctbPattern::setPatternAlias(string line){
  char fname[10000];
  if (sscanf(line.c_str(),"PATCOMPILER %s",fname)) {
    patternCompiler->SetText(fname);
    patternCompilerChanged(patternCompiler->GetText());
  } else if (sscanf(line.c_str(),"PATFILE %s",fname)) {
    patternFile->SetText(fname);
    patternFileChanged(patternFile->GetText());
  }
}


string ctbPattern::getPatternAlias() {
    char line[100000];
    sprintf(line, "PATCOMPILER %s\nPATFILE %s\n",patternCompiler->GetText(),patternFile->GetText());
    return line;
}


void ctbPattern::chooseCompiler() {
  static TString dir(".");
  TGFileInfo fi;
  //fi.fFileTypes = filetypes;
  fi.fIniDir    = StrDup(dir);
  printf("fIniDir = %s\n", fi.fIniDir);
  new TGFileDialog(gClient->GetRoot(), this,  kFDOpen, &fi);
  printf("Open file: %s (dir: %s)\n", fi.fFilename, fi.fIniDir);
  // dir = fi.fIniDir;
  if (fi.fFilename) {
    patternCompiler->SetText(fi.fFilename);
    patternCompilerChanged(patternCompiler->GetText());
  }
}


void ctbPattern::choosePattern() {
  static TString dir(".");
  TGFileInfo fi;
  //fi.fFileTypes = filetypes;
  fi.fIniDir    = StrDup(dir);
  printf("fIniDir = %s\n", fi.fIniDir);
  new TGFileDialog(gClient->GetRoot(), this,  kFDOpen, &fi);
  printf("Open file: %s (dir: %s)\n", fi.fFilename, fi.fIniDir);
  // dir = fi.fIniDir;
  if (fi.fFilename) {
    patternFile->SetText(fi.fFilename);
    patternFileChanged(patternFile->GetText());
  }
  
}


string ctbPattern::getCompiler() {
  return string(patternCompiler->GetText());

}

string ctbPattern::getPatternFile() {
  return string(patternFile->GetText());

}

void ctbPattern::setFrames() {
  try{
    myDet->setNumberOfFrames(eFrames->GetNumber());
  } CATCH_DISPLAY ("Could not set number of frames", "ctbPattern::setFrames")
}

void ctbPattern::setTriggers() {
  try{
    myDet->setNumberOfTriggers(eTriggers->GetNumber());
  } CATCH_DISPLAY ("Could not set number of triggers", "ctbPattern::setTriggers")
}

void ctbPattern::setPeriod() {
  using std::chrono::duration;
  using std::chrono::duration_cast;
  using std::chrono::nanoseconds; 
  try{
    auto timeNs = ctbDefs::ConvertDoubleStoChronoNS(ePeriod->GetNumber());
    myDet->setPeriod(timeNs);
  } CATCH_DISPLAY ("Could not set period", "ctbPattern::setPeriod")
}

void ctbPattern::setAdcFreq() {
  try{
    myDet->setADCClock(eAdcClkFreq->GetNumber());
  } CATCH_DISPLAY ("Could not set adc clock", "ctbPattern::setAdcFreq")
}

void ctbPattern::setRunFreq() {
  try{
    myDet->setRUNClock(eRunClkFreq->GetNumber());
  } CATCH_DISPLAY ("Could not set run clock", "ctbPattern::setRunFreq")
}

void ctbPattern::setDBitFreq() {
  try{
    myDet->setDBITClock(eDBitClkFreq->GetNumber());
  } CATCH_DISPLAY ("Could not set dbit clock", "ctbPattern::setDBitFreq")
}

void ctbPattern::setAdcPhase() {
  try{
    myDet->setADCPhase(eAdcClkPhase->GetNumber());
  } CATCH_DISPLAY ("Could not set adc phase shift", "ctbPattern::setAdcPhase")
}

void ctbPattern::setDBitPhase() {
  try{
    myDet->setDBITPhase(eDBitClkPhase->GetNumber());
  } CATCH_DISPLAY ("Could not set dbit phase shift", "ctbPattern::setDBitPhase")
}

void ctbPattern::setAdcPipeline() {
  try{
    myDet->setADCPipeline(eAdcPipeline->GetNumber());
  } CATCH_DISPLAY ("Could not set adc pipeline", "ctbPattern::setAdcPipeline")
}

void ctbPattern::setDBitPipeline() {
  try{
    myDet->setDBITPipeline(eDBitPipeline->GetNumber());
  } CATCH_DISPLAY ("Could not set dbit pipeline", "ctbPattern::setDBitPipeline")
}


void ctbPattern::setAnalogSamples() {
  try{
    myDet->setNumberOfAnalogSamples(eAnalogSamples->GetNumber());
  } CATCH_DISPLAY ("Could not set number of analog sampels", "ctbPattern::setAnalogSamples")

  analogSamplesChanged(eAnalogSamples->GetNumber());
}

void ctbPattern::setDigitalSamples() {
  try{
    myDet->setNumberOfDigitalSamples(eDigitalSamples->GetNumber());
  } CATCH_DISPLAY ("Could not set number of digital samples", "ctbPattern::setDigitalSamples")

  digitalSamplesChanged(eDigitalSamples->GetNumber());
}

void ctbPattern::setReadoutMode(Bool_t) {
  try {
    slsDetectorDefs::readoutMode flag = slsDetectorDefs::ANALOG_ONLY;
    if (cbAnalog->IsOn() && cbDigital->IsOn()) 
      flag=slsDetectorDefs::ANALOG_AND_DIGITAL;
    else  if (~cbAnalog->IsOn() && cbDigital->IsOn()) 
      flag=slsDetectorDefs::DIGITAL_ONLY;
    else  if (cbAnalog->IsOn() && ~cbDigital->IsOn())       
      flag=slsDetectorDefs::ANALOG_ONLY;
    else {
      throw runtime_error("unkown readout flag");
    }
    myDet->setReadoutMode(flag);
    cout << "Set readout flag: " << flag << endl;
  } CATCH_DISPLAY ("Could not set readout flag", "ctbPattern::setReadoutMode")

  getReadoutMode();
}

void ctbPattern::readoutModeChanged(int flags) {
 Emit("readoutModeChanged(Int_t)",(int)flags);

}

int ctbPattern::getReadoutMode() {  
 int retval=slsDetectorDefs::ANALOG_ONLY;
    
 if (myDet->getDetectorType().squash() == slsDetectorDefs::CHIPTESTBOARD) {
   try{
     retval = myDet->getReadoutMode().tsquash("Different values");
   } CATCH_DISPLAY ("Could not get readout flags", "ctbPattern::getReadoutMode")
      
   switch(retval) {
   case slsDetectorDefs::ANALOG_AND_DIGITAL:
     cout << "analog and digital" << endl;
     cbAnalog->SetOn(kTRUE);
     cbDigital->SetOn(kTRUE);
     break;
   case slsDetectorDefs::DIGITAL_ONLY:
     cout << "digital only" << endl;
     cbAnalog->SetOn(kFALSE);
     cbDigital->SetOn(kTRUE);
     break;
   case slsDetectorDefs::ANALOG_ONLY:
     cout << "analog only" << endl;
     cbAnalog->SetOn(kTRUE);
     cbDigital->SetOn(kFALSE);    
     break;
   default:
     throw("unknown readout flag");
   }
 } else {
   cbAnalog->SetOn(kTRUE);
   cbDigital->SetOn(kFALSE);    
 }
    
    Emit("readoutModeChanged(int)",static_cast<int>(retval));
    return retval;
 
 
}

int ctbPattern::getAnalogSamples() {
  try{
    auto retval = myDet->getNumberOfAnalogSamples().tsquash("Different values");
    eAnalogSamples->SetNumber((Double_t)retval);
    Emit("analogSamplesChanged(const int)", eAnalogSamples->GetNumber());
    return eAnalogSamples->GetNumber();
  } CATCH_DISPLAY ("Could not get number of triggers.", "ctbPattern::update")

  return -1;
}
   
int ctbPattern::getDigitalSamples() { 
  int retval=0;
  if (myDet->getDetectorType().squash() == slsDetectorDefs::CHIPTESTBOARD) {
    try{
      auto retval = myDet->getNumberOfDigitalSamples().tsquash("Different values"); 
    } CATCH_DISPLAY ("Could not get number of digital samples.", "ctbPattern::getDigitalSamples")
	}
  eDigitalSamples->SetNumber((Double_t)retval);
  Emit("digitalSamplesChanged(const int)", eDigitalSamples->GetNumber());
  return eDigitalSamples->GetNumber();
 

  return -1;
}
   

void ctbPattern::analogSamplesChanged(const int t){
  Emit("analogSamplesChanged(const int)", t);
}

void ctbPattern::digitalSamplesChanged(const int t){
  Emit("digitalSamplesChanged(const int)", t);
}
