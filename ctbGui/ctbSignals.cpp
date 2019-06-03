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
#include "multiSlsDetector.h"

using namespace std;



//#define DEFAULTFN "run_0.encal"


ctbSignal::ctbSignal(TGFrame *page, int i,  multiSlsDetector *det)
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

  Long64_t mask=((Long64_t)1<<id);

  // cout << hex << r << dec <<endl;

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
  Long_t mask=b<<id;
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


ctbSignals::ctbSignals(TGVerticalFrame *page, multiSlsDetector *det)
  : TGGroupFrame(page,"IO Signals",kVerticalFrame), myDet(det) {


  SetTitlePos(TGGroupFrame::kLeft);
  page->AddFrame(this,new TGLayoutHints( kLHintsTop | kLHintsExpandX , 10,10,10,10));
  MapWindow();
  

  TGHorizontalFrame *hframe;
  char tit[100];
 
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

// #ifdef CTB
//   idac=62;
//   signals[idac]=new ctbSignal(vframe,idac,myDet);
//   vframe->AddFrame(signals[idac],new TGLayoutHints(kLHintsTop | kLHintsExpandX , 1,1,1,1));
//   signals[idac]->MapWindow();
//   sprintf(tit,"DBIT Latch");

//   signals[idac]->setSignalAlias(tit,-1,-1);

//   signals[idac]->Connect("ToggledSignalOutput(Int_t)","ctbSignals",this,"ToggledOutReg(Int_t)");
//   signals[idac]->Connect("ToggledSignalDbitList(Int_t)","ctbSignals",this,"ToggledDbitList(Int_t)");
//   signals[idac]->Connect("ToggledSignalPlot(Int_t)","ctbSignals",this,"ToggledPlot(Int_t)");


// #endif


  // idac=63;
  // signals[idac]=new ctbSignal(vframe,idac,myDet);
  // vframe->AddFrame(signals[idac],new TGLayoutHints(kLHintsTop | kLHintsExpandX , 1,1,1,1));
  // signals[idac]->MapWindow();
  // sprintf(tit,"ADC Latch");

  // signals[idac]->setSignalAlias(tit,-1,-1);

  // signals[idac]->Connect("ToggledSignalOutput(Int_t)","ctbSignals",this,"ToggledOutReg(Int_t)");
  // signals[idac]->Connect("ToggledSignalDbitList(Int_t)","ctbSignals",this,"ToggledDbitList(Int_t)");
  // signals[idac]->Connect("ToggledSignalPlot(Int_t)","ctbSignals",this,"ToggledPlot(Int_t)");


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
  Long64_t oreg=myDet->setPatternIOControl();//setCTBWord(-1,-1);
  // Long64_t creg=myDet->setPatternClockControl();//setCTBWord(-2,-1);


  char val[1000];
  cout << hex << oreg << dec << endl;
  //  cout << hex << creg << dec << endl;

  sprintf(val,"%llX",oreg);
  //  eIOCntrlRegister->SetHexNumber(oreg);
 

  for (int idac=0; idac<NIOSIGNALS; idac++) {
    signals[idac]->setOutput(oreg);
   
  }

  Long64_t mask;
  std::vector <int> dbitlist=myDet->getReceiverDbitList();
  if (dbitlist.empty())
    for (int is=0; is<64; is++) {
      signals[is]->setDbitList(1);
    } 
  else {
    for (int is=0; is<64; is++) signals[is]->setDbitList(0);
    for (const auto &value : dbitlist)  {
      signals[value]->setDbitList(1);
    }
  }


  eDbitOffset->SetNumber(myDet->getReceiverDbitOffset());

}


string ctbSignals::getSignalParameters() {


  ostringstream line;
  line <<  "patioctrl " << hex << myDet->setPatternIOControl() << dec << endl;//setCTBWord(-1,-1)
  return line.str();


}

void ctbSignals::ToggledOutReg(Int_t mask) {
  
  char val[1000];
  Long64_t oreg=myDet->setPatternIOControl();//setCTBWord(-1,-1);
  Long64_t m=((Long64_t)1)<<mask;
  

  cout << dec << sizeof(Long64_t) << " " << mask << " " << hex << m << " ioreg " << oreg;

 

  if (signals[mask]->isOutput()) {
    cout <<  " or " << m ;
    oreg|=m;
  }  else {
    cout <<  " not " << ~m ;
    oreg&=~m;
  }
  cout <<  " after " << oreg << endl;

  myDet->setPatternIOControl(oreg);//setCTBWord(-1,oreg);
  oreg=myDet->setPatternIOControl();//myDet->setCTBWord(-1,-1);

  cout << dec << sizeof(Long64_t) << " " << mask << " " << hex << m << " ioreg " << oreg << endl;

  sprintf(val,"%llX",oreg);
  //  eIOCntrlRegister->SetHexNumber(oreg);
  eIOCntrlRegister->SetText(val);
  //  eIOCntrlRegister->SetNumber(oreg);

}



void ctbSignals::ToggledDbitList(Int_t mask){
  


  cout << "************* Here" << endl;



  std::vector <int> new_dbitlist;
  std::vector <int> old_dbitlist=myDet->getReceiverDbitList();

  char val[1000];
  Long64_t m=((Long64_t)1)<<mask;

  if (old_dbitlist.empty() &&  signals[mask]->isDbitList()) 
    ;
  else {
    int ns=0;
    for (int is=0; is<64; is++) {
      if (signals[is]->isDbitList()){ 
	new_dbitlist.push_back(is);
	ns++;
	cout << is << " " << ns << endl;
      }
    }
    if (ns>63) new_dbitlist.clear();
    myDet->setReceiverDbitList(new_dbitlist);
  }
  std::vector <int> dbitlist=myDet->getReceiverDbitList();
  if (dbitlist.empty())
    for (int is=0; is<64; is++) signals[is]->setDbitList(1);
  else
    for (int is=0; is<64; is++) signals[is]->setDbitList(0);
    for (const auto &value : dbitlist)  signals[value]->setDbitList(1);

}




void ctbSignals::ToggledPlot(Int_t b) {
  
  Emit("ToggledSignalPlot(Int_t)", b);

}


void ctbSignals::ToggledSignalPlot(Int_t b) {
  
  Emit("ToggledSignalPlot(Int_t)", b);

}


Pixel_t ctbSignals::getColor(int i){
  if (i>=0 && i<NSIGNALS) return signals[i]->getColor();
}

int ctbSignals::getPlot(int i){
  if (i>=0 && i<NSIGNALS) return signals[i]->isPlot();
};

void ctbSignals::setDbitOffset(Long_t) {
  setDbitOffset();
}
void ctbSignals::setDbitOffset(){
  myDet->setReceiverDbitOffset(eDbitOffset->GetNumber());
}
