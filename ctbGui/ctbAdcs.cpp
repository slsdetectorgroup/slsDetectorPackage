// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
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

#include "ctbAdcs.h"
#include "ctbDefs.h"
#include "sls/Detector.h"

using namespace std;



ctbAdc::ctbAdc(TGVerticalFrame *page, int i, sls::Detector *det)
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



   
   

   sAdcInvert= new TGCheckButton(hframe, "Inv");
   hframe->AddFrame( sAdcInvert,new TGLayoutHints(kLHintsTop | kLHintsLeft| kLHintsExpandX, 1, 1, 1, 1));
   sAdcInvert->MapWindow();
   sAdcInvert->Connect("Toggled(Bool_t)","ctbAdc",this,"ToggledInvert(Bool_t)");


   sAdcEnable= new TGCheckButton(hframe, "En");
   hframe->AddFrame( sAdcEnable,new TGLayoutHints(kLHintsTop | kLHintsLeft| kLHintsExpandX, 1, 1, 1, 1));
   sAdcEnable->MapWindow();
   // sAdcEnable->SetOn(kTRUE);
   // sAdcEnable->SetEnabled(kFALSE);
   sAdcEnable->Connect("Toggled(Bool_t)","ctbAdc",this,"ToggledEnable(Bool_t)");



   sAdcPlot= new TGCheckButton(hframe, "Plot");
   hframe->AddFrame( sAdcPlot,new TGLayoutHints(kLHintsTop | kLHintsLeft| kLHintsExpandX, 1, 1, 1, 1));
   sAdcPlot->MapWindow();


   sAdcPlot->Connect("Toggled(Bool_t)","ctbAdc",this,"ToggledPlot(Bool_t)");



   fColorSel = new TGColorSelect(hframe, id+1, 0);

   fColorSel->Connect("ColorSelected(Pixel_t)","ctbAdc",this,"ColorChanged(Pixel_t)");
   hframe->AddFrame(fColorSel, new TGLayoutHints(kLHintsTop |
                          kLHintsLeft, 2, 0, 2, 2));
   

   fColorSel->SetColor(TColor::Number2Pixel(id+1));
   //   sprintf(tit,"adc%d",id);
//    gADC=new TGraph();
//    gADC->SetName(tit);
//    gADC->SetLineColor(id+1);
//    gADC->SetMarkerColor(id+1);
   
   

};
Pixel_t ctbAdc::getColor(){
  return fColorSel->GetColor();
}
Bool_t ctbAdc::getEnabled(){
  return getPlot();
}
Bool_t ctbAdc::getPlot(){
  return sAdcPlot->IsOn();
}
Bool_t ctbAdc::getInverted(){
  return sAdcInvert->IsOn();
}

Bool_t ctbAdc::getEnable(){
  return sAdcEnable->IsOn();
}



void ctbAdc::setInverted(Bool_t b){
  //  cout << id << "set enabled " << b << endl;
  if (b)
    sAdcInvert->SetOn(kTRUE,kTRUE);
  else
    sAdcInvert->SetOn(kFALSE,kTRUE);
    
}


void ctbAdc::setEnable(Bool_t b){
  //  cout << id << "set enabled " << b << endl;
  if (b)
    sAdcEnable->SetOn(kTRUE,kFALSE);
  else
    sAdcEnable->SetOn(kFALSE,kFALSE);
    
}





void ctbAdc::setAdcAlias(char *tit, int plot, int color) {
  if (tit)
    sAdcLabel->SetText(tit);
  if (plot>0)
    sAdcPlot->SetOn(kTRUE,kTRUE);
  else if (plot==0)
    sAdcPlot->SetOn(kFALSE,kTRUE);
  if (color>=0)
    fColorSel->SetColor(color);
  fColorSel->SetEnabled(sAdcPlot->IsOn());
}


string ctbAdc::getAdcAlias() {

  char line[1000];
  sprintf(line,"ADC%d %s %d %lx\n",id,sAdcLabel->GetText()->Data(),sAdcPlot->IsOn(),fColorSel->GetColor());
  return string(line);
}

void ctbAdc::update() {

  
  //Emit("ToggledAdcEnable(Int_t)", id);
  
}


