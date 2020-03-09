
/**
 * @author Ian Johnson
 * @version 1.0
 * Dhanya-05.12.2012- included an additional header
 */


#ifndef SLSQT2DZOOMER_H
#define SLSQT2DZOOMER_H



/**included by Dhanya on 05.12.2012 to avoid compile time errors with the latest gcc*/
#include <cstdio>
/**end of Change by Dhanya*/

#include <qwt_plot_zoomer.h>
#include <qwt_plot_panner.h>

#include "SlsQt2DHist.h"

class SlsQt2DZoomer:public QwtPlotZoomer{
 private:
  SlsQt2DHist* hist;

 public:

  SlsQt2DZoomer(QWidget *canvas):QwtPlotZoomer(canvas){
    setTrackerMode(AlwaysOn);
  }

  void SetHist(SlsQt2DHist* h){
    hist=h;
  }
  
 

  virtual QwtText trackerTextF(const QPointF &pos) const{
    QColor bg(Qt::white);
    bg.setAlpha(200);


    
    //QwtText text = QwtPlotZoomer::trackerText(pos);

    static QwtText text; 
    if(hist){
      static char t[200];
      sprintf(t,"%3.2f, %3.2f, %3.2f",pos.x(),pos.y(),hist->value(pos.x(),pos.y()));
      text.setText(t);
    }else {

    QPoint p=pos.toPoint();
    QwtText text = QwtPlotZoomer::trackerText(p);


    }
    text.setBackgroundBrush( QBrush( bg ));
    return text;
  }
};

#endif
