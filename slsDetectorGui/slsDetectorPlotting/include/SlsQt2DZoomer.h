// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#ifndef SLSQT2DZOOMER_H
#define SLSQT2DZOOMER_H
#include "SlsQt2DHist.h"
#include <cstdio>
#include <qwt_plot_panner.h>
#include <qwt_plot_zoomer.h>

namespace sls {

class SlsQt2DZoomer : public QwtPlotZoomer {
  private:
    SlsQt2DHist *hist;

  public:
    SlsQt2DZoomer(QWidget *canvas) : QwtPlotZoomer(canvas) {
        setTrackerMode(AlwaysOn);
    }

    void SetHist(SlsQt2DHist *h) { hist = h; }

    virtual QwtText trackerTextF(const QPointF &pos) const {
        QColor bg(Qt::white);
        bg.setAlpha(200);

        // QwtText text = QwtPlotZoomer::trackerText(pos);

        static QwtText text;
        if (hist) {
            static char t[200];
            sprintf(t, "%3.2f, %3.2f, %3.2f", pos.x(), pos.y(),
                    hist->value(pos.x(), pos.y()));
            text.setText(t);
        } else {

            QPoint p = pos.toPoint();
            QwtText text = QwtPlotZoomer::trackerText(p);
        }
        text.setBackgroundBrush(QBrush(bg));
        return text;
    }
};

} // namespace sls

#endif