void ctbAdc::ToggledPlot(Bool_t b){

  //  Long_t mask=b<<id;
  //  ToggledAdcPlot(mask);
  cout << "Colsel " << id << " enable " << b << endl;
  if (b)
    fColorSel->SetEnabled(kTRUE);
  else
    fColorSel->SetEnabled(kFALSE);
    
  // fColorSel->SetEnabled(sAdcPlot->IsOn());
  Emit("ToggledAdcPlot(Int_t)", id);
  
}



void ctbAdc::ToggledInvert(Bool_t b){

    
  // fColorSel->SetEnabled(sAdcPlot->IsOn());
  Emit("ToggledAdcInvert(Int_t)", id);
  
}



void ctbAdc::ToggledEnable(Bool_t b){

    
  fColorSel->SetEnabled(sAdcPlot->IsOn());
  Emit("ToggledAdcEnable(Int_t)", id);
  
}






void ctbAdc::ColorChanged(Pixel_t) {
  
  Emit("ToggledAdcPlot(Int_t)", id);

}

void ctbAdc::ToggledAdcPlot(Int_t b){

  
  Emit("ToggledAdcPlot(Int_t)", id);

}

void ctbAdc::ToggledAdcInvert(Int_t b){

  
  Emit("ToggledAdcInvert(Int_t)", id);

}

void ctbAdc::ToggledAdcEnable(Int_t b){

  
  Emit("ToggledAdcEnable(Int_t)", id);

}




void ctbAdc::setEnabled(Bool_t b){
  //  cout << id << "set enabled " << b << endl;
  if (b)
    sAdcPlot->SetOn(kTRUE,kFALSE);
  else
    sAdcPlot->SetOn(kFALSE,kFALSE);
    
}



void ctbAdc::setPlot(Bool_t b){
  //  cout << id << "set enabled " << b << endl;
  if (b)
    sAdcPlot->SetOn(kTRUE,kTRUE);
  else
    sAdcPlot->SetOn(kFALSE,kTRUE);
    
  
}







