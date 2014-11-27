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

#include <stdio.h>
#include <iostream>
#include <fstream>

#include "jctbPattern.h"
#include "multiSlsDetector.h"

using namespace std;





jctbLoop::jctbLoop(TGGroupFrame *page, int i, multiSlsDetector *det) : TGHorizontalFrame(page, 800,800), id(i), myDet(det) {

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
   e->Connect("ReturnPressed()","jctbLoop",this,"setNLoops()");




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

    eLoopStartAddr->SetState(kFALSE);
   
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



    eLoopStopAddr->SetState(kFALSE);
}

void jctbLoop::setNLoops() {

  int start, stop, n;
  

    start=-1;
    stop=-1;
    n=eLoopNumber->GetNumber();
    myDet->setCTBPatLoops(id,start, stop,n);


}



void jctbLoop::update() {

  int start, stop, n;
  

    start=-1;
    stop=-1;
    n=-1;
    myDet->setCTBPatLoops(id,start, stop,n);
    eLoopStartAddr->SetHexNumber(start);
    eLoopStopAddr->SetHexNumber(stop);
    eLoopNumber->SetNumber(n);

}



jctbWait::jctbWait(TGGroupFrame *page, int i, multiSlsDetector *det) : TGHorizontalFrame(page, 800,800), id(i), myDet(det) {

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
   e->Connect("ReturnPressed()","jctbWait",this,"setWaitTime()");



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

   eWaitAddr->SetState(kFALSE);
  
}



void jctbWait::setWaitTime() {


  Long64_t t=eWaitTime->GetNumber();
  
  t=myDet->setCTBPatWaitTime(id,t);
}



void jctbWait::update() {

  int start, stop, n, addr;
  
  Long64_t t=-1;


    t=myDet->setCTBPatWaitTime(id,t);
    addr=myDet->setCTBPatWaitAddr(id,-1);
    eWaitAddr->SetHexNumber(addr);
    eWaitTime->SetNumber(t);

}









jctbPattern::jctbPattern(TGVerticalFrame *page, multiSlsDetector *det)
  : TGGroupFrame(page,"Pattern",kVerticalFrame), myDet(det) {


  SetTitlePos(TGGroupFrame::kLeft);
  page->AddFrame(this,new TGLayoutHints( kLHintsTop | kLHintsExpandX , 10,10,10,10));
  MapWindow();
  

  char tit[100];

 
  TGHorizontalFrame* hframe=new TGHorizontalFrame(this, 800,800);
  AddFrame(hframe,new TGLayoutHints(kLHintsTop | kLHintsExpandX , 1,1,1,1));
  hframe->MapWindow();


   sprintf(tit, "ADC Clock Frequency (MHz): ");

   TGLabel *label= new TGLabel(hframe, tit);
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
   TGTextEntry *e= eAdcClkFreq->TGNumberEntry::GetNumberEntry();
   e->Connect("ReturnPressed()","jctbPattern",this,"setAdcFreq()");
 
   
   label= new TGLabel(hframe, " Phase (0.15ns step): ");
   hframe->AddFrame(label,new TGLayoutHints(kLHintsTop | kLHintsLeft| kLHintsExpandX, 1, 1, 1, 1));
   label->MapWindow();
   label->SetTextJustify(kTextLeft);




  
   eAdcClkPhase = new TGNumberEntry(hframe, 0, 9,999, TGNumberFormat::kNESInteger,
                                               TGNumberFormat::kNEANonNegative, 
                                               TGNumberFormat::kNELLimitMinMax,
                                               0, 200);
   hframe->AddFrame( eAdcClkPhase,new TGLayoutHints(kLHintsTop |  kLHintsExpandX, 1, 1, 1, 1));
   eAdcClkPhase->MapWindow();
   eAdcClkPhase->Resize(150,30);
   e= eAdcClkPhase->TGNumberEntry::GetNumberEntry();
   e->Connect("ReturnPressed()","jctbPattern",this,"setAdcPhase()");
 


   hframe=new TGHorizontalFrame(this, 800,800);
   AddFrame(hframe,new TGLayoutHints(kLHintsTop | kLHintsExpandX , 1,1,1,1));
   hframe->MapWindow();


   sprintf(tit, "Run Clock Frequency (MHz): ");

   label= new TGLabel(hframe, tit);
   hframe->AddFrame(label,new TGLayoutHints(kLHintsTop | kLHintsLeft| kLHintsExpandX, 1, 1, 1, 1));
   label->MapWindow();
   label->SetTextJustify(kTextLeft);



  
   eRunClkFreq = new TGNumberEntry(hframe, 0, 9,999, TGNumberFormat::kNESInteger,
                                               TGNumberFormat::kNEANonNegative, 
                                               TGNumberFormat::kNELLimitMinMax,
                                               0, 160);
   hframe->AddFrame( eRunClkFreq,new TGLayoutHints(kLHintsTop |  kLHintsExpandX, 1, 1, 1, 1));
   eRunClkFreq->MapWindow();
   eRunClkFreq->Resize(150,30);
   e= eRunClkFreq->TGNumberEntry::GetNumberEntry();
   e->Connect("ReturnPressed()","jctbPattern",this,"setRunFreq()");
 

   
   label= new TGLabel(hframe, " Phase (0.15ns step): ");
   hframe->AddFrame(label,new TGLayoutHints(kLHintsTop | kLHintsLeft| kLHintsExpandX, 1, 1, 1, 1));
   label->MapWindow();
   label->SetTextJustify(kTextLeft);




  
   eRunClkPhase = new TGNumberEntry(hframe, 0, 9,999, TGNumberFormat::kNESInteger,
                                               TGNumberFormat::kNEANonNegative, 
                                               TGNumberFormat::kNELLimitMinMax,
                                               0, 200);
   hframe->AddFrame( eRunClkPhase,new TGLayoutHints(kLHintsTop |  kLHintsExpandX, 1, 1, 1, 1));
   eRunClkPhase->MapWindow();
   eRunClkPhase->Resize(150,30);
   e= eRunClkPhase->TGNumberEntry::GetNumberEntry();
   e->Connect("ReturnPressed()","jctbPattern",this,"setRunPhase()");







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
   e->Connect("ReturnPressed()","jctbPattern",this,"setFrames()");

   
   label= new TGLabel(hframe, " Period (s): ");
   hframe->AddFrame(label,new TGLayoutHints(kLHintsTop | kLHintsLeft| kLHintsExpandX, 1, 1, 1, 1));
   label->MapWindow();
   label->SetTextJustify(kTextLeft);




  
   ePeriod = new TGNumberEntry(hframe, 0, 9,999, TGNumberFormat::kNESInteger,
                                               TGNumberFormat::kNEANonNegative, 
                                               TGNumberFormat::kNELNoLimits);
   hframe->AddFrame( ePeriod,new TGLayoutHints(kLHintsTop |  kLHintsExpandX, 1, 1, 1, 1));
   ePeriod->MapWindow();
   ePeriod->Resize(150,30);
   e= ePeriod->TGNumberEntry::GetNumberEntry();
   e->Connect("ReturnPressed()","jctbPattern",this,"setPeriod()");






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

    eLoop[idac]=new jctbLoop(this,idac,myDet);


 
  }




  for (idac=0; idac<NWAITS; idac++) {
 

   eWait[idac]=new jctbWait(this,idac,myDet);

   

  }



  



}

