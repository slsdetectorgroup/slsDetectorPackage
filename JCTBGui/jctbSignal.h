#ifndef JCTBSIGNAL_H
#define JCTBSIGNAL_H
#include  <TGFrame.h>





class TGTextEntry;
class TGLabel;
class TGNumberEntry;
class TGCheckButton;
class TH1I;
class  TGTextButton;
class TGColorSelect;



class multiSlsDetector;

#include <string>
using namespace std;

class jctbSignal : public TGHorizontalFrame {

  // RQ_OBJECT("jctbSignal")

private:

   TGLabel *sLabel;
   TGCheckButton *sOutput;
   TGCheckButton *sClock;
   TGCheckButton *sPlot;
   TGLabel *sValue;
   TGNumberEntry *sEntry;
   TGColorSelect *fColorSel;

   multiSlsDetector *myDet;
   Int_t id;

   TH1I *hsig;

public:

   jctbSignal(TGFrame *page, int i, multiSlsDetector *det);
   int setSignalAlias(char *tit, int plot, int col);
   string getSignalAlias();

   TH1I *getPlot() {return hsig;};
   int setOutput(Long64_t);
   int setClock(Long64_t);

   void ToggledOutput(Bool_t); 
   void ToggledClock(Bool_t); 
   void ToggledPlot(Bool_t); 

   int isClock();
   int isOutput();
   int isPlot();


   void ToggledSignalOutput(Int_t); //*SIGNAL*
   void ToggledSignalClock(Int_t); //*SIGNAL*
   void ToggledSignalPlot(Int_t); //*SIGNAL*



   ClassDef(jctbSignal,0)
};

#endif
