// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2026 Contributors to the SLS Detector Package
#include "SlsQtFastImage.h"

#include <QFontMetrics>
#include <QMouseEvent>
#include <QNativeGestureEvent>
#include <QPainter>
#include <QWheelEvent>

#include <algorithm>
#include <cmath>

namespace sls {

namespace {

constexpr int kHistogramBins = 256;
constexpr int kHistogramIntervalMs = 300;
constexpr int kMaxTicks = 8;
constexpr double kWheelZoomFactor = 1.18;
// widget pixels per image pixel from which the pixels are outlined / labelled
constexpr double kOutlineMinScale = 16.0;
constexpr double kLabelMinScale = 24.0;
// font size of the pixel labels, in widget pixels
constexpr int kLabelMinFont = 8;
constexpr int kLabelMaxFont = 20;
// part of the width and of the height of a pixel that a label may fill
constexpr double kLabelWidth = 0.75;
constexpr double kLabelHeight = 0.3;
// significant digits of values that are not integers
constexpr int kLabelDigits = 5;
constexpr int kReadoutDigits = 8;

// Integers are written out in full so that counts can be read exactly
QString formatValue(double v, int digits) {
    if (v == std::round(v) && std::abs(v) < 1e15)
        return QString::number(v, 'f', 0);
    return QString::number(v, 'g', digits);
}

} // namespace

SlsQtFastImage::SlsQtFastImage(QWidget *parent) : QWidget(parent) {
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    setMinimumSize(50, 50); // the gain plot of the gui is small
    setAttribute(Qt::WA_OpaquePaintEvent, true);

    m_titleFont = font();
    m_titleFont.setPixelSize(13);
    m_titleFont.setBold(true);
    m_axisTitleFont = font();
    m_axisTitleFont.setPixelSize(11);
}

void SlsQtFastImage::setImage(const double *data, int nx, int ny) {
    if (!data || nx <= 0 || ny <= 0)
        return;
    m_data.assign(data, data + static_cast<size_t>(nx) * ny);
    if (nx != m_nx || ny != m_ny) {
        m_nx = nx;
        m_ny = ny;
        m_viewport = QRectF(0, 0, nx, ny);
        m_fillPlot = true;
        m_hasHover = false;
        m_histogramAge.invalidate();
    }
    m_imageDirty = true;
    update();
}

bool SlsQtFastImage::hasImage() const { return m_nx > 0 && m_ny > 0; }

int SlsQtFastImage::imageWidth() const { return m_nx; }

int SlsQtFastImage::imageHeight() const { return m_ny; }

double SlsQtFastImage::value(int ix, int iy) const {
    return m_data[static_cast<size_t>(iy) * m_nx + ix];
}

// ---------------------------------------------------------------------------
// colour mapping
// ---------------------------------------------------------------------------
void SlsQtFastImage::setPalette(plot::Palette palette) {
    if (palette == m_palette)
        return;
    m_palette = palette;
    m_imageDirty = true;
    update();
    emit colorMapChanged();
}

plot::Palette SlsQtFastImage::palette() const { return m_palette; }

void SlsQtFastImage::setLogScale(bool on) {
    if (on == m_logScale)
        return;
    m_logScale = on;
    m_histogramAge.invalidate();
    m_imageDirty = true;
    update();
    emit colorMapChanged();
}

bool SlsQtFastImage::logScale() const { return m_logScale; }

void SlsQtFastImage::setColorRange(double lo, double hi) {
    if (lo == m_colorLo && hi == m_colorHi)
        return;
    m_colorLo = lo;
    m_colorHi = hi;
    m_histogramAge.invalidate();
    m_imageDirty = true;
    update();
    emit colorMapChanged();
}

void SlsQtFastImage::colorRange(double &lo, double &hi) const {
    lo = m_colorLo;
    hi = m_colorHi;
}

void SlsQtFastImage::setInterpolate(bool on) {
    m_interpolate = on;
    update();
}

bool SlsQtFastImage::interpolate() const { return m_interpolate; }

const std::vector<int> &SlsQtFastImage::histogram() const {
    return m_histogram;
}

void SlsQtFastImage::histogramRange(double &lo, double &hi) const {
    lo = m_histogramLo;
    hi = m_histogramHi;
}

void SlsQtFastImage::renderImage() {
    m_imageDirty = false;
    if (m_image.size() != QSize(m_nx, m_ny))
        m_image = QImage(m_nx, m_ny, QImage::Format_RGB32);

    const QRgb *lut = plot::paletteLut(m_palette);
    const bool log = m_logScale;
    const double lo = log ? plot::logClamp(m_colorLo) : m_colorLo;
    const double hi = log ? plot::logClamp(m_colorHi) : m_colorHi;
    const double toIndex = 255.0 / (hi > lo ? hi - lo : 1.0);

    for (int y = 0; y < m_ny; ++y) {
        // QImage rows run top down: this is the only place that flips y
        auto *line = reinterpret_cast<QRgb *>(m_image.scanLine(m_ny - 1 - y));
        const double *row = m_data.data() + static_cast<size_t>(y) * m_nx;
        for (int x = 0; x < m_nx; ++x) {
            const double v = row[x];
            if (!std::isfinite(v)) {
                line[x] = lut[0];
                continue;
            }
            const double index = ((log ? plot::logClamp(v) : v) - lo) * toIndex;
            line[x] = lut[static_cast<int>(std::clamp(index, 0.0, 255.0))];
        }
    }

    // Walking the frame a second time is cheap, but the histogram is only
    // looked at by a person, so a few times per second is enough.
    if (!m_histogramAge.isValid() ||
        m_histogramAge.elapsed() > kHistogramIntervalMs)
        updateHistogram();
}

void SlsQtFastImage::updateHistogram() {
    m_histogramAge.start();
    const bool log = m_logScale;
    double lo = log ? plot::logClamp(m_colorLo) : m_colorLo;
    double hi = log ? plot::logClamp(m_colorHi) : m_colorHi;
    if (!std::isfinite(lo) || !std::isfinite(hi)) {
        lo = 0.0;
        hi = 1.0;
    }
    if (!(hi > lo))
        hi = lo + 1.0;
    const double margin = 0.5 * (hi - lo);
    m_histogramLo = lo - margin;
    m_histogramHi = hi + margin;

    m_histogram.assign(kHistogramBins, 0);
    const double toBin = kHistogramBins / (m_histogramHi - m_histogramLo);
    for (const double raw : m_data) {
        if (!std::isfinite(raw))
            continue;
        const double v = log ? plot::logClamp(raw) : raw;
        if (v < m_histogramLo || v > m_histogramHi)
            continue;
        const int bin = static_cast<int>((v - m_histogramLo) * toBin);
        ++m_histogram[std::min(bin, kHistogramBins - 1)];
    }
    emit histogramChanged();
}

// ---------------------------------------------------------------------------
// viewport
// ---------------------------------------------------------------------------
void SlsQtFastImage::setViewport(double x1, double y1, double x2, double y2) {
    if (!std::isfinite(x1) || !std::isfinite(y1) || !std::isfinite(x2) ||
        !std::isfinite(y2))
        return;
    if (x2 - x1 < kMinimumViewport || y2 - y1 < kMinimumViewport)
        return;
    m_viewport = QRectF(x1, y1, x2 - x1, y2 - y1);
    m_fillPlot = false;
    clampViewport();
    update();
}

// A viewport chosen by the user is grown to the shape of the plot area, as far
// as the image goes, so that it still fills the plot area after a resize.
QRectF SlsQtFastImage::visibleArea() const {
    if (!hasImage())
        return {};
    if (!m_fillPlot)
        return m_viewport;
    const QRect plot = plotRect();
    const double scale = std::min(plot.width() / m_viewport.width(),
                                  plot.height() / m_viewport.height());
    const double w = std::min<double>(m_nx, plot.width() / scale);
    const double h = std::min<double>(m_ny, plot.height() / scale);
    const QPointF center = m_viewport.center();
    return QRectF(std::clamp(center.x() - w / 2, 0.0, m_nx - w),
                  std::clamp(center.y() - h / 2, 0.0, m_ny - h), w, h);
}

void SlsQtFastImage::resetView() {
    if (hasImage())
        showArea(QRectF(0, 0, m_nx, m_ny));
}

// A change made by the user
void SlsQtFastImage::showArea(const QRectF &area) {
    m_viewport = area;
    m_fillPlot = true;
    clampViewport();
    update();
    emit visibleAreaChanged(visibleArea());
}

void SlsQtFastImage::clampViewport() {
    if (!hasImage())
        return;
    const double nx = m_nx;
    const double ny = m_ny;
    const double w =
        std::clamp(m_viewport.width(), std::min(kMinimumViewport, nx), nx);
    const double h =
        std::clamp(m_viewport.height(), std::min(kMinimumViewport, ny), ny);
    const double x = std::clamp(m_viewport.x(), 0.0, nx - w);
    const double y = std::clamp(m_viewport.y(), 0.0, ny - h);
    m_viewport = QRectF(x, y, w, h);
}

// Magnifies by factor around a position in the widget
void SlsQtFastImage::zoomAt(const QPointF &widgetPos, double factor) {
    const Mapping m = mapping();
    const QPointF anchor = m.toData(widgetPos);
    // between the whole image and kMinimumViewport pixels
    const double minScale =
        std::min(m.plot.width() / double(m_nx), m.plot.height() / double(m_ny));
    const double maxScale = std::max(
        minScale, std::min(m.plot.width(), m.plot.height()) / kMinimumViewport);
    const double scale = std::clamp(m.scale * factor, minScale, maxScale);
    const double w = std::min<double>(m_nx, m.plot.width() / scale);
    const double h = std::min<double>(m_ny, m.plot.height() / scale);
    // The anchor stays where it is, except in a direction in which the whole
    // image is visible and therefore centred.
    const double left = m.plot.left() + (m.plot.width() - w * scale) / 2;
    const double top = m.plot.top() + (m.plot.height() - h * scale) / 2;
    showArea(QRectF(anchor.x() - (widgetPos.x() - left) / scale,
                    anchor.y() + (widgetPos.y() - top) / scale - h, w, h));
}

QRect SlsQtFastImage::plotRect() const {
    int left = 0, top = 0, right = 0, bottom = 0;
    if (m_axesVisible) {
        left = m_theme.leftMargin;
        top = m_theme.topMargin;
        right = m_theme.rightMargin;
        bottom = m_theme.bottomMargin;
    }
    if (!m_title.isEmpty())
        top += QFontMetrics(m_titleFont).height() + 4;
    const int axisTitleHeight = QFontMetrics(m_axisTitleFont).height();
    if (!m_xTitle.isEmpty())
        bottom += axisTitleHeight + 4;
    if (!m_yTitle.isEmpty())
        left += axisTitleHeight + 6;
    return QRect(left, top, std::max(10, width() - left - right),
                 std::max(10, height() - top - bottom));
}

SlsQtFastImage::Mapping SlsQtFastImage::mapping() const {
    Mapping m;
    m.plot = plotRect();
    if (!hasImage())
        return m;
    m.area = visibleArea();
    m.scale = std::min(m.plot.width() / m.area.width(),
                       m.plot.height() / m.area.height());
    // centred where the whole image does not fill the plot area
    const double left =
        m.plot.left() + (m.plot.width() - m.area.width() * m.scale) / 2;
    const double top =
        m.plot.top() + (m.plot.height() - m.area.height() * m.scale) / 2;
    m.origin = QPointF(left - m.area.x() * m.scale,
                       top + (m.area.y() + m.area.height()) * m.scale);
    return m;
}

QPointF SlsQtFastImage::widgetToData(const QPointF &widgetPos) const {
    return hasImage() ? mapping().toData(widgetPos) : QPointF();
}

QPointF SlsQtFastImage::dataToWidget(const QPointF &dataPos) const {
    return hasImage() ? mapping().toWidget(dataPos.x(), dataPos.y())
                      : QPointF();
}

void SlsQtFastImage::setAxisRange(double xmin, double xmax, double ymin,
                                  double ymax) {
    if (!(xmax > xmin) || !(ymax > ymin))
        return;
    m_axisRange = QRectF(xmin, ymin, xmax - xmin, ymax - ymin);
    update();
}

SlsQtFastImage::Axis SlsQtFastImage::xAxis() const {
    if (m_axisRange.isNull())
        return {-0.5, 1.0};
    return {m_axisRange.x(), m_axisRange.width() / m_nx};
}

SlsQtFastImage::Axis SlsQtFastImage::yAxis() const {
    if (m_axisRange.isNull())
        return {-0.5, 1.0};
    return {m_axisRange.y(), m_axisRange.height() / m_ny};
}

void SlsQtFastImage::setZoomEnabled(bool on) {
    m_zoomEnabled = on;
    if (!on)
        m_panning = false;
}

bool SlsQtFastImage::zoomEnabled() const { return m_zoomEnabled; }

void SlsQtFastImage::setAxesVisible(bool on) {
    m_axesVisible = on;
    update();
}

bool SlsQtFastImage::axesVisible() const { return m_axesVisible; }

// ---------------------------------------------------------------------------
// overlays and titles
// ---------------------------------------------------------------------------
void SlsQtFastImage::setOverlayRects(const std::vector<QRectF> &rects) {
    m_overlayRects = rects;
    update();
}

const std::vector<QRectF> &SlsQtFastImage::overlayRects() const {
    return m_overlayRects;
}

void SlsQtFastImage::setOverlayLines(const std::vector<QLineF> &lines) {
    m_overlayLines = lines;
    update();
}

void SlsQtFastImage::setTitle(const QString &title) {
    m_title = title;
    update();
}

QString SlsQtFastImage::title() const { return m_title; }

void SlsQtFastImage::setXAxisTitle(const QString &title) {
    m_xTitle = title;
    update();
}

QString SlsQtFastImage::xAxisTitle() const { return m_xTitle; }

void SlsQtFastImage::setYAxisTitle(const QString &title) {
    m_yTitle = title;
    update();
}

QString SlsQtFastImage::yAxisTitle() const { return m_yTitle; }

void SlsQtFastImage::setTitleFont(const QFont &f) {
    m_titleFont = f;
    update();
}

QFont SlsQtFastImage::titleFont() const { return m_titleFont; }

void SlsQtFastImage::setAxisTitleFont(const QFont &f) {
    m_axisTitleFont = f;
    update();
}

QFont SlsQtFastImage::axisTitleFont() const { return m_axisTitleFont; }

// ---------------------------------------------------------------------------
// painting
// ---------------------------------------------------------------------------
void SlsQtFastImage::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.fillRect(rect(), m_theme.background);
    if (!hasImage())
        return;
    if (m_imageDirty)
        renderImage();

