// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
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
#include <TGCanvas.h>
#include <stdlib.h>

#include <TGMenu.h>
#include <TGDockableFrame.h>
//#include <TGMenuBar.h>
//#include <TGPopupMenu.h>




#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>

#include "sls/Detector.h"
#include "ctbDefs.h"
#include "ctbMain.h"
#include "ctbDacs.h"
#include "ctbSlowAdcs.h"
#include "ctbPowers.h"
#include "ctbSignals.h"
#include "ctbPattern.h"
#include "ctbAdcs.h"
#include "ctbAcquisition.h"
//#include "ctbActions.h"

using namespace std;



ctbMain::ctbMain(const TGWindow *p, sls::Detector *det)
  : TGMainFrame(p,800,800), pwrs(NULL), senses(NULL) {

  myDet=det;

  Connect("CloseWindow()", "ctbMain", this, "CloseWindow()");





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

    fMenuFile->Connect("Activated(Int_t)", "ctbMain", this,
                       "HandleMenu(Int_t)");
 

    i_dacs=-1;
    i_pwrs=-1;
    i_senses=-1;
    i_sig=-1;
    i_adcs=-1;
    i_pat=-1;
    i_acq=-1;

    int i_page=0;

















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

  cout << "DACS" << endl;

  TGCompositeFrame *tf = mtab->AddTab("DACs");
  TGVerticalFrame *page=new TGVerticalFrame(tf, 1500,1200);
  tf->AddFrame(page, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY, 10,10,10,1));
  dacs=new ctbDacs(page, myDet);
  i_dacs=i_page++;
  

  cout << "power " << endl;
  tf = mtab->AddTab("Power Supplies");
  page=new TGVerticalFrame(tf, 1500,1200);
  tf->AddFrame(page, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY, 10,10,10,1));
  pwrs=new ctbPowers(page, myDet);

  i_pwrs=i_page++;

  cout << "sense " << endl;
  tf = mtab->AddTab("Sense");
  page=new TGVerticalFrame(tf, 1500,1200);
  tf->AddFrame(page, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY, 10,10,10,1));
  senses=new ctbSlowAdcs(page, myDet);

  i_senses=i_page++;


 
  cout << "signals " << endl;
  tf = mtab->AddTab("Signals");
  page=new TGVerticalFrame(tf, 1500,1200);
  tf->AddFrame(page, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY, 10,10,10,1));
  sig=new ctbSignals(page, myDet);
  sig->Connect("ToggledSignalPlot(Int_t)","ctbMain",this,"setSignalPlot(Int_t)");

  i_sig=i_page++;

  cout << "adcs " << endl;
  tf = mtab->AddTab("ADCs");
  page=new TGVerticalFrame(tf, 1500,1200);
  tf->AddFrame(page, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY, 10,10,10,1));
  adcs=new ctbAdcs(page, myDet);
  adcs->Connect("ToggledAdcPlot(Int_t)","ctbMain",this,"setADCPlot(Int_t)");
  adcs->Connect("AdcEnable(Int_t)","ctbMain",this,"setADCEnable(Int_t)");
  i_adcs=i_page++;


  cout << "pattern" << endl;

  tf = mtab->AddTab("Pattern");
  page=new TGVerticalFrame(tf, 1500,1200);
  tf->AddFrame(page, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY, 10,10,10,1));
  pat=new ctbPattern(page, myDet);
  pat->Connect("patternFileChanged(const char*)","ctbMain",this,"setPatternFile(const char*)");
  pat->Connect("patternCompilerChanged(const char*)","ctbMain",this,"setPatternCompiler(const char*)");
  pat->Connect("analogSamplesChanged(const int)","ctbMain",this,"setAnalogSamples(int)");
  pat->Connect("digitalSamplesChanged(const int)","ctbMain",this,"setDigitalSamples(int)");
  pat->Connect("readoutModeChanged(int)","ctbMain",this,"setReadoutMode(int)");

  i_pat=i_page++;

  cout << "acquisition" << endl;

  tf = mtab->AddTab("Acquisition");
  page=new TGVerticalFrame(tf, 1500,1200);
  tf->AddFrame(page, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY, 10,10,10,1));
  acq=new ctbAcquisition(page, myDet);


  i_acq=i_page++;


  // cout << "actions" << endl;
  // tf = mtab->AddTab("Actions");
  // page=new TGVerticalFrame(tf, 1500,1200);
  // tf->AddFrame(page, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY, 10,10,10,1));
  // actions=new ctbActions(page, myDet);


  // i_actions=i_page++;



   cout << "tabs finished" << endl;

  hpage->AddFrame(mtab,new TGLayoutHints(kLHintsExpandX | kLHintsExpandY, 10,10,10,1));

  vframe->AddFrame(hpage,new TGLayoutHints(kLHintsExpandX | kLHintsExpandY, 10,10,10,1));
  
  AddFrame(vframe,new TGLayoutHints(kLHintsExpandX | kLHintsExpandY, 10,10,10,1));
  vframe->MapWindow();
  hpage->MapWindow();
  mtab->MapWindow();
  page->MapWindow();

  // Sets window name and shows the main frame
   cout << "dockabel" << endl;
   TGDockableFrame *fdock=new TGDockableFrame(hpage);
   hpage->AddFrame(fdock, new TGLayoutHints(kLHintsBottom | kLHintsCenterX | kLHintsExpandX | kLHintsExpandY, 10,10,10,10));
   fdock->MapWindow();

   cout << "canvas" << endl;
