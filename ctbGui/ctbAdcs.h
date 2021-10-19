// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package



#ifndef CTBADCS_H
#define CTBADCS_H
#include  <TGFrame.h>


#define NADCS 32

class TRootEmbeddedCanvas;
class TGButtonGroup;
class TGVerticalFrame;
class TGHorizontalFrame;
class TGTextEntry;
class TGLabel;
class TGNumberEntry;
class TH2F;
class TGComboBox;
class TGCheckButton;
class TGColorSelect;
class TColor;

class   THStack;
class   TGraphErrors;
class  TGTextButton;
class  TGTab;

class TGraph;

namespace sls
{
   class Detector;
};

#include <string>
using namespace std;

class ctbAdc : public TGHorizontalFrame {


 private:
  

   TGLabel *sAdcLabel;
   TGCheckButton *sAdcEnable;
   TGCheckButton *sAdcPlot;
   TGCheckButton *sAdcInvert;

   TGColorSelect  *fColorSel;

   //  TGraph *gADC;

   int id;
   sls::Detector *myDet;
   
 public:
   ctbAdc(TGVerticalFrame *page, int i, sls::Detector *det);


   void setAdcAlias(char *tit, int plot, int color);
   string getAdcAlias();
   void ToggledAdcPlot(Int_t b);
   void ToggledAdcEnable(Int_t b);
   void ToggledAdcInvert(Int_t b);


   void ToggledPlot(Bool_t b);
   void ToggledEnable(Bool_t b);
   void ToggledInvert(Bool_t b);
   void ColorChanged(Pixel_t);
   void setEnabled(Bool_t b);
   Bool_t getEnabled();
   // TGraph *getGraph();
   void update();

   Pixel_t getColor();
   
   Bool_t getEnable();
   void setEnable(Bool_t);
   void setPlot(Bool_t);
   Bool_t getInverted();
   Bool_t getPlot();
   void setInverted(Bool_t);

   ClassDef(ctbAdc,0)
     };



class ctbAdcs : public TGGroupFrame {
private:
  
  ctbAdc *sAdc[NADCS];
   sls::Detector *myDet;

   
  TGTextButton *bCheckAll;
  TGTextButton *bRemoveAll;
  TGTextButton *bCheckHalf[2];
  TGTextButton *bRemoveHalf[2];
  TGNumberEntry *eInversionMask;
  TGNumberEntry *eEnableMask;


/*   TGTextButton *bPlotSelected; */
/*   TGNumberEntry *eMinX; */
/*   TGNumberEntry *eMaxX; */
/*   TGNumberEntry *eMinY; */
/*   TGNumberEntry *eMaxY; */
  
  

/*   TGTextButton *bGetPixel; */
/*   TGNumberEntry *ePixelX; */
/*   TGNumberEntry *ePixelY; */
/*   TGLabel *lPixelValue; */

public:

   ctbAdcs(TGVerticalFrame *page, sls::Detector *det);
   int setAdcAlias(string line);
   string getAdcAlias();
   string getAdcParameters();
   void ToggledAdcPlot(Int_t);
   void ToggledAdcInvert(Int_t);
   void ToggledAdcEnable(Int_t);
   void AdcEnable(Int_t b);
   //   TGraph *getGraph(int i);
   void CheckAll();
   void RemoveAll();
   void update();

   int setInvert(int reg=-1);
   int setEnable(int reg=-1);


   Pixel_t getColor(int i);
   Bool_t getEnabled(int i);
   Bool_t getPlot(int i);
   Bool_t getEnable(int i);

   void CheckHalf0();
   void RemoveHalf0();
   
   void CheckHalf1();
   void RemoveHalf1();
   

   ClassDef(ctbAdcs,0)
};

#endif


