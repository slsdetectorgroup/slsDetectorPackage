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

#include "jctbSignals.h"
#include "jctbSignal.h"
#include "multiSlsDetector.h"

using namespace std;



#define DEFAULTFN "run_0.encal"


jctbSignals::jctbSignals(TGVerticalFrame *page, multiSlsDetector *det)
  : TGGroupFrame(page,"IO Signals",kVerticalFrame), myDet(det) {


  SetTitlePos(TGGroupFrame::kLeft);
  page->AddFrame(this,new TGLayoutHints( kLHintsTop | kLHintsExpandX , 10,10,10,10));
  MapWindow();
  
  cout << "window mapped " << endl;

  TGHorizontalFrame *hframe;
  char tit[100];
 
  TGHorizontalFrame* hhframe=new TGHorizontalFrame(this, 800,800);
  AddFrame(hhframe,new TGLayoutHints(kLHintsTop | kLHintsExpandX , 1,1,1,1));
  hhframe->MapWindow();

  TGVerticalFrame *vframe;




  int idac=0;
  for (idac=0; idac<NIOSIGNALS; idac++) {
    if (idac%27==0) {


    vframe=new TGVerticalFrame(hhframe, 400,800);
     hhframe->AddFrame(vframe,new TGLayoutHints(kLHintsTop | kLHintsExpandX , 1,1,1,1));
    vframe->MapWindow();
   
  

    }


    signals[idac]=new jctbSignal(vframe,idac,myDet);
    signals[idac]->Connect(" ToggledSignalOutput(Int_t)","jctbSignals",this,"ToggledOutReg(Int_t)");
    signals[idac]->Connect(" ToggledSignalClock(Int_t)","jctbSignals",this,"ToggledClockReg(Int_t)");
    signals[idac]->Connect(" ToggledSignalPlot(Int_t)","jctbSignals",this,"ToggledPlot(Int_t)");
 
    vframe->AddFrame(signals[idac],new TGLayoutHints(kLHintsTop | kLHintsExpandX , 1,1,1,1));
    signals[idac]->MapWindow();

  }
  idac=63;
  signals[idac]=new jctbSignal(vframe,idac,myDet);
  vframe->AddFrame(signals[idac],new TGLayoutHints(kLHintsTop | kLHintsExpandX , 1,1,1,1));
  signals[idac]->MapWindow();
  sprintf(tit,"ADC Latch");

  signals[idac]->setSignalAlias(tit,-1,-1);

    signals[idac]->Connect(" ToggledSignalOutput(Int_t)","jctbSignals",this,"ToggledOutReg(Int_t)");
    signals[idac]->Connect(" ToggledSignalClock(Int_t)","jctbSignals",this,"ToggledClockReg(Int_t)");
    signals[idac]->Connect(" ToggledSignalPlot(Int_t)","jctbSignals",this,"ToggledPlot(Int_t)");
 


  hframe=new TGHorizontalFrame(vframe, 800,50);
  vframe->AddFrame(hframe,new TGLayoutHints(kLHintsTop | kLHintsExpandX , 1,1,1,1));
  hframe->MapWindow();


  TGLabel *label= new TGLabel(hframe, "IO Control Register");
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
   

   label= new TGLabel(hframe, "Clock Control Register");
   hframe->AddFrame(label,new TGLayoutHints(kLHintsTop | kLHintsLeft| kLHintsExpandX, 1, 1, 1, 1));
   label->MapWindow();
   label->SetTextJustify(kTextLeft);


   eClkCntrlRegister = new TGNumberEntry(hframe, 0, 16,999, TGNumberFormat::kNESHex,
                                               TGNumberFormat::kNEANonNegative, 
                                               TGNumberFormat::kNELNoLimits);

  hframe->AddFrame(eClkCntrlRegister,new TGLayoutHints(kLHintsTop |  kLHintsExpandX, 1, 1, 1, 1));
   eClkCntrlRegister->MapWindow();
   eClkCntrlRegister->Resize(150,30);
   
   eIOCntrlRegister->SetState(kFALSE);
   eClkCntrlRegister->SetState(kFALSE);

}




int jctbSignals::setSignalAlias(string line) {

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

string jctbSignals::getSignalAlias() {

  ostringstream oss;
  for (int is=0; is<NIOSIGNALS; is++)
    oss << signals[is]->getSignalAlias() << endl;


    return oss.str();

}


void jctbSignals::update() {
  Long64_t oreg=myDet->setCTBWord(-1,-1);
  Long64_t creg=myDet->setCTBWord(-2,-1);


  char val[1000];
  cout << hex << oreg << dec << endl;
  cout << hex << creg << dec << endl;

  sprintf(val,"%llX",oreg);
  //  eIOCntrlRegister->SetHexNumber(oreg);
  eIOCntrlRegister->SetText(val);
  sprintf(val,"%llX",creg);
  //  eClkCntrlRegister->SetHexNumber(creg);
  eClkCntrlRegister->SetText(val);

  for (int idac=0; idac<NIOSIGNALS; idac++) {
    signals[idac]->setOutput(oreg);
    signals[idac]->setClock(creg);
  }



}

void jctbSignals::ToggledOutReg(Int_t mask) {
  
  char val[1000];
  Long64_t oreg=myDet->setCTBWord(-1,-1);
  Long64_t m=((Long64_t)1)<<mask;
  

  if (signals[mask]->isOutput())
    oreg|=m;
  else
    oreg&=~m;
  
  cout << dec << sizeof(Long64_t) << " " << mask << " " << hex << m << " ioreg " << oreg << endl;

  myDet->setCTBWord(-1,oreg);
  oreg=myDet->setCTBWord(-1,-1);

  cout << dec << sizeof(Long64_t) << " " << mask << " " << hex << m << " ioreg " << oreg << endl;

  sprintf(val,"%llX",oreg);
  //  eIOCntrlRegister->SetHexNumber(oreg);
  eIOCntrlRegister->SetText(val);
  //  eIOCntrlRegister->SetNumber(oreg);

}



void jctbSignals::ToggledClockReg(Int_t mask){
  
  char val[1000];
  Long64_t oreg=myDet->setCTBWord(-2,-1);
  Long64_t m=((Long64_t)1)<<mask;

  

  if (signals[mask]->isClock())
    oreg|=m;
  else
    oreg&=~m;
  
  cout << hex << "clkreg " << oreg << endl;

  myDet->setCTBWord(-2,oreg);
  oreg=myDet->setCTBWord(-2,-1);
  sprintf(val,"%llX",oreg);
  //  eIOCntrlRegister->SetHexNumber(oreg);
 
  eClkCntrlRegister->SetText(val);

}




void jctbSignals::ToggledPlot(Int_t mask) {
  
  if (signals[mask]->isPlot())
    cout << "plot signal " << mask << endl;
  else
    cout << "unplot signal " << mask << endl;

}
