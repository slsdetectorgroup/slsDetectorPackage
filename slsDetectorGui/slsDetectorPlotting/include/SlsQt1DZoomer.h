// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package

#ifndef SLSQT1DZOOMER_H
#define SLSQT1DZOOMER_H

#include <qwt_global.h>
#include <qwt_plot_panner.h>
#include <qwt_plot_zoomer.h>

namespace sls {

class SlsQtH1D;

class SlsQt1DZoomer : public QwtPlotZoomer {
  private:
    double x0, x1, y0, y1;
    double firstXgt0, firstYgt0;
    bool xIsLog, yIsLog;

  public:
    SlsQt1DZoomer(QWidget *canvas) : QwtPlotZoomer(canvas) {
        setTrackerMode(AlwaysOn);
        xIsLog = yIsLog = 0;
    }

    double x() { return x0; }
    double x_firstGreaterThan0() { return firstXgt0; }
    double w() { return x1 - x0; }

    double y() { return y0; }
    double y_firstGreaterThan0() { return firstYgt0; }
    double h() { return y1 - y0; }

    void SetZoomBase(double xmin, double ymin, double x_width, double y_width);
    void SetZoomBase(SlsQtH1D *h);
    void ExtendZoomBase(SlsQtH1D *h);
    void ResetZoomBase();

    bool IsLogX() { return xIsLog; }
    bool IsLogY() { return yIsLog; }
    bool SetLogX(bool yes) { return xIsLog = yes; }
    bool SetLogY(bool yes) { return yIsLog = yes; }

    using QwtPlotPicker::trackerText;
    virtual QwtText trackerText(const QPoint &pos) const {

        QColor bg(Qt::white);
        bg.setAlpha(200);
        QwtText text = QwtPlotPicker::trackerText(pos);
        text.setBackgroundBrush(QBrush(bg));
        return text;
    }
};

} // namespace sls

#endif
