#include <TPaveText.h> 
#include <TLegend.h> 
#include <TF1.h> 
#include <TGraphErrors.h> 
#include <TH2F.h> 
#include <TASImage.h> 
#include <TImage.h> 
#include <TFile.h> 
#include <vector> 
#include <string> 
#include <sstream> 
#include <iomanip> 
#include <fstream> 
#include "tiffIO.h"

#include<iostream>

using namespace std;


int main(int argc, char *argv[]) {
/**
 * trial.o [socket ip] [starting port number] [outfname]
 *
 */

  if (argc<3) {
    cout << "Wrong usage! Should be: "<< argv[0] << " infile " << " outfile " << endl;
    return 1;
  }

  uint32  nx, ny;
  
  float *data=ReadFromTiff(argv[1],nx, ny);

  TH2F *h2=NULL;
  if (data) {
    TFile *fout=new TFile(argv[2],"RECREATE");
    if (fout) {
      h2=new TH2F("h2",argv[1],nx,0,nx,ny,0, ny);
      for (int ix=0; ix<nx ; ix++) {
	for (int iy=0; iy<ny ; iy++) {
	  
	  h2->SetBinContent(ix+1, iy+1, data[ix+iy*nx]);
	}
      }
      h2->Write();
      fout->Close();
    }
    delete [] data;
  }
    

 
  return 0;
}

