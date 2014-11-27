
#include <TGButtonGroup.h>
#include <TGNumberEntry.h>
#include <TGLabel.h>
#include <TList.h>
#include <TThread.h>


#include <stdio.h>
#include <iostream>
#include <fstream>

#include "jctbAcquisition.h"
#include "multiSlsDetector.h"
#include "sls_detector_defs.h"

using namespace std;




jctbAcquisition::jctbAcquisition(TGVerticalFrame *page, multiSlsDetector *det) : TGGroupFrame(page,"Acquisition",kVerticalFrame),  myDet(det) {

    page->AddFrame(this,new TGLayoutHints(kLHintsTop | kLHintsExpandX , 10,10,10,10));
    MapWindow();

    TGHorizontalFrame *hframe=new TGHorizontalFrame(this, 800,50);
    AddFrame(hframe,new TGLayoutHints(kLHintsTop | kLHintsExpandX , 10,10,10,10));
    hframe->MapWindow();

    char tit[100];
    

   cFileSave= new TGCheckButton(hframe, "Output file: ");
   hframe->AddFrame(cFileSave,new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 5, 5, 5, 5));
   cFileSave->MapWindow();
   cFileSave->SetTextJustify(kTextRight);
   cFileSave->Connect("Toggled(Bool_t)","jctbAcquisition",this,"setFsave(Bool_t)");

  
   eFname = new TGTextEntry(hframe, (myDet->getFileName()).c_str());

   hframe->AddFrame(eFname,new TGLayoutHints(kLHintsTop |  kLHintsExpandX, 5, 5, 5, 5));
   eFname->MapWindow();
   eFname->Resize(150,30);

   eFname->Connect("ReturnPressed()","jctbAcquisition",this,"setFname()");


   TGLabel *label=new TGLabel(hframe,"index: ");
   hframe->AddFrame(label,new TGLayoutHints(kLHintsTop | kLHintsLeft| kLHintsExpandX, 5, 5, 5, 5));
   label->MapWindow();
   label->SetTextJustify(kTextRight);


   eFindex = new TGNumberEntry(hframe, 0, 9,999, TGNumberFormat::kNESInteger,
                                               TGNumberFormat::kNEANonNegative, 
					 TGNumberFormat::kNELNoLimits);
   hframe->AddFrame( eFindex,new TGLayoutHints(kLHintsTop |  kLHintsExpandX, 1, 1, 1, 1));
   eFindex->MapWindow();
   eFindex->Resize(150,30);
    TGTextEntry *e= eFindex->TGNumberEntry::GetNumberEntry();
    e->Connect("ReturnPressed()","jctbAcquisition",this,"setFindex()");


   hframe=new TGHorizontalFrame(this, 800,50);
   AddFrame(hframe,new TGLayoutHints(kLHintsTop | kLHintsExpandX , 10,10,10,10));
   hframe->MapWindow();

   label=new TGLabel(hframe,"Output directory: ");
   hframe->AddFrame(label,new TGLayoutHints(kLHintsTop | kLHintsLeft| kLHintsExpandX, 5, 5, 5, 5));
   label->MapWindow();
   label->SetTextJustify(kTextRight);



   eOutdir = new TGTextEntry(hframe, (myDet->getFilePath()).c_str());

   hframe->AddFrame(eOutdir,new TGLayoutHints(kLHintsTop |  kLHintsExpandX, 5, 5, 5, 5));
   eOutdir->MapWindow();
   eOutdir->Resize(150,30);


   eOutdir->Connect("ReturnPressed()","jctbAcquisition",this,"setOutdir()");

   hframe=new TGHorizontalFrame(this, 800,50);
    AddFrame(hframe,new TGLayoutHints(kLHintsTop | kLHintsExpandX , 10,10,10,10));
    hframe->MapWindow();

    bStatus=new TGTextButton(hframe, "Start");
    hframe->AddFrame(bStatus,new TGLayoutHints(kLHintsTop |  kLHintsExpandX, 5, 5, 5, 5));
    bStatus->MapWindow();
    bStatus->Connect("Clicked()","jctbAcquisition",this,"toggleAcquisition()");


    acqThread = new TThread("acqThread",
               jctbAcquisition::ThreadHandle,(void*)this);
    // acqThread->Run();
   
    myDet->registerProgressCallback(&progressCallback,(void*)this);

    
    myDet->registerDataCallback(&dataCallback, (void*)this);

}

int jctbAcquisition::dataCallback(detectorData *data, int index, void* pArgs) {


  cout <<"------"<<  index << " " << data->npoints << " "<< data->npy << endl;

  


}




void jctbAcquisition::setOutdir() {

  myDet->setFilePath(eOutdir->GetText());

//   //  cout << "setting dac! "<< id << endl;

//   myDet->setDAC(dacsEntry->GetIntNumber(), (slsDetectorDefs::dacIndex)id, dacsUnit->IsOn());

//   getValue();

}

void jctbAcquisition::setFname() {
  myDet->setFileName(eFname->GetText());
//   int val=myDet->setDAC(-1, (slsDetectorDefs::dacIndex)id, dacsUnit->IsOn());
//   char s[100];

//   sprintf(s,"%d",val);

//   dacsValue->SetText(s);
  

//   return val;

}

void jctbAcquisition::setFindex() {
  myDet->setFileIndex(eFindex->GetNumber());

}


void jctbAcquisition::setFsave(Bool_t b) {
  myDet->enableWriteToFile(b);
  eFname->SetState(b);
  eOutdir->SetState(b);
  
}

void jctbAcquisition::update() {

  
  eFname->SetText((myDet->getFileName()).c_str());
  eOutdir->SetText((myDet->getFilePath()).c_str());
  eFindex->SetNumber(myDet->getFileIndex()); 
  cFileSave->SetOn(myDet->enableWriteToFile());
  eFname->SetState(cFileSave->IsOn());
  eOutdir->SetState(cFileSave->IsOn());
  eFindex->SetState(cFileSave->IsOn());

}



void jctbAcquisition::toggleAcquisition() {



  if (acqThread->GetState()==1 || acqThread->GetState()==6) {
    bStatus->SetText("Stop");
    acqThread->Run();
    //myDet->startAcquisition();
    StopFlag=0;
  } else {
    StopFlag=1; 
    myDet->stopAcquisition();
    bStatus->SetText("Start");
    //  acqThread->Kill();
  }
}

void jctbAcquisition::acquisitionFinished() {
    bStatus->SetText("Start");
}

void jctbAcquisition::startAcquisition(){
  cout << "Detector started " << endl;
  myDet->acquire();
}

void* jctbAcquisition::ThreadHandle(void *arg)
{
   jctbAcquisition *acq = static_cast<jctbAcquisition*>(arg);
   int i=0;

   acq->startAcquisition();
   acq->acquisitionFinished();

   //  while(!(classInstance->StopFlag))
   // {
   // cout << "thread " << i++ << endl;
   // usleep(100000);
   // }
   //myDet->readFrame();

}
 
 int jctbAcquisition::progressCallback(double f,void* arg) {


   // jctbAcquisition *acq = static_cast<jctbAcquisition*>(arg);


   cout << "*********" << f << "*******" << endl;




 }
