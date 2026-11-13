// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2026 Contributors to the SLS Detector Package
#include "SlsQt2DPlot.h"

#include "SlsQtFastImage.h"
#include "SlsQtPlotCommon.h"
#include "SlsQtScalePanel.h"

#include <QLabel>
#include <QLineF>
#include <QVBoxLayout>

#include <algorithm>
#include <cmath>

namespace sls {

namespace {

constexpr int kContourLevels = 9;
// the contours are computed on a grid of at most this many cells per axis
constexpr int kContourGrid = 256;
constexpr size_t kMaxContourLines = 120000;

} // namespace

SlsQt2DPlot::SlsQt2DPlot(QWidget *parent, bool gain)
    : QWidget(parent), m_gain(gain) {
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    m_view = new SlsQtFastImage(this);
    layout->addWidget(m_view, 1);
    connect(m_view, &SlsQtFastImage::viewportChanged, this,
            [this](const QRectF &v) {
                emit PlotZoomedSignal(
                    QRectF(m_hist.GetXMin() + v.x() * m_hist.GetXBinWidth(),
                           m_hist.GetYMin() + v.y() * m_hist.GetYBinWidth(),
                           v.width() * m_hist.GetXBinWidth(),
                           v.height() * m_hist.GetYBinWidth()));
            });

    if (m_gain) {
        m_view->setAxesVisible(false);
        m_view->setZoomEnabled(false);
        m_view->setPalette(plot::Palette::Gain4);
        m_view->setColorRange(0.0, 3.0);
        m_view->setTitle(QStringLiteral("Gain"));
    } else {
        const plot::PlotTheme theme;
        m_zTitleLabel = new QLabel(this);
        m_zTitleLabel->setAlignment(Qt::AlignCenter);
        m_zTitleLabel->setStyleSheet(
            QStringLiteral("background-color: %1; color: %2;")
                .arg(theme.background.name(), theme.text.name()));
        m_zTitleLabel->hide();
        layout->addWidget(m_zTitleLabel);

        m_panel = new SlsQtScalePanel(m_view, this);
        layout->addWidget(m_panel);
        connect(m_panel, &SlsQtScalePanel::colorRangeEdited, this,
                [this] { m_panelZOverride = true; });
        // the contour levels follow the colour range
        connect(m_view, &SlsQtFastImage::colorMapChanged, this, [this] {
            if (m_contourOn)
                updateContours();
        });
    }

    fillTestPlot();
}

// ---------------------------------------------------------------------------
// titles
// ---------------------------------------------------------------------------
void SlsQt2DPlot::SetTitle(const QString &title) { m_view->setTitle(title); }

QString SlsQt2DPlot::title() const { return m_view->title(); }

void SlsQt2DPlot::SetXTitle(const QString &title) {
    m_view->setXAxisTitle(title);
}

void SlsQt2DPlot::SetYTitle(const QString &title) {
    m_view->setYAxisTitle(title);
}

void SlsQt2DPlot::SetZTitle(const QString &title) {
    if (!m_zTitleLabel)
        return;
    m_zTitleLabel->setText(title);
    m_zTitleLabel->setVisible(!title.isEmpty());
}

// ---------------------------------------------------------------------------
// data
// ---------------------------------------------------------------------------
void SlsQt2DPlot::SetData(int nbinsx, double xmin, double xmax, int nbinsy,
                          double ymin, double ymax, const double *d) {
    const int oldNx = m_hist.GetNBinsX();
    const int oldNy = m_hist.GetNBinsY();
    m_hist.SetData(nbinsx, xmin, xmax, nbinsy, ymin, ymax, d);
    if (m_hist.GetNBinsX() != oldNx || m_hist.GetNBinsY() != oldNy) {
        // a zoom or a range picked on the scale panel belonged to the old
        // image, a range fixed with SetZRange() is still wanted
        m_panelZOverride = false;
        m_applyViewPending = true;
    }
    m_view->setImage(m_hist.GetDataPtr(), m_hist.GetNBinsX(),
                     m_hist.GetNBinsY());
    m_contourDirty = true;
    Update();
}

// Shown until the first image arrives
void SlsQt2DPlot::fillTestPlot() {
    const int n = 50;
    const double center = n / 2.0;
    const double maxDistance = std::hypot(center - 0.5, center - 0.5);
    std::vector<double> data(static_cast<size_t>(n) * n);
    for (int iy = 0; iy < n; ++iy) {
        for (int ix = 0; ix < n; ++ix) {
            const double distance =
                std::hypot(center - (ix + 0.5), center - (iy + 0.5));
            data[static_cast<size_t>(iy) * n + ix] =
                10 * (1 - distance / maxDistance);
        }
    }
    SetData(n, 200, 822, n, -0.5, n - 0.5, data.data());
}

void SlsQt2DPlot::Update() {
    if (!m_gain) {
        if (m_applyViewPending)
            applyPendingView();
        if (!m_panelZOverride && !m_userZOverride) {
            if (m_view->logScale())
                m_hist.SetMinimumToFirstGreaterThanZero();
            m_view->setColorRange(m_hist.GetMinimum(), m_hist.GetMaximum());
        }
        if (m_contourOn && m_contourDirty)
            updateContours();
    }
    m_view->update();
}

// ---------------------------------------------------------------------------
// axis ranges and zoom
// ---------------------------------------------------------------------------
double SlsQt2DPlot::dataXToImage(double x) const {
    return (x - m_hist.GetXMin()) / m_hist.GetXBinWidth();
}

double SlsQt2DPlot::dataYToImage(double y) const {
    return (y - m_hist.GetYMin()) / m_hist.GetYBinWidth();
}

void SlsQt2DPlot::SetZoom(const QRectF &rect) {
    const QRectF r = rect.normalized();
    if (r.isEmpty())
        return;
    m_applyViewPending = false;
    m_view->setViewport(dataXToImage(r.left()), dataYToImage(r.top()),
                        dataXToImage(r.right()), dataYToImage(r.bottom()));
}

void SlsQt2DPlot::DisableZoom(bool disable) {
    if (!m_gain)
        m_view->setZoomEnabled(!disable);
}

void SlsQt2DPlot::EnableXAutoScaling() {
    m_autoX = true;
    applyPendingView();
}

void SlsQt2DPlot::EnableYAutoScaling() {
    m_autoY = true;
    applyPendingView();
}

void SlsQt2DPlot::SetXMinMax(double min, double max) {
    m_autoX = false;
    m_reqX0 = min;
    m_reqX1 = max;
    applyPendingView();
}

void SlsQt2DPlot::SetYMinMax(double min, double max) {
    m_autoY = false;
    m_reqY0 = min;
    m_reqY1 = max;
    applyPendingView();
}

void SlsQt2DPlot::applyPendingView() {
    m_applyViewPending = false;
    if (m_gain)
        return;
    double x0 = 0.0, x1 = m_hist.GetNBinsX();
    double y0 = 0.0, y1 = m_hist.GetNBinsY();
    if (!m_autoX) {
        x0 = dataXToImage(std::min(m_reqX0, m_reqX1));
        x1 = dataXToImage(std::max(m_reqX0, m_reqX1));
    }
    if (!m_autoY) {
        y0 = dataYToImage(std::min(m_reqY0, m_reqY1));
        y1 = dataYToImage(std::max(m_reqY0, m_reqY1));
    }
    // the view ignores a viewport below its minimum size, so widen the
    // request around its centre instead
    const auto widen = [](double &lo, double &hi) {
        const double missing = SlsQtFastImage::kMinimumViewport - (hi - lo);
        if (missing > 0) {
            lo -= missing / 2;
            hi += missing / 2;
        }
    };
    widen(x0, x1);
    widen(y0, y1);
    m_view->setViewport(x0, y0, x1, y1);
}

// ---------------------------------------------------------------------------
// z range
// ---------------------------------------------------------------------------
void SlsQt2DPlot::SetLogz(bool enable, bool isMin, bool isMax, double min,
                          double max) {
    if (m_gain)
        return;
    m_view->setLogScale(enable);
    SetZRange(isMin, isMax, min, max);
}

void SlsQt2DPlot::SetZRange(bool isMin, bool isMax, double min, double max) {
    if (m_gain)
        return;
    m_userZOverride = isMin || isMax;
    if (m_userZOverride) {
        if (m_view->logScale())
            m_hist.SetMinimumToFirstGreaterThanZero();
        const double zmin = isMin ? min : m_hist.GetMinimum();
        double zmax = isMax ? max : m_hist.GetMaximum();
        if (!(zmax > zmin))
            zmax = zmin + 1.0;
        m_panelZOverride = false;
        m_view->setColorRange(zmin, zmax);
    }
    Update();
}

// ---------------------------------------------------------------------------
// drawing options
// ---------------------------------------------------------------------------
void SlsQt2DPlot::SetInterpolate(bool enable) {
    m_view->setInterpolate(enable);
}

void SlsQt2DPlot::SetContour(bool enable) {
    m_contourOn = enable;
    updateContours();
}

// The ROIs are in pixels with inclusive limits
void SlsQt2DPlot::EnableRoiBoxes(const std::vector<slsDetectorDefs::ROI> &roi) {
    std::vector<QRectF> rects;
    rects.reserve(roi.size());
    for (const auto &r : roi) {
        if (r.xmax >= r.xmin && r.ymax >= r.ymin)
            rects.emplace_back(r.xmin, r.ymin, r.xmax - r.xmin + 1,
                               r.ymax - r.ymin + 1);
    }
    m_view->setOverlayRects(rects);
}

void SlsQt2DPlot::DisableRoiBoxes() { m_view->setOverlayRects({}); }

// Marching squares for kContourLevels levels spread evenly over the colour
// range. Lines are in pixel coordinates, the value of a pixel sits at its
// centre.
void SlsQt2DPlot::updateContours() {
    m_contourDirty = false;
    std::vector<QLineF> lines;
    const int nx = m_hist.GetNBinsX();
    const int ny = m_hist.GetNBinsY();
    const bool log = m_view->logScale();
    double lo = 0.0, hi = 1.0;
    m_view->colorRange(lo, hi);
    if (log) {
        lo = plot::logClamp(lo);
        hi = plot::logClamp(hi);
    }
    if (!m_contourOn || nx < 2 || ny < 2 || !std::isfinite(lo) ||
        !std::isfinite(hi) || !(hi > lo)) {
        m_view->setOverlayLines(lines);
        return;
    }

    std::vector<double> levels;
    for (int k = 1; k <= kContourLevels; ++k) {
        const double level = lo + (hi - lo) * k / (kContourLevels + 1);
        levels.push_back(log ? std::pow(10.0, level) : level);
    }

    // every step-th pixel, and always the last one so that the contours
    // reach the edge of the image
    const int step = (std::max(nx, ny) + kContourGrid - 1) / kContourGrid;
    const auto gridPoints = [step](int n) {
        std::vector<int> points;
        for (int i = 0; i < n; i += step)
            points.push_back(i);
        if (points.back() != n - 1)
            points.push_back(n - 1);
        return points;
    };
    const std::vector<int> gridX = gridPoints(nx);
    const std::vector<int> gridY = gridPoints(ny);

    // where the level is crossed between p and q
    const auto crossing = [](QPointF p, QPointF q, double pValue, double qValue,
                             double level) {
        const double diff = qValue - pValue;
        const double t = std::abs(diff) > 1e-12
                             ? std::clamp((level - pValue) / diff, 0.0, 1.0)
                             : 0.5;
        return p + (q - p) * t;
    };

    for (size_t gy = 0; gy + 1 < gridY.size(); ++gy) {
        for (size_t gx = 0; gx + 1 < gridX.size(); ++gx) {
            // corners of the cell: a b on the lower edge, d c on the upper
            const int i0 = gridX[gx], i1 = gridX[gx + 1];
            const int j0 = gridY[gy], j1 = gridY[gy + 1];
            const double va = m_hist.GetBinValue(i0, j0);
            const double vb = m_hist.GetBinValue(i1, j0);
            const double vc = m_hist.GetBinValue(i1, j1);
            const double vd = m_hist.GetBinValue(i0, j1);
            if (!std::isfinite(va) || !std::isfinite(vb) ||
                !std::isfinite(vc) || !std::isfinite(vd))
                continue;
            const QPointF a(i0 + 0.5, j0 + 0.5), b(i1 + 0.5, j0 + 0.5);
            const QPointF c(i1 + 0.5, j1 + 0.5), d(i0 + 0.5, j1 + 0.5);

            for (const double level : levels) {
                const int corners = (va > level ? 1 : 0) |
                                    (vb > level ? 2 : 0) |
                                    (vc > level ? 4 : 0) | (vd > level ? 8 : 0);
                if (corners == 0 || corners == 15)
                    continue;
                const QPointF bottom = crossing(a, b, va, vb, level);
                const QPointF right = crossing(b, c, vb, vc, level);
                const QPointF top = crossing(d, c, vd, vc, level);
                const QPointF left = crossing(a, d, va, vd, level);
                // two opposite corners above the level: the centre decides
                // which pairs of edges are connected
                const bool centerAbove = (va + vb + vc + vd) / 4 > level;
                switch (corners) {
                case 1:
                case 14:
                    lines.emplace_back(left, bottom);
                    break;
                case 2:
                case 13:
                    lines.emplace_back(bottom, right);
                    break;
                case 3:
                case 12:
                    lines.emplace_back(left, right);
                    break;
                case 4:
                case 11:
                    lines.emplace_back(right, top);
                    break;
                case 6:
                case 9:
                    lines.emplace_back(bottom, top);
                    break;
                case 7:
                case 8:
                    lines.emplace_back(top, left);
                    break;
                case 5:
                case 10:
                    if (centerAbove == (corners == 5)) {
                        lines.emplace_back(bottom, right);
                        lines.emplace_back(top, left);
                    } else {
                        lines.emplace_back(left, bottom);
                        lines.emplace_back(right, top);
                    }
                    break;
                }
                if (lines.size() >= kMaxContourLines) {
                    m_view->setOverlayLines(lines);
                    return;
                }
            }
        }
    }
    m_view->setOverlayLines(lines);
}

} // namespace sls
