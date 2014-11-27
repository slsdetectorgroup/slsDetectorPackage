#ifndef JCTBSIGNALS_H
#define JCTBSIGNALS_H
#include  <TGFrame.h>


#define NSIGNALS 64
#define NIOSIGNALS 52
#define ADCLATCH 63




class TGNumberEntry;
class multiSlsDetector;
class jctbSignal;

#include <string>
using namespace std;

class jctbSignals : public TGGroupFrame {
private:
 
  jctbSignal *signals[NSIGNALS];

  TGNumberEntry *eIOCntrlRegister;
  TGNumberEntry *eClkCntrlRegister;

  multiSlsDetector *myDet;

public:
  jctbSignals(TGVerticalFrame *page, multiSlsDetector *det);
   int setSignalAlias(string line);
   string getSignalAlias();

   void update();


   void ToggledOutReg(Int_t);
   void ToggledClockReg(Int_t);
   void ToggledPlot(Int_t);
   ClassDef(jctbSignals,0)
};

#endif
