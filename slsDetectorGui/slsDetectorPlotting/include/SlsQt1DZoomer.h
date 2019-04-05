
/**
 * @author Ian Johnson
 * @version 1.0
 */

#ifndef SLSQT1DZOOMER_H
#define SLSQT1DZOOMER_H

#ifndef IAN
typedef  double double32_t;
typedef  float float32_t;
typedef  int int32_t;
#endif


#include <qwt_plot_zoomer.h>
#include <qwt_plot_panner.h>
#include <qwt_global.h>

class SlsQtH1D;

class SlsQt1DZoomer:public QwtPlotZoomer{
 private:
  double x0,x1,y0,y1;
  double firstXgt0,firstYgt0;
  bool xIsLog,yIsLog;

 public:
#if QWT_VERSION < 0x060100
  SlsQt1DZoomer(QwtPlotCanvas *canvas):QwtPlotZoomer(canvas){
#else
  SlsQt1DZoomer(QWidget *canvas):QwtPlotZoomer(canvas){
#endif
    setTrackerMode(AlwaysOn);
    xIsLog=yIsLog=0;
  }

  double x() {return x0;}
  double x_firstGreaterThan0() {return firstXgt0;}
  double w() {return x1-x0;}

  double y() {return y0;}
  double y_firstGreaterThan0() {return firstYgt0;}
  double h() {return y1-y0;}

  void SetZoomBase(double xmin,double ymin,double x_width, double y_width);
  void SetZoomBase(SlsQtH1D* h);
  void ExtendZoomBase(SlsQtH1D* h);
  void ResetZoomBase();

  bool IsLogX(){ return xIsLog;}
  bool IsLogY(){ return yIsLog;}
  bool SetLogX(bool yes) { return xIsLog=yes;}
  bool SetLogY(bool yes) { return yIsLog=yes;}

 
#if QWT_VERSION<0x060000
    virtual QwtText trackerText(const QwtDoublePoint &pos) const{
#else
  virtual QwtText trackerText(const QPointF &pos) const{
#endif
    QColor bg(Qt::white);

#if QT_VERSION >= 0x040300
    bg.setAlpha(200);
#endif

#if QWT_VERSION<0x060000
    QwtText text = QwtPlotZoomer::trackerText(pos);
#else
    QPoint p=pos.toPoint();
    QwtText text = QwtPlotZoomer::trackerText(p);
#endif
    text.setBackgroundBrush( QBrush( bg ));
    return text;
  }

};

#endif
