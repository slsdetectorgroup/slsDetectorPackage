// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2026 Contributors to the SLS Detector Package
#pragma once

// Qt6 port of the fast 2D image view: one cached Format_RGB32 image composed
// from a 256-entry palette LUT, painted with its own axes, grid, tick labels,
// overlays and an optional in-view colour bar.  No qwt, ROOT or OpenGL.
//
// Data coordinates are IMAGE PIXEL EDGE coordinates:
//   x in [0, nx], y in [0, ny], y is UP and row 0 is at the BOTTOM.
// Pixel (ix, iy) covers [ix, ix+1) x [iy, iy+1).  The QImage is composed with
// exactly one flip (QImage row = ny - 1 - dataRow); nothing else flips rows.
//
// Viewport: setViewport()/viewport() and the onZoomChanged callback use the
// same data coordinates (x1,y1,x2,y2 with y1 < y2, y up).  widgetToData() maps
// a widget position to that space.  Programmatic setViewport() does NOT emit
// onZoomChanged (that keeps master/slave view synchronisation free of feedback
// loops); user pan/wheel zoom, resetView() and resizeEvent() do.

#include "SlsQtPlotCommon.h"

#include <QFont>
#include <QImage>
#include <QPointF>
#include <QRectF>
#include <QString>
#include <QTransform>
#include <QWidget>

#include <functional>
#include <vector>

class QPainter;

namespace sls {

class SlsQtFastImage : public QWidget {
    Q_OBJECT

  public:
    // Smallest data width/height the viewport may have, in image pixels.
    static constexpr double kMinimumViewport = 2.0;

    explicit SlsQtFastImage(QWidget *parent = nullptr);
    ~SlsQtFastImage() override;

    // ---- image input (always copied; GUI thread only) ---------------------
    void setImageCopy(const float *data, int nx, int ny);
    void setImageCopy(const double *data, int nx, int ny);

    // ---- colour mapping ---------------------------------------------------
    // Replaces the 256-entry colour table (GUI thread only).  The LUTs returned
    // by sls::plot::paletteLut() are recognised, so the ScalePanel can keep its
    // chooser in sync; a custom table reports paletteIndex() == -1.
    void setPaletteLut(const QRgb *lut);
    int paletteIndex() const; // index into sls::plot::Palette, -1 if custom

    void setLogScale(bool on);
    bool logScale() const;
    void setIntegerValues(bool on);
    bool integerValues() const;
    void setZeroWhite(bool on);
    bool zeroWhite() const;

    void setColorRange(double lo, double hi);
    void colorRange(double &lo, double &hi) const;

    // ---- histogram of the last composed frame -----------------------------
    // 256 bins over the window the scale strip displays: the colour range
    // widened by half a range on either side (log space when log is on).
    // Values outside that window are skipped, not piled into edge bins.
    // Recomputed at most every ~300 ms.
    const std::vector<int> &histogram() const;
    void histogramRange(double &lo, double &hi) const;
    void invalidateHistogram();

    // ---- geometry / zoom --------------------------------------------------
    void setPixelAspectX(double a);
    double pixelAspectX() const;
    void setViewport(double x1, double y1, double x2, double y2);
    void viewport(double &x, double &y, double &w, double &h) const;
    void resetView();
    bool hasImage() const;
    int imageWidth() const;
    int imageHeight() const;

    QPointF widgetToData(const QPointF &widgetPos) const;
    QPointF dataToWidget(const QPointF &dataPos) const;

    // ---- overlays (data coordinates, see above) ---------------------------
    // Editable yellow ROI (gate ROI), as in the reference view.
    void setRoiRect(double x0, double y0, double x1, double y1, bool on = true);
    void setRoiRectVisible(bool on);
    bool roiRect(double &x0, double &y0, double &x1, double &y1) const;

    // Additional yellow rectangles (e.g. every receiver ROI): each QRectF is
    // (x0, y0, width, height) with (x0,y0) the LOWER-LEFT corner in data
    // coordinates.  Replaces the previous list.
    void setOverlayRects(const std::vector<QRectF> &rects);
    void clearOverlayRects();
    const std::vector<QRectF> &overlayRects() const;

    // Red receiver-ROI rectangle, same data coordinates.
    void setReceiverRoiRect(double x0, double y0, double x1, double y1,
                            bool on = true);
    void clearReceiverRoiRect();

    // ---- ROI editor -------------------------------------------------------
    // While active the yellow ROI is edited with the mouse (inside = move,
    // edges/corners = resize, outside = draw a new one) instead of panning.
    // onRoiEdited() reports (x0,y0,x1,y1) in data coordinates on release.
    void beginRoiEdit(bool on);
    bool roiEditActive() const;

    // ---- optional in-view colour bar (the ScalePanel replaces it) ---------
    void setColorBarVisible(bool on);
    bool colorBarVisible() const;
    bool isScaleDragging() const;

