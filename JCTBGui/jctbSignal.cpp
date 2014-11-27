#include <TGButton.h>
#include <TRootEmbeddedCanvas.h>
#include <TGButtonGroup.h>
#include <TGNumberEntry.h>
#include <TGLabel.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <TColor.h>
#include <TGColorSelect.h>


#include "jctbSignal.h"
#include "multiSlsDetector.h"

using namespace std;



jctbSignal::jctbSignal(TGFrame *page, int i,  multiSlsDetector *det)
  : TGHorizontalFrame(page, 800,50), myDet(det), id(i), hsig(NULL) {


  TGHorizontalFrame *hframe=this;
  char tit[100];


   
   sprintf(tit, "BIT%d ",id);

   sLabel= new TGLabel(hframe, tit);
   hframe->AddFrame( sLabel,new TGLayoutHints(kLHintsTop | kLHintsLeft| kLHintsExpandX, 1, 1, 1, 1));
   sLabel->MapWindow();
   sLabel->SetTextJustify(kTextLeft);



   sOutput= new TGCheckButton(hframe, "Out");
   hframe->AddFrame( sOutput,new TGLayoutHints(kLHintsTop | kLHintsLeft| kLHintsExpandX, 1, 1, 1, 1));
   sOutput->MapWindow();

 
   sOutput->Connect("Toggled(Bool_t)","jctbSignal",this,"ToggledOutput(Bool_t)");

   sClock= new TGCheckButton(hframe, "Clk");
   hframe->AddFrame( sClock,new TGLayoutHints(kLHintsTop | kLHintsLeft| kLHintsExpandX, 1, 1, 1, 1));
   sClock->MapWindow();

   sClock->Connect("Toggled(Bool_t)","jctbSignal",this,"ToggledClock(Bool_t)");


   sPlot= new TGCheckButton(hframe, "Plot");
   hframe->AddFrame( sPlot,new TGLayoutHints(kLHintsTop | kLHintsLeft| kLHintsExpandX, 1, 1, 1, 1));
   sPlot->MapWindow();

   sPlot->Connect("Toggled(Bool_t)","jctbSignal",this,"ToggledPlot(Bool_t)");
   
   fColorSel = new TGColorSelect(hframe, id+1, 0);
   hframe->AddFrame(fColorSel, new TGLayoutHints(kLHintsTop |
                          kLHintsLeft, 2, 0, 2, 2));
   

   fColorSel->SetColor(TColor::Number2Pixel(id+1));

   
   ToggledOutput(kFALSE);

   if (id==63) {
     sOutput->SetOn(kTRUE);
     sClock->SetOn(kFALSE);
     sOutput->SetEnabled(kFALSE);
     sClock->SetEnabled(kFALSE);
   }
     

}
int jctbSignal::setSignalAlias(char *tit, int plot, int col) {

  if (tit)
    sLabel->SetText(tit);

  if (plot>0) {
      sPlot->SetOn(kTRUE,kTRUE); 
  } else if (plot==0)
      sPlot->SetOn(kFALSE,kTRUE);
    
  if (col>=0)
    fColorSel->SetColor(col);//TColor::Number2Pixel(col+1));

  fColorSel->SetEnabled(sPlot->IsOn());
  return 0;

}

string jctbSignal::getSignalAlias() {
  

  ostringstream oss;
  oss << "BIT" << dec << id << " " << sLabel->GetText()->Data() << " " << sPlot->IsOn() << hex << " " << fColorSel->GetColor() << endl;
  return oss.str();




}
int jctbSignal::setOutput(Long64_t r) {


  //  cout << hex << r << dec <<endl;

  Long64_t mask=((Long64_t)1<<id);

  if (r&mask)
      sOutput->SetOn(kTRUE,kTRUE);
  else 
      sOutput->SetOn(kFALSE,kTRUE);
    
  return sOutput->IsOn();

}

int jctbSignal::setClock(Long64_t r) {

  Long64_t mask=((Long64_t)1<<id);

  // cout << hex << r << dec <<endl;

  if (r&mask)
      sClock->SetOn(kTRUE,kTRUE);
  else 
      sClock->SetOn(kFALSE,kTRUE);
    
  return sClock->IsOn();

}

int jctbSignal::isClock() { sClock->IsOn();}
int jctbSignal::isOutput() { sOutput->IsOn();}
int jctbSignal::isPlot() { sPlot->IsOn();}


void jctbSignal::ToggledOutput(Bool_t b) {
  Long_t mask=b<<id;
  ToggledSignalOutput(b<<id);
  if (b) {
    sClock->SetEnabled(kTRUE);
    sPlot->SetEnabled(kTRUE);
    if ( sPlot->IsOn())
      fColorSel->SetEnabled(kTRUE);
    else
      fColorSel->SetEnabled(kFALSE);
  } else {
    sClock->SetEnabled(kFALSE);
    sPlot->SetEnabled(kFALSE);
    fColorSel->SetEnabled(kFALSE);
  }


}
void jctbSignal::ToggledClock(Bool_t b){
  Long_t mask=b<<id;
  ToggledSignalClock(mask);


}
void jctbSignal::ToggledPlot(Bool_t b){
  Long_t mask=b<<id;
  ToggledSignalPlot(mask);
    fColorSel->SetEnabled(b);

}


void jctbSignal::ToggledSignalOutput(Int_t b) {
  Emit("ToggledSignalOutput(Int_t)", id);

}
void jctbSignal::ToggledSignalClock(Int_t b){
  Emit("ToggledSignalClock(Int_t)", id);


}
void jctbSignal::ToggledSignalPlot(Int_t b){
  Emit("ToggledSignalPlot(Int_t)", id);

}
