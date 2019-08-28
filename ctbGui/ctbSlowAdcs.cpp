
#include <stdio.h>
#include <iostream>
#include <fstream>

#include <TGTextEntry.h>
#include <TGLabel.h>
#include <TGNumberEntry.h>
#include <TGButton.h>

#include "ctbSlowAdcs.h"
#include "ctbDefs.h"
#include "Detector.h"
#include "sls_detector_defs.h"

using namespace std;




ctbSlowAdc::ctbSlowAdc(TGGroupFrame *page, int idac, sls::Detector *det) : TGHorizontalFrame(page, 800,50) , id(idac), myDet(det) {


    TGHorizontalFrame *hframe=this;

    page->AddFrame(hframe,new TGLayoutHints(kLHintsTop | kLHintsExpandX , 1,1,1,1));
    MapWindow();

    char tit[100];
    

   sprintf(tit, "SENSE %d:",idac-1000);

   dacsLabel= new TGLabel(hframe, tit);// new TGLabel(hframe, tit);
   
 

   hframe->AddFrame(dacsLabel,new TGLayoutHints(kLHintsTop | kLHintsLeft| kLHintsExpandX, 5, 5, 5, 5));
   dacsLabel->MapWindow();
   dacsLabel->SetTextJustify(kTextLeft);

  


   sprintf(tit, "xxx");
   dacsValue= new TGLabel(hframe, tit);
   hframe->AddFrame( dacsValue,new TGLayoutHints(kLHintsTop | kLHintsLeft| kLHintsExpandX, 5, 5, 5, 5));
   dacsValue->MapWindow();
   dacsValue->SetTextJustify(kTextLeft);

   

   TGTextButton *b= new TGTextButton(hframe, "Update");
   hframe->AddFrame( b,new TGLayoutHints(kLHintsTop | kLHintsLeft| kLHintsExpandX, 5, 5, 5, 5));
   b->MapWindow();
   b->SetTextJustify(kTextLeft);

   b->Connect("Clicked()","ctbSlowAdc",this,"getValue()");
}



int ctbSlowAdc::setLabel(char *tit) {
  if(tit)
    dacsLabel->SetText(tit);

  return id;

}

string ctbSlowAdc::getLabel() {
  ostringstream line;
  line << dacsLabel->GetText() <<  endl;

   //  line << "DAC" << dec << id << " " <<  dacsUnit->IsOn() << endl;

  return line.str();
}



int ctbSlowAdc::getValue() {
  try {
    std::string s;

    // temp
    if (id == static_cast<int>(slsDetectorDefs::SLOW_ADC_TEMP)) {

      int val = myDet->getTemperature(static_cast<slsDetectorDefs::dacIndex>(id)).tsquash("Different values");
      cout << "slow adc temp" << " " << val << endl;

      s = to_string(val) + " " + to_string(0x00b0) + "C";//�C
      dacsValue->SetText(s.c_str());
      return val;
    } 
    
    // mv
    else {

      int val = myDet->getSlowADC(static_cast<slsDetectorDefs::dacIndex>(id)).tsquash("Different values");
      cout << "slow adc " << id << " " << val << endl;
      
      s = to_string(val) + " mV"; 
      dacsValue->SetText(s.c_str());
      return val;
    }

  } CATCH_DISPLAY ("Could not get slow dac " + to_string(id) + ".", "ctbSlowAdc::getValue")

  return -1;
}







ctbSlowAdcs::ctbSlowAdcs(TGVerticalFrame *page, sls::Detector *det)   : TGGroupFrame(page,"Sense",kVerticalFrame) , myDet(det){

  SetTitlePos(TGGroupFrame::kLeft);
  page->AddFrame(this,new TGLayoutHints( kLHintsTop | kLHintsExpandX , 10,10,10,10));
  MapWindow();
  
  // cout << "window mapped " << endl;

 
  for (int idac=0; idac<NSLOWADCS + 1; idac++) {

     adcs[idac]=new ctbSlowAdc(this, idac+1000, myDet);

  }
  adcs[NSLOWADCS]->setLabel((char*)"Temperature"); 
}




int ctbSlowAdcs::setSlowAdcAlias(string line) {

  int is=-1, mv=0;
  char tit[100];
  int narg=sscanf(line.c_str(),"SENSE%d %s %d",&is,tit,&mv);
  if (narg<2)
    return -1;
  if (is>=0 && is<NSLOWADCS) 
    adcs[is]->setLabel(tit);
  return is;

}

string ctbSlowAdcs::getSlowAdcAlias() {

  ostringstream line;

  for (int i=0; i<NSLOWADCS; i++)
    line << adcs[i]->getLabel() << endl;
  return line.str();
}




string ctbSlowAdcs::getAdcParameters() {

  ostringstream line;

  for (int i=0; i<NSLOWADCS; i++) {
    //line << "dacs:" << i << " " << dacs[i]->getValue << endl;
    line << "adc:" << i << " " << adcs[i]->getValue() << endl;
  }    
  line << "adc:-1" << adcs[NSLOWADCS]->getValue() << endl;
  return line.str();
}



void  ctbSlowAdcs::update() {
  for (int idac=0; idac<NSLOWADCS+1; idac++) {
    adcs[idac]->getValue();
  }
}