    const Mapping m = mapping();
    drawImage(p, m);
    drawAxes(p, m);
    drawPixelDetail(p, m);
    drawOverlays(p, m);
    drawTitles(p, m);
    drawReadout(p, m);
}

void SlsQtFastImage::drawImage(QPainter &p, const Mapping &m) const {
    p.save();
    p.setClipRect(m.areaRect());
    p.setRenderHint(QPainter::SmoothPixmapTransform,
                    m_interpolate && m.scale < kOutlineMinScale);
    const QRectF target(m.toWidget(0, m_ny),
                        QSizeF(m_nx * m.scale, m_ny * m.scale));
    p.drawImage(target, m_image);
    p.restore();
}

void SlsQtFastImage::drawAxes(QPainter &p, const Mapping &m) const {
    if (!m_axesVisible)
        return;
    const QRect plot = m.plot;
    const Axis xAxis = this->xAxis();
    const Axis yAxis = this->yAxis();
    const double x0 = xAxis.value(m.area.x());
    const double x1 = xAxis.value(m.area.x() + m.area.width());
    const double y0 = yAxis.value(m.area.y());
    const double y1 = yAxis.value(m.area.y() + m.area.height());
    const auto ticksX = plot::computeLinearTicks(x0, x1, kMaxTicks);
    const auto ticksY = plot::computeLinearTicks(y0, y1, kMaxTicks);
    const double stepX = plot::niceStep(x1 - x0, kMaxTicks);
    const double stepY = plot::niceStep(y1 - y0, kMaxTicks);
    const auto widgetX = [&](double x) {
        return m.toWidget(xAxis.pixel(x), 0).x();
    };
    const auto widgetY = [&](double y) {
        return m.toWidget(0, yAxis.pixel(y)).y();
    };

    p.save();
    p.setFont(m_theme.tickFont);

    p.setClipRect(plot);
    p.setPen(QPen(m_theme.grid, 1, Qt::DotLine));
    for (const double x : ticksX)
        p.drawLine(QPointF(widgetX(x), plot.top()),
                   QPointF(widgetX(x), plot.bottom()));
    for (const double y : ticksY)
        p.drawLine(QPointF(plot.left(), widgetY(y)),
                   QPointF(plot.right(), widgetY(y)));
    p.setClipping(false);

    p.setPen(m_theme.tick);
    for (const double x : ticksX) {
        const double wx = widgetX(x);
        p.drawLine(QPointF(wx, plot.bottom() + 1),
                   QPointF(wx, plot.bottom() + 4));
        p.drawText(QRectF(wx - 30, plot.bottom() + 5, 60, 14),
                   Qt::AlignHCenter | Qt::AlignTop, plot::formatTick(x, stepX));
    }
    for (const double y : ticksY) {
        const double wy = widgetY(y);
        p.drawLine(QPointF(plot.left() - 4, wy), QPointF(plot.left() - 1, wy));
        p.drawText(QRectF(plot.left() - 50, wy - 7, 44, 14),
                   Qt::AlignRight | Qt::AlignVCenter,
                   plot::formatTick(y, stepY));
    }
    p.setPen(m_theme.frame);
    p.drawRect(plot.adjusted(0, 0, -1, -1));
    p.restore();
}

