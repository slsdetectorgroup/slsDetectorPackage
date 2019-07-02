#pragma once

#include <qgroupbox.h>
#include <qwidget.h>

#include "SlsQt2DPlot.h"

class QGridLayout;
class QString;
class QToolButton;

class SlsQt2DPlotLayout : public QGroupBox {
    Q_OBJECT

  public:
    SlsQt2DPlotLayout(QWidget * = NULL);
    ~SlsQt2DPlotLayout();

    SlsQt2DPlot *GetPlot();
    void SetXTitle(QString st);
    void SetYTitle(QString st);
    void SetZTitle(QString st);
    void SetInterpolate(bool enable);
    void SetContour(bool enable);
    void SetLogz(bool enable);
    void KeepZRangeIfSet();
    // recalculate zmin and zmax from plot and update z range
    void SetZRange(bool isMin, bool isMax, double min, double max);
  public slots:
    void UpdateZRange(double min, double max);

  private:
    void Layout();

    QGridLayout *the_layout;
    QToolButton *btnInterpolate;
    QToolButton *btnContour;
    QToolButton *btnLogz;
    SlsQt2DPlot *the_plot;

    bool isLog;
    double zmin;
    double zmax;
    bool isZmin;
    bool isZmax;
};
