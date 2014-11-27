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
#include <TGColorSelect.h>
#include <THStack.h>
#include <TGTab.h>

#include <stdio.h>
#include <iostream>
#include <fstream>

#include "jctbAdcs.h"
#include "multiSlsDetector.h"

using namespace std;



jctbAdc::jctbAdc(TGVerticalFrame *page, int i, multiSlsDetector *det)
  : TGHorizontalFrame(page, 800,800), id(i), myDet(det) {

  TGHorizontalFrame *hframe=this;
  char tit[100];

   page->AddFrame(hframe,new TGLayoutHints(kLHintsTop | kLHintsExpandX , 1,1,1,1));
   hframe->MapWindow();


   


   sprintf(tit, "ADC%d", id);

   sAdcLabel= new TGLabel(hframe, tit);
   hframe->AddFrame(sAdcLabel,new TGLayoutHints(kLHintsTop | kLHintsLeft| kLHintsExpandX, 1, 1, 1, 1));
   sAdcLabel->MapWindow();
   sAdcLabel->SetTextJustify(kTextLeft);



   

   sAdcEnable= new TGCheckButton(hframe, "Enable");
   hframe->AddFrame( sAdcEnable,new TGLayoutHints(kLHintsTop | kLHintsLeft| kLHintsExpandX, 1, 1, 1, 1));
   sAdcEnable->MapWindow();
   sAdcEnable->SetOn(kTRUE);
   sAdcEnable->SetEnabled(kFALSE);



   sAdcPlot= new TGCheckButton(hframe, "Plot");
   hframe->AddFrame( sAdcPlot,new TGLayoutHints(kLHintsTop | kLHintsLeft| kLHintsExpandX, 1, 1, 1, 1));
   sAdcPlot->MapWindow();


   sAdcPlot->Connect("Toggled(Bool_t)","jctbAdc",this,"ToggledPlot(Bool_t)");



   fColorSel = new TGColorSelect(hframe, id+1, 0);
   hframe->AddFrame(fColorSel, new TGLayoutHints(kLHintsTop |
                          kLHintsLeft, 2, 0, 2, 2));
   

   fColorSel->SetColor(TColor::Number2Pixel(id+1));


};

void jctbAdc::setAdcAlias(char *tit, int plot, int color) {
  if (tit)
    sAdcLabel->SetText(tit);
  if (plot)
    sAdcPlot->SetOn(kTRUE,kTRUE);
  else
    sAdcPlot->SetOn(kFALSE,kTRUE);
  if (color>=0)
    fColorSel->SetColor(color);
  fColorSel->SetEnabled(sAdcPlot->IsOn());
}


string jctbAdc::getAdcAlias() {

  char line[1000];
  sprintf(line,"ADC%d %s %d %x\n",id,sAdcLabel->GetText()->Data(),sAdcPlot->IsOn(),fColorSel->GetColor());
  return string(line);
}

void jctbAdc::update() {

}


void jctbAdc::ToggledPlot(Bool_t b){

  Long_t mask=b<<id;
  ToggledAdcPlot(mask);
  fColorSel->SetEnabled(b);

}

void jctbAdc::ToggledAdcPlot(Int_t b){

  
  Emit("ToggledAdcPlot(Int_t)", id);

}








jctbAdcs::jctbAdcs(TGVerticalFrame *page, multiSlsDetector *det)
  : TGGroupFrame(page,"Adcs",kVerticalFrame), myDet(det) {


  SetTitlePos(TGGroupFrame::kLeft);
  page->AddFrame(this,new TGLayoutHints( kLHintsTop | kLHintsExpandX , 10,10,10,10));
  MapWindow();
  
  char tit[100];

 
  TGHorizontalFrame* hframe=new TGHorizontalFrame(this, 800,800);
  AddFrame(hframe,new TGLayoutHints(kLHintsTop | kLHintsExpandX , 1,1,1,1));
  hframe->MapWindow();




  int idac=0;




 
  TGHorizontalFrame* hhframe=new TGHorizontalFrame(this, 800,800);
  AddFrame(hhframe,new TGLayoutHints(kLHintsTop | kLHintsExpandX , 1,1,1,1));
  hhframe->MapWindow();

  TGVerticalFrame *vframe;




  for (idac=0; idac<NADCS; idac++) {
    if (idac%16==0) {


    vframe=new TGVerticalFrame(hhframe, 400,800);
     hhframe->AddFrame(vframe,new TGLayoutHints(kLHintsTop | kLHintsExpandX , 1,1,1,1));
    vframe->MapWindow();
   
  

    }

    sAdc[idac]=new jctbAdc(vframe,idac,myDet);
  }


}

void jctbAdcs::update() {

  
  ;

  

}

int jctbAdcs::setAdcAlias(string line) {

  int is=-1, plot=0, color=-1;
  char tit[100];
  int narg=sscanf(line.c_str(),"ADC%d %s %d %x",&is,tit,&plot, &color);
  if (narg<2)
    return -1;
  if (narg!=3)
    color=-1;
  if (is>=0 && is<NADCS) {
    sAdc[is]->setAdcAlias(tit,plot,color);
  }
  return is;

}

string jctbAdcs::getAdcAlias() {

  ostringstream line;

  for (int is=0; is<NADCS; is++)
    line << sAdc[is]->getAdcAlias();

  return line.str();
}
