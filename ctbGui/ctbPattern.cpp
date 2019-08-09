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
#include "multiSlsDetector.h"

using namespace std;





ctbLoop::ctbLoop(TGGroupFrame *page, int i, multiSlsDetector *det) : TGHorizontalFrame(page, 800,800), id(i), myDet(det) {

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

  int start, stop, n;
  

    start=-1;
    stop=-1;
    n=eLoopNumber->GetNumber();
    try{
      myDet->setPatternLoops(id,start, stop,n);
    } catch (...) {

      cout << "Do nothing for this error" << endl;
    }


}



void ctbLoop::update() {

  int start, stop, n;
  
  std::array<int, 3> loop;

 try {
   loop=myDet->getPatternLoops(id);
      } catch (...) {
    
      cout << "Do nothing for this error" << endl;
  }
   

    eLoopStartAddr->SetHexNumber(loop[0]);
    eLoopStopAddr->SetHexNumber(loop[1]);
    eLoopNumber->SetNumber(loop[2]);
}



ctbWait::ctbWait(TGGroupFrame *page, int i, multiSlsDetector *det) : TGHorizontalFrame(page, 800,800), id(i), myDet(det) {

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


  Long64_t t=eWaitTime->GetNumber();
  try{
    t=myDet->setPatternWaitTime(id,t);
  } catch (...) {
    
    cout << "Do nothing for this error" << endl;
  }
}



void ctbWait::update() {

  int start, stop, n, addr;
  
  Long64_t t=-1;
  try{
    
    t=myDet->setPatternWaitTime(id,t);
  } catch (...) {
    
    cout << "Do nothing for this error" << endl;
  }
  try{
    addr=myDet->setPatternWaitAddr(id,-1);
  
  } catch (...) {
    
    cout << "Do nothing for this error" << endl;
  }
    eWaitAddr->SetHexNumber(addr);
    eWaitTime->SetNumber(t);

}









ctbPattern::ctbPattern(TGVerticalFrame *page, multiSlsDetector *det)
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


   sprintf(tit, "Number of cycles: ");

   label= new TGLabel(hframe, tit);
   hframe->AddFrame(label,new TGLayoutHints(kLHintsTop | kLHintsLeft| kLHintsExpandX, 1, 1, 1, 1));
   label->MapWindow();
   label->SetTextJustify(kTextLeft);



  
   eCycles = new TGNumberEntry(hframe, 0, 9,999, TGNumberFormat::kNESInteger,
                                               TGNumberFormat::kNEANonNegative, 
                                               TGNumberFormat::kNELNoLimits);
   hframe->AddFrame( eCycles,new TGLayoutHints(kLHintsTop |  kLHintsExpandX, 1, 1, 1, 1));
   eCycles->MapWindow();
   eCycles->Resize(150,30);
   e= eCycles->TGNumberEntry::GetNumberEntry();
   e->Connect("ReturnPressed()","ctbPattern",this,"setCycles()");


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

  int start, stop, n, addr;
  
  Long_t t;

  
  try {
    n=myDet->setSpeed(slsDetectorDefs::CLOCK_DIVIDER,-1,0);
      } catch (...) {
    
      cout << "Do nothing for this error" << endl;
  }

  eRunClkFreq->SetNumber(n);

  try {
    n=myDet->setSpeed(slsDetectorDefs::ADC_CLOCK,-1,0);
      } catch (...) {
    
      cout << "Do nothing for this error" << endl;
  }

  eAdcClkFreq->SetNumber(n);

  try {
    n=myDet->setSpeed(slsDetectorDefs::ADC_PHASE,-1,0);
      } catch (...) {
    
      cout << "Do nothing for this error" << endl;
  }

  eAdcClkPhase->SetNumber(n);

  try {
    n=myDet->setSpeed(slsDetectorDefs::ADC_PIPELINE,-1,0);
      } catch (...) {
    
      cout << "Do nothing for this error" << endl;
  }


  eAdcPipeline->SetNumber(n);

  try {
    n=myDet->setSpeed(slsDetectorDefs::DBIT_CLOCK,-1,0);
      } catch (...) {
    
      cout << "Do nothing for this error" << endl;
  }


  eDBitClkFreq->SetNumber(n);

  try {
    n=myDet->setSpeed(slsDetectorDefs::DBIT_PHASE,-1,0);
      } catch (...) {
    
      cout << "Do nothing for this error" << endl;
  }


  eDBitClkPhase->SetNumber(n);
try {
  myDet->setSpeed(slsDetectorDefs::DBIT_PIPELINE,0,-1);
      } catch (...) {
    
      cout << "Do nothing for this error" << endl;
  }
  eDBitPipeline->SetNumber(n);

