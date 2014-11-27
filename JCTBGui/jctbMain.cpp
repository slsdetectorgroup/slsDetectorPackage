#include <TApplication.h>
#include <TGClient.h>
#include <TCanvas.h>
#include <TF1.h>
#include <TRandom.h>
#include <TGButton.h>
#include <TRootEmbeddedCanvas.h>
#include <TGButtonGroup.h>
#include <TGNumberEntry.h>
#include <TGLabel.h>
#include <TList.h>
#include <TGFileDialog.h>
#include <TGComboBox.h>
#include <TH2F.h>
#include <TColor.h>
#include <TH1F.h>
#include <TGraphErrors.h>
#include <THStack.h>
#include <TGTab.h>
#include <TApplication.h>



#include <TGMenu.h>
#include <TGDockableFrame.h>
//#include <TGMenuBar.h>
//#include <TGPopupMenu.h>







#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>

#include "multiSlsDetector.h"
#include "jctbMain.h"
#include "jctbDacs.h"
#include "jctbSignals.h"
#include "jctbPattern.h"
#include "jctbAdcs.h"
#include "jctbAcquisition.h"

using namespace std;



jctbMain::jctbMain(const TGWindow *p, multiSlsDetector *det)
  : TGMainFrame(p,800,800) {

  myDet=det;

  Connect("CloseWindow()", "jctbMain", this, "CloseWindow()");





//   fMenuDock = new TGDockableFrame(this);
//   AddFrame(fMenuDock, new TGLayoutHints(kLHintsExpandX, 0, 0, 1, 0));
//   fMenuDock->SetWindowName("GuiTest Menu");

  fMenuBarLayout = new TGLayoutHints(kLHintsTop | kLHintsExpandX);
  fMenuBarItemLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft, 0, 4, 0, 0);
  fMenuBarHelpLayout = new TGLayoutHints(kLHintsTop | kLHintsRight);

  fMenuFile = new TGPopupMenu(gClient->GetRoot());
  int im=0;

   fMenuFile->AddEntry("Open Alias", im++);
   fMenuFile->AddEntry("Save Alias", im++);
   fMenuFile->AddSeparator();
   fMenuFile->AddEntry("Open Parameters", im++);
   fMenuFile->AddEntry("Save Parameters", im++);
   fMenuFile->AddSeparator();
   fMenuFile->AddEntry("Open Configuration", im++);
   fMenuFile->AddEntry("Save Configuration", im++);
   fMenuFile->AddSeparator();
   fMenuFile->AddEntry("Open Pattern", im++);
   fMenuFile->AddEntry("Save Pattern", im++);
   fMenuFile->AddSeparator();
   fMenuFile->AddEntry("Exit", im++);

    fMenuFile->Connect("Activated(Int_t)", "jctbMain", this,
                       "HandleMenu(Int_t)");
 //    fMenuFile->Connect("PoppedUp()", "TestMainFrame", this, "HandlePopup()");
//     fMenuFile->Connect("PoppedDown()", "TestMainFrame", this, "HandlePopdown()");




//    fCascade2Menu = new TGPopupMenu(gClient->GetRoot());
//    fCascade2Menu->AddEntry("ID = 2&3", im++);
//    fCascade2Menu->AddEntry("ID = 2&4", im++);
//    fCascade2Menu->AddEntry("ID = 2&5", im++);

//    fCascade1Menu = new TGPopupMenu(gClient->GetRoot());
//    fCascade1Menu->AddEntry("ID = 4&1", 41);
//    fCascade1Menu->AddEntry("ID = 4&2", 42);
//    fCascade1Menu->AddEntry("ID = 4&3", 43);
//    fCascade1Menu->AddSeparator();
//    fCascade1Menu->AddPopup("Cascade&d 2", fCascade2Menu);

//    fCascadeMenu = new TGPopupMenu(gClient->GetRoot());
//    fCascadeMenu->AddEntry("ID = 5&1", 51);
//    fCascadeMenu->AddEntry("ID = 5&2", 52);
//    fCascadeMenu->AddEntry("ID = 5&3", 53);
//    fCascadeMenu->AddSeparator();
//    fCascadeMenu->AddPopup("&Cascaded 1", fCascade1Menu);

