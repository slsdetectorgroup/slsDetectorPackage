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
    if (nSubPixels>2) {
      calcEta(totquad, sDum, etax, etay);
    } 
    getInterpolatedPosition(x, y, etax,etay, corner, int_x, int_y);
    
    return;
  };

  virtual void getInterpolatedPosition(int x, int y, int *data, double &int_x, double &int_y)
  {
    double sDum[2][2];
    double tot, totquad;
    double etax,etay;
    
    int corner;
    corner=calcQuad(data, tot, totquad, sDum); 
    if (nSubPixels>2) 
      calcEta(totquad, sDum, etax, etay); 
    getInterpolatedPosition(x,y,etax,etay,corner,int_x,int_y);

    return;
  };
  
   virtual void getInterpolatedPosition(int x, int y, double totquad,int quad,double *cl,double &int_x, double &int_y) {
    
     double eta_x, eta_y;
    if (nSubPixels>2) {
     double cc[2][2];
     double *cluster[3];
     cluster[0]=cl;
     cluster[1]=cl+3;
     cluster[2]=cl+6;
     int xoff, yoff;
     
     switch (quad) {
     case BOTTOM_LEFT:
       xoff=0;
       yoff=0;
       break;
     case BOTTOM_RIGHT:
       xoff=1;
       yoff=0;
       break;
     case TOP_LEFT:
       xoff=0;
       yoff=1;
       break;
     case TOP_RIGHT:
       xoff=1;
       yoff=1;
       break;
     default:
       ;
     } 
     cc[0][0]=cluster[yoff][xoff];
     cc[1][0]=cluster[yoff+1][xoff];
     cc[0][1]=cluster[yoff][xoff+1];
     cc[1][1]=cluster[yoff+1][xoff+1];
     calcEta(totquad,cc,eta_x,eta_y);
    }
    // cout << x << " " << y << " " << eta_x << " " << eta_y << " " << int_x << " " << int_y << endl;
    return getInterpolatedPosition(x,y,eta_x, eta_y,quad,int_x,int_y);







  }


  virtual void getInterpolatedPosition(int x, int y, double totquad,int quad,int *cl,double &int_x, double &int_y) {
    
     double cc[2][2];
     int *cluster[3];
     int xoff, yoff;
     cluster[0]=cl;
     cluster[1]=cl+3;
     cluster[2]=cl+6;
     
     switch (quad) {
     case BOTTOM_LEFT:
       xoff=0;
       yoff=0;
       break;
     case BOTTOM_RIGHT:
       xoff=1;
       yoff=0;
       break;
     case TOP_LEFT:
       xoff=0;
       yoff=1;
       break;
     case TOP_RIGHT:
       xoff=1;
       yoff=1;
       break;
     default:
       ;
     } 
     double etax, etay;
     if (nSubPixels>2) { 
       cc[0][0]=cluster[yoff][xoff];
       cc[1][0]=cluster[yoff+1][xoff];
       cc[0][1]=cluster[yoff][xoff+1];
       cc[1][1]=cluster[yoff+1][xoff+1];
       calcEta(totquad,cc,etax,etay);
     }
     //   cout << x << " " << y << " " << etax << " " << etay << " " << int_x << " " << int_y << endl;
     return getInterpolatedPosition(x,y,etax, etay,quad,int_x,int_y);

  }



  virtual void getInterpolatedPosition(int x, int y, double etax, double etay, int corner, double &int_x, double &int_y)
  {
   
    double xpos_eta,ypos_eta;
    double dX,dY;
    switch (corner)
      {
      case TOP_LEFT:
	dX=-1.; 
	dY=0; 
	break;
      case TOP_RIGHT:
	dX=0; 
	dY=0; 
	break;
      case BOTTOM_LEFT:
	dX=-1.; 
	dY=-1.; 
	break;
      case BOTTOM_RIGHT:
	dX=0; 
	dY=-1.; 
	break;
      default:
	cout << "bad quadrant" << endl;
	dX=0.; 
	dY=0.;
      }
    
    
    if (nSubPixels>2) {
      xpos_eta=(etax)+dX;
      ypos_eta=(etay)+dY;
    } else {
      xpos_eta=0.5*dX+0.25;
      ypos_eta=0.5*dY+0.25;
    }
    int_x=((double)x) +  xpos_eta;
    int_y=((double)y) +  ypos_eta;
    //  cout <<"**"<< x << " " << y << " " << xpos_eta << " " << ypos_eta << " " << corner << endl;
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
  virtual int addToFlatField(int *cluster, double &etax, double &etay){};
  virtual int addToFlatField(double etax, double etay){};
  virtual int addToFlatField(double totquad,int quad,double *cl,double &etax, double &etay) {};
  virtual int addToFlatField(double totquad,int quad,int *cl,double &etax, double &etay) {};

 protected:
  ;


};

#endif