ctbAdcs::ctbAdcs(TGVerticalFrame *page, sls::Detector *det)
  : TGGroupFrame(page,"Adcs",kVerticalFrame), myDet(det) {


  SetTitlePos(TGGroupFrame::kLeft);
  page->AddFrame(this,new TGLayoutHints( kLHintsTop | kLHintsExpandX , 10,10,10,10));
  MapWindow();
  

 
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

    sAdc[idac]=new ctbAdc(vframe,idac,myDet);

    
    sAdc[idac]->Connect("ToggledAdcPlot(Int_t)","ctbAdcs",this,"ToggledAdcPlot(Int_t)");
    sAdc[idac]->Connect("ToggledAdcInvert(Int_t)","ctbAdcs",this,"ToggledAdcInvert(Int_t)");
    sAdc[idac]->Connect("ToggledAdcEnable(Int_t)","ctbAdcs",this,"ToggledAdcEnable(Int_t)");

  }

  hframe=new TGHorizontalFrame(this, 800,800);
  AddFrame(hframe,new TGLayoutHints(kLHintsTop | kLHintsExpandX , 1,1,1,1));
  hframe->MapWindow();


    bCheckHalf[0]=new TGTextButton(hframe, "All 0-15");
    hframe->AddFrame(bCheckHalf[0],new TGLayoutHints(kLHintsTop |  kLHintsExpandX, 5, 5, 5, 5));
    bCheckHalf[0]->MapWindow();
    bCheckHalf[0]->Connect("Clicked()","ctbAdcs",this,"CheckHalf0()");


    bRemoveHalf[0]=new TGTextButton(hframe, "None 0-15");
    hframe->AddFrame(bRemoveHalf[0],new TGLayoutHints(kLHintsBottom |  kLHintsExpandX, 5, 5, 5, 5));
    bRemoveHalf[0]->MapWindow();
    bRemoveHalf[0]->Connect("Clicked()","ctbAdcs",this,"RemoveHalf0()");


    bCheckHalf[1]=new TGTextButton(hframe, "All 16-23");
    hframe->AddFrame(bCheckHalf[1],new TGLayoutHints(kLHintsTop |  kLHintsExpandX, 5, 5, 5, 5));
    bCheckHalf[1]->MapWindow();
    bCheckHalf[1]->Connect("Clicked()","ctbAdcs",this,"CheckHalf1()");
    //  bCheckAll->Connect("Clicked()","ctbAdcs",this,"CheckAll()");


    bRemoveHalf[1]=new TGTextButton(hframe, "None 16-23");
    hframe->AddFrame(bRemoveHalf[1],new TGLayoutHints(kLHintsBottom |  kLHintsExpandX, 5, 5, 5, 5));
    bRemoveHalf[1]->MapWindow();
    bRemoveHalf[1]->Connect("Clicked()","ctbAdcs",this,"RemoveHalf1()");
    // bRemoveAll->Connect("Clicked()","ctbAdcs",this,"RemoveAll()");




  hframe=new TGHorizontalFrame(this, 800,800);
  AddFrame(hframe,new TGLayoutHints(kLHintsTop | kLHintsExpandX , 1,1,1,1));
  hframe->MapWindow();


    bCheckAll=new TGTextButton(hframe, "All");
    hframe->AddFrame(bCheckAll,new TGLayoutHints(kLHintsTop |  kLHintsExpandX, 5, 5, 5, 5));
    bCheckAll->MapWindow();
    bCheckAll->Connect("Clicked()","ctbAdcs",this,"CheckAll()");


    bRemoveAll=new TGTextButton(hframe, "None");
    hframe->AddFrame(bRemoveAll,new TGLayoutHints(kLHintsBottom |  kLHintsExpandX, 5, 5, 5, 5));
    bRemoveAll->MapWindow();
    bRemoveAll->Connect("Clicked()","ctbAdcs",this,"RemoveAll()");



  hframe=new TGHorizontalFrame(this, 800,50);
 AddFrame(hframe,new TGLayoutHints(kLHintsTop | kLHintsExpandX , 1,1,1,1));
  hframe->MapWindow();


  TGLabel *label= new TGLabel(hframe, "Inversion mask: ");
   hframe->AddFrame(label,new TGLayoutHints(kLHintsTop | kLHintsLeft| kLHintsExpandX, 1, 1, 1, 1));
   label->MapWindow();
   label->SetTextJustify(kTextLeft);


   eInversionMask = new TGNumberEntry(hframe, 0, 16,999, TGNumberFormat::kNESHex,
                                               TGNumberFormat::kNEANonNegative, 
                                               TGNumberFormat::kNELNoLimits);

  hframe->AddFrame(eInversionMask,new TGLayoutHints(kLHintsTop |  kLHintsExpandX, 1, 1, 1, 1));
   eInversionMask->MapWindow();
   eInversionMask->Resize(150,30);
   eInversionMask->SetState(kFALSE);


  hframe=new TGHorizontalFrame(this, 800,50);
 AddFrame(hframe,new TGLayoutHints(kLHintsTop | kLHintsExpandX , 1,1,1,1));
  hframe->MapWindow();


 label= new TGLabel(hframe, "Enable mask: ");
   hframe->AddFrame(label,new TGLayoutHints(kLHintsTop | kLHintsLeft| kLHintsExpandX, 1, 1, 1, 1));
   label->MapWindow();
   label->SetTextJustify(kTextLeft);


   eEnableMask = new TGNumberEntry(hframe, 0, 16,999, TGNumberFormat::kNESHex,
                                               TGNumberFormat::kNEANonNegative, 
                                               TGNumberFormat::kNELNoLimits);

   hframe->AddFrame(eEnableMask,new TGLayoutHints(kLHintsTop |  kLHintsExpandX, 1, 1, 1, 1));
   eEnableMask->MapWindow();
   eEnableMask->Resize(150,30);
   eEnableMask->SetState(kFALSE);

}


int ctbAdcs::setEnable(int reg) {

  try {
    if (reg > -1) {
      myDet->setADCEnableMask(reg);
    }
    auto retval = myDet->getADCEnableMask().tsquash("Different values");
    eEnableMask->SetHexNumber(retval);
    return retval;
  } CATCH_DISPLAY ("Could not set/get adc enablemask.", "ctbAdcs::setEnable")
  
  return -1;
}

