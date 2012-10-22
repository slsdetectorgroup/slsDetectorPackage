
/**
 * @author Ian Johnson
 * @version 1.0
 */


#ifndef SLSQT2DZOOMER_H
#define SLSQT2DZOOMER_H


#include <qwt_plot_zoomer.h>
#include <qwt_plot_panner.h>

#include "SlsQt2DHist.h"

class SlsQt2DZoomer:public QwtPlotZoomer{
 private:
  SlsQt2DHist* hist;

 public:
  SlsQt2DZoomer(QwtPlotCanvas *canvas):QwtPlotZoomer(canvas){
    setTrackerMode(AlwaysOn);
  }

  void SetHist(SlsQt2DHist* h){
    hist=h;
  }
  
 
#if QWT_VERSION<0x060000
    virtual QwtText trackerText(const QwtDoublePoint &pos) const{
#else
  virtual QwtText trackerText(const QPointF &pos) const{
#endif
    QColor bg(Qt::white);
#if QT_VERSION >= 0x040300
    bg.setAlpha(200);
#endif

    
    //QwtText text = QwtPlotZoomer::trackerText(pos);

    static QwtText text; 
    if(hist){
      static char t[200];
      sprintf(t,"%3.2f, %3.2f, %3.2f",pos.x(),pos.y(),hist->value(pos.x(),pos.y()));
      text.setText(t);
    }else {
#if QWT_VERSION<0x060000
    QwtText text = QwtPlotZoomer::trackerText(pos);
#else
    QPoint p=pos.toPoint();
    QwtText text = QwtPlotZoomer::trackerText(p);
#endif

    }
    text.setBackgroundBrush( QBrush( bg ));
    return text;
  }
};

#endif
