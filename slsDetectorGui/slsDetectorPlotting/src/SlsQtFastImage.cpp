// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2026 Contributors to the SLS Detector Package
#include "SlsQtFastImage.h"

#include <QDateTime>
#include <QFontMetrics>
#include <QMouseEvent>
#include <QPainter>
#include <QWheelEvent>

#include <algorithm>
#include <cmath>
#include <cstring>

namespace sls {

namespace {

constexpr int kScaleBarWidth = 50; // optional in-view colour bar, widget px
constexpr QRgb kWhite = qRgb(255, 255, 255);

} // namespace

SlsQtFastImage::SlsQtFastImage(QWidget *parent) : QWidget(parent) {
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    setMinimumSize(200, 200);
    setAttribute(Qt::WA_OpaquePaintEvent, true);
    setAttribute(Qt::WA_NoSystemBackground, true);

    const QFont base = font();
    m_titleFont = base;
    m_titleFont.setPixelSize(13);
    m_titleFont.setBold(true);
    m_axisTitleFont = base;
    m_axisTitleFont.setPixelSize(11);
    m_tickFont = base;
    m_tickFont.setPixelSize(10);

    setPaletteLut(plot::paletteLut(plot::Palette::Color));
}

SlsQtFastImage::~SlsQtFastImage() = default;

// ---------------------------------------------------------------------------
// image input
// ---------------------------------------------------------------------------
void SlsQtFastImage::setImageCopy(const float *data, int nx, int ny) {
    if (!data || nx <= 0 || ny <= 0)
        return;
    const size_t n = static_cast<size_t>(nx) * static_cast<size_t>(ny);
    if (m_owned.size() != n)
        m_owned.resize(n);
    std::memcpy(m_owned.data(), data, n * sizeof(float));
    const bool newSize = (nx != m_nx || ny != m_ny);
    m_data = m_owned.data();
    m_nx = nx;
    m_ny = ny;
    m_dirty = true;
    if (newSize) {
        m_hist.clear();
        m_histMs = 0;
        m_vpW = -1.0;
        m_vpH = -1.0;
    }
    if (m_vpW < 0)
        m_vpW = nx;
    if (m_vpH < 0)
        m_vpH = ny;
    clampViewport();
    update();
}

void SlsQtFastImage::setImageCopy(const double *data, int nx, int ny) {
    if (!data || nx <= 0 || ny <= 0)
        return;
    const size_t n = static_cast<size_t>(nx) * static_cast<size_t>(ny);
    if (m_owned.size() != n)
        m_owned.resize(n);
    for (size_t i = 0; i < n; i++)
        m_owned[i] = static_cast<float>(data[i]);
    const bool newSize = (nx != m_nx || ny != m_ny);
    m_data = m_owned.data();
    m_nx = nx;
    m_ny = ny;
    m_dirty = true;
    if (newSize) {
        m_hist.clear();
        m_histMs = 0;
        m_vpW = -1.0;
        m_vpH = -1.0;
    }
    if (m_vpW < 0)
        m_vpW = nx;
    if (m_vpH < 0)
        m_vpH = ny;
    clampViewport();
    update();
}

// ---------------------------------------------------------------------------
// colour mapping
// ---------------------------------------------------------------------------
void SlsQtFastImage::setPaletteLut(const QRgb *lut) {
    if (!lut)
        return;
    if (onPaletteChanged)
        onPaletteChanged(lut);
    std::memcpy(m_palette, lut, sizeof(m_palette));
    m_paletteIndex = -1;
    for (int i = 0; i <= int(plot::Palette::Gain4); i++) {
        if (lut == plot::paletteLut(static_cast<plot::Palette>(i))) {
            m_paletteIndex = i;
            break;
        }
    }
    m_dirty = true;
    update();
}

int SlsQtFastImage::paletteIndex() const { return m_paletteIndex; }

void SlsQtFastImage::setLogScale(bool on) {
    if (m_logScale == on)
        return;
    m_logScale = on;
    m_histMs = 0; // bins follow the mapping
    m_dirty = true;
    update();
}

bool SlsQtFastImage::logScale() const { return m_logScale; }

void SlsQtFastImage::setIntegerValues(bool on) {
    m_integerValues = on;
    update();
}

bool SlsQtFastImage::integerValues() const { return m_integerValues; }

void SlsQtFastImage::setZeroWhite(bool on) {
    m_zeroWhite = on;
    m_dirty = true;
    update();
}

bool SlsQtFastImage::zeroWhite() const { return m_zeroWhite; }

void SlsQtFastImage::setColorRange(double lo, double hi) {
    m_clo = lo;
    m_chi = hi;
    m_defClo = lo;
    m_defChi = hi;
    m_histMs = 0; // the bins follow the window: recompute on the next render
    m_dirty = true;
    if (onColorRangeChanged)
        onColorRangeChanged(lo, hi);
    update();
}

void SlsQtFastImage::colorRange(double &lo, double &hi) const {
    lo = m_clo;
    hi = m_chi;
}

const std::vector<int> &SlsQtFastImage::histogram() const { return m_hist; }

void SlsQtFastImage::histogramRange(double &lo, double &hi) const {
    lo = m_histLo;
    hi = m_histHi;
}

void SlsQtFastImage::invalidateHistogram() {
    m_hist.clear();
    m_histMs = 0;
}

// ---------------------------------------------------------------------------
// geometry / zoom
// ---------------------------------------------------------------------------
void SlsQtFastImage::setPixelAspectX(double a) {
    m_pixelAspectX = (a > 0.0 && std::isfinite(a)) ? a : 1.0;
    update();
}

double SlsQtFastImage::pixelAspectX() const { return m_pixelAspectX; }

void SlsQtFastImage::setViewport(double x1, double y1, double x2, double y2) {
    if (!std::isfinite(x1) || !std::isfinite(y1) || !std::isfinite(x2) ||
        !std::isfinite(y2))
        return;
    if (x2 - x1 < kMinimumViewport || y2 - y1 < kMinimumViewport)
        return;
    m_vpX = x1;
    m_vpY = y1;
    m_vpW = x2 - x1;
    m_vpH = y2 - y1;
    clampViewport();
    update();
}

void SlsQtFastImage::viewport(double &x, double &y, double &w,
                              double &h) const {
    x = m_vpX;
    y = m_vpY;
    w = vpW();
    h = vpH();
}

void SlsQtFastImage::resetView() {
    if (!hasImage())
        return; // nothing to reset yet (and 0,0,0,0 would collapse the map)
    m_vpX = 0.0;
    m_vpY = 0.0;
    m_vpW = m_nx;
    m_vpH = m_ny;
    if (onZoomChanged)
        onZoomChanged(0.0, 0.0, double(m_nx), double(m_ny));
    update();
}

bool SlsQtFastImage::hasImage() const { return m_nx > 0 && m_ny > 0; }

int SlsQtFastImage::imageWidth() const { return m_nx; }

int SlsQtFastImage::imageHeight() const { return m_ny; }

QPointF SlsQtFastImage::widgetToData(const QPointF &widgetPos) const {
    if (!hasImage())
        return QPointF(0.0, 0.0);
    const Fit fit = computeFit();
    if (!(fit.sx > 0.0) || !(fit.sy > 0.0))
        return QPointF(0.0, 0.0);
    const double x =
        m_vpX + (widgetPos.x() - fit.ir.left() - fit.ox) / fit.sx;
    const double y = (m_vpY + vpH()) -
                     (widgetPos.y() - fit.ir.top() - fit.oy) / fit.sy;
    return QPointF(x, y);
}

QPointF SlsQtFastImage::dataToWidget(const QPointF &dataPos) const {
    if (!hasImage())
        return QPointF(0.0, 0.0);
    const Fit fit = computeFit();
    const double wx = fit.ir.left() + fit.ox + (dataPos.x() - m_vpX) * fit.sx;
    const double wy = fit.ir.top() + fit.oy +
                      ((m_vpY + vpH()) - dataPos.y()) * fit.sy;
    return QPointF(wx, wy);
}

// ---------------------------------------------------------------------------
// overlays / ROI editor
// ---------------------------------------------------------------------------
void SlsQtFastImage::setRoiRect(double x0, double y0, double x1, double y1,
                                bool on) {
    m_roiX0 = x0;
    m_roiY0 = y0;
    m_roiX1 = x1;
    m_roiY1 = y1;
    m_roiOn = on;
    update();
}

void SlsQtFastImage::setRoiRectVisible(bool on) {
    m_roiOn = on;
    update();
}

bool SlsQtFastImage::roiRect(double &x0, double &y0, double &x1,
                             double &y1) const {
    x0 = m_roiX0;
    y0 = m_roiY0;
    x1 = m_roiX1;
    y1 = m_roiY1;
    return m_roiOn;
}

void SlsQtFastImage::setOverlayRects(const std::vector<QRectF> &rects) {
    m_overlays = rects;
    update();
}

void SlsQtFastImage::clearOverlayRects() {
    m_overlays.clear();
    update();
}

const std::vector<QRectF> &SlsQtFastImage::overlayRects() const {
    return m_overlays;
}

void SlsQtFastImage::setReceiverRoiRect(double x0, double y0, double x1,
                                        double y1, bool on) {
    m_rxX0 = x0;
    m_rxY0 = y0;
    m_rxX1 = x1;
    m_rxY1 = y1;
    m_rxOn = on;
    update();
}

void SlsQtFastImage::clearReceiverRoiRect() {
    m_rxOn = false;
    update();
}

void SlsQtFastImage::beginRoiEdit(bool on) {
    m_roiEdit = on;
    m_roiGrab = 0;
    setCursor(on ? Qt::CrossCursor : Qt::ArrowCursor);
    update();
}

bool SlsQtFastImage::roiEditActive() const { return m_roiEdit; }

void SlsQtFastImage::setColorBarVisible(bool on) {
    m_showBar = on;
    update();
}

bool SlsQtFastImage::colorBarVisible() const { return m_showBar; }

bool SlsQtFastImage::isScaleDragging() const { return m_dragScale; }

void SlsQtFastImage::setAxesVisible(bool on) {
    m_axesVisible = on;
    update();
}

bool SlsQtFastImage::axesVisible() const { return m_axesVisible; }

void SlsQtFastImage::setZoomEnabled(bool on) {
    m_zoomEnabled = on;
    if (!on) {
        m_dragging = false;
        m_dragScale = false;
    }
    update();
}

bool SlsQtFastImage::zoomEnabled() const { return m_zoomEnabled; }

// ---------------------------------------------------------------------------
// titles
// ---------------------------------------------------------------------------
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

void SlsQtFastImage::setTickFont(const QFont &f) {
    m_tickFont = f;
    update();
}

QFont SlsQtFastImage::tickFont() const { return m_tickFont; }

// ---------------------------------------------------------------------------
// layout helpers
// ---------------------------------------------------------------------------
QRect SlsQtFastImage::imageRect() const {
    const QFontMetrics titleFm(m_titleFont);
    const QFontMetrics axisFm(m_axisTitleFont);
    int top = m_axesVisible ? m_theme.topMargin : 0;
    int bottom = m_axesVisible ? m_theme.bottomMargin : 0;
    int left = m_axesVisible ? m_theme.leftMargin : 0;
    int right = m_axesVisible ? m_theme.rightMargin : 0;
    if (!m_title.isEmpty())
        top += titleFm.height() + 4;
    if (!m_xTitle.isEmpty())
        bottom += axisFm.height() + 4;
    if (!m_yTitle.isEmpty())
        left += axisFm.height() + 6;
    if (m_showBar)
        right += kScaleBarWidth;
    int x0 = left, y0 = top;
    int x1 = width() - right, y1 = height() - bottom;
    if (x1 - x0 < 10)
        x1 = x0 + 10;
    if (y1 - y0 < 10)
        y1 = y0 + 10;
    return QRect(x0, y0, x1 - x0, y1 - y0);
}

QRect SlsQtFastImage::scaleRect() const {
    if (!m_showBar)
        return QRect();
    const QRect ir = imageRect();
    return QRect(ir.right() + 4, ir.top(), kScaleBarWidth - 4, ir.height());
}

SlsQtFastImage::Fit SlsQtFastImage::computeFit() const {
    Fit f;
    f.ir = imageRect();
    if (!hasImage() || !(vpW() > 0.0) || !(vpH() > 0.0))
        return f;
    if (m_pixelAspectX != 1.0) {
        const double fitY = double(f.ir.height()) / vpH();
        const double fitX = double(f.ir.width()) / (vpW() * m_pixelAspectX);
        f.sy = std::min(fitY, fitX);
        f.sx = f.sy * m_pixelAspectX;
    } else {
        const double fitX = double(f.ir.width()) / vpW();
        const double fitY = double(f.ir.height()) / vpH();
        f.sx = f.sy = std::min(fitX, fitY);
    }
    f.ox = (f.ir.width() - vpW() * f.sx) * 0.5;
    f.oy = (f.ir.height() - vpH() * f.sy) * 0.5;
    return f;
}

// Keeps the viewport inside the image and no smaller than the minimum.
void SlsQtFastImage::clampViewport() {
    if (!hasImage())
        return;
    if (m_vpW > m_nx)
        m_vpW = m_nx;
    if (m_vpH > m_ny)
        m_vpH = m_ny;
    if (m_vpW < std::min(kMinimumViewport, double(m_nx)))
        m_vpW = std::min(kMinimumViewport, double(m_nx));
    if (m_vpH < std::min(kMinimumViewport, double(m_ny)))
        m_vpH = std::min(kMinimumViewport, double(m_ny));
    if (m_vpX < 0.0)
        m_vpX = 0.0;
    if (m_vpX + vpW() > m_nx)
        m_vpX = m_nx - vpW();
    if (m_vpY < 0.0)
        m_vpY = 0.0;
    if (m_vpY + vpH() > m_ny)
        m_vpY = m_ny - vpH();
}

// The whole-frame view is letterboxed on purpose; any other view keeps the
// shape of the plot area so the image always fills it (used on resize and
// when the viewport is initialised for zoom, not during wheel zoom).
void SlsQtFastImage::matchViewportToWindow() {
    if (!hasImage())
        return;
    if (vpW() >= m_nx - 0.5 && vpH() >= m_ny - 0.5)
        return;
    const QRect ir = imageRect();
    if (ir.width() <= 0 || ir.height() <= 0)
        return;
    const double ar = double(ir.width()) / double(ir.height()) / m_pixelAspectX;
    double w = vpW(), h = vpH();
    if (w / h > ar)
        h = w / ar; // too wide -> grow vertically
    else
        w = h * ar; // too tall -> grow horizontally
    if (w > m_nx) { // grown past the sensor: shrink both, keep the shape
        h *= m_nx / w;
        w = m_nx;
    }
    if (h > m_ny) {
        w *= m_ny / h;
        h = m_ny;
    }
    const double cx = m_vpX + vpW() / 2.0, cy = m_vpY + vpH() / 2.0;
    m_vpW = w;
    m_vpH = h;
    m_vpX = cx - w / 2.0;
    m_vpY = cy - h / 2.0;
    clampViewport();
    if (onZoomChanged)
        onZoomChanged(m_vpX, m_vpY, m_vpX + vpW(), m_vpY + vpH());
}

void SlsQtFastImage::resizeEvent(QResizeEvent *) {
    matchViewportToWindow();
    update();
}

// ---------------------------------------------------------------------------
// composition: one scanline write per image row, exactly one flip
// ---------------------------------------------------------------------------
void SlsQtFastImage::renderImage() {
    m_dirty = false;
    if (!m_data || m_nx <= 0 || m_ny <= 0)
        return;
    if (m_image.size() != QSize(m_nx, m_ny) ||
        m_image.format() != QImage::Format_RGB32)
        m_image = QImage(m_nx, m_ny, QImage::Format_RGB32);

    // log: values <= 0 are capped at 1e-2 (log10 = -2), so an empty pixel is
    // still distinguishable instead of becoming -inf
    const double loD = m_logScale ? sls::plot::logClamp(m_clo) : m_clo;
    const double hiD = m_logScale ? sls::plot::logClamp(m_chi) : m_chi;
    const double range = (hiD > loD) ? (hiD - loD) : 1.0;
    const double scale = 255.0 / range;
    const bool logScale = m_logScale;
    const bool zeroWhite = m_zeroWhite;
    const QRgb *palette = m_palette;

    for (int y = 0; y < m_ny; y++) {
        // QImage row 0 is the top, data row 0 is the bottom: the flip happens
        // here, once, and nowhere else.
        QRgb *line = reinterpret_cast<QRgb *>(m_image.scanLine(m_ny - 1 - y));
        const float *src = m_data + static_cast<size_t>(y) * size_t(m_nx);
        if (logScale) {
            for (int x = 0; x < m_nx; x++) {
                const double v = src[x];
                if (!std::isfinite(v)) {
                    line[x] = palette[0];
                    continue;
                }
                if (zeroWhite && !(v > 0.0)) {
                    line[x] = kWhite;
                    continue;
                }
                int idx = int((sls::plot::logClamp(v) - loD) * scale);
                if (idx < 0)
                    idx = 0;
                else if (idx > 255)
                    idx = 255;
                line[x] = palette[idx];
            }
        } else {
            for (int x = 0; x < m_nx; x++) {
                const double v = src[x];
                if (!std::isfinite(v)) {
                    line[x] = palette[0];
                    continue;
                }
                if (zeroWhite && !(v > 0.0)) {
                    line[x] = kWhite;
                    continue;
                }
                int idx = int((v - loD) * scale);
                if (idx < 0)
                    idx = 0;
                else if (idx > 255)
                    idx = 255;
                line[x] = palette[idx];
            }
        }
    }

    // Histogram for the scale panel: walking the whole frame is cheap enough at
    // the image cadence, but the panel only needs the shape, so a few times a
    // second is plenty.  Bin over the window the strip displays (the colour
    // range widened by half a range; log space when log), skip values outside
    // it - a photon map accumulates to thousands of counts while its scale
    // shows 0..12, so full-range binning would put the whole window in one bin.
    const qint64 now = QDateTime::currentMSecsSinceEpoch();
    if (m_hist.empty() || m_hist.size() != 256 || now - m_histMs > 300) {
        m_histMs = now;
        double dlo, dhi;
        if (m_logScale) {
            const double a = sls::plot::logClamp(m_clo);
            const double b = sls::plot::logClamp(m_chi);
            dlo = a;
            dhi = b;
        } else {
            dlo = m_clo;
            dhi = m_chi;
        }
        if (!std::isfinite(dlo) || !std::isfinite(dhi))
            dlo = 0.0, dhi = 1.0;
        if (!(dhi > dlo))
            dhi = dlo + 1.0;
        const double span = dhi - dlo;
        double hlo = dlo - 0.5 * span;
        double hhi = dhi + 0.5 * span;
        if (!(hhi > hlo))
            hhi = hlo + 1.0;
        m_histLo = hlo;
        m_histHi = hhi;
        m_hist.assign(256, 0);
        const double sc = 255.0 / (hhi - hlo);
        const size_t n = size_t(m_nx) * size_t(m_ny);
        for (size_t p = 0; p < n; p++) {
            const double raw = m_data[p];
            if (!std::isfinite(raw))
                continue;
            const double v = logScale ? sls::plot::logClamp(raw) : raw;
            if (v < hlo || v > hhi)
                continue; // outside the visible window
            int b = int((v - hlo) * sc);
            if (b < 0)
                b = 0;
            else if (b > 255)
                b = 255;
            m_hist[b]++;
        }
        if (onImageChanged)
            onImageChanged();
    }
}

// ---------------------------------------------------------------------------
// painting
// ---------------------------------------------------------------------------
void SlsQtFastImage::paintEvent(QPaintEvent *) {
    if (m_dirty)
        renderImage();
    QPainter p(this);
    p.fillRect(rect(), m_theme.background);
    if (!hasImage())
        return;

    const Fit fit = computeFit();
    m_sx = fit.sx;
    drawImage(p, fit);
    drawAxes(p, fit);
    drawPixelDetail(p, fit);
    // Overlays last (before titles) so a ROI edge that lies on a pixel
    // boundary is not hidden by the pixel grid.
    drawOverlays(p, fit);
    if (onPaintOverlay) {
        // Data-space overlay hook (contour lines etc.), clipped like the image.
        p.save();
        p.setClipRect(fit.ir);
        const QTransform dataToWidget(
            fit.sx, 0.0, 0.0, -fit.sy,
            fit.ir.left() + fit.ox - m_vpX * fit.sx,
            fit.ir.top() + fit.oy + (m_vpY + vpH()) * fit.sy);
        onPaintOverlay(p, dataToWidget);
        p.restore();
    }
    drawTitles(p, fit);
    if (m_showBar)
        drawColorBar(p);
}

void SlsQtFastImage::drawImage(QPainter &p, const Fit &fit) {
    const QRect ir = fit.ir;
    p.save();
    // Zoomed in, the scaled image is larger than the plot area: without this
    // clip it spills over the frame and the tick labels.
    p.setClipRect(ir);
    p.translate(ir.left() + fit.ox, ir.top() + fit.oy);
    p.scale(fit.sx, fit.sy);
    // Display row of the top edge of the viewport (QImage rows are top-down).
    const double topRow = double(m_ny) - (m_vpY + vpH());
    p.translate(-m_vpX, -topRow);
    p.drawImage(QPoint(0, 0), m_image);
    p.restore();
}

void SlsQtFastImage::drawOverlays(QPainter &p, const Fit &fit) {
    const QRect ir = fit.ir;
    auto toWidget = [&](double x, double y) {
        return QPointF(ir.left() + fit.ox + (x - m_vpX) * fit.sx,
                       ir.top() + fit.oy + ((m_vpY + vpH()) - y) * fit.sy);
    };
    auto rectWidget = [&](double x0, double y0, double x1, double y1) {
        const QRectF r(toWidget(x0, y0), toWidget(x1, y1));
        return r.normalized();
    };

    // Overlays live in the same plot area: clip them as well.
    p.save();
    p.setClipRect(ir);

    // Editable yellow ROI (gate ROI), drawn in data coordinates.
    if (m_roiOn && m_roiX1 > m_roiX0 && m_roiY1 > m_roiY0) {
        const QRectF r = rectWidget(m_roiX0, m_roiY0, m_roiX1, m_roiY1);
        QPen pen(Qt::yellow);
        pen.setWidth(m_roiEdit ? 2 : 1);
        pen.setCosmetic(true);
        p.setPen(pen);
        p.setBrush(Qt::NoBrush);
        p.drawRect(r);
        if (m_roiEdit) { // corner and edge handles: what can be grabbed
            const double hw = 3.0; // half handle size, in widget pixels
            const double xs[3] = {r.left(), r.center().x(), r.right()};
            const double ys[3] = {r.top(), r.center().y(), r.bottom()};
            p.setBrush(QColor(255, 255, 80));
            for (int iy = 0; iy < 3; iy++) {
                for (int ix = 0; ix < 3; ix++) {
                    if (ix == 1 && iy == 1)
                        continue; // centre handle would only be confusing
                    p.drawRect(
                        QRectF(xs[ix] - hw, ys[iy] - hw, 2 * hw, 2 * hw));
                }
            }
        }
    }

    // Additional yellow rectangles (e.g. all receiver ROIs).
    if (!m_overlays.empty()) {
        QPen pen(Qt::yellow);
        pen.setWidth(1);
        pen.setCosmetic(true);
        p.setPen(pen);
        p.setBrush(Qt::NoBrush);
        for (const QRectF &o : m_overlays) {
            if (!(o.width() > 0.0) || !(o.height() > 0.0))
                continue;
            p.drawRect(rectWidget(o.left(), o.top(), o.left() + o.width(),
                                  o.top() + o.height()));
        }
    }

    // Red receiver-ROI box, same data coordinates.
    if (m_rxOn && m_rxX1 > m_rxX0 && m_rxY1 > m_rxY0) {
        QPen pen(Qt::red);
        pen.setWidth(1);
        pen.setCosmetic(true);
        p.setPen(pen);
        p.setBrush(Qt::NoBrush);
        p.drawRect(rectWidget(m_rxX0, m_rxY0, m_rxX1, m_rxY1));
    }
    p.restore(); // back to the full widget for the axes
}

void SlsQtFastImage::drawAxes(QPainter &p, const Fit &fit) {
    if (!m_axesVisible)
        return;
    const QRect ir = fit.ir;
    p.setFont(m_tickFont);
    const double x0 = m_vpX, x1 = m_vpX + vpW();
    const double y0 = m_vpY, y1 = m_vpY + vpH();
    const double stepX = sls::plot::niceStep(x1 - x0, 8);
    const double stepY = sls::plot::niceStep(y1 - y0, 8);
    const std::vector<double> ticksX = sls::plot::computeLinearTicks(x0, x1, 8);
    const std::vector<double> ticksY = sls::plot::computeLinearTicks(y0, y1, 8);

    p.save();
    p.setClipRect(ir);
    QPen grid(m_theme.grid);
    grid.setStyle(Qt::DotLine);
    QPen tick(m_theme.tick);
    tick.setWidth(1);
    for (double gx : ticksX) {
        const double wx = fit.ir.left() + fit.ox + (gx - m_vpX) * fit.sx;
        p.setPen(grid);
        p.drawLine(QPointF(wx, ir.top()), QPointF(wx, ir.bottom()));
    }
    for (double gy : ticksY) {
        const double wy =
            fit.ir.top() + fit.oy + ((m_vpY + vpH()) - gy) * fit.sy;
        p.setPen(grid);
        p.drawLine(QPointF(ir.left(), wy), QPointF(ir.right(), wy));
    }
    p.restore();

    // ticks and labels outside the frame
    p.save();
    p.setPen(tick);
    for (double gx : ticksX) {
        const double wx = fit.ir.left() + fit.ox + (gx - m_vpX) * fit.sx;
        p.drawLine(QPointF(wx, ir.bottom() + 1), QPointF(wx, ir.bottom() + 4));
        p.setPen(m_theme.tick);
        p.drawText(QRectF(wx - 30, ir.bottom() + 5, 60, 14),
                   Qt::AlignHCenter | Qt::AlignTop,
                   sls::plot::formatTick(gx, stepX));
        p.setPen(tick);
    }
    for (double gy : ticksY) {
        const double wy =
            fit.ir.top() + fit.oy + ((m_vpY + vpH()) - gy) * fit.sy;
        p.drawLine(QPointF(ir.left() - 4, wy), QPointF(ir.left() - 1, wy));
        p.drawText(QRectF(ir.left() - 50, wy - 7, 44, 14),
                   Qt::AlignRight | Qt::AlignVCenter,
                   sls::plot::formatTick(gy, stepY));
    }
    p.setPen(QPen(m_theme.frame));
    p.setBrush(Qt::NoBrush);
    p.drawRect(ir.adjusted(0, 0, -1, -1));
    p.restore();
}

void SlsQtFastImage::drawPixelDetail(QPainter &p, const Fit &fit) {
    // Pixel outlines and in-pixel values, once one pixel is big enough.  The
    // thresholds are computed from the fit scale, not from a fixed zoom level.
    if (m_sx < 16.0)
        return;
    const QRect ir = fit.ir;
    const double x0 = m_vpX, x1 = m_vpX + vpW();
    const double y0 = m_vpY, y1 = m_vpY + vpH();
    p.save();
    p.setClipRect(ir);
    // thin outline around every pixel (only when a sane number is visible)
    if (vpW() <= 256 && vpH() <= 256) {
        QPen px(QColor(55, 55, 55));
        px.setCosmetic(true);
        p.setPen(px);
        for (int gx = int(std::floor(x0)); gx <= int(std::ceil(x1)); gx++) {
            const double wx = ir.left() + fit.ox + (gx - m_vpX) * fit.sx;
            if (wx < ir.left() - 0.5 || wx > ir.right() + 0.5)
                continue;
            p.drawLine(QPointF(wx, ir.top()), QPointF(wx, ir.bottom()));
        }
        for (int gy = int(std::floor(y0)); gy <= int(std::ceil(y1)); gy++) {
            const double wy =
                ir.top() + fit.oy + ((m_vpY + vpH()) - gy) * fit.sy;
            if (wy < ir.top() - 0.5 || wy > ir.bottom() + 0.5)
                continue;
            p.drawLine(QPointF(ir.left(), wy), QPointF(ir.right(), wy));
        }
    }
    if (m_sx >= 50.0 && vpW() <= 64 && vpH() <= 64) {
        const int maxChars = m_integerValues ? 5 : 6;
        const double cell = std::min(fit.sx, fit.sy);
        const double bySize = cell * 0.30;
        const double byWidth = fit.sx * 0.92 / (0.62 * maxChars);
        QFont f = m_tickFont;
        f.setPixelSize(std::max(
            7, std::min(20, int(std::min(bySize, byWidth)))));
        p.setFont(f);
        for (int gy = int(std::floor(y0)); gy <= int(std::ceil(y1)); gy++) {
            if (gy < 0 || gy >= m_ny)
                continue;
            for (int gx = int(std::floor(x0)); gx <= int(std::ceil(x1)); gx++) {
                if (gx < 0 || gx >= m_nx)
                    continue;
                const double v = m_data[gx + gy * m_nx];
                if (!std::isfinite(v))
                    continue;
                // text colour from the pixel that was actually painted
                const QRgb c = m_image.pixel(gx, m_ny - 1 - gy);
                const int lum =
                    (qRed(c) * 299 + qGreen(c) * 587 + qBlue(c) * 114) / 1000;
                p.setPen(lum > 140 ? QColor(15, 15, 15)
                                   : QColor(245, 245, 245));
                const QString txt =
                    m_integerValues
                        ? QString::number(qint64(std::llround(v)))
                        : QString::number(v, 'f', 1);
                const QPointF ctr =
                    QPointF(ir.left() + fit.ox + (gx + 0.5 - m_vpX) * fit.sx,
                            ir.top() + fit.oy +
                                ((m_vpY + vpH()) - (gy + 0.5)) * fit.sy);
                p.drawText(QRectF(ctr.x() - fit.sx * 0.5,
                                  ctr.y() - fit.sy * 0.5, fit.sx, fit.sy),
                           Qt::AlignCenter, txt);
            }
        }
    }
    p.restore();
}

void SlsQtFastImage::drawTitles(QPainter &p, const Fit &fit) {
    const QRect ir = fit.ir;
    if (!m_title.isEmpty()) {
        p.setFont(m_titleFont);
        p.setPen(m_theme.text);
        p.drawText(QRectF(0, 1, width(), ir.top() - 2),
                   Qt::AlignHCenter | Qt::AlignVCenter, m_title);
    }
    if (!m_yTitle.isEmpty()) {
        const QFontMetrics fm(m_axisTitleFont);
        p.save();
        p.setFont(m_axisTitleFont);
        p.setPen(m_theme.text);
        p.translate(fm.height() / 2.0 + 4.0, ir.center().y());
        p.rotate(-90.0);
        p.drawText(QRectF(-ir.height() / 2.0, -fm.height() / 2.0,
                          ir.height(), fm.height()),
                   Qt::AlignCenter, m_yTitle);
        p.restore();
    }
    if (!m_xTitle.isEmpty()) {
        p.setFont(m_axisTitleFont);
        p.setPen(m_theme.text);
        p.drawText(QRectF(ir.left(), ir.bottom() + m_theme.bottomMargin,
                          ir.width(),
                          height() - ir.bottom() - m_theme.bottomMargin),
                   Qt::AlignHCenter | Qt::AlignTop, m_xTitle);
    }
}

void SlsQtFastImage::drawColorBar(QPainter &p) {
    const QRect sr = scaleRect();
    if (sr.height() <= 0)
        return;
    for (int yy = sr.top(); yy <= sr.bottom(); yy++) {
        const double frac = 1.0 - double(yy - sr.top()) / sr.height();
        int idx = int(frac * 255.0);
        if (idx < 0)
            idx = 0;
        else if (idx > 255)
            idx = 255;
        p.setPen(m_palette[idx]);
        p.drawLine(sr.left(), yy, sr.right(), yy);
    }
    p.setPen(m_theme.frame);
    p.drawRect(sr.adjusted(-1, -1, 1, 1));
    p.setFont(m_tickFont);
    p.setPen(m_theme.text);
    p.drawText(sr.left(), sr.top() + 1, sr.width(), 12, Qt::AlignCenter,
               QString::number(m_chi, 'g', 4));
    p.drawText(sr.left(), sr.bottom() - 11, sr.width(), 12, Qt::AlignCenter,
               QString::number(m_clo, 'g', 4));
}

double SlsQtFastImage::valueAt(const QPoint &pos) const {
    const QRect sr = scaleRect();
    if (sr.height() <= 0 || !sr.contains(pos))
        return m_clo - 1.0;
    const double frac = 1.0 - double(pos.y() - sr.top()) / sr.height();
    return m_clo + frac * (m_chi - m_clo);
}

// ---------------------------------------------------------------------------
// interaction
// ---------------------------------------------------------------------------
void SlsQtFastImage::mousePressEvent(QMouseEvent *e) {
    if (e->button() != Qt::LeftButton)
        return;
    const QPoint ip = e->position().toPoint();
    if (m_roiEdit && hasImage()) {
        // which part of the yellow rectangle was grabbed?
        const QPointF d = widgetToData(QPointF(ip));
        const QPointF d2 = widgetToData(QPointF(ip + QPoint(8, 0)));
        const double tol = std::max(1e-9, std::abs(d2.x() - d.x()));
        const bool nearL = std::abs(d.x() - m_roiX0) <= tol;
        const bool nearR = std::abs(d.x() - m_roiX1) <= tol;
        const bool nearB = std::abs(d.y() - m_roiY0) <= tol;
        const bool nearT = std::abs(d.y() - m_roiY1) <= tol;
        const bool inX = d.x() >= m_roiX0 - tol && d.x() <= m_roiX1 + tol;
        const bool inY = d.y() >= m_roiY0 - tol && d.y() <= m_roiY1 + tol;
        m_roiGrab = 0;
        if (nearL && nearB)
            m_roiGrab = 6;
        else if (nearR && nearB)
            m_roiGrab = 7;
        else if (nearL && nearT)
            m_roiGrab = 8;
        else if (nearR && nearT)
            m_roiGrab = 9;
        else if (nearL && inY)
            m_roiGrab = 2;
        else if (nearR && inY)
            m_roiGrab = 3;
        else if (nearB && inX)
            m_roiGrab = 4;
        else if (nearT && inX)
            m_roiGrab = 5;
        else if (inX && inY)
            m_roiGrab = 1;
        else
            m_roiGrab = 10; // start a new rectangle at the cursor
        m_roiPress = d;
        m_roiStart[0] = m_roiX0;
        m_roiStart[1] = m_roiY0;
        m_roiStart[2] = m_roiX1;
        m_roiStart[3] = m_roiY1;
        if (m_roiGrab == 10) {
            m_roiX0 = m_roiX1 = std::max(0.0, std::min(double(m_nx), d.x()));
            m_roiY0 = m_roiY1 = std::max(0.0, std::min(double(m_ny), d.y()));
            m_roiOn = true;
        }
        update();
        return;
    }
    const QRect sr = scaleRect();
    if (sr.height() > 0 && sr.contains(ip)) {
        m_dragScale = true;
        m_dragScaleY = ip.y();
        m_dragClo = m_clo;
        m_dragChi = m_chi;
        m_dragMax = (ip.y() < sr.top() + sr.height() / 2);
        setCursor(Qt::SizeVerCursor);
        return;
    }
    if (!m_zoomEnabled)
        return; // fixed view (gain map): no panning
    m_dragging = true;
    m_lastPos = QPointF(ip);
    m_dragVpX = m_vpX;
    m_dragVpY = m_vpY;
    setCursor(Qt::ClosedHandCursor);
}

void SlsQtFastImage::mouseMoveEvent(QMouseEvent *e) {
    const QPointF pos = e->position();
    if (m_roiEdit && hasImage()) {
        const QPointF d = widgetToData(pos);
        const double x = std::max(0.0, std::min(double(m_nx), d.x()));
        const double y = std::max(0.0, std::min(double(m_ny), d.y()));
        constexpr double kMin = 2.0; // a ROI smaller than this is unusable
        if (m_roiGrab == 10) { // dragging out a new rectangle
            m_roiX0 = std::min(m_roiPress.x(), x);
            m_roiX1 = std::max(m_roiPress.x(), x);
            m_roiY0 = std::min(m_roiPress.y(), y);
            m_roiY1 = std::max(m_roiPress.y(), y);
            m_roiOn = true;
        } else if (m_roiGrab == 1) { // move the whole rectangle
            const double dx = x - m_roiPress.x();
            const double dy = y - m_roiPress.y();
            double x0 = m_roiStart[0] + dx, x1 = m_roiStart[2] + dx;
            double y0 = m_roiStart[1] + dy, y1 = m_roiStart[3] + dy;
            if (x0 < 0.0) {
                x1 -= x0;
                x0 = 0.0;
            }
            if (x1 > m_nx) {
                x0 -= (x1 - m_nx);
                x1 = m_nx;
            }
            if (y0 < 0.0) {
                y1 -= y0;
                y0 = 0.0;
            }
            if (y1 > m_ny) {
                y0 -= (y1 - m_ny);
                y1 = m_ny;
            }
            m_roiX0 = x0;
            m_roiX1 = x1;
            m_roiY0 = y0;
            m_roiY1 = y1;
        } else if (m_roiGrab != 0) { // one edge or one corner
            double x0 = m_roiStart[0], y0 = m_roiStart[1];
            double x1 = m_roiStart[2], y1 = m_roiStart[3];
            const bool left = (m_roiGrab == 2 || m_roiGrab == 6 ||
                               m_roiGrab == 8);
            const bool right = (m_roiGrab == 3 || m_roiGrab == 7 ||
                                m_roiGrab == 9);
            const bool bottom = (m_roiGrab == 4 || m_roiGrab == 6 ||
                                 m_roiGrab == 7);
            const bool top = (m_roiGrab == 5 || m_roiGrab == 8 ||
                              m_roiGrab == 9);
            if (left && x <= x1 - kMin)
                x0 = x;
            if (right && x >= x0 + kMin)
                x1 = x;
            if (bottom && y <= y1 - kMin)
                y0 = y;
            if (top && y >= y0 + kMin)
                y1 = y;
            m_roiX0 = x0;
            m_roiY0 = y0;
            m_roiX1 = x1;
            m_roiY1 = y1;
        }
        update();
        if (m_roiGrab == 0) { // hovering: show what grabbing here would do
            const QPointF dd = widgetToData(pos);
            const QPointF dd2 = widgetToData(pos + QPointF(8, 0));
            const double tol = std::max(1e-9, std::abs(dd2.x() - dd.x()));
            const bool nearL = std::abs(dd.x() - m_roiX0) <= tol;
            const bool nearR = std::abs(dd.x() - m_roiX1) <= tol;
            const bool nearB = std::abs(dd.y() - m_roiY0) <= tol;
            const bool nearT = std::abs(dd.y() - m_roiY1) <= tol;
            const bool inside = dd.x() >= m_roiX0 - tol &&
                                dd.x() <= m_roiX1 + tol &&
                                dd.y() >= m_roiY0 - tol &&
                                dd.y() <= m_roiY1 + tol;
            if ((nearL && nearT) || (nearR && nearB))
                setCursor(Qt::SizeFDiagCursor);
            else if ((nearL && nearB) || (nearR && nearT))
                setCursor(Qt::SizeBDiagCursor);
            else if (nearL || nearR)
                setCursor(Qt::SizeHorCursor);
            else if (nearT || nearB)
                setCursor(Qt::SizeVerCursor);
            else
                setCursor(inside ? Qt::OpenHandCursor : Qt::CrossCursor);
        }
        return;
    }
    if (m_dragScale) {
        const QRect sr = scaleRect();
        if (sr.height() <= 0)
            return;
        const double delta = double(e->position().toPoint().y() - m_dragScaleY) /
                             double(sr.height()) * (m_dragChi - m_dragClo);
        if (m_dragMax) {
            m_chi = m_dragChi - delta;
            if (m_chi < m_clo + 1.0)
                m_chi = m_clo + 1.0;
        } else {
            m_clo = m_dragClo - delta;
            if (m_clo > m_chi - 1.0)
                m_clo = m_chi - 1.0;
        }
        m_dirty = true;
        update();
        return;
    }
    if (!m_dragging) {
        const QPoint hp = pos.toPoint();
        const QRect ir2 = imageRect();
        const QRect sr = scaleRect();
        if (sr.height() > 0 && sr.contains(hp)) {
            setCursor(Qt::SizeVerCursor);
            setToolTip(QStringLiteral("Scale: %1")
                           .arg(valueAt(hp), 0, 'f', 0));
        } else if (ir2.contains(hp) && hasImage()) {
            setCursor(Qt::OpenHandCursor);
            // widgetToData() returns data rows (row 0 = bottom), matching the
            // ROOT plots and the ROI list: (0,0) is the bottom-left pixel.
            const QPointF d = widgetToData(pos);
            const int ix = int(std::floor(d.x()));
            const int iy = int(std::floor(d.y()));
            if (ix >= 0 && ix < m_nx && iy >= 0 && iy < m_ny)
                setToolTip(QStringLiteral("(x=%1, y=%2) = %3")
                               .arg(ix)
                               .arg(iy)
                               .arg(double(m_data[ix + iy * m_nx]), 0, 'f', 1));
        } else {
            setCursor(Qt::ArrowCursor);
            setToolTip(QString());
        }
        return;
    }
    const QPointF now = widgetToData(pos);
    const QPointF then = widgetToData(m_lastPos);
    m_vpX = m_dragVpX - (now.x() - then.x());
    m_vpY = m_dragVpY - (now.y() - then.y());
    clampViewport();
    update();
    if (onZoomChanged)
        onZoomChanged(m_vpX, m_vpY, m_vpX + vpW(), m_vpY + vpH());
}

void SlsQtFastImage::mouseReleaseEvent(QMouseEvent *e) {
    if (e->button() == Qt::LeftButton && m_roiGrab != 0) {
        m_roiGrab = 0;
        if (onRoiEdited && m_roiX1 - m_roiX0 >= 2.0 &&
            m_roiY1 - m_roiY0 >= 2.0) {
            onRoiEdited(int(std::lround(m_roiX0)),
                        int(std::lround(m_roiY0)),
                        int(std::lround(m_roiX1)),
                        int(std::lround(m_roiY1)));
        }
        return;
    }
    if (e->button() == Qt::LeftButton) {
        if (m_dragging && onZoomChanged)
            onZoomChanged(m_vpX, m_vpY, m_vpX + vpW(), m_vpY + vpH());
        m_dragging = false;
        const bool wasScale = m_dragScale;
        m_dragScale = false;
        setCursor(Qt::ArrowCursor);
        if (wasScale && onScaleReleased)
            onScaleReleased(); // one histogram/scale refresh, after the drag
    }
}

void SlsQtFastImage::wheelEvent(QWheelEvent *e) {
    if (!hasImage() || !m_zoomEnabled)
        return;
    const QPointF imgPt = widgetToData(e->position());
    const double mx = imgPt.x(), my = imgPt.y();

    const double factor = (e->angleDelta().y() > 0) ? 1.0 / 1.18 : 1.18;
    double nw = vpW() * factor, nh = vpH() * factor;
    if (nw < std::min(kMinimumViewport, double(m_nx)))
        nw = std::min(kMinimumViewport, double(m_nx));
    if (nh < std::min(kMinimumViewport, double(m_ny)))
        nh = std::min(kMinimumViewport, double(m_ny));
    if (nw > m_nx)
        nw = m_nx;
    if (nh > m_ny)
        nh = m_ny;

    // Keep the data point under the cursor fixed.  (The reference reshaped the
    // viewport to the window while zooming in; that moves the cursor point and
    // breaks the "zoom keeps the cursor point fixed" invariant.)
    const double fracX = (mx - m_vpX) / vpW();
    const double fracY = (my - m_vpY) / vpH();
    m_vpX = mx - fracX * nw;
    m_vpY = my - fracY * nh;
    m_vpW = nw;
    m_vpH = nh;
    clampViewport();

    if (onZoomChanged)
        onZoomChanged(m_vpX, m_vpY, m_vpX + vpW(), m_vpY + vpH());
    setToolTip(QStringLiteral("Zoom: %1%").arg(int(100.0 * m_nx / vpW())));
    update();
}

void SlsQtFastImage::mouseDoubleClickEvent(QMouseEvent *e) {
    const QRect sr = scaleRect();
    if (sr.height() > 0 && sr.contains(e->position().toPoint())) {
        m_clo = m_defClo;
        m_chi = m_defChi;
        m_histMs = 0;
        m_dirty = true;
        update();
    } else if (m_zoomEnabled) {
        resetView();
    }
}

} // namespace sls
