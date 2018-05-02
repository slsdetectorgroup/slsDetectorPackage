#ifndef NO_INTERPOLATION_H
#define NO_INTERPOLATION_H

/* #ifdef MYROOT1 */
/* #include <TObject.h> */
/* #include <TTree.h> */
/* #include <TH2F.h> */
/* #include <TROOT.h> */
/* #include <TRandom.h> */
/* #endif */

#include  <cstdlib>
#include "slsInterpolation.h"



class noInterpolation : public slsInterpolation{
 public:
 noInterpolation(int nx=400, int ny=400, int ns=25) : slsInterpolation(nx,ny,ns) {};// {eventGenerator=new TRandom();};
 noInterpolation(noInterpolation *orig) : slsInterpolation(orig){};
  virtual void prepareInterpolation(int &ok){ok=1;};
 
  //////////////////////////////////////////////////////////////////////////////
  //////////// /*It return position hit for the event in input */ //////////////

  virtual noInterpolation* Clone() {

    return new noInterpolation(this);

  };


  virtual void getInterpolatedPosition(int x, int y, double *data, double &int_x, double &int_y)
  {
    //Random coordinate in the Pixel reference
    int_x = x + ((double)rand())/((double)RAND_MAX) -0.5;//eventGenerator->Uniform(-0.5,0.5);
    int_y = y + ((double)rand())/((double)RAND_MAX) -0.5;//eventGenerator->Uniform(-0.5,0.5);
   
    return ;
  };

 virtual void getInterpolatedPosition(int x, int y, int *data, double &int_x, double &int_y)
  {
    return getInterpolatedPosition(x, y, (double*)NULL, int_x, int_y);
  }
  

  virtual void getInterpolatedPosition(int x, int y, double etax, double etay, int corner, double &int_x, double &int_y)
  {
    getInterpolatedPosition(x, y, (double*)NULL, int_x, int_y);
  };




  virtual void getInterpolatedPosition(int x, int y, double totquad,int quad,double *cl,double &etax, double &etay){
    getInterpolatedPosition(x, y, (double*)NULL, etax, etay);
  };


  virtual void getInterpolatedPosition(int x, int y, double totquad,int quad,int *cl,double &etax, double &etay){
    getInterpolatedPosition(x, y, (double*)NULL, etax, etay);
  };


  
  //////////////////////////////////////////////////////////////////////////////////////
  virtual void getPositionETA3(int x, int y, double *data, double &int_x, double &int_y)
  {
    //Random coordinate in the Pixel reference
    int_x = x + ((double)rand())/((double)RAND_MAX) -0.5;//eventGenerator->Uniform(-0.5,0.5);
    int_y = y + ((double)rand())/((double)RAND_MAX) -0.5;//eventGenerator->Uniform(-0.5,0.5);
   
    return ;
  };

  virtual void getPositionETA3(int x, int y, int *data, double &int_x, double &int_y)
  {
    return getPositionETA3(x, y, (double*)NULL, int_x, int_y);
  };


  
  //////////////////////////////////////////////////////////////////////////////////////
  virtual int addToFlatField(double *cluster, double &etax, double &etay){return 0;};
  
  virtual int addToFlatField(int *cluster, double &etax, double &etay){return 0;};
  
  virtual int addToFlatField(double etax, double etay){return 0;};
  
  virtual int addToFlatField(double totquad,int quad,double *cl,double &etax, double &etay){return 0;};


  virtual int addToFlatField(double totquad,int quad,int *cl,double &etax, double &etay){return 0;};


 protected:
  ;
  // TRandom *eventGenerator;
  //  ClassDefNV(slsInterpolation,1);
  // #pragma link C++ class slsInterpolation-;
};

#endif
