// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
/* TODO! short description */
#include "SlsQt1DZoomer.h"
#include "SlsQt1DPlot.h"
#include <iostream>
#include <qwt_plot.h>
#include <qwt_scale_div.h>

namespace sls {

void SlsQt1DZoomer::ResetZoomBase() {
    SetZoomBase(x0, y0, x1 - x0,
                y1 - y0); // for going between log and nonlog plots
}

void SlsQt1DZoomer::SetZoomBase(double xmin, double ymin, double x_width,
                                double y_width) {
    if (xIsLog && xmin <= 0) {
        double xmax = xmin + x_width;
        xmin = firstXgt0 * 0.98;
        if (xmax <= xmin)
            x_width = firstXgt0;
        else
            x_width = xmax - xmin;
    }
    if (yIsLog && ymin <= 0) {
        double ymax = ymin + y_width;
        ymin = firstYgt0 * 0.98;
        if (ymax <= ymin)
            y_width = firstYgt0;
        else
            y_width = ymax - ymin;
    }

    if (plot()) {
        if (xIsLog) {
            double xmin_curr =
                plot()->axisScaleDiv(QwtPlot::xBottom).lowerBound();
            double xmax_curr =
                plot()->axisScaleDiv(QwtPlot::xBottom).upperBound();
            if (xmin_curr < xmin)
                xmin_curr = xmin;
            if (xmax_curr > xmin + x_width)
                xmax_curr = xmin + x_width;
            plot()->setAxisScale(QwtPlot::xBottom, xmin_curr, xmax_curr);
        }
        if (yIsLog) {
            double ymin_curr =
                plot()->axisScaleDiv(QwtPlot::yLeft).lowerBound();
            double ymax_curr =
                plot()->axisScaleDiv(QwtPlot::yLeft).upperBound();
            if (ymin_curr < ymin)
                ymin_curr = ymin;
            if (ymax_curr > ymin + y_width)
                ymax_curr = ymin + y_width;
            plot()->setAxisScale(QwtPlot::yLeft, ymin_curr, ymax_curr);
        }
        plot()->replot();
    }
    setZoomBase(QRectF(xmin, ymin, x_width, y_width));
}

void SlsQt1DZoomer::SetZoomBase(SlsQtH1D *h) {
    x0 = h->GetXMin() < 0 ? h->GetXMin() * 1.02 : h->GetXMin() / 1.02;
    x1 = h->GetXMax() < 0 ? h->GetXMax() / 1.02 : h->GetXMax() * 1.02;
    y0 = h->GetYMin() < 0 ? h->GetYMin() * 1.02 : h->GetYMin() / 1.02;
    y1 = h->GetYMax() < 0 ? h->GetYMax() / 1.02 : h->GetYMax() * 1.02;

    firstXgt0 = h->GetFirstXgtZero(); // for log plots
    firstYgt0 = h->GetFirstYgtZero(); // for log plots

    ResetZoomBase();
}

void SlsQt1DZoomer::ExtendZoomBase(SlsQtH1D *h) {
    double h_x0 = h->GetXMin() < 0 ? h->GetXMin() * 1.02 : h->GetXMin() / 1.02;
    double h_x1 = h->GetXMax() < 0 ? h->GetXMax() / 1.02 : h->GetXMax() * 1.02;
    double h_y0 = h->GetYMin() < 0 ? h->GetYMin() * 1.02 : h->GetYMin() / 1.02;
    double h_y1 = h->GetYMax() < 0 ? h->GetYMax() / 1.02 : h->GetYMax() * 1.02;

    if (h_x0 < x0)
        x0 = h_x0;
    if (h_x1 > x1)
        x1 = h_x1;
    if (h_y0 < y0)
        y0 = h_y0;
    if (h_y1 > y1)
        y1 = h_y1;

    if (h->GetFirstXgtZero() < firstXgt0)
        firstXgt0 = h->GetFirstXgtZero();
    if (h->GetFirstYgtZero() < firstYgt0)
        firstYgt0 = h->GetFirstYgtZero();

    ResetZoomBase();
}

} // namespace sls