// Outlines the pixels and writes their values into them
void SlsQtFastImage::drawPixelDetail(QPainter &p, const Mapping &m) const {
    if (m.scale < kOutlineMinScale)
        return;
    const QRectF clip = m.areaRect();
    const int ix0 = static_cast<int>(std::floor(m.area.x()));
    const int iy0 = static_cast<int>(std::floor(m.area.y()));
    const int ix1 = static_cast<int>(std::ceil(m.area.x() + m.area.width()));
    const int iy1 = static_cast<int>(std::ceil(m.area.y() + m.area.height()));

    p.save();
    p.setClipRect(clip);
    p.setPen(QColor(55, 55, 55));
    for (int ix = ix0; ix <= ix1; ++ix) {
        const double wx = m.toWidget(ix, 0).x();
        p.drawLine(QPointF(wx, clip.top()), QPointF(wx, clip.bottom()));
    }
    for (int iy = iy0; iy <= iy1; ++iy) {
        const double wy = m.toWidget(0, iy).y();
        p.drawLine(QPointF(clip.left(), wy), QPointF(clip.right(), wy));
    }

    if (m.scale >= kLabelMinScale) {
        // One font size for all labels: as large as the longest one allows
        QString longest;
        for (int iy = iy0; iy < iy1; ++iy) {
            for (int ix = ix0; ix < ix1; ++ix) {
                const QString text = formatValue(value(ix, iy), kLabelDigits);
                if (text.size() > longest.size())
                    longest = text;
            }
        }
        QFont f = m_theme.tickFont;
        f.setPixelSize(kLabelMaxFont);
        const double width = QFontMetricsF(f).horizontalAdvance(longest);
        const double byWidth =
            kLabelMaxFont * kLabelWidth * m.scale / std::max(width, 1.0);
        const double byHeight = kLabelHeight * m.scale;
        const int fontSize = static_cast<int>(
            std::min({double(kLabelMaxFont), byWidth, byHeight}));
        if (fontSize >= kLabelMinFont) {
            f.setPixelSize(fontSize);
            p.setFont(f);
            for (int iy = iy0; iy < iy1; ++iy) {
                for (int ix = ix0; ix < ix1; ++ix) {
                    // dark text on bright pixels and the other way round
                    const QRgb c = m_image.pixel(ix, m_ny - 1 - iy);
                    p.setPen(qGray(c) > 140 ? QColor(15, 15, 15)
                                            : QColor(245, 245, 245));
                    const QRectF cell(m.toWidget(ix, iy + 1),
                                      QSizeF(m.scale, m.scale));
                    p.drawText(cell, Qt::AlignCenter,
                               formatValue(value(ix, iy), kLabelDigits));
                }
            }
        }
    }
    p.restore();
}

