
/**
 * @author Ian Johnson
 * @version 1.0
 */


#include <iostream>
#include <cmath>
#include "SlsQt2DHist.h"
  
using std::cout;
using std::endl;

SlsQt2DHist::SlsQt2DHist(int nbinsx, double xmin, double xmax, int nbinsy, double ymin, double ymax, double* d,double zmin,double zmax):QwtRasterData(){    
  interp=0;
  nx_array=ny_array=0;data=0;
  SetData(nbinsx,xmin,xmax,nbinsy,ymin,ymax,d,zmin,zmax);
}


SlsQt2DHist::~SlsQt2DHist(){if(data) delete data;}

int SlsQt2DHist::GetBinIndex(int bx, int by){
  int b = bx*ny+by;
  if(b<0 || b>=nb){
    cout<<"GetBinIndex:: Incorrect indicies bx and by returning overflow bin;"<<endl;
    return nb;
  }
  return b;
}

int SlsQt2DHist::FindBinIndex(double x, double y){
  return GetBinIndex(int((x-x_min)/x_width),int((y-y_min)/y_width));
}

double SlsQt2DHist::GetBinValue(int bx,int by){
  return data[GetBinIndex(bx,by)];
}


void SlsQt2DHist::SetBinValue(int bx,int by,double v){
  z_mean_has_been_calculated = 0;
  data[GetBinIndex(bx,by)] = v;
}

void SlsQt2DHist::SetData(int nbinsx, double xmin, double xmax, int nbinsy,double ymin, double ymax, double *d,double zmin,double zmax){
  z_mean_has_been_calculated = 0;
  if(xmax<xmin||ymax<ymin) cout<<"Warning input range invalid."<<endl;

  x_width = (xmax - xmin)/nbinsx;
  y_width = (ymax - ymin)/nbinsy;

  if(x_min!=xmin||x_max!=xmax||y_min!=ymin||y_max!=ymax){
    x_min=xmin;x_max=xmax;
    y_min=ymin;y_max=ymax;
   
#if QWT_VERSION<0x060000	  
    setBoundingRect(QRectF(xmin,ymin,x_max-x_min,y_max-y_min));
#else
    setInterval( Qt::XAxis,QwtInterval(xmin,xmax));
    setInterval( Qt::YAxis,QwtInterval(ymin,ymax));
    	//  setInterval( Qt::ZAxis,QwtInterval(zmin,zmax));
    //setInterval( Qt::ZAxis,QwtInterval(0.,1.));
#endif

  }

  if(nbinsx*nbinsy<1){
    cout<<"Exitting: SlsQt2DHist::SetData() number of bins must be greater than zero."<<endl; 
    exit(1);
  }

  if(nbinsx*nbinsy>nx_array*ny_array){
    if(data) delete data;
    data = new double [nbinsx*nbinsy+1];  //one for under/overflow bin
    nx_array = nbinsx;
    ny_array = nbinsy;
  }
  nx=nbinsx;
  ny=nbinsy;
  nb=nx*ny;
  data[nb]=0;//set over flow to zero
  if(d){
    memcpy(data,d,nb*sizeof(double));
    SetMinMax(zmin,zmax);
  }
}
  
void SlsQt2DHist::SetMinMax(double zmin,double zmax){
  if(zmin<zmax){
    z_min=zmin;
    z_max=zmax;
  }else{
    z_mean_has_been_calculated = 1;
    z_min=data[0];
    z_mean=0;
    z_max=data[0];
    for(int i=0;i<nb;i++){
      if(data[i]<z_min) z_min=data[i];
      if(data[i]>z_max) z_max=data[i];
      z_mean+=data[i];
    }
    z_mean/=nb;
    if(z_min>0) z_min/=1.02; else z_min*=1.02;
    if(z_max>0) z_max*=1.02; else z_max/=1.02;
  }
#if QWT_VERSION<0x060000	  
    ;
#else
	setInterval( Qt::ZAxis,QwtInterval(z_min,z_max));
#endif
 
}

double SlsQt2DHist::GetMean(){
  if(!z_mean_has_been_calculated){
    z_mean_has_been_calculated = 1;
    z_mean=0;
    for(int i=0;i<nb;i++) z_mean+=data[i];
    z_mean/=nb;
  }

  return z_mean;
}

double SlsQt2DHist::SetMinimumToFirstGreaterThanZero(){
  z_min=fabs(z_max)+1;
  for(int i=0;i<nb;i++){
    if(data[i]>0 && data[i]<z_min) z_min=data[i];
  }
#if QWT_VERSION<0x060000	  
    ;
#else
	setInterval( Qt::ZAxis,QwtInterval(z_min,z_max));
#endif
 
  return z_min;
}



