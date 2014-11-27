#ifndef JCTBMAIN_H
#define JCTBMAIN_H
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

class   jctbDacs;

 
class  jctbSignals;

class multiSlsDetector;

class  jctbPattern;
class  jctbAdcs;
class  jctbAcquisition;

#include <string>
using namespace std;

class jctbMain : public TGMainFrame {
private:


  multiSlsDetector *myDet;



   TRootEmbeddedCanvas *fEcanvas;
   TRootEmbeddedCanvas *fModulecanvas;
   TGButtonGroup *br;
   
   TGTab *mtab;


   jctbDacs *dacs;

 
  jctbSignals *sig;
  jctbAdcs *adcs;



  jctbPattern *pat;
  jctbAcquisition *acq;

   TGDockableFrame    *fMenuDock;

   TGMenuBar          *fMenuBar;
   TGPopupMenu        *fMenuFile, *fMenuTest, *fMenuView, *fMenuHelp;
   TGPopupMenu        *fCascadeMenu, *fCascade1Menu, *fCascade2Menu;
   TGPopupMenu        *fMenuNew1, *fMenuNew2;
   TGLayoutHints      *fMenuBarLayout, *fMenuBarItemLayout, *fMenuBarHelpLayout;

public:
   jctbMain(const TGWindow *p, multiSlsDetector *det);


   int  loadAlias(string fname);
   int  saveAlias(string fname);
   int  loadParameters(string fname);
   int  saveParameters(string fname);
   int  loadConfiguration(string fname);
   int  saveConfiguration(string fname);
   void tabSelected(Int_t);

   void CloseWindow();

   void HandleMenu(Int_t);

   ClassDef(jctbMain,0)
};

#endif
