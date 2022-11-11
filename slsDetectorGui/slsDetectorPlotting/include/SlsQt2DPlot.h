// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once
#include "SlsQt2DHist.h"
#include "SlsQt2DZoomer.h"
#include <array>
#include <qlist.h>
#include <qwt_plot.h>
#include <qwt_plot_shapeitem.h>
#include <qwt_plot_spectrogram.h>

class QwtPlotPanner;
class QwtScaleWidget;
class QwtLinearColorMap;

namespace sls {

class SlsQt2DPlot : public QwtPlot {
    Q_OBJECT

  public:
    SlsQt2DPlot(QWidget * = NULL, bool gain = false);
    ~SlsQt2DPlot();
    void SetTitle(QString title);
    void SetXTitle(QString title);
    void SetYTitle(QString title);
    void SetZTitle(QString title);
    void SetTitleFont(const QFont &f);
    void SetXFont(const QFont &f);
    void SetYFont(const QFont &f);
    void SetZFont(const QFont &f);

    void UnZoom(bool replot = true);
    void SetZoom(double xmin, double ymin, double x_width, double y_width);
    void DisableZoom(bool disable);
    void EnableXAutoScaling() { setAxisAutoScale(QwtPlot::xBottom, true); };
    void EnableYAutoScaling() { setAxisAutoScale(QwtPlot::yLeft, true); };
    void SetXMinMax(double min, double max) {
        setAxisScale(QwtPlot::xBottom, min, max);
    };
    void SetYMinMax(double min, double max) {
        setAxisScale(QwtPlot::yLeft, min, max);
    };
    double GetXMinimum() { return hist->GetXMin(); };
    double GetXMaximum() { return hist->GetXMax(); };
    double GetYMinimum() { return hist->GetYMin(); };
    double GetYMaximum() { return hist->GetYMax(); };
    double GetZMinimum() { return hist->GetMinimum(); }
    double GetZMaximum() { return hist->GetMaximum(); }
    void SetZMinMax(double zmin = 0, double zmax = -1);
    void SetZMinimumToFirstGreaterThanZero() {
        hist->SetMinimumToFirstGreaterThanZero();
    }
    double GetZMean() { return hist->GetMean(); }

    void SetData(int nbinsx, double xmin, double xmax, int nbinsy, double ymin,
                 double ymax, double *d, double zmin = 0, double zmax = -1) {
        hist->SetData(nbinsx, xmin, xmax, nbinsy, ymin, ymax, d, zmin, zmax);
    }

    double *GetDataPtr() { return hist->GetDataPtr(); }
    int GetBinIndex(int bx, int by) { return hist->GetBinIndex(bx, by); }
    int FindBinIndex(double x, double y) { return hist->FindBinIndex(x, y); }
    void SetBinValue(int bx, int by, double v) { hist->SetBinValue(bx, by, v); }
    double GetBinValue(int bx, int by) { return hist->GetBinValue(bx, by); }
    void FillTestPlot(int i = 0);
    void Update();

    void SetInterpolate(bool enable);
    void SetContour(bool enable);
    void SetLogz(bool enable, bool isMin, bool isMax, double min, double max);
    void SetZRange(bool isMin, bool isMax, double min, double max);
    void LogZ(bool on = 1);
    void EnableRoiBox(std::array<int, 4> roi);
    void DisableRoiBox();

  public slots:
    void showSpectrogram(bool on);
    void SetZoom(const QRectF &rect);

  private slots:
    void GetPannedCoord(int, int);

  signals:
    void PlotZoomedSignal(const QRectF &);

  private:
    void SetupZoom();
    void SetupColorMap();
    bool gainPlot{false};

    QwtLinearColorMap *myColourMap(QVector<double> colourStops);
    QwtLinearColorMap *myColourMap(int log = 0);

    QwtPlotSpectrogram *d_spectrogram{nullptr};
    SlsQt2DHist *hist{nullptr};
    SlsQt2DZoomer *zoomer{nullptr};
    QwtPlotPanner *panner{nullptr};
    QwtScaleWidget *rightAxis{nullptr};
    QList<double> contourLevelsLinear;
    QList<double> contourLevelsLog;
    bool disableZoom{false};
    int isLog;
    QwtPlotShapeItem *roiBox{nullptr};
};

} // namespace sls
