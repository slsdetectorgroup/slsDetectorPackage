// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2026 Contributors to the SLS Detector Package
#ifndef SLSQT1DPLOT_H
#define SLSQT1DPLOT_H

// Qt5/Qt6 replacement for the qwt based 1D plotting layer of slsDetectorGui.
//
// SlsQt1DPlot is a plain QWidget that paints its own frame, ticks, grid,
// clipped curves, legend, ROI boxes and tracker and implements the zoom / pan
// interaction (rubber-band zoom, wheel zoom, middle-drag pan, right click zoom
// out one level, Ctrl+right reset).  SlsQtH1D is a plain data container plus
// curve description (no QwtPlotCurve base).  No qwt, ROOT or OpenGL.

#include "SlsQtPlotCommon.h"
#include "sls/sls_detector_defs.h"

#include <QColor>
#include <QFont>
#include <QPen>
#include <QPointF>
#include <QRectF>
#include <QString>
#include <QWidget>

#include <vector>

namespace sls {

class SlsQt1DPlot;

// One 1D histogram/curve.  The public data API is the one the old
// QwtPlotCurve derived class exposed, so qDrawPlot/qCloneWidget do not need
// changes apart from the legend attribute (now setLegendVisible()).
class SlsQtH1D {
  public:
    SlsQtH1D(QString title, int n, double xmin, double xmax,
             double *data = nullptr);
    SlsQtH1D(QString title, int n, double *data_x, double *data_y);
    ~SlsQtH1D();

    void Attach(SlsQt1DPlot *p);
    void Detach(SlsQt1DPlot *p);

    int SetLineColor(int c = -1);
    int SetLineWidth(int w = 1);
    void SetLineStyle(int s = 0);
    void setStyleLinesorDots(bool isLines);
    void setSymbolMarkers(bool isMarker);

    // Replaces the old QwtPlotItem::Legend attribute: while false the plot
    // draws no legend entry for this curve (used by the gain plots).
    void setLegendVisible(bool visible) { m_legendVisible = visible; }
    bool legendVisible() const { return m_legendVisible; }

    void SetData(int n, double xmin, double xmax, double *d = nullptr);
    void SetData(int n, double *dx, double *dy);

    double *GetX() { return m_x.data(); }
    double *GetY() { return m_y.data(); }
    int GetNBinsX() const { return m_ndata; }

    double FillBin(int bx, double v = 1);
    double Fill(double x, double v = 1);
    double SetBinContent(int bx, double v);
    double SetContent(double x, double v);
    int FindBinIndex(double px);

    double GetXMin() const { return m_ndata > 0 ? m_x.front() : 0.0; }
    double GetFirstXgtZero() const { return m_firstXgt0; }
    double GetXMax() const {
        return m_ndata > 0 ? m_x[static_cast<size_t>(m_ndata - 1)] : 0.0;
    }
    double GetYMin() const { return m_ymin; }
    double GetFirstYgtZero() const { return m_firstYgt0; }
    double GetYMax() const { return m_ymax; }

    SlsQtH1D *Add(double v);

    QString title() const { return m_title; }
    void setTitle(const QString &title) { m_title = title; }

    // Read access for the painter only.
    const QPen &pen() const { return m_pen; }
    bool isLines() const { return m_linesMode; }
    bool isMarkers() const { return m_markerMode; }

  private:
    friend class SlsQt1DPlot;

    void initialize();
    void setUpArrays(int n);
    int checkIndex(int bx);

    QString m_title;
    int m_ndata{0};
    double m_dx{0.0};
    std::vector<double> m_x;
    std::vector<double> m_y;
    double m_ymin{0.0};
    double m_ymax{0.0};
    double m_firstXgt0{-1.0};
    double m_firstYgt0{-1.0};
    bool m_linesMode{true};
    bool m_markerMode{false};
    bool m_legendVisible{true};
    QPen m_pen;
    SlsQt1DPlot *m_plot{nullptr};
};

// Custom painted 1D plot (replaces QwtPlot + QwtPlotZoomer + QwtPlotPanner).
class SlsQt1DPlot : public QWidget {
    Q_OBJECT

  public:
    explicit SlsQt1DPlot(QWidget *parent = nullptr, bool gain = false);
    ~SlsQt1DPlot() override;

    void SetTitle(QString title);
    void SetXTitle(QString title);
    void SetYTitle(QString title);
    void SetTitleFont(const QFont &f);
    void SetXFont(const QFont &f);
    void SetYFont(const QFont &f);

    void InsertHLine(double y);
    void RemoveHLine();
    void InsertVLine(double v);
    void RemoveVLine();

    // Disables every zoom/pan interaction (rubber band, wheel zoom, middle
    // drag panning and the right-click zoom out), like the old zoomer/panner
    // based implementation did.
    void DisableZoom(bool disable);

