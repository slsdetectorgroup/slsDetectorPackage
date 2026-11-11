// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2026 Contributors to the SLS Detector Package
#include "SlsQt2DPlot.h"

#include "SlsQtFastImage.h"
#include "SlsQtPlotCommon.h"
#include "SlsQtScalePanel.h"

#include <QLabel>
#include <QPainter>
#include <QSize>
#include <QSizePolicy>
#include <QTransform>
#include <QVBoxLayout>

#include <algorithm>
#include <cmath>
#include <cstring>

namespace sls {

namespace {
constexpr int kLowResGrid = 256;  // marching-squares grid is subsampled to this
constexpr size_t kMaxContourSegs = 120000;
} // namespace

SlsQt2DPlot::SlsQt2DPlot(QWidget *parent, bool gain)
    : QWidget(parent), m_gain(gain) {
    m_hist = new SlsQt2DHist();
    std::memcpy(m_palette, plot::paletteLut(plot::Palette::Color),
                sizeof(m_palette));
    setupUi();

    m_view->onZoomChanged = [this](double x1, double y1, double x2, double y2) {
        onViewZoomed(x1, y1, x2, y2);
    };
    m_view->onColorRangeChanged = [this](double, double) {
        if (m_syncingZ)
            return;
        // The user dragged the scale/histogram markers or edited the spin
        // boxes: that range is now authoritative until the Plot tab forces a
        // new one through SetZRange()/SetZMinMax().
        m_panelZOverride = true;
        m_smoothDirty = true;
        m_contourDirty = true;
        if (m_interp)
            rebuildSmoothImage();
        if (m_contourOn)
            updateContours();
        if (m_view)
            m_view->update();
    };
    m_view->onPaletteChanged = [this](const QRgb *lut) {
        std::memcpy(m_palette, lut, sizeof(m_palette));
        m_smoothDirty = true;
        if (m_interp)
            rebuildSmoothImage();
        if (m_view)
            m_view->update();
    };
    m_view->onPaintOverlay = [this](QPainter &p, const QTransform &t) {
        paintOverlay(p, t);
    };

    if (m_gain) {
        // Plain 0..3 gain map: fixed palette/range, no axes, no zoom.  The
        // scale panel is not created for it (the parent plot keeps the
        // colour controls).
        m_view->setAxesVisible(false);
        m_view->setZoomEnabled(false);
        m_view->setIntegerValues(true);
        m_view->setPaletteLut(plot::paletteLut(plot::Palette::Gain4));
        std::memcpy(m_palette, plot::paletteLut(plot::Palette::Gain4),
                    sizeof(m_palette));
        m_view->setTitle(QStringLiteral("Gain"));
        applyZRange(0.0, 3.0);
    }

    FillTestPlot();
    Update();
}

SlsQt2DPlot::~SlsQt2DPlot() {
    // The child widgets are destroyed after this body: drop the callbacks that
    // capture this (and m_hist) so nothing can call into a half-destroyed
    // plot while the QWidget children are torn down.
    if (m_view) {
        m_view->onZoomChanged = nullptr;
        m_view->onColorRangeChanged = nullptr;
        m_view->onPaletteChanged = nullptr;
        m_view->onPaintOverlay = nullptr;
    }
    delete m_hist;
}

void SlsQt2DPlot::setupUi() {
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    m_view = new SlsQtFastImage(this);
    layout->addWidget(m_view, 1);

    if (!m_gain) {
        // One compact caption row for the z-axis (colour scale) title: the old
        // qwt plot showed it next to the colour bar, the ScalePanel replaces
        // the bar, so it lives directly above the panel (same dark style as
        // the histogram strip, no extra heavy widget).
        m_zTitleLabel = new QLabel(this);
        m_zFont = m_view->axisTitleFont();
        m_zTitleLabel->setFont(m_zFont);
        m_zTitleLabel->setAlignment(Qt::AlignCenter);
        m_zTitleLabel->setContentsMargins(0, 1, 0, 1);
        m_zTitleLabel->setSizePolicy(QSizePolicy::Preferred,
                                     QSizePolicy::Fixed);
        m_zTitleLabel->setStyleSheet(QStringLiteral(
            "QLabel { background-color: rgb(28, 28, 28);"
            " color: rgb(230, 230, 230); }"));
        m_zTitleLabel->hide(); // shown by SetZTitle() once a title is set
        layout->addWidget(m_zTitleLabel, 0);

        // Fixed 78 px histogram strip + palette/log/min-max controls.
        m_panel = new SlsQtScalePanel(m_view, this);
        layout->addWidget(m_panel, 0);
    }
}

// ---------------------------------------------------------------------------
// titles
// ---------------------------------------------------------------------------
void SlsQt2DPlot::SetTitle(QString title) { setTitle(title); }

void SlsQt2DPlot::setTitle(const QString &title) {
    if (m_view)
        m_view->setTitle(title);
}

QString SlsQt2DPlot::title() const { return m_view ? m_view->title() : QString(); }

void SlsQt2DPlot::SetXTitle(QString title) {
    if (m_view)
        m_view->setXAxisTitle(title);
}

void SlsQt2DPlot::SetYTitle(QString title) {
    if (m_view)
        m_view->setYAxisTitle(title);
}

void SlsQt2DPlot::SetZTitle(QString title) {
    m_zTitle = title;
    if (!m_zTitleLabel || m_gain)
        return;
    m_zTitleLabel->setText(title);
    // No empty caption row: the panel keeps its exact 78 px height/look when
    // no z title is configured.
    m_zTitleLabel->setVisible(!title.isEmpty());
}

void SlsQt2DPlot::SetTitleFont(const QFont &f) {
    if (m_view)
        m_view->setTitleFont(f);
}

void SlsQt2DPlot::SetXFont(const QFont &f) {
    if (m_view)
        m_view->setAxisTitleFont(f);
}

void SlsQt2DPlot::SetYFont(const QFont &f) {
    if (m_view)
        m_view->setAxisTitleFont(f);
}

void SlsQt2DPlot::SetZFont(const QFont &f) {
    m_zFont = f;
    if (m_zTitleLabel)
        m_zTitleLabel->setFont(f);
}

// ---------------------------------------------------------------------------
// viewport / zoom
// ---------------------------------------------------------------------------
void SlsQt2DPlot::UnZoom(bool replot) {
    m_applyViewPending = false;
    if (m_view && m_view->hasImage())
        m_view->resetView();
    if (replot) {
        if (m_view)
            m_view->update();
        if (m_panel)
            m_panel->refresh();
    }
}

void SlsQt2DPlot::SetZoom(double xmin, double ymin, double x_width,
                          double y_width) {
    SetZoom(QRectF(xmin, ymin, x_width, y_width));
}

void SlsQt2DPlot::SetZoom(const QRectF &rect) {
    if (!m_view || !m_hist)
        return;
    const QRectF r = rect.normalized();
    if (!(r.width() > 0.0) || !(r.height() > 0.0))
        return;
    m_applyViewPending = false;
    m_view->setViewport(dataXToImage(r.left()), dataYToImage(r.top()),
                        dataXToImage(r.right()), dataYToImage(r.bottom()));
    m_view->update();
}

void SlsQt2DPlot::DisableZoom(bool disable) {
    m_zoomDisabled = disable;
    if (m_view && !m_gain)
        m_view->setZoomEnabled(!disable);
}

void SlsQt2DPlot::EnableXAutoScaling() {
    if (m_gain)
        return;
    m_autoX = true;
    m_applyViewPending = true;
    applyPendingView();
}

void SlsQt2DPlot::EnableYAutoScaling() {
    if (m_gain)
        return;
    m_autoY = true;
    m_applyViewPending = true;
    applyPendingView();
}

void SlsQt2DPlot::SetXMinMax(double min, double max) {
    if (m_gain)
        return;
    m_autoX = false;
    m_reqX0 = min;
    m_reqX1 = max;
    m_applyViewPending = true;
    applyPendingView();
}

void SlsQt2DPlot::SetYMinMax(double min, double max) {
    if (m_gain)
        return;
    m_autoY = false;
    m_reqY0 = min;
    m_reqY1 = max;
    m_applyViewPending = true;
    applyPendingView();
}

void SlsQt2DPlot::applyPendingView() {
    m_applyViewPending = false;
    if (m_gain || !m_view || !m_hist)
        return;
    const int nx = m_hist->GetNBinsX();
    const int ny = m_hist->GetNBinsY();
    if (nx < 1 || ny < 1)
        return;

    double u0 = 0.0, u1 = double(nx);
    double v0 = 0.0, v1 = double(ny);
    if (!m_autoX) {
        u0 = dataXToImage(m_reqX0);
        u1 = dataXToImage(m_reqX1);
    }
    if (!m_autoY) {
        v0 = dataYToImage(m_reqY0);
        v1 = dataYToImage(m_reqY1);
    }
    if (u1 < u0)
        std::swap(u0, u1);
    if (v1 < v0)
        std::swap(v0, v1);

    // FastImageView refuses viewports smaller than kMinimumViewport; expand
    // instead of silently ignoring the request.
    const double kMin = SlsQtFastImage::kMinimumViewport;
    if (!(u1 - u0 >= kMin)) {
        const double c = 0.5 * (u0 + u1);
        u0 = c - 0.5 * kMin;
        u1 = c + 0.5 * kMin;
    }
    if (!(v1 - v0 >= kMin)) {
        const double c = 0.5 * (v0 + v1);
        v0 = c - 0.5 * kMin;
        v1 = c + 0.5 * kMin;
    }
    m_view->setViewport(u0, v0, u1, v1);
}

void SlsQt2DPlot::onViewZoomed(double x1, double y1, double x2, double y2) {
    if (!m_hist)
        return;
    double dx1 = imageXToData(x1), dx2 = imageXToData(x2);
    double dy1 = imageYToData(y1), dy2 = imageYToData(y2);
    if (dx2 < dx1)
        std::swap(dx1, dx2);
    if (dy2 < dy1)
        std::swap(dy1, dy2);
    emit PlotZoomedSignal(QRectF(dx1, dy1, dx2 - dx1, dy2 - dy1));
}

// ---------------------------------------------------------------------------
// z range / colour mapping
// ---------------------------------------------------------------------------
void SlsQt2DPlot::applyZRange(double lo, double hi) {
    if (!m_view)
        return;
    if (m_gain) {
        double a = 0.0, b = 0.0;
        m_view->colorRange(a, b);
        if (a == 0.0 && b == 3.0)
            return;
        m_syncingZ = true;
        m_view->setColorRange(0.0, 3.0);
        m_syncingZ = false;
        return;
    }
    double curLo = 0.0, curHi = 0.0;
    m_view->colorRange(curLo, curHi);
    if (curLo == lo && curHi == hi)
        return;
    m_syncingZ = true;
    m_view->setColorRange(lo, hi);
    m_syncingZ = false;
    m_smoothDirty = true;
    m_contourDirty = true;
    if (m_interp)
        rebuildSmoothImage();
    if (m_contourOn)
        updateContours();
    if (m_panel)
        m_panel->refresh();
}

void SlsQt2DPlot::SetZMinMax(double zmin, double zmax) {
    if (m_gain)
        return;
    m_hist->SetMinMax(zmin, zmax);
    // An explicitly requested range is authoritative: keep it across the
    // per-frame Update() calls until SetZRange(false, false, ...) clears it.
    m_userZOverride = true;
    m_panelZOverride = false;
    applyZRange(m_hist->GetMinimum(), m_hist->GetMaximum());
    if (m_view)
        m_view->update();
    if (m_panel)
        m_panel->refresh();
}

void SlsQt2DPlot::SetZMinimumToFirstGreaterThanZero() {
    if (m_gain)
        return;
    m_hist->SetMinimumToFirstGreaterThanZero();
    // This is the automatic log minimum, so it releases both overrides.
    m_userZOverride = false;
    m_panelZOverride = false;
    Update();
}

void SlsQt2DPlot::SetLogz(bool enable, bool isMin, bool isMax, double min,
                          double max) {
    LogZ(enable);
    SetZRange(isMin, isMax, min, max);
}

void SlsQt2DPlot::SetZRange(bool isMin, bool isMax, double min, double max) {
    if (m_gain)
        return;
    if (m_logZ)
        m_hist->SetMinimumToFirstGreaterThanZero();
    if (isMin || isMax) {
        double zmin = isMin ? min : m_hist->GetMinimum();
        double zmax = isMax ? max : m_hist->GetMaximum();
        if (!(zmax > zmin))
            zmax = zmin + 1.0;
        m_hist->SetMinMax(zmin, zmax);
        // An explicit range from the Plot tab (qTabPlot's min/max z controls)
        // wins over both the panel and the auto range, and must survive the
        // Update() that follows (in particular the log-min re-derivation).
        m_userZOverride = true;
        m_panelZOverride = false;
        applyZRange(zmin, zmax);
    } else {
        // Neither end is pinned: fall back to the automatic range on the next
        // Update() (the scale-panel override, if any, still takes precedence
        // there).  qDrawPlot calls this every frame while the z-range check
        // boxes are off.
        m_userZOverride = false;
    }
    Update();
}

void SlsQt2DPlot::LogZ(bool on) {
    if (m_gain)
        return;
    m_logZ = (on != 0);
    if (m_view)
        m_view->setLogScale(m_logZ);
    m_smoothDirty = true;
    m_contourDirty = true;
    if (m_panel)
        m_panel->setImageLog(m_logZ); // keeps the panel's lin/log chooser in sync
    Update();
}

// ---------------------------------------------------------------------------
// data
// ---------------------------------------------------------------------------
void SlsQt2DPlot::SetData(int nbinsx, double xmin, double xmax, int nbinsy,
                          double ymin, double ymax, double *d, double zmin,
                          double zmax) {
    m_hist->SetData(nbinsx, xmin, xmax, nbinsy, ymin, ymax, d, zmin, zmax);
    const bool newGeometry = (m_hist->GetNBinsX() != m_lastNx ||
                              m_hist->GetNBinsY() != m_lastNy);
    m_lastNx = m_hist->GetNBinsX();
    m_lastNy = m_hist->GetNBinsY();
    if (newGeometry) {
        // New frame geometry: drop the scale-panel override and fit the whole
        // frame (zoom is preserved across frames of the same size).  An
        // explicit Plot-tab range (m_userZOverride) survives: qDrawPlot still
        // has those min/max values and re-applies them every frame.
        m_panelZOverride = false;
        m_applyViewPending = true;
    }
    if (zmax != -1.0) {
        // The frame itself carries an explicit z range: it takes over from any
        // previous user/panel range (and Update() must apply it, not skip).
        m_userZOverride = false;
        m_panelZOverride = false;
    }
    pushImage();
    m_smoothDirty = true;
    m_contourDirty = true;
    Update();
}

void SlsQt2DPlot::pushImage() {
    if (!m_view || !m_hist)
        return;
    const int nx = m_hist->GetNBinsX();
    const int ny = m_hist->GetNBinsY();
    if (nx < 1 || ny < 1)
        return;
    m_view->setImageCopy(m_hist->GetDataPtr(), nx, ny);
}

void SlsQt2DPlot::SetBinValue(int bx, int by, double v) {
    m_hist->SetBinValue(bx, by, v);
    if (m_view && m_hist->GetNBinsX() > 0 && m_hist->GetNBinsY() > 0) {
        pushImage();
        m_smoothDirty = true;
        m_contourDirty = true;
        Update();
    }
}

void SlsQt2DPlot::FillTestPlot(int mode) {
    const int nx = 50;
    const int ny = 50;
    m_testData.assign(static_cast<size_t>(nx) * static_cast<size_t>(ny), 0.0);
    const double dmax =
        std::sqrt(std::pow(nx / 2.0 - 0.5, 2) + std::pow(ny / 2.0 - 0.5, 2));
    for (int i = 0; i < nx; i++) {
        for (int j = 0; j < ny; j++) {
            const double d = std::sqrt(std::pow(nx / 2.0 - (i + 0.5), 2) +
                                       std::pow(ny / 2.0 - (j + 0.5), 2));
            m_testData[static_cast<size_t>(i) +
                       static_cast<size_t>(j) * static_cast<size_t>(nx)] =
                (mode % 3) ? 10 * d / dmax : 10 * (1 - d / dmax);
        }
    }
    SetData(nx, 200, 822, ny, -0.5, ny - 0.5, m_testData.data());
}

void SlsQt2DPlot::Update() {
    if (!m_view)
        return;
    if (m_gain) {
        m_view->update();
        return;
    }
    if (m_applyViewPending)
        applyPendingView();
    if (!m_panelZOverride && !m_userZOverride && m_hist) {
        if (m_logZ)
            m_hist->SetMinimumToFirstGreaterThanZero();
        applyZRange(m_hist->GetMinimum(), m_hist->GetMaximum());
    }
    if (m_interp && m_smoothDirty)
        rebuildSmoothImage();
    if (m_contourOn && m_contourDirty)
        updateContours();
    m_view->update();
    if (m_panel)
        m_panel->refresh();
}

// ---------------------------------------------------------------------------
// rendering options
// ---------------------------------------------------------------------------
void SlsQt2DPlot::SetInterpolate(bool enable) {
    m_interp = enable;
    m_smoothDirty = true;
    if (m_interp)
        rebuildSmoothImage();
    else {
        m_smooth = QImage();
        m_smoothDirty = false;
    }
    if (m_view)
        m_view->update();
}

void SlsQt2DPlot::SetContour(bool enable) {
    m_contourOn = enable;
    m_contourDirty = true;
    if (m_contourOn)
        updateContours();
    else
        m_contourSegments.clear();
    if (m_view)
        m_view->update();
}

void SlsQt2DPlot::showSpectrogram(bool on) {
    m_showImage = on;
    if (m_view)
        m_view->update();
}

// ---------------------------------------------------------------------------
// ROI overlays: the ROI list is in bin indices; pixel (ix,iy) covers
// [ix,ix+1)x[iy,iy+1) in FastImageView coordinates, so the conversion is the
// identity on the index (not on the data coordinate).
// ---------------------------------------------------------------------------
void SlsQt2DPlot::EnableRoiBoxes(std::vector<slsDetectorDefs::ROI> roi) {
    m_roiRects.clear();
    m_roiRects.reserve(roi.size());
    for (const auto &r : roi) {
        if (r.xmax < r.xmin || r.ymax < r.ymin)
            continue;
        m_roiRects.emplace_back(QRectF(double(r.xmin), double(r.ymin),
                                       double(r.xmax - r.xmin + 1),
                                       double(r.ymax - r.ymin + 1)));
    }
    if (m_view) {
        m_view->setOverlayRects(m_roiRects);
        m_view->update();
    }
}

void SlsQt2DPlot::DisableRoiBoxes() {
    m_roiRects.clear();
    if (m_view) {
        m_view->clearOverlayRects();
        m_view->update();
    }
}

// ---------------------------------------------------------------------------
// coordinate conversions: data (bin-centre) <-> image (pixel-edge)
// ---------------------------------------------------------------------------
double SlsQt2DPlot::dataXToImage(double x) const {
    const double w = m_hist->GetXBinWidth();
    return (w > 0.0) ? (x - m_hist->GetXMin()) / w : 0.0;
}

double SlsQt2DPlot::dataYToImage(double y) const {
    const double w = m_hist->GetYBinWidth();
    return (w > 0.0) ? (y - m_hist->GetYMin()) / w : 0.0;
}

double SlsQt2DPlot::imageXToData(double u) const {
    return m_hist->GetXMin() + u * m_hist->GetXBinWidth();
}

double SlsQt2DPlot::imageYToData(double v) const {
    return m_hist->GetYMin() + v * m_hist->GetYBinWidth();
}

// ---------------------------------------------------------------------------
// bilinear interpolation overlay
//
// SlsQtFastImage always displays one colour per data pixel.  When the user
// enables "interpolate" we compose the same colour-mapped image at data
// resolution and let QPainter resample it bilinearly on top of the (opaque)
// pixel image; pixel detail / deep zoom keeps the crisp pixel view.  The ROI
// boxes are redrawn above it so they stay visible.
// ---------------------------------------------------------------------------
void SlsQt2DPlot::rebuildSmoothImage() {
    m_smoothDirty = false;
    if (!m_interp || !m_view || !m_hist) {
        m_smooth = QImage();
        return;
    }
    const int nx = m_hist->GetNBinsX();
    const int ny = m_hist->GetNBinsY();
    const double *src = m_hist->GetDataPtr();
    if (nx < 1 || ny < 1 || !src) {
        m_smooth = QImage();
        return;
    }
    if (m_smooth.size() != QSize(nx, ny) ||
        m_smooth.format() != QImage::Format_RGB32)
        m_smooth = QImage(nx, ny, QImage::Format_RGB32);

    double lo = 0.0, hi = 1.0;
    m_view->colorRange(lo, hi);
    const bool log = m_view->logScale();
    const double loD = log ? plot::logClamp(lo) : lo;
    const double hiD = log ? plot::logClamp(hi) : hi;
    double range = hiD - loD;
    if (!std::isfinite(range) || !(range > 0.0))
        range = 1.0;
    const double scale = 255.0 / range;
    const bool zeroWhite = m_view->zeroWhite();
    const QRgb *pal = m_palette;

    for (int y = 0; y < ny; y++) {
        // Exactly one flip, like SlsQtFastImage: QImage row = ny-1-dataRow.
        QRgb *line = reinterpret_cast<QRgb *>(m_smooth.scanLine(ny - 1 - y));
        const double *s = src + static_cast<size_t>(y) * static_cast<size_t>(nx);
        for (int x = 0; x < nx; x++) {
            const double v = s[x];
            if (!std::isfinite(v)) {
                line[x] = pal[0];
                continue;
            }
            if (zeroWhite && !(v > 0.0)) {
                line[x] = qRgb(255, 255, 255);
                continue;
            }
            int idx =
                static_cast<int>(((log ? plot::logClamp(v) : v) - loD) * scale);
            if (idx < 0)
                idx = 0;
            else if (idx > 255)
                idx = 255;
            line[x] = pal[idx];
        }
    }
}

// ---------------------------------------------------------------------------
// marching squares: ~9 levels over the visible colour range, on a subsampled
// grid (the full 1024x512 grid at 9 levels is unnecessary and slow), in
// FastImageView pixel-edge coordinates.  The segments are stored in data
// coordinates and clipped to the plot rect by the FastImageView paint hook.
// ---------------------------------------------------------------------------
void SlsQt2DPlot::updateContours() {
    m_contourSegments.clear();
    m_contourDirty = false;
    if (!m_contourOn || m_gain || !m_hist || !m_view)
        return;
    const int nx = m_hist->GetNBinsX();
    const int ny = m_hist->GetNBinsY();
    const double *d = m_hist->GetDataPtr();
    if (!d || nx < 2 || ny < 2)
        return;

    double lo = 0.0, hi = 1.0;
    m_view->colorRange(lo, hi);
    std::vector<double> levels;
    levels.reserve(9);
    if (m_logZ) {
        double a = (lo > 0.0 && std::isfinite(lo)) ? std::log10(lo) : 0.0;
        double b = (hi > 0.0 && std::isfinite(hi)) ? std::log10(hi) : a + 1.0;
        if (!(b > a))
            b = a + 1.0;
        for (int k = 1; k <= 9; k++)
            levels.push_back(std::pow(10.0, a + (b - a) * k / 10.0));
    } else {
        if (!std::isfinite(lo) || !std::isfinite(hi) || !(hi > lo))
            return;
        for (int k = 1; k <= 9; k++)
            levels.push_back(lo + (hi - lo) * k / 10.0);
    }

    const int step =
        std::max(1, static_cast<int>(std::ceil(
                        double(std::max(nx, ny)) / double(kLowResGrid))));

    auto intersect = [](double pu, double pv, double qu, double qv, double pval,
                        double qval, double L) {
        const double dv = qval - pval;
        double t = (std::abs(dv) > 1e-12) ? (L - pval) / dv : 0.5;
        t = std::min(1.0, std::max(0.0, t));
        return QPointF(pu + (qu - pu) * t, pv + (qv - pv) * t);
    };

    // Subsample the grid, but always include the last row/column: a plain
    // "i + step < nx" loop left the outermost (up to step-1 pixels wide) band
    // without contour segments (R8).  Clamping the last sample to nx-1/ny-1
    // makes the contours cover the full frame.
    std::vector<int> gridX;
    gridX.reserve(static_cast<size_t>(nx) / static_cast<size_t>(step) + 2);
    for (int i = 0; i < nx; i += step)
        gridX.push_back(i);
    if (gridX.back() != nx - 1)
        gridX.push_back(nx - 1);
    std::vector<int> gridY;
    gridY.reserve(static_cast<size_t>(ny) / static_cast<size_t>(step) + 2);
    for (int j = 0; j < ny; j += step)
        gridY.push_back(j);
    if (gridY.back() != ny - 1)
        gridY.push_back(ny - 1);

    for (size_t gy = 0; gy + 1 < gridY.size(); gy++) {
        const int j = gridY[gy];
        const int jNext = gridY[gy + 1];
        for (size_t gx = 0; gx + 1 < gridX.size(); gx++) {
            const int i = gridX[gx];
            const int iNext = gridX[gx + 1];
            const size_t i0 = static_cast<size_t>(i);
            const size_t i1 = static_cast<size_t>(iNext);
            const size_t j0 = static_cast<size_t>(j);
            const size_t j1 = static_cast<size_t>(jNext);
            const double va = d[i0 + j0 * nx];
            const double vb = d[i1 + j0 * nx];
            const double vc = d[i1 + j1 * nx];
            const double vd = d[i0 + j1 * nx];
            if (!std::isfinite(va) || !std::isfinite(vb) ||
                !std::isfinite(vc) || !std::isfinite(vd))
                continue;
            const double u0 = i + 0.5;
            const double u1 = iNext + 0.5;
            const double v0 = j + 0.5;
            const double v1 = jNext + 0.5;
            for (double L : levels) {
                const int code = (va > L ? 1 : 0) | (vb > L ? 2 : 0) |
                                 (vc > L ? 4 : 0) | (vd > L ? 8 : 0);
                if (code == 0 || code == 15)
                    continue;
                const QPointF e0 =
                    intersect(u0, v0, u1, v0, va, vb, L); // bottom
                const QPointF e1 =
                    intersect(u1, v0, u1, v1, vb, vc, L); // right
                const QPointF e2 =
                    intersect(u0, v1, u1, v1, vd, vc, L); // top
                const QPointF e3 =
                    intersect(u0, v0, u0, v1, va, vd, L); // left
                const bool centerHigh = (va + vb + vc + vd) * 0.25 > L;
                auto add = [this](const QPointF &a, const QPointF &b) {
                    m_contourSegments.push_back({a, b});
                };
                switch (code) {
                case 1:
                    add(e3, e0);
                    break;
                case 2:
                    add(e0, e1);
                    break;
                case 3:
                    add(e3, e1);
                    break;
                case 4:
                    add(e1, e2);
                    break;
                case 5:
                    if (centerHigh) {
                        add(e0, e1);
                        add(e2, e3);
                    } else {
                        add(e3, e0);
                        add(e1, e2);
                    }
                    break;
                case 6:
                    add(e0, e2);
                    break;
                case 7:
                    add(e2, e3);
                    break;
                case 8:
                    add(e2, e3);
                    break;
                case 9:
                    add(e0, e2);
                    break;
                case 10:
                    if (centerHigh) {
                        add(e3, e0);
                        add(e1, e2);
                    } else {
                        add(e0, e1);
                        add(e2, e3);
                    }
                    break;
                case 11:
                    add(e1, e2);
                    break;
                case 12:
                    add(e1, e3);
                    break;
                case 13:
                    add(e0, e1);
                    break;
                case 14:
                    add(e3, e0);
                    break;
                default:
                    break;
                }
                if (m_contourSegments.size() >= kMaxContourSegs)
                    return;
            }
        }
    }
}

// ---------------------------------------------------------------------------
// clipped paint hook
// ---------------------------------------------------------------------------
void SlsQt2DPlot::drawRois(QPainter &p, const QTransform &t) const {
    if (m_roiRects.empty())
        return;
    QPen pen(Qt::yellow);
    pen.setWidth(1);
    pen.setCosmetic(true);
    p.save();
    p.setPen(pen);
    p.setBrush(Qt::NoBrush);
    for (const QRectF &r : m_roiRects) {
        const QRectF wr =
            QRectF(t.map(r.topLeft()), t.map(r.bottomRight())).normalized();
        p.drawRect(wr);
    }
    p.restore();
}

void SlsQt2DPlot::paintOverlay(QPainter &p, const QTransform &t) {
    if (!m_view)
        return;

    bool smoothDrawn = false;
    if (!m_showImage) {
        // WqtPlotSpectrogram image-mode equivalent: hide the image (contours
        // and ROI boxes keep painting on the dark background).
        p.fillRect(p.viewport(), plot::PlotTheme().background);
    } else if (m_interp && !m_smooth.isNull()) {
        // Deep zoom keeps the crisp pixel view (grid + values must stay
        // readable); otherwise the bilinear image replaces the blocky one.
        const double sx = std::abs(t.m11());
        if (sx > 0.0 && sx < 16.0) {
            const QRectF target =
                QRectF(t.map(QPointF(0.0, 0.0)),
                       t.map(QPointF(double(m_hist->GetNBinsX()),
                                     double(m_hist->GetNBinsY()))))
                    .normalized();
            p.save();
            p.setRenderHint(QPainter::SmoothPixmapTransform, true);
            p.drawImage(target, m_smooth);
            p.restore();
            smoothDrawn = true;
        }
    }
    if (!m_showImage || smoothDrawn)
        drawRois(p, t);

    if (m_contourOn && !m_contourSegments.empty()) {
        QPen pen(QColor(255, 255, 255, 190));
        pen.setWidth(1);
        p.save();
        p.setPen(pen);
        p.setBrush(Qt::NoBrush);
        for (const ContourSegment &s : m_contourSegments)
            p.drawLine(t.map(s.a), t.map(s.b));
        p.restore();
    }
}

} // namespace sls