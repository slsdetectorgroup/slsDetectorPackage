// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2026 Contributors to the SLS Detector Package
#pragma once

#include "SlsQtPlotCommon.h"

#include <QElapsedTimer>
#include <QFont>
#include <QImage>
#include <QLineF>
#include <QPointF>
#include <QRectF>
#include <QString>
#include <QWidget>

#include <vector>

class QPainter;

namespace sls {

/**
 * Colour mapped 2D image with axes, zoom and pan.
 *
 * All coordinates are in image pixels: pixel (ix, iy) covers
 * [ix, ix+1) x [iy, iy+1), y points up and row 0 is at the bottom. The
 * viewport, the overlays and the signals use these coordinates.
 *
 * The wheel zooms around the cursor, dragging pans and a double click shows
 * the whole image again. Once the pixels are large enough they are outlined
 * and labelled with their value.
 */
class SlsQtFastImage : public QWidget {
    Q_OBJECT

  public:
    /** Smallest width and height of the viewport */
    static constexpr double kMinimumViewport = 2.0;

    explicit SlsQtFastImage(QWidget *parent = nullptr);

    /** Copies the data, which is stored x fastest: data[ix + iy * nx] */
    void setImage(const double *data, int nx, int ny);
    bool hasImage() const;
    int imageWidth() const;
    int imageHeight() const;

    void setPalette(plot::Palette palette);
    plot::Palette palette() const;
    /** Map log10 of the values, see plot::logClamp() for values <= 0 */
    void setLogScale(bool on);
    bool logScale() const;
    void setColorRange(double lo, double hi);
    void colorRange(double &lo, double &hi) const;
    /** Blend neighbouring pixels. Ignored once the pixels are outlined. */
    void setInterpolate(bool on);
    bool interpolate() const;

    /**
     * 256 bins over histogramRange(): the colour range widened by half its
     * width on both sides, in log10 of the values with a log scale. Values
     * outside that range are not counted. Updated while painting, at most a
     * few times per second.
     */
    const std::vector<int> &histogram() const;
    void histogramRange(double &lo, double &hi) const;

    /** Does not emit viewportChanged() */
    void setViewport(double x1, double y1, double x2, double y2);
    void viewport(double &x, double &y, double &w, double &h) const;
    void resetView();
    QPointF widgetToData(const QPointF &widgetPos) const;
    QPointF dataToWidget(const QPointF &dataPos) const;

    void setZoomEnabled(bool on);
    bool zoomEnabled() const;
    void setAxesVisible(bool on);
    bool axesVisible() const;

    /** Yellow rectangles, QRectF(x, y, width, height) with (x, y) the lower
     * left corner */
    void setOverlayRects(const std::vector<QRectF> &rects);
    const std::vector<QRectF> &overlayRects() const;
    /** White line segments, e.g. contour lines */
    void setOverlayLines(const std::vector<QLineF> &lines);

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

  signals:
    /** The user zoomed or panned, or a resize changed the visible area.
     * QRectF(x, y, width, height) with (x, y) the lower left corner. */
    void viewportChanged(const QRectF &viewport);
    /** Palette, log scale or colour range changed */
    void colorMapChanged();
    void histogramChanged();

  protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

  private:
    /** Where the viewport ends up in the widget */
    struct Mapping {
        QRect plot;         // widget area the image is drawn in
        double scale = 1.0; // widget pixels per image pixel
        QPointF origin;     // widget position of image coordinate (0, 0)

        QPointF toWidget(double x, double y) const {
            return {origin.x() + x * scale, origin.y() - y * scale};
        }
        QPointF toData(const QPointF &w) const {
            return {(w.x() - origin.x()) / scale, (origin.y() - w.y()) / scale};
        }
    };

    QRect plotRect() const;
    Mapping mapping() const;
    void clampViewport();
    void matchViewportToWindow();
    void zoomAt(const QPointF &widgetPos, double factor);
    void renderImage();
    void updateHistogram();
    void drawImage(QPainter &p, const Mapping &m) const;
    void drawAxes(QPainter &p, const Mapping &m) const;
    void drawPixelDetail(QPainter &p, const Mapping &m) const;
    void drawOverlays(QPainter &p, const Mapping &m) const;
    void drawTitles(QPainter &p, const Mapping &m) const;
    double value(int ix, int iy) const;

    std::vector<double> m_data;
    int m_nx = 0;
    int m_ny = 0;
    QImage m_image; // m_data through the colour map, row 0 at the top
    bool m_imageDirty = true;

    plot::Palette m_palette = plot::Palette::Color;
    double m_colorLo = 0.0;
    double m_colorHi = 1.0;
    bool m_logScale = false;
    bool m_interpolate = false;

    std::vector<int> m_histogram;
    double m_histogramLo = 0.0;
    double m_histogramHi = 1.0;
    QElapsedTimer m_histogramAge;

    QRectF m_viewport; // (x, y) is the lower left corner
    bool m_zoomEnabled = true;
    bool m_axesVisible = true;
    bool m_panning = false;
    QPointF m_panStart;         // widget position of the press
    QPointF m_panStartViewport; // lower left corner at the press

    std::vector<QRectF> m_overlayRects;
    std::vector<QLineF> m_overlayLines;

    plot::PlotTheme m_theme;
    QString m_title;
    QString m_xTitle;
    QString m_yTitle;
    QFont m_titleFont;
    QFont m_axisTitleFont;
};

} // namespace sls
