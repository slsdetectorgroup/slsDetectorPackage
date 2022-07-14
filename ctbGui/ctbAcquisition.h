// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#ifndef CTBACQUISITION_H
#define CTBACQUISITION_H
#include  <TGFrame.h>

#include "ctbAdcs.h"
#include "ctbSignals.h"
#include "ctbPattern.h"
class TGTextEntry;
class TGLabel;
class TGNumberEntry;
class TGCheckButton;
class TThread;
class TGraph;
class TMultiGraph;
class THStack;
class TGButtonGroup;
class TGRadioButton;
class TGComboBox;
class TTimer;
class TCanvas;
class TH2F;
class TH1F;
class TGLabel;
class TGTextButton;

namespace sls
{
   class Detector;
   class detectorData;
};

template <class dataType> class slsDetectorData;

class singlePhotonDetector;
//class singlePhotonDetector;
class commonModeSubtraction;

#include <string>
#include <stdint.h>
using namespace std;

class ctbAcquisition : public TGGroupFrame {


  enum {DESERIALIZER, MOENCH04, MOENCH02, MOENCH03, IMAGE32B, IMAGE16B, ADCSAR2, MYTHEN301, MYTHEN302};


 private:
  TGTextEntry *eOutdir;
  TGTextEntry *eFname;
  TGNumberEntry *eFindex;
  TGCheckButton *cFileSave;


  TGNumberEntry *eSerOff;
  TGNumberEntry *eDynRange;
  TGNumberEntry *eNumCount;


  TGNumberEntry *ePixX;
  TGNumberEntry *ePixY;

  TGNumberEntry *eFitADC;
  TGNumberEntry *eBitPlot;
  TGNumberEntry *eMinRaw;
  TGNumberEntry *eMaxRaw;
  TGNumberEntry *eMinPedSub;
  TGNumberEntry *eMaxPedSub;
  TGCheckButton *cMinMaxRaw;
  TGCheckButton *cMinMaxPedSub;




  TGNumberEntry *eMeasurements;



  TGTextButton *bStatus;
  // TGTextButton 
  TGCheckButton *cCompile;
  TGTextButton *cLoad;
  //  TGCheckButton *cRun;
  
  TThread *acqThread;


  THStack *adcStack;
  THStack *bitStack;
  THStack *countsStack;


  TH1F *adcHisto[NADCS];
  TH1F *countsHisto[NADCS];
 
  TH1F *bitHisto[NSIGNALS];
  float bitOffset[NSIGNALS];
  
  // int enableFlag[NADCS+4];
  int roMode;

  int dBitOffset;



  TH1F *adcFit;
  TH1F *bitPlot;
  TH1F *countsFit;



  TH2F *h2DMapAn; // for 2D detectors
  TH2F *h2DMapDig; // for 2D detectors
  TH1F *h1DMap; //for 1D detectors

  //  TH2F *h2Scan; // for 2D detectors
  // TMultiGraph *mgAdcs;
  // TH1I *plotAdc[NADCS];


  sls::Detector* myDet;
  
  int plotFlag[NADCS];
  int bitPlotFlag[NSIGNALS];
  
  int ip;
  // int nChannels;
  // int chanEnable;
  //int nADCs;

  std::vector <int> dbitlist;
  std::vector <int> adclist;

  TGButtonGroup *bgPlot;// = new TGVButtonGroup(main_frame);
  TGRadioButton *rbPlotOff;
  TGRadioButton *rbWaveform;
  TGRadioButton *rbDistribution;
  TGRadioButton *rb2D;
  //  TGRadioButton *rbScan;
  TGComboBox *cbDetType;
  TGCheckButton *cbGetPedestal;
  TGCheckButton *cbSubtractPedestal;
  TGCheckButton *cbCommonMode;
  TGTextButton *bResetPedestal;

  TGLabel *lClickX;
  TGLabel *lClickY;
  TGLabel *lClickValue;


  TCanvas *myCanvas;
  TTimer *plotTimer;

  char patternFile[10000];
  char patternCompiler[10000];
  
  int globalPlot;
  int adcPlot;
  int dbitPlot;
  int tenG;

  int nAnalogSamples, nDigitalSamples;
  // int iScanStep;

  slsDetectorData<uint16_t> *dataStructure;
  singlePhotonDetector *photonFinder;
    //singlePhotonDetector *photonFinder;
  commonModeSubtraction *commonMode;
  int cmSub;

  int stop;

  uint64_t dBitMask;

  int deserializer;

 public:
   ctbAcquisition(TGVerticalFrame*,  sls::Detector*);
   void setOutdir();
   void setFname();
   void setMeasurements();
   void setFsave(Bool_t);
   void changePlot(Int_t);
   void changeDetector(Int_t);
   void changePlot();
   void changeDetector();
   void setFindex();
   void Draw();
   void setCanvas(TCanvas*);

   void toggleAcquisition();
   void loadPattern();
   static void* ThreadHandle(void *arg);
   void update();
   void acquisitionFinished();
   //  string getParameters();

   void setGraph (int i ,int en, Pixel_t col);
   void setBitGraph (int i ,int en, Pixel_t col);
   void startAcquisition();
   static   void progressCallback(double,void*);
   static void dataCallback(sls::detectorData*, long unsigned int, unsigned int,  void*);
   int StopFlag;
   
   int plotData(sls::detectorData*, int);

   void setPatternFile(const char* t);

   void setPatternCompiler(const char* t);

   void setAnalogSamples(int);
   void setDigitalSamples(int);
   
   void setADCEnable(Int_t);
   void setDbitEnable(Int_t);
   void setReadoutMode(int);
   void updateChans();

   void resetPedestal();

   void ToggleCommonMode(Bool_t);
   void TogglePedSub(Bool_t);
   void ChangeHistoLimitsPedSub(Long_t );
   void ChangeHistoLimitsRaw(Long_t);
   void ChangeHistoLimitsPedSub( );
   void ChangeHistoLimitsRaw();
   void ChangeHistoLimitsPedSub(Bool_t );
   void ChangeHistoLimitsRaw(Bool_t);
   

   void ChangeSerialOffset();
   void ChangeSerialOffset(Long_t);
   void ChangeNumberOfChannels();
   void ChangeNumberOfChannels(Long_t);
   void ChangeDynamicRange();
   void ChangeDynamicRange(Long_t);
   void ChangeImagePixels();
   void ChangeImagePixels(Long_t);

   void canvasClicked();
   void FitADC();
   void plotBit();
   ClassDef(ctbAcquisition,0)
};

#endif