try {
  n=myDet->setTimer(slsDetectorDefs::FRAME_NUMBER,-1);
      } catch (...) {
    
      cout << "Do nothing for this error" << endl;
  }

  eFrames->SetNumber(n);

  
try {
  n=myDet->setTimer(slsDetectorDefs::FRAME_PERIOD,-1);
      } catch (...) {
    
      cout << "Do nothing for this error" << endl;
  }



 ePeriod->SetNumber(((Double_t)n)*1E-9);

 try {
   n=myDet->setTimer(slsDetectorDefs::CYCLES_NUMBER,-1);
      } catch (...) {
    
      cout << "Do nothing for this error" << endl;
  }

  eCycles->SetNumber(n);


 // try {
 //   myDet->setTimer(slsDetectorDefs::MEASUREMENTS_NUMBER,-1);
 //      } catch (...) {
    
 //      cout << "Do nothing for this error" << endl;
 //  }
   
 //  eMeasurements->SetNumber(n);

  start=-1;
  stop=-1;
  n=-1;
  std::array<int, 3> loop;

 try {
   loop=myDet->getPatternLoops(-1);
      } catch (...) {
    
      cout << "Do nothing for this error" << endl;
  }
   
  eStartAddr->SetHexNumber(loop[0]);
  eStopAddr->SetHexNumber(loop[1]);
 
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
    sprintf("PATCOMPILER %s\nPATFILE %s\n",patternCompiler->GetText(),patternFile->GetText());
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
  try {
  myDet->setTimer(slsDetectorDefs::FRAME_NUMBER,eFrames->GetNumber());
      } catch (...) {
    
      cout << "Do nothing for this error" << endl;
  }
}

void ctbPattern::setCycles() {
  try {
  myDet->setTimer(slsDetectorDefs::CYCLES_NUMBER,eFrames->GetNumber());
      } catch (...) {
    
      cout << "Do nothing for this error" << endl;
  }
}

// void ctbPattern::setMeasurements() {
//   try {
//   myDet->setTimer(slsDetectorDefs::MEASUREMENTS_NUMBER,eFrames->GetNumber());
//       } catch (...) {
    
//       cout << "Do nothing for this error" << endl;
//   }
// }




void ctbPattern::setPeriod() {
  try {
  myDet->setTimer(slsDetectorDefs::FRAME_PERIOD,ePeriod->GetNumber()*1E9);
      } catch (...) {
    
      cout << "Do nothing for this error" << endl;
  }
}




void ctbPattern::setAdcFreq() {
  try {
  myDet->setSpeed(slsDetectorDefs::ADC_CLOCK,eAdcClkFreq->GetNumber(),0);
      } catch (...) {
    
      cout << "Do nothing for this error" << endl;
  }
}
void ctbPattern::setRunFreq() {
  try{
    myDet->setSpeed(slsDetectorDefs::CLOCK_DIVIDER,eRunClkFreq->GetNumber(),0);
      } catch (...) {
    
      cout << "Do nothing for this error" << endl;
  }

}
void ctbPattern::setDBitFreq() {
  // cout <<"Not setting dbit frequency to " << eDBitClkFreq->GetNumber()<< endl;
  try {
  myDet->setSpeed(slsDetectorDefs::DBIT_CLOCK,eDBitClkFreq->GetNumber(),0);
      } catch (...) {
    
      cout << "Do nothing for this error" << endl;
  }

}
void ctbPattern::setAdcPhase() {
  try {
    myDet->setSpeed(slsDetectorDefs::ADC_PHASE,eAdcClkPhase->GetNumber(),0);
      } catch (...) {
    
      cout << "Do nothing for this error" << endl;
  }

}

void ctbPattern::setDBitPhase() {
  // cout <<"Not setting dbit phase to " << eDBitClkPhase->GetNumber()<< endl;
  try {
    myDet->setSpeed(slsDetectorDefs::DBIT_PHASE,eDBitClkPhase->GetNumber(),0);
      } catch (...) {
    
      cout << "Do nothing for this error" << endl;
  }

}


void ctbPattern::setAdcPipeline() {
  try {
    myDet->setSpeed(slsDetectorDefs::ADC_PIPELINE,eAdcPipeline->GetNumber(),0);
      } catch (...) {
    
      cout << "Do nothing for this error" << endl;
  }
}


void ctbPattern::setDBitPipeline() {
  // cout <<"Not setting dbit pipeline to " << eDBitPipeline->GetNumber() << endl;
  try {
    myDet->setSpeed(slsDetectorDefs::DBIT_PIPELINE,eDBitPipeline->GetNumber(),0);
      } catch (...) {
    
      cout << "Do nothing for this error" << endl;
  }
}