//    fMenuTest = new TGPopupMenu(gClient->GetRoot());
//    fMenuTest->AddLabel("Test different features...");
//    fMenuTest->AddSeparator();
//    fMenuTest->AddEntry("&Dialog...", im++);
//    fMenuTest->AddEntry("&Message Box...", im++);
//    fMenuTest->AddEntry("&Sliders...", im++);
//    fMenuTest->AddEntry("Sh&utter...", im++);
//    fMenuTest->AddEntry("&List Directory...", im++);
//    fMenuTest->AddEntry("&File List...", im++);
//    fMenuTest->AddEntry("&Progress...", im++);
//    fMenuTest->AddEntry("&Number Entry...", im++);
//    fMenuTest->AddEntry("F&ont Dialog...", im++);
//    fMenuTest->AddSeparator();
//    fMenuTest->AddEntry("Add New Menus", im++);
//    fMenuTest->AddSeparator();
//    fMenuTest->AddPopup("&Cascaded menus", fCascadeMenu);

//    fMenuView = new TGPopupMenu(gClient->GetRoot());
//    fMenuView->AddEntry("&Dock", im++);
//    fMenuView->AddEntry("&Undock", im++);
//    fMenuView->AddSeparator();
//    fMenuView->AddEntry("Enable U&ndock", im++);
//    fMenuView->AddEntry("Enable &Hide", im++);
//    fMenuView->DisableEntry(im);

//    fMenuDock->EnableUndock(kTRUE);
//    fMenuDock->EnableHide(kTRUE);
//    fMenuView->CheckEntry(im);
//    fMenuView->CheckEntry(im);

//    // When using the DockButton of the MenuDock,
//    // the states 'enable' and 'disable' of menus have to be updated.
//    fMenuDock->Connect("Undocked()", "TestMainFrame", this, "HandleMenu(=M_VIEW_UNDOCK)");

//    fMenuHelp = new TGPopupMenu(gClient->GetRoot());
//    fMenuHelp->AddEntry("&Contents", im++);
//    fMenuHelp->AddEntry("&Search...", im++);
//    fMenuHelp->AddSeparator();
//    fMenuHelp->AddEntry("&About", im++);

//    fMenuNew1 = new TGPopupMenu();
//    fMenuNew1->AddEntry("Remove New Menus", im++);

//    fMenuNew2 = new TGPopupMenu();
//    fMenuNew2->AddEntry("Remove New Menus", im++);

   // Menu button messages are handled by the main frame (i.e. "this")
   // HandleMenu() method.
//    fMenuFile->Connect("Activated(Int_t)", "TestMainFrame", this,
//                       "HandleMenu(Int_t)");
//    fMenuFile->Connect("PoppedUp()", "TestMainFrame", this, "HandlePopup()");
//    fMenuFile->Connect("PoppedDown()", "TestMainFrame", this, "HandlePopdown()");
//    fMenuTest->Connect("Activated(Int_t)", "TestMainFrame", this,
//                       "HandleMenu(Int_t)");
//    fMenuView->Connect("Activated(Int_t)", "TestMainFrame", this,
//                       "HandleMenu(Int_t)");
//    fMenuHelp->Connect("Activated(Int_t)", "TestMainFrame", this,
//                       "HandleMenu(Int_t)");
//    fCascadeMenu->Connect("Activated(Int_t)", "TestMainFrame", this,
//                          "HandleMenu(Int_t)");
//    fCascade1Menu->Connect("Activated(Int_t)", "TestMainFrame", this,
//                           "HandleMenu(Int_t)");
//    fCascade2Menu->Connect("Activated(Int_t)", "TestMainFrame", this,
//                           "HandleMenu(Int_t)");
//    fMenuNew1->Connect("Activated(Int_t)", "TestMainFrame", this,
//                       "HandleMenu(Int_t)");
//    fMenuNew2->Connect("Activated(Int_t)", "TestMainFrame", this,
//                       "HandleMenu(Int_t)");




















  TGVerticalFrame *vframe=new TGVerticalFrame(this, 800,1200); //main frame



   fMenuBar = new TGMenuBar(vframe, 1, 1, kHorizontalFrame);
   fMenuBar->AddPopup("&File", fMenuFile, fMenuBarItemLayout);
