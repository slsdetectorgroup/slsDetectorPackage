// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#ifndef CTBSIGNALS_H
#define CTBSIGNALS_H
#include  <TGFrame.h>


#define NSIGNALS 64

#define NIOSIGNALS 64 //for moench board was 52


#define ADCLATCH 63
#define DIGSIGLATCH 62




class TGTextEntry;
class TGLabel;
class TGNumberEntry;
class TGCheckButton;
class TH1I;
class  TGTextButton;
class TGColorSelect;



class TGNumberEntry;
namespace sls
{
   class Detector;
};
class ctbSignal;

#include <string>
using namespace std;

class ctbSignal : public TGHorizontalFrame {

  // RQ_OBJECT("ctbSignal")

private:

   TGLabel *sLabel;
   TGCheckButton *sOutput;
   TGCheckButton *sDbitList;
   TGCheckButton *sPlot;
   TGLabel *sValue;
   TGNumberEntry *sEntry;
   TGColorSelect *fColorSel;

   sls::Detector *myDet;
   Int_t id;

   TH1I *hsig;

public:

   ctbSignal(TGFrame *page, int i, sls::Detector *det);
   int setSignalAlias(char *tit, int plot, int col);
   string getSignalAlias();

   TH1I *getPlot() {return hsig;};
   int setOutput(Long64_t);
   int fixOutput(int);
   int setDbitList(Long64_t);

   void ToggledOutput(Bool_t); 
   void ToggledDbitList(Bool_t); 
   void ToggledPlot(Bool_t); 
   void ColorChanged(Pixel_t); 

   int isDbitList();
   int isOutput();
   int isPlot();
   Pixel_t getColor();


   void ToggledSignalOutput(Int_t); //*SIGNAL*
   void ToggledSignalDbitList(Int_t); //*SIGNAL*
   void ToggledSignalPlot(Int_t); //*SIGNAL*



   ClassDef(ctbSignal,0)
};

class ctbSignals : public TGGroupFrame {
private:
 
  ctbSignal *signals[NSIGNALS];

  TGNumberEntry *eIOCntrlRegister;
  TGNumberEntry *eDbitOffset;

  sls::Detector *myDet;

public:
  ctbSignals(TGVerticalFrame *page, sls::Detector *det);
   int setSignalAlias(string line);
   string getSignalAlias();

   int getPlot(int);
   Pixel_t getColor(int);
   
   void update();
   // void saveParameters();
   string getSignalParameters();

   //void setDbitList(Int_t);
   void setDbitOffset(Long_t);
   void setDbitOffset();

   void ToggledOutReg(Int_t);
   void ToggledDbitList(Int_t);
   void ToggledPlot(Int_t);
   void ToggledSignalPlot(Int_t); //*SIGNAL*

   ClassDef(ctbSignals,0)
};

#endif