    void EnableXAutoScaling();
    void EnableYAutoScaling();
    void SetYStep(int step) { m_yStep = step; }
    void SetXMinMax(double min, double max);
    void SetYMinMax(double min, double max);
    double GetXMinimum() const;
    double GetXMaximum() const;
    double GetYMinimum() const;
    double GetYMaximum() const;

    bool isGainPlot() const { return m_gainPlot; }
    bool isZoomDisabled() const { return m_disableZoom; }
    // Currently visible data range (x=left, y=bottom, width/height > 0).
    QRectF viewRect() const;
    int zoomLevel() const { return m_zoomIndex; }

    void SetZoom(double xmin, double ymin, double x_width, double y_width);
    void SetZoomBase(double xmin, double ymin, double x_width,
                     double y_width);

    void alignScales();

    void SetLogX(bool yes = true);
    void SetLogY(bool yes = true);

    void EnableRoiBoxes(std::vector<slsDetectorDefs::ROI> roi, int ymin,
                        int ymax);
    void DisableRoiBoxes();

  signals:
    void PlotZoomedSignal(const QRectF &rect);

  public slots:
    void SetZoomX(const QRectF &rect);
    void UnZoom();
    void Update();

  protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void leaveEvent(QEvent *event) override;

  private:
    friend class SlsQtH1D;

    struct Geometry {
        QRect plot; // plot rectangle in widget pixels
        std::vector<double> xTicks;
        std::vector<double> yTicks;
        std::vector<QString> xLabels;
        std::vector<QString> yLabels;
        int titleH{0};
        int xTitleH{0};
        int yTitleW{0};
        int legendH{0};
    };

    // histogram list
    void addHistogram(SlsQtH1D *h);
    void removeHistogram(SlsQtH1D *h);

    // ranges / zoom
    void recomputeBase();
    void clampView();
    void setView(double xLo, double xHi, double yLo, double yHi);
    QRectF baseRect() const;
    QRectF currentRect() const;
    void resetZoom(bool emitSignal);
    void zoomOutOneLevel();
    void applyZoomRect(const QRectF &rect);
    void pushZoomRect(const QRectF &rect, bool emitSignal);
    void zoomAbout(const QPointF &pos, double factor, bool push);
    bool zoomAllowed() const { return !m_disableZoom && !m_gainPlot; }
    void emitZoomed();

    // coordinate transforms
    double tx(double v) const;
    double ty(double v) const;
    double itx(double t) const;
    double ity(double t) const;
    QPointF toWidget(double x, double y, const Geometry &g) const;
    QPointF toData(const QPointF &pos, const Geometry &g) const;

    // painting helpers
    Geometry computeGeometry() const;
    std::vector<double> tickValues(double lo, double hi, bool log,
                                   int maxTicks) const;
    std::vector<QString> tickLabels(const std::vector<double> &ticks,
                                    double lo, double hi, bool log) const;
    void paintGrid(QPainter &p, const Geometry &g) const;
    void paintCurves(QPainter &p, const Geometry &g) const;
    void paintDecorations(QPainter &p, const Geometry &g) const;
    void paintFrame(QPainter &p, const Geometry &g) const;
    void paintLegend(QPainter &p, const Geometry &g) const;
    void paintTracker(QPainter &p, const Geometry &g) const;
    void paintRubberBand(QPainter &p, const Geometry &g) const;

    // state
    bool m_gainPlot{false};
    std::vector<SlsQtH1D *> m_hists;

    QString m_title;
    QString m_xTitle;
    QString m_yTitle;
    QFont m_titleFont;
    QFont m_xTitleFont;
    QFont m_yTitleFont;
    QFont m_tickFont;
    sls::plot::PlotTheme m_theme;

    bool m_logX{false};
    bool m_logY{false};
    bool m_xAuto{true};
    bool m_yAuto{true};
    bool m_disableZoom{false};
    int m_yStep{0};

    double m_xLo{0.0}, m_xHi{1.0}, m_yLo{0.0}, m_yHi{1.0};
    double m_bxLo{0.0}, m_bxHi{1.0}, m_byLo{0.0}, m_byHi{1.0};

    std::vector<QRectF> m_zoomStack;
    int m_zoomIndex{0};

    bool m_rubberBand{false};
    QPointF m_rubberStart;
    QPointF m_rubberCur;

    bool m_panning{false};
    QPointF m_panStart;
    double m_panXnLo{0.0}, m_panXnHi{1.0}, m_panYnLo{0.0}, m_panYnHi{1.0};

    bool m_trackValid{false};
    QPointF m_trackPos;

    bool m_hasHLine{false};
    double m_hLine{0.0};
    bool m_hasVLine{false};
    double m_vLine{0.0};

    bool m_roiOn{false};
    std::vector<slsDetectorDefs::ROI> m_rois;
    int m_roiYMin{0};
    int m_roiYMax{0};
};

} // namespace sls

#endif