    // Axes can be turned off for the gain map, which is a plain 0..3 mosaic
    // without ticks (the plot then fills the whole widget).
    void setAxesVisible(bool on);
    bool axesVisible() const;
    // Zoom/pan interaction can be turned off (gain map: fixed range).
    void setZoomEnabled(bool on);
    bool zoomEnabled() const;

    // ---- title and axis titles -------------------------------------------
    void setTitle(const QString &title);
    QString title() const;
    void setXAxisTitle(const QString &title);
    QString xAxisTitle() const;
    void setYAxisTitle(const QString &title);
    QString yAxisTitle() const;
    void setTitleFont(const QFont &f);
    QFont titleFont() const;
    void setAxisTitleFont(const QFont &f);
    QFont axisTitleFont() const;
    void setTickFont(const QFont &f);
    QFont tickFont() const;

    // ---- callbacks (GUI thread) ------------------------------------------
    std::function<void()> onImageChanged;
    // reported in data coordinates (x1, y1, x2, y2), y up
    std::function<void(double, double, double, double)> onZoomChanged;
    std::function<void(double, double)> onColorRangeChanged;
    std::function<void(const QRgb *)> onPaletteChanged;
    // x0, y0, x1, y1 in data coordinates (integers, ROI list convention)
    std::function<void(int, int, int, int)> onRoiEdited;
    // one histogram/scale refresh, after an in-view colour bar drag
    std::function<void()> onScaleReleased;
    // Called at the end of paintEvent, clipped to the plot rectangle.  The
    // painter is in widget coordinates and `dataToWidget` maps data
    // coordinates (x right, y up) to widget coordinates - use it for contour
    // lines, polygons, markers, ...
    std::function<void(QPainter &, const QTransform &)> onPaintOverlay;

  protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

  private:
    struct Fit {
        QRect ir;        // plot rectangle in widget pixels
        double sx = 1.0; // screen pixels per data x unit
        double sy = 1.0; // screen pixels per data y unit
        double ox = 0.0; // centering offset inside ir, widget pixels
        double oy = 0.0;
    };

    void renderImage();
    Fit computeFit() const;
    QRect imageRect() const;
    QRect scaleRect() const;
    void clampViewport();
    void matchViewportToWindow();
    void drawImage(QPainter &p, const Fit &fit);
    void drawOverlays(QPainter &p, const Fit &fit);
    void drawAxes(QPainter &p, const Fit &fit);
    void drawTitles(QPainter &p, const Fit &fit);
    void drawColorBar(QPainter &p);
    void drawPixelDetail(QPainter &p, const Fit &fit);
    double valueAt(const QPoint &pos) const;

    double vpW() const { return (m_vpW < 0) ? m_nx : m_vpW; }
    double vpH() const { return (m_vpH < 0) ? m_ny : m_vpH; }

    std::vector<float> m_owned; // widget-owned copy of the last frame
    const float *m_data = nullptr;
    int m_nx = 0, m_ny = 0;
    QImage m_image;
    bool m_dirty = true;
    QRgb m_palette[256];
    int m_paletteIndex = int(plot::Palette::Color);

    double m_clo = 0.0, m_chi = 4000.0;
    double m_defClo = 0.0, m_defChi = 4000.0;
    bool m_logScale = false;
    bool m_integerValues = false;
    bool m_zeroWhite = false;
    bool m_showBar = false;
    bool m_axesVisible = true;
    bool m_zoomEnabled = true;

    std::vector<int> m_hist;
    double m_histLo = 0.0, m_histHi = 1.0;
    qint64 m_histMs = 0;

    double m_pixelAspectX = 1.0;
    double m_vpX = 0.0, m_vpY = 0.0;
    double m_vpW = -1.0, m_vpH = -1.0;
    double m_sx = 1.0; // last painted screen pixels per data x unit

    bool m_roiOn = false;
    double m_roiX0 = 0.0, m_roiY0 = 0.0, m_roiX1 = 0.0, m_roiY1 = 0.0;
    std::vector<QRectF> m_overlays;
    bool m_rxOn = false;
    double m_rxX0 = 0.0, m_rxY0 = 0.0, m_rxX1 = 0.0, m_rxY1 = 0.0;

    bool m_roiEdit = false;
    int m_roiGrab = 0; // 0 none, 1 move, 2 left, 3 right, 4 bottom, 5 top,
                       // 6 bl, 7 br, 8 tl, 9 tr, 10 new rectangle
    QPointF m_roiPress{0, 0};
    double m_roiStart[4] = {0, 0, 0, 0};

    bool m_dragging = false;
    QPointF m_lastPos;
    double m_dragVpX = 0.0, m_dragVpY = 0.0;

    bool m_dragScale = false; // in-view colour bar drag
    bool m_dragMax = false;
    int m_dragScaleY = 0;
    double m_dragClo = 0.0, m_dragChi = 0.0;

    sls::plot::PlotTheme m_theme;
    QString m_title, m_xTitle, m_yTitle;
    QFont m_titleFont, m_axisTitleFont, m_tickFont;
};

} // namespace sls