int ctbAdcs::setInvert(int reg) {

  try {
    if (reg > -1) {
      myDet->setADCInvert(reg);
    }
    auto retval = myDet->getADCInvert().tsquash("Different values");
    eInversionMask->SetHexNumber(retval);
    return retval;
  } CATCH_DISPLAY ("Could not set/get adc enablemask.", "ctbAdcs::setEnable")
  
  return -1;
}



void ctbAdcs::update() {
  Int_t invreg;
  Int_t disreg;

  disreg=setEnable();
  invreg=setInvert();
  
  for (int is=0; is<NADCS; is++) {
    sAdc[is]->setAdcAlias(NULL,-1,-1);
    if (invreg & (1<<is) )
      sAdc[is]->setInverted(kTRUE);
    else
      sAdc[is]->setInverted(kFALSE);

    if (disreg & (1<<is) )
      sAdc[is]->setEnable(kTRUE);
    else
      sAdc[is]->setEnable(kFALSE);
  }
  
   Emit("AdcEnable(Int_t)", disreg);

}
string ctbAdcs::getAdcParameters() {
    ostringstream line;
  line << "reg "<< hex  << setInvert() << "# ADC invert reg" << dec << endl;
  line << "reg "<< hex  << setEnable() << " # ADC enable reg"<< dec <<  endl;
  return line.str();

}


void ctbAdcs::CheckAll() {
  for (int is=0; is<NADCS; is++){
    sAdc[is]->setPlot(kTRUE);
  }
}


void ctbAdcs::RemoveAll() {
  for (int is=0; is<NADCS; is++) {
    sAdc[is]->setPlot(kFALSE);
  }
}



void ctbAdcs::CheckHalf0() {
  for (int is=0; is<NADCS/2; is++) {
    sAdc[is]->setPlot(kTRUE);
  }
}


void ctbAdcs::RemoveHalf0() {
  for (int is=0; is<NADCS/2; is++){
    sAdc[is]->setPlot(kFALSE);
  }
}

void ctbAdcs::CheckHalf1() {
  for (int is=NADCS/2; is<NADCS; is++){
    sAdc[is]->setPlot(kTRUE);
  }
}


void ctbAdcs::RemoveHalf1() {
  for (int is=NADCS/2; is<NADCS; is++){
    sAdc[is]->setPlot(kFALSE);
  }
}


int ctbAdcs::setAdcAlias(string line) {

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

string ctbAdcs::getAdcAlias() {

  ostringstream line;

  for (int is=0; is<NADCS; is++)
    line << sAdc[is]->getAdcAlias();

  return line.str();
}


void ctbAdcs::ToggledAdcPlot(Int_t b){

  
  Emit("ToggledAdcPlot(Int_t)", b);

}

void ctbAdcs::AdcEnable(Int_t b){
   Emit("AdcEnable(Int_t)", b);
}


void ctbAdcs::ToggledAdcEnable(Int_t b){

   Int_t oreg=setEnable();
   Int_t m=1<<b;
  
   if (sAdc[b]->getEnable())
     oreg|=m;
   else
     oreg&=~m;

   setEnable(oreg);
  
   Emit("AdcEnable(Int_t)", oreg);
}


void ctbAdcs::ToggledAdcInvert(Int_t b){

   Int_t oreg=setInvert();
   Int_t m=1<<b;
  

   if (sAdc[b]->getInverted())
     oreg|=m;
   else
     oreg&=~m;
  
   setInvert(oreg);
}





Pixel_t ctbAdcs::getColor(int i){
  if (i>=0 && i<NADCS)
    return sAdc[i]->getColor();
  return static_cast<Pixel_t>(-1);
}

Bool_t ctbAdcs::getEnabled(int i){
  if (i>=0 && i<NADCS)
    return sAdc[i]->getEnabled();
  return static_cast<Bool_t>(-1);
}

Bool_t ctbAdcs::getEnable(int i){
  if (i>=0 && i<NADCS)
    return sAdc[i]->getEnable();
  return static_cast<Bool_t>(-1);
}

Bool_t ctbAdcs::getPlot(int i){
  if (i>=0 && i<NADCS)
    return sAdc[i]->getPlot();
  return static_cast<Bool_t>(-1);
}