//    // Creates widgets of the example


   fEcanvas = new TRootEmbeddedCanvas ("Ecanvas",fdock,800,800);//hpage,800,800);
   //fEcanvas = new TRootEmbeddedCanvas ("Ecanvas",this,800,800);//hpage,800,800);
  // fEcanvas->Resize();
  //  fEcanvas->GetCanvas()->Update();
  //AddFrame(fEcanvas, new TGLayoutHints(kLHintsBottom | kLHintsCenterX | kLHintsExpandX | kLHintsExpandY, 10,10,10,10));

  // // hpage->
  fdock->AddFrame(fEcanvas, new TGLayoutHints(kLHintsBottom | kLHintsCenterX | kLHintsExpandX | kLHintsExpandY, 10,10,10,10));


  fEcanvas->MapWindow();

  acq->setCanvas(getCanvas());



  hpage->MapSubwindows();
   mtab->Connect("Selected(Int_t)","ctbMain",this,"tabSelected(Int_t)");



   cout << "connect mtab" << endl;

  try{  
   setReadoutMode(pat->getReadoutMode());  
  } CATCH_DISPLAY ("Could not get readout flags", "ctbPattern::getReadoutMode")

   setADCEnable(adcs->setEnable());
   setAnalogSamples(pat->getAnalogSamples());
   setDigitalSamples(pat->getDigitalSamples());
   
   tabSelected(0);

   SetWindowName("CTB Gui");
   MapSubwindows();
   Resize(1500,1200);

   MapWindow();
}

void ctbMain::CloseWindow() {
   gApplication->Terminate();
}

TCanvas* ctbMain::getCanvas() {
  return fEcanvas->GetCanvas();
}



void ctbMain::HandleMenu(Int_t id)
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
             printf("Open file: %s (dir: %s)\n", fi.fFilename, fi.fIniDir);
	     // dir = fi.fIniDir;
	     if (fi.fFilename)
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
             printf("Save file: %s (dir: %s)\n", fi.fFilename, fi.fIniDir);
	     // dir = fi.fIniDir;
	     if (fi.fFilename)
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
	     if (fi.fFilename)
	       loadParameters(fi.fFilename);
          }
         break;

   case 3: // fMenuFile->AddEntry("Open Configuration", im++);
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
	     if (fi.fFilename)
	       loadConfiguration(fi.fFilename);
          }
         break;
 
   case 4: //fMenuFile->AddEntry("Open Pattern", im++);
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
	     if (fi.fFilename)
	       loadParameters(fi.fFilename);
          }
     break;

   case 5:   //fMenuFile->AddEntry("Save Pattern", im++);
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
	     if (fi.fFilename)
	       savePattern(fi.fFilename);
          }
         break;

   case 6: //  fMenuFile->AddEntry("Exit", im++);
     CloseWindow();

      default:
         printf("Menu item %d selected\n", id);
         break;
   }
}



