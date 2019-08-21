#ifndef GHOSTSUMMATION_H
#define GHOSTSUMMATION_H

#include <cmath>
#include "slsDetectorData.h"

template <class dataType> class ghostSummation {

  /** @short virtual calss to handle ghosting*/ 

 public:
  
  /** constructor
      \param xt crosstalk
  */
 ghostSummation(slsDetectorData<dataType> *d, double xt) : xtalk(xt),det(d)  {
    if (det)
      det->getDetectorSize(nx,ny);
    ghost=new double[nx*ny];
  };

  ghostSummation(ghostSummation *orig) {
    xtalk=orig->xtalk; 
    det=orig->det;
    ghost=new double[nx*ny];
    
  }
  ~ghostSummation() {delete [] ghost;};

  virtual ghostSummation *Clone() {
    new ghostSummation(this);
  }

  double getXTalk(){return xtalk;};
  void setXTalk(double g) {xtalk=g;};

  virtual double calcGhost(char *data, int ix, int iy=1){ghost[iy*nx+ix]=0;};

  virtual void calcGhost(char *data){
    for (int iy=0; iy<ny; iy++)
      for (int ix=0; ix<nx; ix++)
	ghost[iy*nx+ix]=calcGhost(data, ix, iy);
  }
  virtual double getGhost(int ix, int iy) {
    if (ix<0 || ix>=nx || iy<0 || iy>=ny) return 0;
    return ghost[iy*nx+ix];
  }

 protected:
    double xtalk;
    slsDetectorData<dataType> *det;
    double *ghost;
    int nx, ny;

};





#endif
