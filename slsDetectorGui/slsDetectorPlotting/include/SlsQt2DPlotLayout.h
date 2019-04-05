/**
 * @author Ian Johnson
 * @version 1.0
 * @comments
 * 19.06.2012 All modifications with the Ian flag has been made since
 * z_range_ne and the buttons are defined in another class.
 * Logz button and z_range_ne have wrappers to connect them
 * 05.05.2013 Added ResetZMinZMax
 */


#ifndef SLSQT2DPLOTLAYOUT_H
#define SLSQT2DPLOTLAYOUT_H

#ifndef IAN
typedef  double double32_t;
typedef  float float32_t;
typedef  int int32_t;
#endif


#include <qwidget.h>
#include <qgroupbox.h>

#include "SlsQtNumberEntry.h"
#include "SlsQt2DPlot.h"

class QGridLayout;
class QString;
class QToolButton;


class SlsQt2DPlotLayout: public QGroupBox{
  Q_OBJECT

public:

    SlsQt2DPlotLayout(QWidget * = NULL);
    ~SlsQt2DPlotLayout();

    SlsQt2DPlot* GetPlot(){return the_plot;}
    void         SetXTitle(QString st);
    void         SetYTitle(QString st);
    void         SetZTitle(QString st);
    void 		 UpdateNKeepSetRangeIfSet();


private:
    QGridLayout* 		the_layout;
    QToolButton* 		btnInterpolate;
    QToolButton* 		btnContour;
    QToolButton* 		btnLogz;
    SlsQt2DPlot*    	the_plot;
    SlsQtNumberEntry* 	z_range_ne;

#ifndef IAN
    bool zRangeChecked;
#endif

    void CreateTheButtons();
    void ConnectSignalsAndSlots();
    void Layout();

public slots:
void SetZScaleToLog(bool yes);
void ResetRange();

#ifndef IAN
void SetZRange(double,double);
void EnableZRange(bool enable);
void ResetZMinZMax(bool zmin, bool zmax, double min, double max);

#endif

signals:
void InterpolateSignal(bool);
void ContourSignal(bool);

};

#endif

