/**
 * @author Ian Johnson
 * @version 1.0
 */


#ifndef SLSQT2DPLOTLAYOUT_H
#define SLSQT2DPLOTLAYOUT_H

#include <qwidget.h>
#include <qgroupbox.h>

#include "SlsQtNumberEntry.h"
#include "SlsQt2DPlot.h"

class QGridLayout;
class QString;



class SlsQt2DPlotLayout: public QGroupBox{
  Q_OBJECT

public:

    SlsQt2DPlotLayout(QWidget * = NULL);
    ~SlsQt2DPlotLayout();

    SlsQt2DPlot*    GetPlot()   {return the_plot;}
    void         SetXTitle(QString st);
    void         SetYTitle(QString st);
    void         SetZTitle(QString st);

    void UpdateNKeepSetRangeIfSet();

private:
    QGridLayout* the_layout;
    SlsQt2DPlot*    the_plot;

    SlsQtNumberEntry* z_range_ne;

    bool logsChecked;

    void ConnectSignalsAndSlots();

    void Layout();

public slots:
void SetZScaleToLog(bool yes);
void ResetRange();
void SetTitle(QString st);

signals:
void InterpolateSignal(bool);
void ContourSignal(bool);
};

#endif

