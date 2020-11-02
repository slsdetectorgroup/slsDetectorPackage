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


#include "ctbSignals.h"
#include "ctbDefs.h"
#include "sls/Detector.h"

using namespace std;



//#define DEFAULTFN "run_0.encal"


ctbSignal::ctbSignal(TGFrame *page, int i,  sls::Detector *det)
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

 
   sOutput->Connect("Toggled(Bool_t)","ctbSignal",this,"ToggledOutput(Bool_t)");

   sDbitList= new TGCheckButton(hframe, "DB List");
   hframe->AddFrame( sDbitList,new TGLayoutHints(kLHintsTop | kLHintsLeft| kLHintsExpandX, 1, 1, 1, 1));
   sDbitList->MapWindow();

   sDbitList->Connect("Toggled(Bool_t)","ctbSignal",this,"ToggledDbitList(Bool_t)");


   sPlot= new TGCheckButton(hframe, "Plot");
   hframe->AddFrame( sPlot,new TGLayoutHints(kLHintsTop | kLHintsLeft| kLHintsExpandX, 1, 1, 1, 1));
   sPlot->MapWindow();

   sPlot->Connect("Toggled(Bool_t)","ctbSignal",this,"ToggledPlot(Bool_t)");
   
   fColorSel = new TGColorSelect(hframe, id+1, 0);
   fColorSel->Connect("ColorSelected(Pixel_t)","ctbSignal",this,"ColorChanged(Pixel_t)");
   hframe->AddFrame(fColorSel, new TGLayoutHints(kLHintsTop |
                          kLHintsLeft, 2, 0, 2, 2));
   

   fColorSel->SetColor(TColor::Number2Pixel(id+1));

   
   ToggledOutput(kFALSE);

  
   ToggledPlot(kFALSE);

  //  if (id==63) {
//      sOutput->SetOn(kTRUE);
//      sOutput->SetEnabled(kFALSE);
//    }
// #ifdef CTB
//    if (id==62) {
//      sOutput->SetOn(kTRUE);
//      sOutput->SetEnabled(kFALSE);
//    }

//    // if (id>=32 && id<48)
//    //   fixOutput(1);
//    // else if (id>=48 && id<64)
//    //   fixOutput(0);

