
#include <stdio.h>
#include <iostream>
#include <fstream>

#include "jctbDac.h"
#include "jctbDacs.h"
#include "multiSlsDetector.h"
#include "sls_detector_defs.h"

using namespace std;






jctbDacs::jctbDacs(TGVerticalFrame *page, multiSlsDetector *det)   : TGGroupFrame(page,"DACs",kVerticalFrame) , myDet(det){


  SetTitlePos(TGGroupFrame::kLeft);
  page->AddFrame(this,new TGLayoutHints( kLHintsTop | kLHintsExpandX , 10,10,10,10));
  MapWindow();
  
  // cout << "window mapped " << endl;

  for (int idac=0; idac<NDACS; idac++) {

     dacs[idac]=new jctbDac(this, idac, myDet);

  }


}


int jctbDacs::setDacAlias(string line) {

  int is=-1, mv=0;
  char tit[100];
  int narg=sscanf(line.c_str(),"DAC%d %s %d",&is,tit,&mv);
  if (narg<2)
    return -1;
  if (is>=0 && is<NDACS) 
    dacs[is]->setLabel(tit,mv);
  return is;

}

string jctbDacs::getDacAlias() {

  ostringstream line;

  for (int i=0; i<NDACS; i++)
    line << dacs[i]->getLabel() << endl;
  return line.str();

}




string jctbDacs::getDacParameters() {


  ostringstream line;

  for (int i=0; i<NDACS; i++) {
    //line << "dacs:" << i << " " << dacs[i]->getValue << endl;
    line << "dac:" << i << " " << dacs[i]->getValue() << endl;
  } 
  return line.str();


}



void  jctbDacs::update() {

  for (int idac=0; idac<NDACS; idac++) {

    dacs[idac]->getValue();

  }


}
