#ifndef JCTBACQUISITION_H
#define JCTBACQUISITION_H
#include  <TGFrame.h>



class TGTextEntry;
class TGLabel;
class TGNumberEntry;
class TGCheckButton;
class TThread;

class 	TGTextButton;

class multiSlsDetector;
class detectorData;

#include <string>
using namespace std;

class jctbAcquisition : public TGGroupFrame {


 private:
  TGTextEntry *eOutdir;
  TGTextEntry *eFname;
  TGNumberEntry *eFindex;
  TGCheckButton *cFileSave;
  TGTextButton *bStatus;
  // TGTextButton 
  
  TThread *acqThread;

  multiSlsDetector* myDet;
  

  


 public:
   jctbAcquisition(TGVerticalFrame*,  multiSlsDetector*);
   void setOutdir();
   void setFname();
   void setFsave(Bool_t);
   void setFindex();
   void toggleAcquisition();
   static void* ThreadHandle(void *arg);
   void update();
   void acquisitionFinished();

   void startAcquisition();
   static   int progressCallback(double,void*);
   static int dataCallback(detectorData*, int, void*);
   int StopFlag;

   ClassDef(jctbAcquisition,0)
};


#endif