//    fMenuBar->AddPopup("&Test", fMenuTest, fMenuBarItemLayout);
//    fMenuBar->AddPopup("&View", fMenuView, fMenuBarItemLayout);
//    fMenuBar->AddPopup("&Help", fMenuHelp, fMenuBarHelpLayout);

   vframe->AddFrame(fMenuBar, fMenuBarLayout);








 
  TGHorizontalFrame* hpage=new TGHorizontalFrame(vframe, 800,1200); //horizontal frame. Inside there should be the tab and the canvas
  mtab=new TGTab(hpage, 1500, 1200); //tab!
  //  page=new TGVerticalFrame(mtab, 1500,1200);

  TGCompositeFrame *tf = mtab->AddTab("DACs");
  TGVerticalFrame *page=new TGVerticalFrame(tf, 1500,1200);
  tf->AddFrame(page, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY, 10,10,10,1));
  dacs=new jctbDacs(page, myDet);

  
  tf = mtab->AddTab("Signals");
  page=new TGVerticalFrame(tf, 1500,1200);
  tf->AddFrame(page, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY, 10,10,10,1));
  sig=new jctbSignals(page, myDet);


  tf = mtab->AddTab("ADCs");
  page=new TGVerticalFrame(tf, 1500,1200);
  tf->AddFrame(page, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY, 10,10,10,1));
  adcs=new jctbAdcs(page, myDet);




  tf = mtab->AddTab("Pattern");
  page=new TGVerticalFrame(tf, 1500,1200);
  tf->AddFrame(page, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY, 10,10,10,1));
  pat=new jctbPattern(page, myDet);



  tf = mtab->AddTab("Acquisition");
  page=new TGVerticalFrame(tf, 1500,1200);
  tf->AddFrame(page, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY, 10,10,10,1));
  acq=new jctbAcquisition(page, myDet);





  hpage->AddFrame(mtab,new TGLayoutHints(kLHintsExpandX | kLHintsExpandY, 10,10,10,1));

  vframe->AddFrame(hpage,new TGLayoutHints(kLHintsExpandX | kLHintsExpandY, 10,10,10,1));
  
  AddFrame(vframe,new TGLayoutHints(kLHintsExpandX | kLHintsExpandY, 10,10,10,1));
  vframe->MapWindow();
  hpage->MapWindow();
  mtab->MapWindow();
  page->MapWindow();

  // Sets window name and shows the main frame
   SetWindowName("JCTB Gui");
   MapSubwindows();
   MapWindow();
   Resize(1500,1200);


//    // Creates widgets of the example
  fEcanvas = new TRootEmbeddedCanvas ("Ecanvas",hpage,800,800);
  fEcanvas->Resize();
  fEcanvas->GetCanvas()->Update();
  
  hpage->AddFrame(fEcanvas, new TGLayoutHints(kLHintsBottom | kLHintsCenterX | kLHintsExpandX | kLHintsExpandY, 10,10,10,10));


  fEcanvas->MapWindow();
  hpage->MapSubwindows();
   mtab->Connect("Selected(Int_t)","jctbMain",this,"tabSelected(Int_t)");


   tabSelected(0);
}

void jctbMain::CloseWindow() {
   gApplication->Terminate();
}





