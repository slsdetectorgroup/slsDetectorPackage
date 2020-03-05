
#include <stdio.h>
#include <iostream>
#include <fstream>

#include <TGTextEntry.h>
#include <TGLabel.h>
#include <TGNumberEntry.h>
#include <TGButton.h>

#include "ctbDacs.h"
#include "ctbDefs.h"
#include "Detector.h"
#include "sls_detector_defs.h"

using namespace std;


ctbDac::ctbDac(TGGroupFrame *page, int idac, sls::Detector *det) : TGHorizontalFrame(page, 800,50) , id(idac), myDet(det) {

    TGHorizontalFrame *hframe=this;

    page->AddFrame(hframe,new TGLayoutHints(kLHintsTop | kLHintsExpandX , 1,1,1,1));
    MapWindow();

    char tit[100];
    

   sprintf(tit, "DAC %d:",idac);

   dacsLabel= new TGCheckButton(hframe, tit);// new TGLabel(hframe, tit);
   dacsLabel->SetOn(kTRUE, kTRUE);
   
   dacsLabel->Connect("Toggled(Bool_t)","ctbDac",this,"setOn(Bool_t)");


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
   // if (idac!=slsDetectorDefs::ADC_VPP) {
     hframe->AddFrame( dacsUnit,new TGLayoutHints(kLHintsTop | kLHintsLeft| kLHintsExpandX, 5, 5, 5, 5));
     dacsUnit->MapWindow();
     if (idac==slsDetectorDefs::ADC_VPP) {
       dacsUnit->SetEnabled(kFALSE);
       hframe->HideFrame(dacsUnit);
       dacsUnit->MapWindow();
       cout << "hiding!" << endl;
     }
     if (idac==slsDetectorDefs::HIGH_VOLTAGE) {
       dacsUnit->SetText("V"); 
       dacsUnit->SetOn(kTRUE,kTRUE);
       dacsUnit->SetEnabled(kFALSE);
     }
     //}



   sprintf(tit, "xxx");
   dacsValue= new TGLabel(hframe, tit);
   hframe->AddFrame( dacsValue,new TGLayoutHints(kLHintsTop | kLHintsLeft| kLHintsExpandX, 5, 5, 5, 5));
   dacsValue->MapWindow();
   dacsValue->SetTextJustify(kTextLeft);

   TGTextEntry *e=dacsEntry->TGNumberEntry::GetNumberEntry();
   e->Connect("ReturnPressed()","ctbDac",this,"setValue()");
   // e->Connect("ValueSet(Long_t)","ctbDac",this,"setValue(Long_t)");
   dacsEntry->Connect("ValueSet(Long_t)","ctbDac",this,"setValue(Long_t)");
   //  cout << "(((((((((((((((((((((((((((((((" << dacsEntry->GetListOfSignals()->At(0)->IsA() << endl;
   

}



int ctbDac::setLabel(char *tit, int mv) {
  if(tit)
    dacsLabel->SetText(tit);
  if (mv==1)
    dacsUnit->SetOn(kTRUE,kTRUE);
  else if (mv==0)
    dacsUnit->SetOn(kFALSE,kTRUE);
  // else if (mv==2) {
  //   ;}
  // else if  (mv==3)
  //   ; 
  return id;

}

string ctbDac::getLabel() {
  ostringstream line;
  line << dacsLabel->GetText() << " " <<  dacsUnit->IsOn() << endl;
   //  line << "DAC" << dec << id << " " <<  dacsUnit->IsOn() << endl;
  return line.str();

}

int ctbDac::getMoenchDacId() {
  slsDetectorDefs::dacIndex moenchDacIndices[8] = {slsDetectorDefs::VBP_COLBUF, slsDetectorDefs::VIPRE, slsDetectorDefs::VIN_CM, slsDetectorDefs::VB_SDA, slsDetectorDefs::VCASC_SFP, slsDetectorDefs::VOUT_CM, slsDetectorDefs::VIPRE_CDS, slsDetectorDefs::IBIAS_SFP}; 

  if (id >= 8) {
    return id;
  }
  return static_cast<int>(moenchDacIndices[id]);
}

