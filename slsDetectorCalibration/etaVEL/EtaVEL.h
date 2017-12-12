#include <iostream>
#include <TGraph.h>
#include <TAxis.h>
#include <TMultiGraph.h>
#include <TH2D.h>
#include <TMath.h>
#include <TObject.h>
#include <TBuffer.h>

#include <TMatrixD.h>

#include <TDecompSVD.h>
//#include <TDecompQRH.h>


#include <TH1.h>
#include <TMath.h>
#include <vector>

#include <ostream>
#include <istream>

using namespace std;

#ifndef ETAVPS
#define ETAVPS

typedef struct {
  int itN;
  double *xPos;
  double *yPos;
  double *binCont;
} itLog;



class EtaVEL : public TObject{

 public:
 EtaVEL(int numberOfPixels = 25, double minn=0., double maxx=1., int nnx=160, int nny=160) : nPixels(numberOfPixels), min(minn), max(maxx), converged(0), nx(nnx), ny(nny), chi_sq(0){
    //acc = 0.02; 
    ds = 0.005;
    
    init();
  }
  void init(){
    double pOffset = (max-min)/(double)nPixels;
    xPPos = new double[(nPixels+1)*(nPixels+1)+1];
    yPPos = new double[(nPixels+1)*(nPixels+1)+1];
    binCont = new double[nPixels*nPixels+1];
    totCont = 0.;
    edgeL = new double[2*nPixels*(nPixels+1)+1];

    for(int ii = 0; ii < 2*nPixels*(nPixels+1)+1; ii++){
      edgeL[ii] = 1.0;
      //cout << "ii " << ii << endl;
    }

    for(int x = 0; x < nPixels+1; x++){
      for(int y = 0; y < nPixels+1; y++){
	xPPos[getCorner(x,y)] = min + (double)x * pOffset;
	yPPos[getCorner(x,y)] = min + (double)y * pOffset;

	if(x < nPixels && y < nPixels)	binCont[getBin(x,y)] = 0;
      }
    }
    //    edgeL[1] = 3.0;
    updatePixelCorner();
    it = 0;
    
    log = new itLog[nIterations];
  }

  void fill(double x, double y, double amount = 1.){
    totCont+=amount;
    int bin = findBin(x,y);
    if(bin < 0) { 
      //cout << "can not find bin x: " << x << " y: " << y << endl; 
      totCont-=amount; 
    }
    binCont[bin]+=amount;
   
  }

  int getBin(int x, int y){
    if(x < 0 || x >= nPixels || y < 0 || y >= nPixels){
      //cout << "getBin: out of bounds : x " << x << " y " << y << endl;
      return 0;
    }
    return y*nPixels+x+1;
  }

  int getXBin(int bin){
    return (bin-1)%nPixels;
  }
  
  int getYBin(int bin){
    return (bin-1)/nPixels;
  }

  int getCorner(int x, int y){
    return y*(nPixels+1)+x+1;
  }

  int getEdgeX(int x,int row){
    int ret = row*nPixels+x+1;    
    //cout << "| edge X x " << x << " row " << row << ": "<< ret << " | ";
    return ret;
  }

  int getEdgeY(int col, int y){
    int ret = nPixels*(nPixels+1)+col*nPixels+y+1;
    //cout << "| edge Y col " << col << " y " << y << ": "<< ret << " | ";
    return ret;
  }
  

  int getIt(){ return it; };

  int getNPixels(){ return nPixels; }
  double *getXPPos(){ return xPPos; }
  double *getYPPos(){ return yPPos; }

  void updatePixelCorner();
  double *getPixelCorners(int x, int y);
  int findBin(double xx, double yy);
  void createLogEntry();
 
  void updatePixelPos();
  double *getSizeMap();
  double *getChangeMap();
  TH2D *getContent(int it=-1, int changeType = 0);
  TMultiGraph *plotPixelBorder(int plotCenters=0);
  TMultiGraph *plotLog(int stepSize=1, int maxIt=-1);
  void printGrid();
  TH1D *getCounts();

  void serialize(ostream &o);
  void deserialize(istream &is);

  int converged ;
  double getChiSq(){return chi_sq;};

 private:
  itLog *log;
  int it;
  const static int nIterations =10000;
  int nx, ny;
  int nPixels;
  double *xPPos;
  double *yPPos;
  double *binCont;
  double totCont;
  double *edgeL;
  //  double acc; 
  double ds;
  double min,max;
  double chi_sq;

  ClassDefNV(EtaVEL,1);
  #pragma link C++ class EtaVEL-;
};

#endif
