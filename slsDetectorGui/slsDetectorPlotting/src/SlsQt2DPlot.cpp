// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "SlsQt2DPlot.h"
#include "qDefs.h"
#include "qVersionResolve.h"
#include "sls/logger.h"

// Suppressing warning until qwt has a fix, avoid
// patching and should be backwards compatible
// https://doc.qt.io/qt-5/qflags-obsolete.html
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#include <qlist.h>
#include <qtoolbutton.h>
#include <qwt_color_map.h>
#include <qwt_plot_layout.h>
#include <qwt_plot_panner.h>
#include <qwt_plot_spectrogram.h>
#include <qwt_plot_zoomer.h>
#include <qwt_scale_draw.h>
#include <qwt_scale_engine.h>
#include <qwt_scale_widget.h>
#pragma GCC diagnostic pop

#include <cmath>
#include <iostream>

namespace sls {

SlsQt2DPlot::SlsQt2DPlot(QWidget *parent, bool gain)
    : QwtPlot(parent), gainPlot(gain) {
    isLog = 0;
    axisScaleEngine(QwtPlot::yLeft)->setAttribute(QwtScaleEngine::Floating);
    axisScaleEngine(QwtPlot::xBottom)->setAttribute(QwtScaleEngine::Floating);
    d_spectrogram = new QwtPlotSpectrogram();
    hist = new SlsQt2DHist();
    SetupZoom();
    SetupColorMap();
    d_spectrogram->setData(hist);
    d_spectrogram->attach(this);
    plotLayout()->setAlignCanvasToScales(true);
    FillTestPlot();
    setFont(qDefs::GetDefaultFont());
    SetTitleFont(qDefs::GetDefaultFont());
    SetXFont(qDefs::GetDefaultFont());
    SetYFont(qDefs::GetDefaultFont());
    SetZFont(qDefs::GetDefaultFont());
    Update();

    if (gainPlot) {
        setTitle("Gain");
        SetZTitle("");
        enableAxis(QwtPlot::yLeft, false);
        enableAxis(QwtPlot::xBottom, false);
        DisableZoom(true);
        // set only major ticks from 0 to 3
        auto div = axisScaleEngine(QwtPlot::yRight)->divideScale(0, 3, 3, 0, 1);
        setAxisScaleDiv(QwtPlot::yRight, div);
    }
}

SlsQt2DPlot::~SlsQt2DPlot() = default;

void SlsQt2DPlot::SetTitle(QString title) { setTitle(title); }

void SlsQt2DPlot::SetXTitle(QString title) {
    setAxisTitle(QwtPlot::xBottom, title);
}

void SlsQt2DPlot::SetYTitle(QString title) {
    setAxisTitle(QwtPlot::yLeft, title);
}

void SlsQt2DPlot::SetZTitle(QString title) {
    setAxisTitle(QwtPlot::yRight, title);
}

void SlsQt2DPlot::SetTitleFont(const QFont &f) {
    QwtText t("");
    t.setFont(f);
    t.setRenderFlags(Qt::AlignLeft | Qt::AlignVCenter);
    setTitle(t);
}

void SlsQt2DPlot::SetXFont(const QFont &f) {
    QwtText t("");
    t.setFont(f);
    setAxisTitle(QwtPlot::xBottom, t);
}

void SlsQt2DPlot::SetYFont(const QFont &f) {
    QwtText t("");
    t.setFont(f);
    setAxisTitle(QwtPlot::yLeft, t);
}

void SlsQt2DPlot::SetZFont(const QFont &f) {
    QwtText t("");
    t.setFont(f);
    setAxisTitle(QwtPlot::yRight, t);
}

void SlsQt2DPlot::SetupColorMap() {
    d_spectrogram->setColorMap(myColourMap(0));
    for (double level = 0.5; level < 10.0; level += 1.0)
        (contourLevelsLinear) += level;
    d_spectrogram->setContourLevels(contourLevelsLinear);

    for (double level = 0.5; level < 10.0; level += 1.0)
        (contourLevelsLog) += (pow(10, 2 * level / 10.0) - 1) / 99.0 * 10;

    // A color bar on the right axis
    rightAxis = axisWidget(QwtPlot::yRight);

    rightAxis->setTitle("Intensity");
    rightAxis->setColorBarEnabled(true);
    enableAxis(QwtPlot::yRight);
}

void SlsQt2DPlot::FillTestPlot(int mode) {
    static int nx = 50;
    static int ny = 50;
    static double *the_data = nullptr;
    if (the_data == nullptr)
        the_data = new double[nx * ny];

    double dmax = sqrt(pow(nx / 2.0 - 0.5, 2) + pow(ny / 2.0 - 0.5, 2));
    for (int i = 0; i < nx; i++) {
        for (int j = 0; j < ny; j++) {
            double d = sqrt(pow(nx / 2.0 - (i + 0.5), 2) +
                            pow(ny / 2.0 - (j + 0.5), 2));

            if (mode % 3)
                the_data[i + j * nx] = 10 * d / dmax;
            else
                the_data[i + j * nx] = 10 * (1 - d / dmax);
        }
    }

    hist->SetData(nx, 200, 822, ny, -0.5, ny - 0.5, the_data);
}

void SlsQt2DPlot::SetupZoom() {
    // LeftButton for the zooming
    // MiddleButton for the panning
    // RightButton: zoom out by 1
    // Ctrl+RighButton: zoom out to full size

    zoomer = new SlsQt2DZoomer(canvas());
    zoomer->SetHist(hist);
    zoomer->setMousePattern(QwtEventPattern::MouseSelect2, Qt::RightButton,
                            Qt::ControlModifier);
    zoomer->setMousePattern(QwtEventPattern::MouseSelect3, Qt::RightButton);
    panner = new QwtPlotPanner(canvas());
    panner->setAxisEnabled(QwtPlot::yRight, false);
    panner->setMouseButton(Qt::MiddleButton);

    // Avoid jumping when labels with more/less digits
    // appear/disappear when scrolling vertically

    const QFontMetrics fm(axisWidget(QwtPlot::yLeft)->font());
    QwtScaleDraw *sd = axisScaleDraw(QwtPlot::yLeft);
    sd->setMinimumExtent(qResolve_GetQFontWidth(fm, "100.00"));
    const QColor c(Qt::darkBlue);
    zoomer->setRubberBandPen(c);
    zoomer->setTrackerPen(c);

    connect(zoomer, SIGNAL(zoomed(const QRectF &)), this,
            SIGNAL(PlotZoomedSignal(const QRectF &)));

    connect(panner, SIGNAL(panned(int, int)), this,
            SLOT(GetPannedCoord(int, int)));
}

void SlsQt2DPlot::UnZoom(bool replot) {

    zoomer->setZoomBase(QRectF(hist->GetXMin(), hist->GetYMin(),
                               hist->GetXMax() - hist->GetXMin(),
                               hist->GetYMax() - hist->GetYMin()));
    zoomer->setZoomBase(replot); // Call replot for the attached plot before
                                 // initializing the zoomer with its scales.
                                 // zoomer->zoom(0);
}

void SlsQt2DPlot::GetPannedCoord(int, int) {
    double xmin = invTransform(QwtPlot::xBottom, 0);
    double xmax = invTransform(QwtPlot::xBottom, canvas()->rect().width());
    double ymax = invTransform(QwtPlot::yLeft, 0);
    double ymin = invTransform(QwtPlot::yLeft, canvas()->rect().height());
    LOG(logDEBUG1) << "Rect1  " << xmin << "\t" << xmax << "\t" << ymin << "\t"
                   << ymax;
    QPointF topLeft = QPointF(xmin, ymin);
    QPointF bottomRight = QPointF(xmax, ymax);
    const QRectF rectf = QRectF(topLeft, bottomRight);
    rectf.getCoords(&xmin, &ymin, &xmax, &ymax);
    LOG(logDEBUG1) << "RectF  " << xmin << "\t" << xmax << "\t" << ymin << "\t"
                   << ymax;
    emit PlotZoomedSignal(rectf);
}

void SlsQt2DPlot::SetZoom(const QRectF &rect) {
    double xmin = 0, xmax = 0, ymin = 0, ymax = 0;
    rect.getCoords(&xmin, &ymin, &xmax, &ymax);
    LOG(logDEBUG1) << "Plot zooming in to " << xmin << " " << xmax << " "
                   << ymin << " " << ymax;
    SetXMinMax(xmin, xmax);
    SetYMinMax(ymin, ymax);
    replot();
}

void SlsQt2DPlot::SetZoom(double xmin, double ymin, double x_width,
                          double y_width) {
    zoomer->setZoomBase(QRectF(xmin, ymin, x_width, y_width));
}

void SlsQt2DPlot::DisableZoom(bool disable) {
    if (disableZoom != disable) {
        disableZoom = disable;

#ifdef VERBOSE
        if (disable)
            std::cout << "Disabling zoom\n";
        else
            std::cout << "Enabling zoom\n";
#endif
        if (disable) {
            if (zoomer) {
                zoomer->setMousePattern(QwtEventPattern::MouseSelect1,
                                        Qt::NoButton);
                zoomer->setMousePattern(QwtEventPattern::MouseSelect2,
                                        Qt::NoButton, Qt::ControlModifier);
                zoomer->setMousePattern(QwtEventPattern::MouseSelect3,
                                        Qt::NoButton);
            }
            if (panner)
                panner->setMouseButton(Qt::NoButton);
        } else {
            if (zoomer) {
                zoomer->setMousePattern(QwtEventPattern::MouseSelect1,
                                        Qt::LeftButton);
                zoomer->setMousePattern(QwtEventPattern::MouseSelect2,
                                        Qt::RightButton, Qt::ControlModifier);
                zoomer->setMousePattern(QwtEventPattern::MouseSelect3,
                                        Qt::RightButton);
            }
            if (panner)
                panner->setMouseButton(Qt::MiddleButton);
        }
    }
}

void SlsQt2DPlot::SetZMinMax(double zmin, double zmax) {
    hist->SetMinMax(zmin, zmax);
}

QwtLinearColorMap *SlsQt2DPlot::myColourMap(QVector<double> colourStops) {

    int ns = 5;

    double r[] = {0.00, 0.00, 0.87, 1.00, 0.51};
    double g[] = {0.00, 0.81, 1.00, 0.20, 0.00};
    double b[] = {0.51, 1.00, 0.12, 0.00, 0.00};

    // QColor c1, c2, c;
    QColor c2, c;
    c2.setRgbF(r[ns - 1], g[ns - 1], b[ns - 1]);
    QwtLinearColorMap *copyMap = new QwtLinearColorMap(Qt::lightGray, c2);

    for (int is = 0; is < ns - 1; is++) {
        c.setRgbF(r[is], g[is], b[is]);
        copyMap->addColorStop(colourStops[is], c);
    }

    return copyMap;
}

QwtLinearColorMap *SlsQt2DPlot::myColourMap(int log) {
    QVector<double> cs{0.0, 0.34, 0.61, 0.84, 1.0};
    if (log) {
        for (int i = 0; i < cs.size(); ++i)
            cs[i] = (pow(10, 2 * cs[i]) - 1) / 99.0;
    }
    return myColourMap(cs);
}

void SlsQt2DPlot::Update() {
    if (isLog)
        hist->SetMinimumToFirstGreaterThanZero();
    const QwtInterval zInterval = d_spectrogram->data()->interval(Qt::ZAxis);
    rightAxis->setColorMap(zInterval, myColourMap(isLog));
    if (!zoomer->zoomRectIndex())
        UnZoom();
    if (!gainPlot) {
        setAxisScale(QwtPlot::yRight, zInterval.minValue(),
                     zInterval.maxValue());
    }
    plotLayout()->setAlignCanvasToScales(true);
    replot();
}

void SlsQt2DPlot::SetInterpolate(bool enable) {
    hist->Interpolate(enable);
    Update();
}

void SlsQt2DPlot::SetContour(bool enable) {
    d_spectrogram->setDisplayMode(QwtPlotSpectrogram::ContourMode, enable);
    Update();
}

void SlsQt2DPlot::SetLogz(bool enable, bool isMin, bool isMax, double min,
                          double max) {
    LogZ(enable);
    SetZRange(isMin, isMax, min, max);
}

void SlsQt2DPlot::SetZRange(bool isMin, bool isMax, double min, double max) {
    if (isLog) {
        SetZMinimumToFirstGreaterThanZero();
    }

    // set zmin and zmax
    if (isMin || isMax) {
        double zmin = (isMin ? min : GetZMinimum());
        double zmax = (isMax ? max : GetZMaximum());
        SetZMinMax(zmin, zmax);
    }

    Update();
}

void SlsQt2DPlot::LogZ(bool on) {
    if (on) {
        isLog = 1;
        d_spectrogram->setColorMap(myColourMap(isLog));
        setAxisScaleEngine(QwtPlot::yRight, new QwtLogScaleEngine);
        d_spectrogram->setContourLevels(contourLevelsLog);
    } else {
        isLog = 0;
        d_spectrogram->setColorMap(myColourMap(isLog));
        setAxisScaleEngine(QwtPlot::yRight, new QwtLinearScaleEngine);
        d_spectrogram->setContourLevels(contourLevelsLinear);
    }
    Update();
}

void SlsQt2DPlot::showSpectrogram(bool on) {
    d_spectrogram->setDisplayMode(QwtPlotSpectrogram::ImageMode, on);
    d_spectrogram->setDefaultContourPen(on ? QPen() : QPen(Qt::NoPen));
    Update();
}

void SlsQt2DPlot::EnableRoiBox(std::array<int, 4> roi) {
    if (roiBox == nullptr) {
        roiBox = new QwtPlotShapeItem();
    }
    roiBox->setPen(QColor(Qt::yellow), 2.0, Qt::SolidLine);

    // TopLeft - BottomRight (max points are +1 on graph)
    QRect myRect(QPoint(roi[0], roi[2]), QPoint(roi[1] - 1, roi[3] - 1));
    roiBox->setRect(QRectF(myRect));

    roiBox->attach(this);
    replot();
}

void SlsQt2DPlot::DisableRoiBox() {
    if (roiBox != nullptr) {
        roiBox->detach();
        replot();
    }
}

} // namespace sls