void ctbPattern::setAnalogSamples() {
  try {
  myDet->setTimer(slsDetectorDefs::ANALOG_SAMPLES,eAnalogSamples->GetNumber());
      } catch (...) {
    
      cout << "Do nothing for this error" << endl;
  } 
  analogSamplesChanged(eAnalogSamples->GetNumber());
}

void ctbPattern::setDigitalSamples() {
  try {
  myDet->setTimer(slsDetectorDefs::DIGITAL_SAMPLES,eDigitalSamples->GetNumber()); 
      } catch (...) {
    
      cout << "Do nothing for this error" << endl;
  }
  digitalSamplesChanged(eDigitalSamples->GetNumber());
}

void ctbPattern::setReadoutMode(Bool_t) {
  // cout << "Set readout mode to be implemented" << endl;
  slsDetectorDefs::readOutFlags flags;
  if (cbAnalog->IsOn() && cbDigital->IsOn()) flags=slsDetectorDefs::ANALOG_AND_DIGITAL;
  else  if (~cbAnalog->IsOn() && cbDigital->IsOn()) flags=slsDetectorDefs::DIGITAL_ONLY;
  else  if (cbAnalog->IsOn() && ~cbDigital->IsOn()) flags=slsDetectorDefs::NORMAL_READOUT;
  else flags=slsDetectorDefs::GET_READOUT_FLAGS;
  try {
    myDet->setReadOutFlags(flags);
  } catch (...) {
    
      cout << "Do nothing for this error" << endl;
  }
  cout << "Set readout flags " << hex << flags << dec << endl;
  getReadoutMode();
  // myDet->setTimer(slsDetectorDefs::SAMPLES_CTB,eSamples->GetNumber()); 
  //samplesChanged(eSamples->GetNumber());
}

void ctbPattern::readoutModeChanged(int flags) {
 Emit("readoutModeChanged(Int_t)",(int)flags);

}

int ctbPattern::getReadoutMode() {
  // cout << "Get readout mode to be implemented" << endl; 
  slsDetectorDefs::readOutFlags flags;
  try {
  flags=(slsDetectorDefs::readOutFlags) myDet->setReadOutFlags();
      } catch (...) {
    
      cout << "Do nothing for this error" << endl;
  }

  cout << "++++++++++++++++++++"<< hex << flags << dec << endl;
  if (flags&slsDetectorDefs::ANALOG_AND_DIGITAL) {
    cout << "analog and digital" << hex << slsDetectorDefs::ANALOG_AND_DIGITAL << dec<< endl;
    cbAnalog->SetOn(kTRUE);
    cbDigital->SetOn(kTRUE);
  } else if  (flags&slsDetectorDefs::DIGITAL_ONLY) {
    cout << "digital only"  << hex << slsDetectorDefs::DIGITAL_ONLY << dec << endl;
    cbAnalog->SetOn(kFALSE);
    cbDigital->SetOn(kTRUE);
  }// else if  (flags==slsDetectorDefs::NORMAL_READOUT) {
  //   cbAnalog->SetOn(kTRUE);
  //   cbDigital->SetOn(kFALSE);
  // }
  else {
    cout << "analog only" << endl;
    flags=slsDetectorDefs::NORMAL_READOUT;
    cbAnalog->SetOn(kTRUE);
    cbDigital->SetOn(kFALSE);
  }

  Emit("readoutModeChanged(int)",(int)flags);
  return (int)flags;

  // myDet->setTimer(slsDetectorDefs::SAMPLES_CTB,eSamples->GetNumber()); 
  //samplesChanged(eSamples->GetNumber());
}

int ctbPattern::getAnalogSamples() {
  int n;
  try {
    n=(myDet->setTimer(slsDetectorDefs::ANALOG_SAMPLES,-1));
      } catch (...) {
    
      cout << "Do nothing for this error" << endl;
  }
  eAnalogSamples->SetNumber((Double_t)n);
  Emit("analogSamplesChanged(const int)", eAnalogSamples->GetNumber());
  return eAnalogSamples->GetNumber();
}
   
int ctbPattern::getDigitalSamples() {
  int n;
  try {

    n=(myDet->setTimer(slsDetectorDefs::DIGITAL_SAMPLES,-1));
      } catch (...) {
    
      cout << "Do nothing for this error" << endl;
  }

  eDigitalSamples->SetNumber(((Double_t)n));
  Emit("digitalSamplesChanged(const int)", eDigitalSamples->GetNumber());
  return eDigitalSamples->GetNumber();
}
   


void ctbPattern::analogSamplesChanged(const int t){
  Emit("analogSamplesChanged(const int)", t);
}

void ctbPattern::digitalSamplesChanged(const int t){
  Emit("digitalSamplesChanged(const int)", t);
}