int  ctbMain::setADCPlot(Int_t i) {

  //  cout << "ADC " << i << " plot or color toggled" << endl;
  //  acq->setGraph(i,adcs->getGraph(i));
  acq->setGraph(i,adcs->getEnabled(i),adcs->getColor(i));
  return -1;
}


int  ctbMain::setSignalPlot(Int_t i) {

  //  cout << "ADC " << i << " plot or color toggled" << endl;
  //  acq->setGraph(i,adcs->getGraph(i));
  acq->setBitGraph(i,sig->getPlot(i),sig->getColor(i));
  return -1;
}



void  ctbMain::loadConfiguration(string fname) {
  try{
    myDet->loadConfig(fname);
  } CATCH_DISPLAY ("Could not load config.", "ctbMain::loadConfiguration")
}

void  ctbMain::loadParameters(string fname) {
  try{
    myDet->loadParameters(fname);
  } CATCH_DISPLAY ("Could not load parameters.", "ctbMain::loadParameters")
}

void  ctbMain::savePattern(string fname) {
  try{
    myDet->savePattern(fname);
  } CATCH_DISPLAY ("Could not save pattern.", "ctbMain::savePattern")
}



int  ctbMain::loadAlias(string fname) {


  string line;
  char aaaa[1000];
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
      else if (sscanf(line.c_str(),"PAT%s",aaaa)>0) {
	pat->setPatternAlias(line);
	//	cout << "---------" << line<< endl;
      }	else if (sscanf(line.c_str(),"V%s",aaaa)>0) {
	    if (pwrs)	pwrs->setPwrAlias(line);
	//	cout << "+++++++++" << line<< endl;
      } else if (sscanf(line.c_str(),"SENSE%d",&i)>0) {
	if (senses) senses->setSlowAdcAlias(line);
	//	cout << "+++++++++" << line<< endl;
      } 
      
    }
    myfile.close();
  }

  else cout << "Unable to open file"; 

  return 0;

}





int  ctbMain::saveAlias(string fname) {


  string line;
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
    if (pwrs) myfile << pwrs->getPwrAlias();
    if (senses) myfile << senses->getSlowAdcAlias();
	//	cout << "+++++++++" << line<< endl;
    //  } else if (sscanf(line.c_str(),"ADC%d",&i)>0) {
    myfile << adcs->getAdcAlias();
	//	cout << "---------" << line<< endl;
    //  }      //  else
      //	cout << "<<<<<<<" << line << endl;
    myfile << pat->getPatternAlias();
      
    //}
    myfile.close();
  }

  else cout << "Unable to open file"; 

  return 0;

}











void ctbMain::tabSelected(Int_t i) {

  //  cout << "Selected tab " << i << endl;
  // cout << "Current tab is " <<  mtab->GetCurrent() << endl;

  if (i==i_dacs) dacs->update();
  else if (i==i_pwrs) pwrs->update();
  else if (i==i_senses) ;//senses->update();
 else if (i==i_sig) sig->update();
 else if (i==i_adcs) adcs->update();
 else if (i==i_pat) pat->update();
 else if (i==i_acq) acq->update();
 else if (i==i_acq) acq->update();
  // else if (i==i_actions) actions->update();
 else cout << "Unknown tab " << i << endl;
 

}

void ctbMain::setPatternFile(const char* t) {
  acq->setPatternFile(t);

}

void ctbMain::setPatternCompiler(const char* t) {
  acq->setPatternCompiler(t);


}

void ctbMain::setAnalogSamples(const int n) {
  acq->setAnalogSamples(n);


}

void ctbMain::setDigitalSamples(const int n) {
  acq->setDigitalSamples(n);


}

void ctbMain::setReadoutMode(int flags) {
  acq->setReadoutMode(flags);
}

void ctbMain::setADCEnable(Int_t reg){
  acq->setADCEnable(reg);
}