// Position and value of the pixel under the cursor, in a corner of the plot
// area that the cursor is not in
void SlsQtFastImage::drawReadout(QPainter &p, const Mapping &m) const {
    if (!m_hasHover)
        return;
    const QString text =
        QStringLiteral("x %1   y %2   %3")
            .arg(formatValue(xAxis().value(m_hover.x() + 0.5), kReadoutDigits),
                 formatValue(yAxis().value(m_hover.y() + 0.5), kReadoutDigits),
                 formatValue(value(m_hover.x(), m_hover.y()), kReadoutDigits));
    p.save();
    QFont f = m_theme.tickFont;
    f.setPixelSize(12);
    p.setFont(f);
    QRectF box(QPointF(), p.fontMetrics().size(0, text) + QSize(12, 6));
    box.moveTopRight(m.plot.topRight() + QPointF(-4, 5));
    if (box.adjusted(-20, -20, 20, 20).contains(m_cursor))
        box.moveLeft(m.plot.left() + 5);
    p.fillRect(box, QColor(0, 0, 0, 170));
    p.setPen(m_theme.text);
    p.drawText(box, Qt::AlignCenter, text);
    p.restore();
}

void SlsQtFastImage::drawOverlays(QPainter &p, const Mapping &m) const {
    p.save();
    p.setClipRect(m.areaRect());
    p.setBrush(Qt::NoBrush);

    p.setPen(QColor(255, 255, 255, 190));
    for (const QLineF &l : m_overlayLines)
        p.drawLine(m.toWidget(l.x1(), l.y1()), m.toWidget(l.x2(), l.y2()));

    p.setPen(Qt::yellow);
    for (const QRectF &r : m_overlayRects) {
        if (r.width() > 0.0 && r.height() > 0.0)
            p.drawRect(QRectF(m.toWidget(r.x(), r.y() + r.height()),
                              r.size() * m.scale));
    }
    p.restore();
}