void jctbPattern::update() {

  int start, stop, n, addr;
  
  Long_t t;



  eAdcClkFreq->SetNumber(myDet->setSpeed(slsDetectorDefs::ADC_CLOCK,-1));
  eRunClkFreq->SetNumber(myDet->setSpeed(slsDetectorDefs::CLOCK_DIVIDER,-1));
  //ADC_PHASE
  //PHASE_SHIFT
  eFrames->SetNumber(myDet->setTimer(slsDetectorDefs::FRAME_NUMBER,-1));
  ePeriod->SetNumber(((Double_t)myDet->setTimer(slsDetectorDefs::FRAME_PERIOD,-1))*1E-9);

  start=-1;
  stop=-1;
  n=-1;
  myDet->setCTBPatLoops(-1,start, stop,n);
  eStartAddr->SetHexNumber(start);
  eStopAddr->SetHexNumber(stop);

  for (int iloop=0; iloop<NLOOPS; iloop++) {
    eLoop[iloop]->update();
    
  }

  for (int iwait=0; iwait<NWAITS; iwait++) {
    eWait[iwait]->update();
  }

  

}

void jctbPattern::setFrames() {
  myDet->setTimer(slsDetectorDefs::FRAME_NUMBER,eFrames->GetNumber());
}

void jctbPattern::setPeriod() {
  myDet->setTimer(slsDetectorDefs::FRAME_PERIOD,eFrames->GetNumber()*1E9);
}




void jctbPattern::setAdcFreq() {
  myDet->setSpeed(slsDetectorDefs::ADC_CLOCK,eAdcClkFreq->GetNumber());
}
void jctbPattern::setRunFreq() {
  myDet->setSpeed(slsDetectorDefs::CLOCK_DIVIDER,eRunClkFreq->GetNumber());

}
void jctbPattern::setAdcPhase() {
  myDet->setSpeed(slsDetectorDefs::ADC_PHASE,eAdcClkPhase->GetNumber());

}


void jctbPattern::setRunPhase() {
  myDet->setSpeed(slsDetectorDefs::PHASE_SHIFT,eRunClkPhase->GetNumber());

}
   
