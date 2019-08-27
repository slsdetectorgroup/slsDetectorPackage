#ifndef CTBMAIN_H
#define CTBMAIN_H
#include <TGFrame.h>


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
class  TGTextButton;
class  TGTab;

class   TGMenuBar;
class   TGPopupMenu;
class   TGDockableFrame;
class TGLayoutHints;
class TGCanvas;
class TCanvas;

class   ctbDacs;
class   ctbSlowAdcs;
class   ctbPowers;

 
class  ctbSignals;

#include "Detector.h"

class  ctbPattern;
class  ctbAdcs;
class  ctbAcquisition;
//class  ctbActions;

#include <string>
using namespace std;

class ctbMain : public TGMainFrame { 
private:


  sls::Detector *myDet;



   TRootEmbeddedCanvas *fEcanvas;
   TRootEmbeddedCanvas *fModulecanvas;
   TGButtonGroup *br;
   
   TGTab *mtab;

   
   ctbDacs *dacs;
   int i_dacs;

   ctbPowers *pwrs;
   int i_pwrs;

   ctbSlowAdcs *senses;
   int i_senses;

 
  ctbSignals *sig;
  int i_sig;


  ctbAdcs *adcs;
  int i_adcs;
  

  ctbPattern *pat;
  int i_pat;

  ctbAcquisition *acq;
  int i_acq;

  // ctbActions *actions;
  int i_actions;

   TGDockableFrame    *fMenuDock;

   TGMenuBar          *fMenuBar;
   TGPopupMenu        *fMenuFile, *fMenuTest, *fMenuView, *fMenuHelp;
   TGPopupMenu        *fCascadeMenu, *fCascade1Menu, *fCascade2Menu;
   TGPopupMenu        *fMenuNew1, *fMenuNew2;
   TGLayoutHints      *fMenuBarLayout, *fMenuBarItemLayout, *fMenuBarHelpLayout;
   TGCanvas *myCanvas;


public:
   ctbMain(const TGWindow *p, sls::Detector *det);


   int  loadAlias(string fname);
   int  saveAlias(string fname);
   int  loadConfiguration(string fname);
   void tabSelected(Int_t);
   int setADCPlot(Int_t);
   int setSignalPlot(Int_t);
   void CloseWindow();

   void setPatternFile(const char* t);

   void setPatternCompiler(const char* t);
   void setAnalogSamples(const int);
   void setDigitalSamples(const int);
   void setReadoutMode(int);
   void setADCEnable(Int_t);

   void HandleMenu(Int_t);
   TCanvas* getCanvas();
   ClassDef(ctbMain,0)
};

#endif