void SlsQtFastImage::drawTitles(QPainter &p, const Mapping &m) const {
    const QRect plot = m.plot;
    p.save();
    p.setPen(m_theme.text);
    if (!m_title.isEmpty()) {
        p.setFont(m_titleFont);
        p.drawText(QRectF(0, 1, width(), plot.top() - 2), Qt::AlignCenter,
                   m_title);
    }
    p.setFont(m_axisTitleFont);
    if (!m_xTitle.isEmpty()) {
        const int y = plot.bottom() + m_theme.bottomMargin;
        p.drawText(QRectF(plot.left(), y, plot.width(), height() - y),
                   Qt::AlignHCenter | Qt::AlignTop, m_xTitle);
    }
    if (!m_yTitle.isEmpty()) {
        const double h = p.fontMetrics().height();
        p.translate(h / 2 + 4, plot.center().y());
        p.rotate(-90);
        p.drawText(QRectF(-plot.height() / 2.0, -h / 2, plot.height(), h),
                   Qt::AlignCenter, m_yTitle);
    }
    p.restore();
}

// ---------------------------------------------------------------------------
// interaction
// ---------------------------------------------------------------------------
void SlsQtFastImage::mousePressEvent(QMouseEvent *e) {
    if (e->button() != Qt::LeftButton || !m_zoomEnabled || !hasImage())
        return;
    m_panning = true;
    m_panStart = plot::eventPos(e);
    m_panStartArea = visibleArea();
    setCursor(Qt::ClosedHandCursor);
}

