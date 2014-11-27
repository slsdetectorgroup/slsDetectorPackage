
#include <TGButtonGroup.h>
#include <TGNumberEntry.h>
#include <TGLabel.h>
#include <TList.h>

#include <stdio.h>
#include <iostream>
#include <fstream>

#include "jctbDac.h"
#include "multiSlsDetector.h"
#include "sls_detector_defs.h"

using namespace std;




jctbDac::jctbDac(TGGroupFrame *page, int idac, multiSlsDetector *det) : TGHorizontalFrame(page, 800,50) , id(idac), myDet(det) {


    TGHorizontalFrame *hframe=this;

    page->AddFrame(hframe,new TGLayoutHints(kLHintsTop | kLHintsExpandX , 10,10,10,10));
    MapWindow();

    char tit[100];
    

   sprintf(tit, "DAC %d:",idac);

   dacsLabel= new TGLabel(hframe, tit);
   
   hframe->AddFrame(dacsLabel,new TGLayoutHints(kLHintsTop | kLHintsLeft| kLHintsExpandX, 5, 5, 5, 5));
   dacsLabel->MapWindow();
   dacsLabel->SetTextJustify(kTextLeft);

  
   dacsEntry = new TGNumberEntry(hframe, 0, 9,999, TGNumberFormat::kNESInteger,
                                               TGNumberFormat::kNEANonNegative, 
                                               TGNumberFormat::kNELLimitMinMax,
                                               0, 65535);

  hframe->AddFrame(dacsEntry,new TGLayoutHints(kLHintsTop |  kLHintsExpandX, 5, 5, 5, 5));
   dacsEntry->MapWindow();
   dacsEntry->Resize(150,30);


   dacsUnit= new TGCheckButton(hframe, "mV");
   hframe->AddFrame( dacsUnit,new TGLayoutHints(kLHintsTop | kLHintsLeft| kLHintsExpandX, 5, 5, 5, 5));
   dacsUnit->MapWindow();



   sprintf(tit, "xxx");
   dacsValue= new TGLabel(hframe, tit);
   hframe->AddFrame( dacsValue,new TGLayoutHints(kLHintsTop | kLHintsLeft| kLHintsExpandX, 5, 5, 5, 5));
   dacsValue->MapWindow();
   dacsValue->SetTextJustify(kTextLeft);

   TGTextEntry *e=dacsEntry->TGNumberEntry::GetNumberEntry();
   e->Connect("ReturnPressed()","jctbDac",this,"setValue()");
   //  cout << "(((((((((((((((((((((((((((((((" << dacsEntry->GetListOfSignals()->At(0)->IsA() << endl;
   

}



int jctbDac::setLabel(char *tit, int mv) {
  if(tit)
    dacsLabel->SetText(tit);
  if (mv>0)
    dacsUnit->SetOn(kTRUE,kTRUE);
  else if (mv==0)
    dacsUnit->SetOn(kFALSE,kTRUE);
  
      
  return id;

}

string jctbDac::getLabel() {

  ostringstream line;
  
  line << "DAC" << dec << id << " " <<  dacsUnit->IsOn() << endl;

  return line.str();

}


void jctbDac::setValue() {



  //  cout << "setting dac! "<< id << endl;

  myDet->setDAC(dacsEntry->GetIntNumber(), (slsDetectorDefs::dacIndex)id, dacsUnit->IsOn());

  getValue();

}

int jctbDac::getValue() {
  
  int val=myDet->setDAC(-1, (slsDetectorDefs::dacIndex)id, dacsUnit->IsOn());
  char s[100];
  cout << "dac " << id << " " << val << endl;
  sprintf(s,"%d",val);

  dacsValue->SetText(s);
  

  return val;

}