void ctbDac::setValue(Long_t a) {setValue();}

void ctbDac::setValue() {
  cout << "setting dac! "<< id << " value " << dacsEntry->GetIntNumber() << " units " << dacsUnit->IsOn() << endl;

  try {
    int sid = id;
    if (myDet->getDetectorType().squash() == slsDetectorDefs::MOENCH) {
      sid = getMoenchDacId();
    }
    myDet->setDAC(static_cast<slsDetectorDefs::dacIndex>(sid), dacsEntry->GetIntNumber(), dacsUnit->IsOn()); 
  } CATCH_DISPLAY ("Could not set dac " + to_string(id) + ".", "ctbDac::setValue")

  getValue();
}

void ctbDac::setOn(Bool_t b) {
  //  cout << "setting dac! "<< id << endl;
  if ( dacsLabel->IsOn()) {
    setValue();
  } else {
    try {
      int sid = id;
      if (myDet->getDetectorType().squash() == slsDetectorDefs::MOENCH) {
	sid = getMoenchDacId();
      }
      myDet->setDAC(static_cast<slsDetectorDefs::dacIndex>(sid), -100, false);
    } CATCH_DISPLAY ("Could not power off dac " + to_string(id) + ".", "ctbDac::setOn")
  }
  getValue();
}

int ctbDac::getValue() { 
  try {
    int sid = id;
    if (myDet->getDetectorType().squash() == slsDetectorDefs::MOENCH) {
      sid = getMoenchDacId();
    }
    int val = myDet->getDAC(static_cast<slsDetectorDefs::dacIndex>(sid), dacsUnit->IsOn()).tsquash("Different values"); 
    cout << "dac " << id << " " << val << endl;
    dacsValue->SetText(to_string(val).c_str());
    if (val >= 0) {
      dacsLabel->SetOn(kTRUE);
    } else {
      dacsLabel->SetOn(kFALSE);
    }
    return val;
  } CATCH_DISPLAY ("Could not get dac " + to_string(id) + ".", "ctbDac::getValue")
  
  return -1;
}


ctbDacs::ctbDacs(TGVerticalFrame *page, sls::Detector *det)   : TGGroupFrame(page,"DACs",kVerticalFrame) , myDet(det){

  SetTitlePos(TGGroupFrame::kLeft);
  page->AddFrame(this,new TGLayoutHints( kLHintsTop | kLHintsExpandX , 10,10,10,10));
  MapWindow();
  
  // cout << "window mapped " << endl;

  for (int idac=0; idac<NDACS; idac++) {

     dacs[idac]=new ctbDac(this, idac, myDet);

  }
  dacs[NDACS]=new ctbDac(this, slsDetectorDefs::ADC_VPP, myDet);
  dacs[NDACS+1]=new ctbDac(this, slsDetectorDefs::HIGH_VOLTAGE, myDet);
  dacs[NDACS]->setLabel((char*)"ADC Vpp",2); 
  dacs[NDACS+1]->setLabel((char*)"High Voltage",3); 
}


int ctbDacs::setDacAlias(string line) {
  int is=-1, mv=0;
  char tit[100];
  int narg=sscanf(line.c_str(),"DAC%d %s %d",&is,tit,&mv);
  if (narg<2)
    return -1;
  if (is>=0 && is<NDACS) 
    dacs[is]->setLabel(tit,mv);
  return is;

}

string ctbDacs::getDacAlias() {
  ostringstream line;

  for (int i=0; i<NDACS; i++)
    line << dacs[i]->getLabel() << endl;
  return line.str();
}





string ctbDacs::getDacParameters() {
  ostringstream line;

  for (int i=0; i<NDACS; i++) {
    //line << "dacs:" << i << " " << dacs[i]->getValue << endl;
    line << "dac:" << i << " " << dacs[i]->getValue() << endl;
  } 
  return line.str();
}



void  ctbDacs::update() {
  for (int idac=0; idac<NDACS+1; idac++) {
    dacs[idac]->getValue();
  }
}
