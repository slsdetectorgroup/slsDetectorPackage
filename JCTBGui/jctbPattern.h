#ifndef JCTBPATTERN_H
#define JCTBPATTERN_H
#include  <TGFrame.h>


#define NLOOPS 3
#define NWAITS 3
#define NADCS 32
#define PATLEN 1024

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

class   THStack;
class   TGraphErrors;
class energyCalibration;
class  TGTextButton;
class  TGTab;

class multiSlsDetector;


#include <string>
using namespace std;



class jctbLoop : public TGHorizontalFrame {
 

 private:

   TGNumberEntry *eLoopStartAddr; 
   TGNumberEntry *eLoopStopAddr; 
   TGNumberEntry *eLoopNumber; 

   int id;

   multiSlsDetector *myDet;

 public:
   jctbLoop(TGGroupFrame *page, int i,multiSlsDetector *det);
   
   void setNLoops();
   void update();

   ClassDef(jctbLoop,0)
     };

class jctbWait : public TGHorizontalFrame {
 

 private:

   TGNumberEntry *eWaitAddr; 
   TGNumberEntry *eWaitTime; 

   int id;

   multiSlsDetector *myDet;

 public:
   jctbWait(TGGroupFrame *page, int i,multiSlsDetector *det);
   
   void setWaitTime();
   void update();

   ClassDef(jctbWait,0)
     };







class jctbPattern : public TGGroupFrame {
private:
   

   TGNumberEntry *eAdcClkFreq;
   TGNumberEntry *eRunClkFreq;
   TGNumberEntry *eAdcClkPhase;
   TGNumberEntry *eRunClkPhase;

   TGNumberEntry *eStartAddr;
   TGNumberEntry *eStopAddr;
   TGNumberEntry *eFrames;
   TGNumberEntry *ePeriod;
   
   jctbLoop *eLoop[NLOOPS];
   jctbWait *eWait[NWAITS];


   char pat[PATLEN*8];

   multiSlsDetector *myDet;

public:

   jctbPattern(TGVerticalFrame *page, multiSlsDetector *det);

   void update();
   void setAdcFreq();
   void setRunFreq();
   void setAdcPhase();
   void setRunPhase();
   void setFrames();
   void setPeriod();

   

   ClassDef(jctbPattern,0)
};

#endif