void SlsQtFastImage::mouseMoveEvent(QMouseEvent *e) {
    if (!hasImage())
        return;
    const Mapping m = mapping();
    if (m_panning) {
        const QPointF delta = (plot::eventPos(e) - m_panStart) / m.scale;
        showArea(m_panStartArea.translated(-delta.x(), delta.y()));
        return;
    }

    updateHover(plot::eventPos(e));
}

// Finds the pixel under the cursor, for the readout
void SlsQtFastImage::updateHover(const QPointF &cursor) {
    const Mapping m = mapping();
    const QPointF d = m.toData(cursor);
    const QPoint pixel(static_cast<int>(std::floor(d.x())),
                       static_cast<int>(std::floor(d.y())));
    const bool onImage = m.areaRect().contains(cursor) &&
                         QRect(0, 0, m_nx, m_ny).contains(pixel);
    if (!m_panning)
        setCursor(onImage && m_zoomEnabled ? Qt::OpenHandCursor
                                           : Qt::ArrowCursor);
    m_cursor = cursor;
    if (onImage != m_hasHover || pixel != m_hover) {
        m_hasHover = onImage;
        m_hover = pixel;
        update();
    }
}

void SlsQtFastImage::leaveEvent(QEvent *) {
    if (m_hasHover) {
        m_hasHover = false;
        update();
    }
}

