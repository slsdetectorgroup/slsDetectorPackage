#ifndef CTBPATTERN_H
#define CTBPATTERN_H
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
class TGTextEntry;
class TGCheckButton;

class   THStack;
class   TGraphErrors;
class energyCalibration;
class  TGTextButton;
class  TGTab;

class multiSlsDetector;


#include <string>
using namespace std;



class ctbLoop : public TGHorizontalFrame {
 

 private:

   TGNumberEntry *eLoopStartAddr; 
   TGNumberEntry *eLoopStopAddr; 
   TGNumberEntry *eLoopNumber; 

   int id;

   multiSlsDetector *myDet;

 public:
   ctbLoop(TGGroupFrame *page, int i,multiSlsDetector *det);
   
   void setNLoops();
   void update();

   ClassDef(ctbLoop,0)
     };

class ctbWait : public TGHorizontalFrame {
 

 private:

   TGNumberEntry *eWaitAddr; 
   TGNumberEntry *eWaitTime; 

   int id;

   multiSlsDetector *myDet;

 public:
   ctbWait(TGGroupFrame *page, int i,multiSlsDetector *det);
   
   void setWaitTime();
   void update();

   ClassDef(ctbWait,0)
     };







class ctbPattern : public TGGroupFrame {
private:
   

   TGNumberEntry *eAdcClkFreq;
   TGNumberEntry *eRunClkFreq;
   TGNumberEntry *eDBitClkFreq;
   TGNumberEntry *eAdcClkPhase;
   TGNumberEntry *eDBitClkPhase;
   //TGNumberEntry *eRunClkPhase;

   TGNumberEntry *eStartAddr;
   TGNumberEntry *eStopAddr;
   TGNumberEntry *eFrames;
   TGNumberEntry *ePeriod;
   TGNumberEntry *eCycles;
   TGNumberEntry *eMeasurements;
   TGNumberEntry *eAdcPipeline;
   TGNumberEntry *eDBitPipeline;
   
   ctbLoop *eLoop[NLOOPS];
   ctbWait *eWait[NWAITS];

   TGTextEntry *patternCompiler;
   TGTextEntry *patternFile;
   
   TGTextButton *browseCompiler;
   TGTextButton *browseFile;
   

   TGNumberEntry *eAnalogSamples;
   TGNumberEntry *eDigitalSamples;

  TGCheckButton *cbAnalog;
  TGCheckButton *cbDigital;
  
   char pat[PATLEN*8];

   multiSlsDetector *myDet;

public:

   ctbPattern(TGVerticalFrame *page, multiSlsDetector *det);

   void update();
   void setAdcFreq();
   void setRunFreq();
   void setDBitFreq();
   void setAdcPhase();
   void setDBitPhase();
   // void setRunPhase();
   void setAdcPipeline();
   void setDBitPipeline();
   void setFrames();
   void setCycles();
   void setMeasurements();
   void setPeriod();

   
   void chooseCompiler();
   void choosePattern();

   string getCompiler();
   string getPatternFile();
   
   void setPatternAlias(string);
   string getPatternAlias();


   int getAnalogSamples();
   void  setAnalogSamples();
   int getDigitalSamples();
   void  setDigitalSamples();
   void  setReadoutMode(Bool_t);
   int  getReadoutMode();


   void setFile();
   void setCompiler();
   void patternFileChanged(const char*);
   void patternCompilerChanged(const char*);
   void analogSamplesChanged(const int t);
   void digitalSamplesChanged(const int t);
   void readoutModeChanged(int);


   ClassDef(ctbPattern,0)
};

#endif
