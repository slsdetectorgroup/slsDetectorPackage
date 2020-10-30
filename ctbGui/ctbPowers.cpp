#include  <TGFrame.h>


#include <TGButtonGroup.h>
#include <TGNumberEntry.h>
#include <TGLabel.h>
#include <TList.h>

#include <stdio.h>
#include <iostream>
#include <fstream>

#include "ctbDefs.h"
#include "ctbDacs.h"
#include "ctbPowers.h"
#include "sls/Detector.h"
#include "sls_detector_defs.h"

using namespace std;



ctbPower::ctbPower(TGGroupFrame* f, int i,  sls::Detector* d)
  : ctbDac(f, i, d) 
{
  cout << "****************************************************************power " << i << endl;
     dacsUnit->SetOn(kTRUE); 
     dacsUnit->SetEnabled(kFALSE);

     switch(i) {
     case slsDetectorDefs::V_POWER_IO: 
     	 dacsLabel->SetText("VIO"); 
     	 break; 
     case slsDetectorDefs::V_POWER_A: 
     	 dacsLabel->SetText("VA"); 
     	 break;  
     case slsDetectorDefs::V_POWER_B: 
     	 dacsLabel->SetText("VB"); 
     	 break;   
     case slsDetectorDefs::V_POWER_C: 
     	 dacsLabel->SetText("VC");
     	 break;    
     case slsDetectorDefs::V_POWER_D: 
     	 dacsLabel->SetText("VD");
     	 break;      
     case slsDetectorDefs::V_POWER_CHIP: 
     	 dacsLabel->SetText("VCHIP"); 
     	 dacsLabel->SetEnabled(kFALSE);
     	 break;    
     default: 
     	 dacsLabel->SetText("Bad index"); 
     	 break; 
     };
     

     TGTextEntry *e=dacsEntry->TGNumberEntry::GetNumberEntry();
     e->Disconnect 	("ReturnPressed()");
     e->Disconnect 	("ValueSet(Long_t)");
     
     e->Connect("ReturnPressed()","ctbPower",this,"setValue()");
     dacsEntry->Connect("ValueSet(Long_t)","ctbPower",this,"setValue(Long_t)");
 };


string ctbPower::getLabel() {

  ostringstream line;
  switch (id) {
     case slsDetectorDefs::V_POWER_IO: 
	 line << "VIO"; 
	 break; 
     case slsDetectorDefs::V_POWER_A: 
	 line << "VA"; 
	 break;  
     case slsDetectorDefs::V_POWER_B: 
	 line << "VB"; 
	 break;   
     case slsDetectorDefs::V_POWER_C: 
	 line << "VC"; 
	 break;    
     case slsDetectorDefs::V_POWER_D: 
	 line << "VD"; 
	 break;      
     case slsDetectorDefs::V_POWER_CHIP: 
	 line << "VCHIP"; 
	 break;    
     default:  
	 line << "VBAD"; 
	 break; 

  }
  line << " " <<  dacsLabel->GetText() << endl;
  return line.str();
}

void ctbPower::setValue(Long_t a) {ctbPower::setValue();}

void ctbPower::setValue() {
  cout << "***************************Setting power " << dacsEntry->GetIntNumber() << " " << id << " " << 1 << endl;
 
  try {
    myDet->setVoltage(static_cast<slsDetectorDefs::dacIndex>(id), dacsEntry->GetIntNumber());
  } CATCH_DISPLAY ("Could not set power " + to_string(id) + ".", "ctbPower::setValue")

  getValue();
}


int ctbPower::getValue() {
  try {

  int val = myDet->getVoltage(static_cast<slsDetectorDefs::dacIndex>(id)).tsquash("Different values");
  cout << "****************************Getting power " << val << " " << id << " " << 1 << endl;

  dacsValue->SetText(to_string(val).c_str());
  if (val > 0) {
    if (id != static_cast<int>(slsDetectorDefs::V_POWER_CHIP))
      dacsLabel->SetOn(kTRUE);
  } else {
    dacsLabel->SetOn(kFALSE);
  }

  return val;

  } CATCH_DISPLAY ("Could not get power " + to_string(id) + ".", "ctbPower::getValue") 

  return -1;
}



ctbPowers::ctbPowers(TGVerticalFrame* page, sls::Detector* det)   : TGGroupFrame(page,"Power Supplies",kVerticalFrame) , myDet(det){


  SetTitlePos(TGGroupFrame::kLeft);
  page->AddFrame(this,new TGLayoutHints( kLHintsTop | kLHintsExpandX , 10,10,10,10));
  MapWindow();
  
  // cout << "window mapped " << endl;

  for (int idac=0; idac<NPOWERS; idac++) {
     dacs[idac]=new ctbPower(this, slsDetectorDefs::V_POWER_A+idac, myDet);

  }
}


int ctbPowers::setPwrAlias(string line) {

  int is=-1;
  char tit[100];

  if (sscanf(line.c_str(),"VA %s",tit)) {
    dacs[0]->setLabel(tit,1);
    is=0;
  }

  if (sscanf(line.c_str(),"VB %s",tit)) {
    dacs[1]->setLabel(tit,1);
    is=1;
  }

  if (sscanf(line.c_str(),"VC %s",tit)) {
    dacs[2]->setLabel(tit,1);
    is=2;
  }

  if (sscanf(line.c_str(),"VD %s",tit)) {
    dacs[3]->setLabel(tit,1);
    is=3;
  }

  if (sscanf(line.c_str(),"VIO %s",tit)) {
    dacs[4]->setLabel(tit,1);
    is=4;
  }

  if (sscanf(line.c_str(),"VCHIP %s",tit)) {
    dacs[5]->setLabel(tit,1);
    is=5;
  }
    
  return is;

}

string ctbPowers::getPwrAlias() {

  ostringstream line;

  for (int i=0; i<NPOWERS; i++)
    line << dacs[i]->getLabel() << endl;
  return line.str();

}




string ctbPowers::getPwrParameters() {

  ostringstream line;
  line << "v_a" << " " << dacs[0]->getValue() << " mv" << endl;
  line << "v_b" << " " << dacs[1]->getValue() << " mv" << endl;
  line << "v_c" << " " << dacs[2]->getValue() << " mv" << endl;
  line << "v_d" << " " << dacs[3]->getValue() << " mv" << endl;
  line << "v_io" << " " << dacs[4]->getValue() << " mv" << endl;
  line << "v_chip" << " " << dacs[5]->getValue() << " mv" << endl;
  // for (int i=0; i<POWERS; i++) {
  //   //line << "dacs:" << i << " " << dacs[i]->getValue << endl;
  //   line << "dac:" << i << " " << dacs[i]->getValue() << endl;
  // } 
  return line.str();
}



void  ctbPowers::update() {
  for (int idac=0; idac<NPOWERS; idac++) {
    dacs[idac]->getValue();

  }
}