void jctbMain::HandleMenu(Int_t id)
{
   // Handle menu items.
   
  
 

   switch (id) {

   case 0:   // fMenuFile->AddEntry("Open Alias", im++);
     cout << "Open Alias" << endl;
         {
             static TString dir(".");
             TGFileInfo fi;
             //fi.fFileTypes = filetypes;
             fi.fIniDir    = StrDup(dir);
             printf("fIniDir = %s\n", fi.fIniDir);
             new TGFileDialog(gClient->GetRoot(), this, kFDOpen, &fi);
             printf("Open file: %s (dir: %s)\n", fi.fFilename);
	     // dir = fi.fIniDir;
	     loadAlias(fi.fFilename);
          }
         break;

   case 1: // fMenuFile->AddEntry("Save Alias", im++);
     cout << "Save Alias" << endl;
         {
             static TString dir(".");
             TGFileInfo fi;
             //fi.fFileTypes = filetypes;
             fi.fIniDir    = StrDup(dir);
             printf("fIniDir = %s\n", fi.fIniDir);
             new TGFileDialog(gClient->GetRoot(), this, kFDSave, &fi);
             printf("Save file: %s (dir: %s)\n", fi.fFilename);
	     // dir = fi.fIniDir;
	     saveAlias(fi.fFilename);
          }
         break;

   case 2: //fMenuFile->AddEntry("Open Parameters", im++);
     cout << "Open Parameters" << endl;
         {
             static TString dir(".");
             TGFileInfo fi;
             //fi.fFileTypes = filetypes;
             fi.fIniDir    = StrDup(dir);
             printf("fIniDir = %s\n", fi.fIniDir);
             new TGFileDialog(gClient->GetRoot(), this, kFDOpen, &fi);
             printf("Open file: %s (dir: %s)\n", fi.fFilename, fi.fIniDir);
	     // dir = fi.fIniDir;
	     loadParameters(fi.fFilename);
          }
         break;

   case 3: //fMenuFile->AddEntry("Save Parameters", im++);
     cout << "Save Parameters" << endl;
         {
             static TString dir(".");
             TGFileInfo fi;
             //fi.fFileTypes = filetypes;
             fi.fIniDir    = StrDup(dir);
             printf("fIniDir = %s\n", fi.fIniDir);
             new TGFileDialog(gClient->GetRoot(), this, kFDSave, &fi);
             printf("Open file: %s (dir: %s)\n", fi.fFilename, fi.fIniDir);
	     // dir = fi.fIniDir;
	     saveParameters(fi.fFilename);
          }
         break;

   case 4: // fMenuFile->AddEntry("Open Configuration", im++);
     cout << "Open configuration" << endl;
         {
             static TString dir(".");
             TGFileInfo fi;
             //fi.fFileTypes = filetypes;
             fi.fIniDir    = StrDup(dir);
             printf("fIniDir = %s\n", fi.fIniDir);
             new TGFileDialog(gClient->GetRoot(), this, kFDOpen, &fi);
             printf("Open file: %s (dir: %s)\n", fi.fFilename, fi.fIniDir);
	     // dir = fi.fIniDir;
          }
         break;
 
   case 5: //  fMenuFile->AddEntry("Save Configuration", im++);
     cout << "Save configuration" << endl;
         {
             static TString dir(".");
             TGFileInfo fi;
             //fi.fFileTypes = filetypes;
             fi.fIniDir    = StrDup(dir);
             printf("fIniDir = %s\n", fi.fIniDir);
             new TGFileDialog(gClient->GetRoot(), this, kFDSave, &fi);
             printf("Open file: %s (dir: %s)\n", fi.fFilename, fi.fIniDir);
	     // dir = fi.fIniDir;
          }
         break;

   case 6: //fMenuFile->AddEntry("Open Pattern", im++);
     cout << "Open pattern" << endl;
         {
             static TString dir(".");
             TGFileInfo fi;
             //fi.fFileTypes = filetypes;
             fi.fIniDir    = StrDup(dir);
             printf("fIniDir = %s\n", fi.fIniDir);
             new TGFileDialog(gClient->GetRoot(), this, kFDOpen, &fi);
             printf("Open file: %s (dir: %s)\n", fi.fFilename, fi.fIniDir);
	     // dir = fi.fIniDir;
          }
     break;

   case 7:   //fMenuFile->AddEntry("Save Pattern", im++);
     cout << "Save pattern" << endl;
         {
             static TString dir(".");
             TGFileInfo fi;
             //fi.fFileTypes = filetypes;
             fi.fIniDir    = StrDup(dir);
             printf("fIniDir = %s\n", fi.fIniDir);
             new TGFileDialog(gClient->GetRoot(), this, kFDSave, &fi);
             printf("Open file: %s (dir: %s)\n", fi.fFilename, fi.fIniDir);
	     // dir = fi.fIniDir;
          }
         break;

   case 8: //  fMenuFile->AddEntry("Exit", im++);
     CloseWindow();

      default:
         printf("Menu item %d selected\n", id);
         break;
   }
}