// #endif
}
int ctbSignal::setSignalAlias(char *tit, int plot, int col) {

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

string ctbSignal::getSignalAlias() {
  

  ostringstream oss;
  oss << "BIT" << dec << id << " " << sLabel->GetText()->Data() << " " << sPlot->IsOn() << hex << " " << fColorSel->GetColor() << endl;
  return oss.str();




}
int ctbSignal::setOutput(Long64_t r) {


  //  cout << hex << r << dec <<endl;

  Long64_t mask=((Long64_t)1<<id);

  if (r&mask)
      sOutput->SetOn(kTRUE,kTRUE);
  else 
      sOutput->SetOn(kFALSE,kTRUE);
    
  return sOutput->IsOn();

}

int ctbSignal::fixOutput(int i) {

  if (i) {
    sPlot->SetOn(kFALSE);
    //sClock->SetOn(kFALSE,kTRUE);
    sOutput->SetOn(kTRUE);
    //   sPlot->SetEnabled(kFALSE);
    //  sClock->SetEnabled(kTRUE);
  } else {
    sOutput->SetOn(kFALSE,kTRUE);
    //  sClock->SetOn(kFALSE);  
    // sClock->SetEnabled(kFALSE);
    sPlot->SetEnabled(kTRUE);
  }
  sOutput->SetEnabled(kFALSE);
  return 0;

}

int ctbSignal::setDbitList(Long64_t r) {

  if (r)
      sDbitList->SetOn(kTRUE,kFALSE);
  else 
      sDbitList->SetOn(kFALSE,kFALSE);
    
  return sDbitList->IsOn();

}

int ctbSignal::isDbitList() { return sDbitList->IsOn();}
int ctbSignal::isOutput() { return sOutput->IsOn();}
int ctbSignal::isPlot() { return sPlot->IsOn();}
Pixel_t ctbSignal::getColor(){return fColorSel->GetColor();}

void ctbSignal::ToggledOutput(Bool_t b) {
  ToggledSignalOutput(id);
  if (b) {
    // sClock->SetEnabled(kTRUE);
    sPlot->SetOn(kFALSE);
    // sPlot->SetEnabled(kFALSE);
    fColorSel->SetEnabled(kFALSE);
  } else {
    // sClock->SetEnabled(kFALSE);
    // sClock->SetOn(kFALSE);
    sPlot->SetEnabled(kTRUE);
    if ( sPlot->IsOn())
      fColorSel->SetEnabled(kFALSE);
    else
      fColorSel->SetEnabled(kTRUE);
  }


}

void ctbSignal::ToggledDbitList(Bool_t b){
  Long_t mask=id;
  ToggledSignalDbitList(mask);
}


void ctbSignal::ToggledPlot(Bool_t b){
  Long_t mask=b<<id;
  ToggledSignalPlot(mask);
  fColorSel->SetEnabled(b);
}

void ctbSignal::ColorChanged(Pixel_t p){
  ToggledSignalPlot(id);
}


void ctbSignal::ToggledSignalOutput(Int_t b) {
  cout << "Toggle signal " << id << " " << b  << " " <<  sOutput->IsOn() <<endl;;
  Emit("ToggledSignalOutput(Int_t)", id);
}

void ctbSignal::ToggledSignalDbitList(Int_t b){
  cout << "Toggle dbitlist " << id << " " << b << endl;;
  Emit("ToggledSignalDbitList(Int_t)", id);
}

void ctbSignal::ToggledSignalPlot(Int_t b){
  Emit("ToggledSignalPlot(Int_t)", id);
}


ctbSignals::ctbSignals(TGVerticalFrame *page, sls::Detector *det)
  : TGGroupFrame(page,"IO Signals",kVerticalFrame), myDet(det) {


  SetTitlePos(TGGroupFrame::kLeft);
  page->AddFrame(this,new TGLayoutHints( kLHintsTop | kLHintsExpandX , 10,10,10,10));
  MapWindow();
  

  TGHorizontalFrame *hframe;
 
  TGHorizontalFrame* hhframe=new TGHorizontalFrame(this, 800,800);
  AddFrame(hhframe,new TGLayoutHints(kLHintsTop | kLHintsExpandX , 1,1,1,1));
  hhframe->MapWindow();

  TGVerticalFrame *vframe;




  int idac=0;
  for (idac=0; idac<NSIGNALS; idac++) {
    if (idac%((NSIGNALS+2)/2)==0) {
    vframe=new TGVerticalFrame(hhframe, 400,800);
     hhframe->AddFrame(vframe,new TGLayoutHints(kLHintsTop | kLHintsExpandX , 1,1,1,1));
    vframe->MapWindow();
    }


    signals[idac]=new ctbSignal(vframe,idac,myDet);
    signals[idac]->Connect("ToggledSignalOutput(Int_t)","ctbSignals",this,"ToggledOutReg(Int_t)");
    signals[idac]->Connect("ToggledSignalDbitList(Int_t)","ctbSignals",this,"ToggledDbitList(Int_t)");
    signals[idac]->Connect("ToggledSignalPlot(Int_t)","ctbSignals",this,"ToggledPlot(Int_t)");
 
    vframe->AddFrame(signals[idac],new TGLayoutHints(kLHintsTop | kLHintsExpandX , 1,1,1,1));
    signals[idac]->MapWindow();


  }

  hframe=new TGHorizontalFrame(vframe, 800,50);
  vframe->AddFrame(hframe,new TGLayoutHints(kLHintsTop | kLHintsExpandX , 1,1,1,1));
  hframe->MapWindow();


  TGLabel *label= new TGLabel(hframe, "IO Control Register: ");
   hframe->AddFrame(label,new TGLayoutHints(kLHintsTop | kLHintsLeft| kLHintsExpandX, 1, 1, 1, 1));
   label->MapWindow();
   label->SetTextJustify(kTextLeft);


   eIOCntrlRegister = new TGNumberEntry(hframe, 0, 16,999, TGNumberFormat::kNESHex,
                                               TGNumberFormat::kNEANonNegative, 
                                               TGNumberFormat::kNELNoLimits);

  hframe->AddFrame(eIOCntrlRegister,new TGLayoutHints(kLHintsTop |  kLHintsExpandX, 1, 1, 1, 1));
   eIOCntrlRegister->MapWindow();
   eIOCntrlRegister->Resize(150,30);




    hframe=new TGHorizontalFrame(vframe, 800,50);
    vframe->AddFrame(hframe,new TGLayoutHints(kLHintsTop | kLHintsExpandX , 1,1,1,1));
    hframe->MapWindow();
   

   label= new TGLabel(hframe, "DBit Offset: ");
   hframe->AddFrame(label,new TGLayoutHints(kLHintsTop | kLHintsLeft| kLHintsExpandX, 1, 1, 1, 1));
   label->MapWindow();
   label->SetTextJustify(kTextLeft);


   eDbitOffset = new TGNumberEntry(hframe, 0, 9,999, TGNumberFormat::kNESInteger,
                                               TGNumberFormat::kNEANonNegative, 
                                               TGNumberFormat::kNELNoLimits);

  hframe->AddFrame(eDbitOffset,new TGLayoutHints(kLHintsTop |  kLHintsExpandX, 1, 1, 1, 1));
   eDbitOffset->MapWindow();
   eDbitOffset->Resize(150,30);
   
 
  TGTextEntry *e= eDbitOffset->TGNumberEntry::GetNumberEntry();
    e->Connect("ReturnPressed()","ctbSignals",this,"setDbitOffset()");

    e->Connect("ValueSet(Long_t)","ctbSignals",this,"setDbitOffset(Long_t)");
}




int ctbSignals::setSignalAlias(string line) {

  int is=-1, plot=0, col=-1;
  char tit[100];
  int narg=sscanf(line.c_str(),"BIT%d %s %d %d",&is,tit,&plot,&col);
  if (narg<2)
    return -1;
  if (is>=0 && is<NIOSIGNALS) {
    signals[is]->setSignalAlias(tit,plot,col);
  }
  return is;

}

string ctbSignals::getSignalAlias() {

  ostringstream oss;
  for (int is=0; is<NIOSIGNALS; is++)
    oss << signals[is]->getSignalAlias() << endl;


    return oss.str();

}


void ctbSignals::update() {
  try {

    Long64_t oreg = static_cast<Long64_t>(myDet->getPatternIOControl().tsquash("Different values"));
    cout << hex << oreg << dec << endl;

    for (int idac=0; idac<NIOSIGNALS; idac++) {
      signals[idac]->setOutput(oreg);
    }

  } CATCH_DISPLAY ("Could not get patternIOcontrol.", "ctbSignals::update")

  if (myDet->getDetectorType().squash() == slsDetectorDefs::MOENCH) {
    // enable all
    for (int is=0; is<64; is++) {
      signals[is]->setDbitList(1);
    } 
    eDbitOffset->SetNumber(0);
  } 
  
  // ctb
  else {
    try {

      auto dbitlist = myDet->getRxDbitList().tsquash("Different values");
      // enable all
      if (dbitlist.empty()) {
        for (int is=0; is<64; is++) {
          signals[is]->setDbitList(1);
        } 
      }
      else {
        // disable all
        for (int is=0; is<64; is++) {
          signals[is]->setDbitList(0);
        }
        // enable selected
        for (const auto &value : dbitlist)  {
          signals[value]->setDbitList(1);
        }
      }

    } CATCH_DISPLAY ("Could not get receiver dbit list.", "ctbSignals::update")

    try {
      auto val = myDet->getRxDbitOffset().tsquash("Different values");
      eDbitOffset->SetNumber(val);
    } CATCH_DISPLAY ("Could not get receiver dbit offset.", "ctbSignals::update")
  }
}


string ctbSignals::getSignalParameters() {

  try {

    auto val = myDet->getPatternIOControl().tsquash("Different values");
    ostringstream line;
    line <<  "patioctrl " << hex << val << dec << endl;
    return line.str();

  } CATCH_DISPLAY ("Could not get patternIOcontrol.", "ctbSignals::getSignalParameters")

  return ("");
}

void ctbSignals::ToggledOutReg(Int_t mask) {
  try {

    Long64_t oreg = static_cast<Long64_t>(myDet->getPatternIOControl().tsquash("Different values"));
    Long64_t m=((Long64_t)1)<<mask;
    cout << dec << sizeof(Long64_t) << " " << mask << " " << hex << m << " ioreg " << oreg;

    if (signals[mask]->isOutput()) {
      cout <<  " or " << m ;
      oreg|=m;
    } else {
      cout <<  " not " << ~m ;
      oreg&=~m;
    }
    cout <<  " after " << oreg << endl;

    myDet->setPatternIOControl(static_cast<uint64_t>(oreg));
    oreg = static_cast<Long64_t>(myDet->getPatternIOControl().tsquash("Different values"));
    cout << dec << sizeof(Long64_t) << " " << mask << " " << hex << m << " ioreg " << oreg << endl;

    eIOCntrlRegister->SetText(to_string(oreg).c_str());

  } CATCH_DISPLAY ("Could not get/set patternIOcontrol.", "ctbSignals::ToggledOutReg")

}



void ctbSignals::ToggledDbitList(Int_t mask){
  try {

    auto dbitlist = myDet->getRxDbitList().tsquash("Different values");

    // anyway all enabled
    if ((dbitlist.empty()) && (signals[mask]->isDbitList())) {
      ;
    } 
    // set the dbitlist
    else  {
      std::vector <int> new_dbitlist;
      for (int is=0; is<64; is++) {
        if (signals[is]->isDbitList()){ 
          new_dbitlist.push_back(is);
          cout << is << " " << new_dbitlist.size() - 1 << endl;
        }
      }
      if (new_dbitlist.size() > 64) 
        new_dbitlist.clear();
      myDet->setRxDbitList(new_dbitlist);
      // get list again
      dbitlist = myDet->getRxDbitList().tsquash("Different values");
    }
    
    // enable all
    if (dbitlist.empty()) {
      for (int is=0; is<64; is++) {
        signals[is]->setDbitList(1);
      } 
    }
    else {
      // disable all
      for (int is=0; is<64; is++) {
        signals[is]->setDbitList(0);
      }
      // enable selected
      for (const auto &value : dbitlist)  {
        signals[value]->setDbitList(1);
      }
    }

  } CATCH_DISPLAY ("Could not get/set receiver dbit list.", "ctbSignals::ToggledDbitList")
}




void ctbSignals::ToggledPlot(Int_t b) {
  
  Emit("ToggledSignalPlot(Int_t)", b);

}


void ctbSignals::ToggledSignalPlot(Int_t b) {
  
  Emit("ToggledSignalPlot(Int_t)", b);

}


Pixel_t ctbSignals::getColor(int i){
  if (i>=0 && i<NSIGNALS) 
    return signals[i]->getColor();
  return static_cast<Pixel_t>(-1);
}

int ctbSignals::getPlot(int i){
  if (i>=0 && i<NSIGNALS) 
    return signals[i]->isPlot();
  return -1;
};

void ctbSignals::setDbitOffset(Long_t) {
  setDbitOffset();
}

void ctbSignals::setDbitOffset(){
  try {
    myDet->setRxDbitOffset(eDbitOffset->GetNumber());
  } CATCH_DISPLAY ("Could not set receiver dbit offset.", "ctbSignals::setDbitOffset")
}
