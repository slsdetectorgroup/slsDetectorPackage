#include "interpolation_EtaVEL.h"
#include "TH2F.h"
#include "TCanvas.h"
#include "TROOT.h"
//#include "EtaVEL.h"
#include "EtaVEL.cpp"
/*
Zum erstellen der correction map ist createGainAndEtaFile(...) in EVELAlg.C der entry point.
Zum erstellen des HR images ist createImage(...) der entry point.
*/
interpolation_EtaVEL::interpolation_EtaVEL(int nx, int ny, int ns, double etamin, double etamax, int p) : slsInterpolation(nx, ny, ns), newEta(NULL), heta(NULL), plot(p) {
  newEta = new EtaVEL(nSubPixels,etamin,etamax,nPixelsX, nPixelsY);
  heta= new TH2F("heta","heta",50*nSubPixels, etamin,etamax,50*nSubPixels, etamin,etamax);
  heta->SetStats(kFALSE);
}

interpolation_EtaVEL::~interpolation_EtaVEL() {
  delete newEta;
  delete heta;
}


void interpolation_EtaVEL::prepareInterpolation(int &ok, int maxit) {
  int nit=0;
  while ((newEta->converged != 1) && nit++<maxit) {
    cout << " -------------- new step "<< nit << endl;
    iterate();
  }
  if (plot) {
    Draw();
    gPad->Modified();
    gPad->Update();
  }
  if (newEta->converged==1) ok=1; else ok=0;
}

int interpolation_EtaVEL::addToFlatField(Double_t *cluster, Double_t &etax, Double_t &etay) {
  Double_t sum, totquad, sDum[2][2];
  int corner =calcEta(cluster, etax, etay, sum, totquad, sDum);
  //check if it's OK...should redo it every time?
  //or should we fill a finer histogram and afterwards re-fill the newEta?
  addToFlatField(etax, etay);
  return corner;
}

int interpolation_EtaVEL::addToFlatField(Double_t etax, Double_t etay) {
  // newEta->fill(etaX,etaY);
  heta->Fill(etax,etay);
  return 0;
}

void interpolation_EtaVEL::iterate() {
  cout << " -------------- newEta refilled"<< endl;
  for (int ibx=0; ibx<heta->GetNbinsX(); ibx++) {
    for (int iby=0; iby<heta->GetNbinsY(); iby++) {
      newEta->fill(heta->GetXaxis()->GetBinCenter(ibx+1),heta->GetYaxis()->GetBinCenter(iby+1),heta->GetBinContent(ibx+1,iby+1));
      }
  }
  newEta->updatePixelPos();
  cout << " -------------- pixelPosition updated"<< endl;
}

void interpolation_EtaVEL::DrawH() {
      heta->Draw("col");
      (newEta->plotPixelBorder())->Draw();
}


void interpolation_EtaVEL::getInterpolatedPosition(Int_t x, Int_t y, Double_t *cluster, Double_t &int_x, Double_t &int_y) {

  Double_t etax, etay, sum, totquad, sDum[2][2];

  int corner =calcEta(cluster, etax, etay, sum, totquad, sDum);
  
  int bin = newEta->findBin(etax,etay);
  if (bin<=0) {
    int_x=-1;
    int_y=-1;
    return;
  }
  double subX = ((double)(newEta->getXBin(bin))+.5)/((double)newEta->getNPixels());
  double subY = ((double)(newEta->getYBin(bin))+.5)/((double)newEta->getNPixels());
  
  double dX, dY;
  switch (corner) {
  case TOP_LEFT:
    dX=-1.; 
    dY=+1.; 
    break;
  case TOP_RIGHT:
    dX=+1.; 
    dY=+1.; 
    break;
  case BOTTOM_LEFT:
    dX=-1.; 
    dY=-1.; 
    break;
  case BOTTOM_RIGHT:
    dX=+1.; 
    dY=-1.; 
    break;
  default:
    dX=0; 
    dY=0;
  }
  
  int_x=((double)x)+ subX+0.5*dX; 
  int_y=((double)y)+ subY+0.5*dY;
  
  // cout << corner << " " << subX<< " " << subY << " " << dX << " " << dY << " " << int_x << " " << int_y << endl;

};


// void interpolation_EtaVEL::Streamer(TBuffer &b){newEta->Streamer(b);};
void interpolation_EtaVEL::getInterpolatedBin(Double_t *cluster, Int_t &int_x, Int_t &int_y) {

  Double_t etax, etay, sum, totquad, sDum[2][2];

  int corner =calcEta(cluster, etax, etay, sum, totquad, sDum);
  
  int bin = newEta->findBin(etax,etay);
  if (bin<0) {
    int_x=-1;
    int_y=-1;
    return;
  }
  int_x=newEta->getXBin(bin);
  int_y=newEta->getYBin(bin);
  


};