void SlsQtFastImage::mouseReleaseEvent(QMouseEvent *e) {
    if (e->button() != Qt::LeftButton || !m_panning)
        return;
    m_panning = false;
    setCursor(Qt::OpenHandCursor);
}

void SlsQtFastImage::mouseDoubleClickEvent(QMouseEvent *) {
    if (m_zoomEnabled)
        resetView();
}

void SlsQtFastImage::wheelEvent(QWheelEvent *e) {
    const int delta = e->angleDelta().y();
    if (!hasImage() || !m_zoomEnabled || delta == 0)
        return;
    // A mouse wheel sends 120 per notch, a touchpad many small steps
    zoomAt(e->position(), std::pow(kWheelZoomFactor, delta / 120.0));
    updateHover(e->position());
}

// Pinch on a touchpad
bool SlsQtFastImage::event(QEvent *e) {
    if (e->type() == QEvent::NativeGesture && hasImage() && m_zoomEnabled) {
        const auto *gesture = static_cast<QNativeGestureEvent *>(e);
        if (gesture->gestureType() == Qt::ZoomNativeGesture) {
            // value() is the change of size since the previous event
            zoomAt(plot::eventPos(gesture), std::max(0.1, 1 + gesture->value()));
            updateHover(plot::eventPos(gesture));
            return true;
        }
    }
    return QWidget::event(e);
}

// A layout resizes the widget from within other calls, for example when a
// neighbour is shown. Whoever made that call does not expect a signal in the
// middle of it (qDrawPlot holds a mutex there that its slot takes as well), so
// the new visible area is reported from the event loop.
void SlsQtFastImage::resizeEvent(QResizeEvent *) {
    if (!hasImage() || !m_fillPlot || m_resizeReportPending)
        return;
    m_resizeReportPending = true;
    QMetaObject::invokeMethod(
        this,
        [this] {
            m_resizeReportPending = false;
            if (hasImage())
                emit visibleAreaChanged(visibleArea());
        },
        Qt::QueuedConnection);
}

} // namespace sls
