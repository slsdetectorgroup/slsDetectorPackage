#pragma once

#include <qwidget.h>
#include <qgroupbox.h>

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
    void 	KeepZRangeIfSet();
// recalculate zmin and zmax from plot and update z range
void SetZRange(bool isMin, bool isMax, double min, double max);
void SetInterpolate(bool enable);
void SetContour(bool enable);
void SetLogz(bool enable);

private:
    QGridLayout* 		the_layout;
    QToolButton* 		btnInterpolate;
    QToolButton* 		btnContour;
    QToolButton* 		btnLogz;
    SlsQt2DPlot*    	the_plot;

    void Layout();
    bool isLog;
    double zmin;
    double zmax;
    bool isZmin;
    bool isZmax;

public slots:
void SetZScaleToLog(bool enable);
void ResetRange();

// update z range
void UpdateZRange(double min, double max) ;

};
