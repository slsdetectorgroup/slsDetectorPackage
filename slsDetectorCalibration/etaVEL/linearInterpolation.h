#ifndef LINEAR_INTERPOLATION_H
#define LINEAR_INTERPOLATION_H

//#include <TObject.h>
//#include <TTree.h>
//#include <TH2F.h>

#include "slsInterpolation.h"

class linearInterpolation : public slsInterpolation{

 public:
 linearInterpolation(int nx=400, int ny=400, int ns=25) : slsInterpolation(nx,ny,ns) {};
  linearInterpolation(linearInterpolation *orig) : slsInterpolation(orig) {};

  virtual void prepareInterpolation(int &ok){ok=1;};

  virtual linearInterpolation* Clone() {

    return new linearInterpolation(this);

  };


   //////////////////////////////////////////////////////////////////////////////
  //////////// /*It return position hit for the event in input */ //////////////
  virtual void getInterpolatedPosition(int x, int y, double *data, double &int_x, double &int_y)
  {
    double sDum[2][2];
    double tot, totquad;
    double etax,etay;
    
    int corner;
    corner=calcQuad(data, tot, totquad, sDum); 
    
    calcEta(totquad, sDum, etax, etay); 
    getInterpolatedPosition(x, y, etax,etay, corner, int_x, int_y);
    
    return;
  };
  

  virtual void getInterpolatedPosition(int x, int y, double etax, double etay, int corner, double &int_x, double &int_y)
  {
   
    double xpos_eta,ypos_eta;
    double dX,dY;
    switch (corner)
      {
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
	dX=0.; 
	dY=0.;
      }
    
    
    xpos_eta=(etax);
    ypos_eta=(etay);
    
    int_x=((double)x) + 0.5*dX + xpos_eta;
    int_y=((double)y) + 0.5*dY + ypos_eta;
    
    return;
  };







  ////////////////////////////////////////////////////////////////////////////////////////////////////////
  virtual void getPositionETA3(int x, int y, double *data, double &int_x, double &int_y)
   {
    double sDum[2][2];
    double tot, totquad;
    double eta3x,eta3y;
    
    calcQuad(data, tot, totquad, sDum); 
    calcEta3(data,eta3x, eta3y,tot); 
    
    double xpos_eta,ypos_eta;
    
    xpos_eta=eta3x;
    ypos_eta=eta3y;
    
    int_x=((double)x) + xpos_eta;
    int_y=((double)y) + ypos_eta;
    
    return;
  };
  ////////////////////////////////////////////////////////////////////////////////////////////////////////
 
  virtual int addToFlatField(double *cluster, double &etax, double &etay){};
  virtual int addToFlatField(double etax, double etay){};
  

 protected:
  ;


};

#endif
