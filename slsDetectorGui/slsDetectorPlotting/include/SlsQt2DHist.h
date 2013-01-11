
/**
 * @author Ian Johnson
 * @version 1.0
 */


#ifndef SLSQT2DHIST_H
#define SLSQT2DHIST_H


#ifndef IAN
typedef  double double32_t;
typedef  float float32_t;
typedef  int int32_t;
#endif



#if QT_VERSION >= 0x040000
#include <qprintdialog.h>
#endif
#include <qwt_color_map.h>
#include <qwt_plot_spectrogram.h>
#include <qwt_scale_widget.h>
#include <qwt_scale_draw.h>


class SlsQt2DHist: public QwtRasterData{
  
 private:

  double  x_min,x_max,y_min,y_max;
  double  x_width,y_width;

  int     nx,ny,nb;
  double  *data;
  double  z_min,z_mean,z_max;
  bool    z_mean_has_been_calculated;
  
  int     nx_array,ny_array;

  bool interp;

  static double value_between_points(double p1,double v1,double p2,double v2,double p){ //linear extrap
    return (v2-v1)/(p2-p1)*(p-p1)+v1;
  }
  
 public:
  SlsQt2DHist(int nbinsx=10, double xmin=0, double xmax=10, int nbinsy=10, double ymin=0, double ymax=10, double* d=0,double zmin=0,double zmax=-1);
  virtual ~SlsQt2DHist();

  double  GetXMin()      {return x_min;}
  double  GetXMax()      {return x_max;}
  double  GetXBinWidth() {return x_width;}
  double  GetYMin()      {return y_min;}
  double  GetYMax()      {return y_max;}
  double  GetYBinWidth() {return y_width;}
  double  GetMinimum()   {return z_min;}
  double  GetMaximum()   {return z_max;}
  double  GetMean();

  int     GetNBinsX(){return nx;}
  int     GetNBinsY(){return ny;}
  double  GetBinValue(int bx,int by);
  int     GetBinIndex(int bx,int by);
  double* GetDataPtr(){return data;}

  void    Interpolate(bool on=1) {interp=on;}
  void    SetBinValue(int bx,int by,double v);
  void    SetData(int nbinsx, double xmin, double xmax, int nbinsy,double ymin, double ymax,double *d,double zmin=0, double zmax=-1);
  
  double  SetMinimumToFirstGreaterThanZero();
  void    SetMinimum(double zmin) {z_min=zmin;}
  void    SetMaximum(double zmax) {z_max=zmax;}
  void    SetMinMax(double zmin=0,double zmax=-1);

  int     FindBinIndex(double x, double y);



  virtual QwtRasterData *copy() const{ 
    //this function does not create a new SlsQt2DHistData instance,
    //just passes a pointer so that data is common to both the copy and the original instance
    return (QwtRasterData*) this;
  }

#if QWT_VERSION<0x060000
   virtual QwtDoubleInterval range() const{ return QwtDoubleInterval(z_min,z_max);}
#else
  virtual QwtInterval range() const{ return QwtInterval(z_min,z_max);}
  virtual QwtInterval interval(Qt::Axis axis) const { 
    switch (axis){
    case Qt::ZAxis:
      return QwtInterval(z_min,z_max);
    case Qt::XAxis:
      return QwtInterval(x_min,x_max);
    case Qt::YAxis:
      return QwtInterval(y_min,y_max);
    default:
      return QwtInterval(z_min,z_max);
    };
  };
#endif



  virtual double value(double x, double y) const{
    //if(!interp){ //default is box like plot
      int index = int((x-x_min)/x_width) + int((y-y_min)/y_width)*nx;
      if(index<0||index>nb) index = nb;
      if(!interp) return data[index];
      //}

    
    int x_int  = int((x-x_min)/x_width-0.5);
      if(x_int<0) x_int = 0; else if(x_int>nx-2) x_int = nx-2;
    int y_int  = int((y-y_min)/y_width-0.5);
      if(y_int<0) y_int = 0; else if(y_int>ny-2) y_int = ny-2;

    int b00 =  x_int*ny    + y_int;
    int b01 =  x_int*ny    + y_int+1;
    int b10 = (x_int+1)*ny + y_int;
    int b11 = (x_int+1)*ny + y_int+1;

    //vertical extrap
      double y0 = y_min+(y_int+0.5)*y_width;
      double y1 = y_min+(y_int+1.5)*y_width;
      double left_v   = value_between_points(y0,data[b00],y1,data[b01],y);
      double right_v  = value_between_points(y0,data[b10],y1,data[b11],y);
    //horazontal extrap



      return 0.5;


	return value_between_points(x_min+(x_int+0.5)*x_width,left_v,
				    x_min+(x_int+1.5)*x_width,right_v,x);
  }
  
};


#endif
