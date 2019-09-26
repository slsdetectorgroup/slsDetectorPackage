#ifndef INTERPOLATION_ETAVEL_H
#define INTERPOLATION_ETAVEL_H

#include <slsInterpolation.h>
#include "EtaVEL.h"
#include "TH2F.h"
//#include "EtaVEL.cpp"
//class EtaVEL;

class  interpolation_EtaVEL: public slsInterpolation {

 public:
  interpolation_EtaVEL(int nx=40, int ny=160, int ns=25, double etamin=-0.02, double etamax=1.02, int p=0);
  ~interpolation_EtaVEL();
 
 
  //create eta distribution, eta rebinnining etc.
  //returns flat field image
  void prepareInterpolation(int &ok){prepareInterpolation(ok,10000);};
  void prepareInterpolation(int &ok, int maxit);

  //create interpolated image
  //returns interpolated image
  
  //return position inside the pixel for the given photon
  void getInterpolatedPosition(Int_t x, Int_t y, Double_t *data, Double_t &int_x, Double_t &int_y);
  void getInterpolatedBin(Double_t *cluster, Int_t &int_x, Int_t &int_y);
 


  int addToFlatField(Double_t *cluster, Double_t &etax, Double_t &etay);
  int addToFlatField(Double_t etax, Double_t etay);
  int setPlot(int p=-1) {if (p>=0) plot=p; return plot;};
  int WriteH(){newEta->Write("newEta"); heta->Write("heta");};
  EtaVEL *setEta(EtaVEL *ev){if (ev) {delete newEta; newEta=ev;} return newEta;};
  TH2F *setEta(TH2F *ev){if (ev) {delete heta; heta=ev;} return heta;};
  void iterate();
  void DrawH();
  double getChiSq(){return newEta->getChiSq();};
 


 protected:
  EtaVEL *newEta;
  TH2F *heta;
  int plot;

  //  ClassDefNV(interpolation_EtaVEL,1);
  // #pragma link C++ class interpolation_EtaVEL-;
};

#endif
