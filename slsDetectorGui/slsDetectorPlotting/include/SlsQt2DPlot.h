
/**
 * @author Ian Johnson
 * @version 1.0
 * Modifications:
 * 19.06.2012: {Some functions have been added by Dhanya to enable zooming in and out
 * without using mouse control:
 * DisableZoom,
 * SetXMinMax,SetYMinMax,
 * GetXMinimum,GetXMaximum,GetYMinimum,GetYMaximum}
 */


#ifndef SLSQT2DPLOT_H
#define SLSQT2DPLOT_H

#ifndef IAN
typedef  double double32_t;
typedef  float float32_t;
typedef  int int32_t;
#endif



#include <qwt_plot.h>
#include <qlist.h>
#include <qwt_plot_spectrogram.h>

#include  "SlsQt2DZoomer.h"
#include  "SlsQt2DHist.h"


class  QwtPlotPanner;
class  QwtScaleWidget;
class  QwtLinearColorMap;


class SlsQt2DPlot: public QwtPlot{
    Q_OBJECT

private:
    QwtPlotSpectrogram *d_spectrogram;
    SlsQt2DHist* hist;
    SlsQt2DZoomer* zoomer;
    QwtPlotPanner* panner;
    QwtScaleWidget *rightAxis;

    QwtLinearColorMap* colorMapLinearScale;
    QwtLinearColorMap* colorMapLogScale;
#if QWT_VERSION<0x060000
    QwtValueList*    contourLevelsLinear;
    QwtValueList*    contourLevelsLog;
#else 
    QList<double>     contourLevelsLinear;
    QList<double>   contourLevelsLog;
#endif

    void  SetupZoom();
    void  SetupColorMap();

    QwtLinearColorMap* myColourMap(QVector<double> colourStops);
    QwtLinearColorMap* myColourMap(int log=0);

    int isLog;


public:
    SlsQt2DPlot(QWidget * = NULL);

    //    SlsQt2DHist *GetHistogram(){ return hist; }
    void   UnZoom();
    void   SetZoom(double xmin,double ymin,double x_width,double y_width);


    /**	This group of functions have been added by Dhanya on 19.06.2012 to be able to
    	use zooming functionality without mouse control*/
    void   DisableZoom(bool disableZoom);
    void SetXMinMax(double min,double max){setAxisScale(QwtPlot::xBottom,min,max);};
    void SetYMinMax(double min,double max){setAxisScale(QwtPlot::yLeft,min,max);};
    double GetXMinimum(){return hist->GetXMin();};
    double GetXMaximum(){return hist->GetXMax();};
    double GetYMinimum(){return hist->GetYMin();};
    double GetYMaximum(){return hist->GetYMax();};
    /**---*/


    double GetZMinimum(){ return hist->GetMinimum();}
    double GetZMaximum(){ return hist->GetMaximum();}
    void   SetZMinMax(double zmin=0,double zmax=-1);
    void   SetZMinimumToFirstGreaterThanZero(){hist->SetMinimumToFirstGreaterThanZero();}
    double GetZMean()   { return hist->GetMean();}

    void   SetData(int nbinsx, double xmin, double xmax, int nbinsy,double ymin, double ymax,double *d,double zmin=0, double zmax=-1){
      hist->SetData(nbinsx,xmin,xmax,nbinsy,ymin,ymax,d,zmin,zmax);
    }



    double* GetDataPtr()                        {return hist->GetDataPtr();}
    int     GetBinIndex(int bx,int by)          {return hist->GetBinIndex(bx,by);}
    int     FindBinIndex(double x,double y)     {return hist->FindBinIndex(x,y);}
    void    SetBinValue(int bx,int by,double v) { hist->SetBinValue(bx,by,v);}
    double  GetBinValue(int bx,int by)          {return hist->GetBinValue(bx,by);} 


    void             FillTestPlot(int i=0);
    void             Update();

public slots:
    void LogZ(bool on=1);
    void InterpolatedPlot(bool on);
    void showContour(bool on);
    void showSpectrogram(bool on);
    //    void printPlot();

};

#endif