int  jctbMain::loadConfiguration(string fname) {

    myDet->readConfigurationFile(fname);

//   string line;
//   int i;
//   ifstream myfile (fname.c_str());
//   if (myfile.is_open())
//   {
//     while ( getline (myfile,line) )
//     {

	
      
//     }
//     myfile.close();
//   }

//   else cout << "Unable to open file"; 

  return 0;

}





int  jctbMain::saveConfiguration(string fname) {


  string line;
  int i;
  ofstream myfile (fname.c_str());
  if (myfile.is_open())
  {


   myfile.close();
  }

  else cout << "Unable to open file"; 

  return 0;

}







int  jctbMain::loadParameters(string fname) {

    myDet->retrieveDetectorSetup(fname);

//   string line;
//   int i;
//   ifstream myfile (fname.c_str());
//   if (myfile.is_open())
//   {
//     while ( getline (myfile,line) )
//     {

	
      
//     }
//     myfile.close();
//   }

//   else cout << "Unable to open file"; 

  return 0;

}





int  jctbMain::saveParameters(string fname) {


  string line;
  int i;
  ofstream myfile (fname.c_str());
  if (myfile.is_open())
  {

   myfile << dacs->getDacParameters();
   myfile.close();
  }

  else cout << "Unable to open file"; 

  return 0;

}





int  jctbMain::loadAlias(string fname) {


  string line;
  int i;
  ifstream myfile (fname.c_str());
  if (myfile.is_open())
  {
    while ( getline (myfile,line) )
    {
      // cout << line ;
      if (sscanf(line.c_str(),"BIT%d",&i)>0) {
	//cout << "*******" << line<< endl;
	sig->setSignalAlias(line);
      // cout << line ;
      } else if (sscanf(line.c_str(),"DAC%d",&i)>0) {
	dacs->setDacAlias(line);
	//	cout << "+++++++++" << line<< endl;
      } else if (sscanf(line.c_str(),"ADC%d",&i)>0) {
	adcs->setAdcAlias(line);
	//	cout << "---------" << line<< endl;
      }      //  else
      //	cout << "<<<<<<<" << line << endl;
	
      
    }
    myfile.close();
  }

  else cout << "Unable to open file"; 

  return 0;

}





int  jctbMain::saveAlias(string fname) {


  string line;
  int i;
  ofstream myfile (fname.c_str());
  if (myfile.is_open())
  {
    //while ( getline (myfile,line) )
    // {
      // cout << line ;
      //if (sscanf(line.c_str(),"BIT%d",&i)>0) {
	//cout << "*******" << line<< endl;
    myfile << sig->getSignalAlias();
      // cout << line ;
    //  } else if (sscanf(line.c_str(),"DAC%d",&i)>0) {
    myfile << dacs->getDacAlias();
	//	cout << "+++++++++" << line<< endl;
    //  } else if (sscanf(line.c_str(),"ADC%d",&i)>0) {
    myfile << adcs->getAdcAlias();
	//	cout << "---------" << line<< endl;
    //  }      //  else
      //	cout << "<<<<<<<" << line << endl;
	
      
    //}
    myfile.close();
  }

  else cout << "Unable to open file"; 

  return 0;

}











void jctbMain::tabSelected(Int_t i) {

  //  cout << "Selected tab " << i << endl;
  // cout << "Current tab is " <<  mtab->GetCurrent() << endl;

  switch (i) {

  case 0: //dacs
    dacs->update();
    break;
  case 1: //signals
    sig->update();
    break;

  case 2: //adcs
    adcs->update();
    break;

  case 3: //pattern
    pat->update();
    break;

  case 4: //acq
    acq->update();
    break;

  default:
    ;


  }


}
