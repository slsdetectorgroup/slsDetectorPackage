 
/**
 * @author Ian Johnson
 * @version 1.0
 */


#include <iostream>
#include <stdio.h>

#include <stdlib.h>

#include "Eiger.h"


using namespace std;

int main(int argc, char* argv[]){

  cout<<"\n\n\n\n\n\n\n\n\n\n"<<endl;
  int n = (argc>1) ? atoi(argv[1]):5; 

  //Feb *f = new Feb();
  //  f->Test();
  //delete f;
  //return 0;

  Eiger* e = new Eiger();    
  e->SetNImages(n);
    e->SetDynamicRange(32);
    e->SetExposureTime(0.02);
    e->SetExposurePeriod(0.050);
    e->StartAcquisition();

  delete e;

  return 0;
}
