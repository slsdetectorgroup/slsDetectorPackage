#include <TApplication.h>
#include <TColor.h>

#include <TStyle.h>
#include <TROOT.h>

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>


#include "multiSlsDetector.h"
#include "sls_detector_defs.h"
#include "sls_receiver_defs.h"
#include "jctbMain.h"
using namespace std;


int main(int argc, char **argv) {


  string afname, cfname, pfname;
  int id=0;

  int af=0, cf=0, pf=0;


  cout << " *** " << argc << endl;
  for (int ia=0; ia<argc; ia++) {
    if (strcmp(argv[ia],"-alias")==0) {
      if (ia+1<argc) {
	afname=argv[ia+1];
	ia++;
	af=1;
      }

    } else if  (strcmp(argv[ia],"-config")==0) {
      if (ia+1<argc) {
	cfname=argv[ia+1];
	ia++;
	cf=1;
      }


    } else if  (strcmp(argv[ia],"-par")==0) {
      if (ia+1<argc) {
	pfname=argv[ia+1];
	ia++;
	pf=1;
      }


    } else if  (strcmp(argv[ia],"-id")==0) {
      if (ia+1<argc) {
	id=atoi(argv[ia+1]);
	ia++;
      }
    } 
  }
  

  cout << " *** "  << endl;
  
  /****** Create detector ****************/
  multiSlsDetector *myDet=new multiSlsDetector(id);
  myDet->setOnline(slsDetectorDefs::ONLINE_FLAG);


  if (cf) {
    myDet->readConfigurationFile(cfname);
  } else
    cout << "No config file specified" << endl;


  cout << "Created multi detector id " << id << " hostname " << myDet->getHostname() << endl;

  if (pf) {
    myDet->retrieveDetectorSetup(pfname);
  } else
    cout << "No parameter file specified" << endl;





  /***********Create GUI stuff *******************/
   TApplication theApp("App",&argc,argv);


 gStyle->SetDrawBorder(0);
  gStyle->SetCanvasColor(kWhite);
  gStyle->SetCanvasDefH(800);
  gStyle->SetCanvasDefW(800);
  gStyle->SetCanvasBorderMode(0);
  gStyle->SetPadBorderMode(0);
  gStyle->SetPaintTextFormat("5.2f");
  gStyle->SetLineWidth(2);
  gStyle->SetTextSize(1.1);
  gStyle->SetLabelSize(0.04,"xy");
  gStyle->SetTitleSize(0.05,"xy");
  gStyle->SetTitleOffset(1.0,"x");
  gStyle->SetTitleOffset(1.1,"y");
  gStyle->SetPadTopMargin(0.15);
  gStyle->SetPadRightMargin(0.15);
  gStyle->SetPadBottomMargin(0.15);
  gStyle->SetPadLeftMargin(0.15);
  gStyle->SetLegendBorderSize(1);
  gStyle->SetFrameBorderMode(0);
  gStyle->SetFrameFillColor(kWhite);
  // gStyle->SetLegendFillColor(kWhite);
  gStyle->SetTitleFillColor(kWhite);
  gStyle->SetFillColor(kWhite);
  gStyle->SetStatFontSize(0.03);
  gStyle->SetStatBorderSize(1);
  gStyle->SetStatFormat("6.4g");
  gStyle->SetStatX(0.95);
  gStyle->SetStatY(0.95);
  gStyle->SetStatW(0.2);
  gStyle->SetStatH(0.2);
  gStyle->SetTitleX(0.1);
  gStyle->SetTitleY(0.95);
  gStyle->SetTitleBorderSize(0);
  gStyle->SetTitleFontSize(0.05);
  gROOT->SetStyle("Default");


  TColor::InitializeColors();
  const Int_t NRGBs = 5;
  const Int_t NCont = 90;

  Double_t stops[NRGBs] = { 0.00, 0.34, 0.61, 0.84, 1.00 };
  Double_t red[NRGBs]   = { 0.00, 0.00, 0.87, 1.00, 0.51 };
  Double_t green[NRGBs] = { 0.00, 0.81, 1.00, 0.20, 0.00 };
  Double_t blue[NRGBs]  = { 0.51, 1.00, 0.12, 0.00, 0.00 };
  TColor::CreateGradientColorTable(NRGBs, stops, red, green, blue, NCont);
  gStyle->SetNumberContours(NCont);


  gROOT->ForceStyle();
  jctbMain *mf=new jctbMain(gClient->GetRoot(),myDet);

  cout << " *** " << argc << endl;
  for (int ia=0; ia<argc; ia++)
    cout << argv[ia] << endl;
  

  cout << " *** "  << endl;
  
  if (af)
    mf->loadAlias(afname);
  else
    cout << "no alias specified" << endl;

   theApp.Run();
   return 0;
}
