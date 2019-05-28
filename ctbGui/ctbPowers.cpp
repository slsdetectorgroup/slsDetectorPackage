#include  <TGFrame.h>


#include <TGButtonGroup.h>
#include <TGNumberEntry.h>
#include <TGLabel.h>
#include <TList.h>

#include <stdio.h>
#include <iostream>
#include <fstream>

#include "ctbDacs.h"
#include "ctbPowers.h"
#include "multiSlsDetector.h"
#include "sls_detector_defs.h"

using namespace std;



ctbPower::ctbPower(TGGroupFrame* f, int i,  multiSlsDetector* d)
  : ctbDac(f, i, d) 
{
  cout << "****************************************************************power " << i << endl;
     dacsUnit->SetOn(kTRUE); 
     dacsUnit->SetEnabled(kFALSE);
     int ii=0;

     switch(i) {
     case slsDetectorDefs::V_POWER_IO: 
     	 dacsLabel->SetText("VIO"); 
	 ii=slsDetectorDefs::I_POWER_IO;
     	 break; 
     case slsDetectorDefs::V_POWER_A: 
     	 dacsLabel->SetText("VA"); 
	 ii=slsDetectorDefs::I_POWER_A;
     	 break;  
     case slsDetectorDefs::V_POWER_B: 
     	 dacsLabel->SetText("VB"); 
	 ii=slsDetectorDefs::I_POWER_B;
     	 break;   
     case slsDetectorDefs::V_POWER_C: 
     	 dacsLabel->SetText("VC");
	 ii=slsDetectorDefs::I_POWER_C; 
     	 break;    
     case slsDetectorDefs::V_POWER_D: 
     	 dacsLabel->SetText("VD");
	 ii=slsDetectorDefs::I_POWER_D;  
     	 break;      
     case slsDetectorDefs::V_POWER_CHIP: 
     	 dacsLabel->SetText("VCHIP"); 
     	 dacsLabel->SetEnabled(kFALSE);
	 ii=-1;
     	 break;    
     default: 
     	 dacsLabel->SetText("Bad index"); 
	 ii=-1;
     	 break; 
     };
     
     // ctbSlowAdc *vm=new ctbSlowAdc(f,i,d);
     // vm->setLabel("V: ");
     // if (ii>=0) {
     //   ctbSlowAdc *im=new ctbSlowAdc(f,ii,d);
     //   im->setLabel("I: ");
     // }

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


void ctbPower::setValue(Long_t a) {cout << "ssssssssss" << endl; ctbPower::setValue();}
void ctbPower::setValue() {


  cout << "***************************Setting power " << dacsEntry->GetIntNumber() << " " << (slsDetectorDefs::dacIndex)id <<" " << 1 << endl;
  myDet->setDAC(dacsEntry->GetIntNumber(), (slsDetectorDefs::dacIndex)id, 1);

  getValue();

}


int ctbPower::getValue() {
  
  int val=myDet->setDAC(-1, (slsDetectorDefs::dacIndex)id, 1);
  char s[100];
  cout << "****************************Getting power " << val << " " << (slsDetectorDefs::dacIndex)id <<" " << 1 << endl;
  sprintf(s,"%d",val);
  dacsValue->SetText(s);
  if (val>0) {
    if (id!=slsDetectorDefs::V_POWER_CHIP)
      dacsLabel->SetOn(kTRUE);
  } else {
    dacsLabel->SetOn(kFALSE);
  }
  

  return val;

}



ctbPowers::ctbPowers(TGVerticalFrame* page, multiSlsDetector* det)   : TGGroupFrame(page,"Power Supplies",kVerticalFrame) , myDet(det){


  SetTitlePos(TGGroupFrame::kLeft);
  page->AddFrame(this,new TGLayoutHints( kLHintsTop | kLHintsExpandX , 10,10,10,10));
  MapWindow();
  
  // cout << "window mapped " << endl;

  for (int idac=0; idac<NPOWERS; idac++) {
     dacs[idac]=new ctbPower(this, slsDetectorDefs::V_POWER_A+idac, myDet);

  }
}


int ctbPowers::setPwrAlias(string line) {

  int is=-1, mv=0;
  char tit[100];
  int narg;

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
