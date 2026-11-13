// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2026 Contributors to the SLS Detector Package

// Custom Qt6 painting based 1D plot.  See SlsQt1DPlot.h for the contract.

#include "SlsQt1DPlot.h"

#include <QEvent>
#include <QFontMetrics>
#include <QMouseEvent>
#include <QPainter>
#include <QPolygonF>
#include <QWheelEvent>

#include <algorithm>
#include <cmath>
#include <utility>

namespace sls {

namespace {

constexpr int kTickLen = 4;
constexpr double kMinSpan = 1e-9;
constexpr double kPixelClamp = 1.0e6;

// Colours of SlsQtH1D::SetLineColor(), bright enough for the dark
// background.
QColor lineColorForIndex(int c) {
    switch (c) {
    case 0:
        return QColor(Qt::white);
    case 1:
        return QColor(Qt::red);
    case 2:
        return QColor(Qt::blue);
    case 3:
        return QColor(Qt::green);
    case 4:
        return QColor(Qt::magenta);
    case 5:
        return QColor(Qt::cyan);
    case 6:
        return QColor(220, 200, 40); // dark yellow
    case 7:
        return QColor(190, 190, 190); // gray
    case 8:
        return QColor(110, 160, 255); // dark blue, lightened
    case 9:
        return QColor(90, 210, 130); // dark green, lightened
    case 10:
        return QColor(230, 130, 230); // dark magenta, lightened
    default:
        return QColor(Qt::white);
    }
}

QRectF rectFromRanges(double xLo, double xHi, double yLo, double yHi) {
    return QRectF(xLo, yLo, xHi - xLo, yHi - yLo);
}

QString formatValue(double v) { return QString::number(v, 'g', 5); }

} // namespace

// ---------------------------------------------------------------------------
// SlsQtH1D
// ---------------------------------------------------------------------------

SlsQtH1D::SlsQtH1D(QString title, int n, double xmin, double xmax, double *data)
    : m_title(std::move(title)) {
    initialize();
    SetData(n, xmin, xmax, data);
}

SlsQtH1D::SlsQtH1D(QString title, int n, double *data_x, double *data_y)
    : m_title(std::move(title)) {
    initialize();
    SetData(n, data_x, data_y);
}

SlsQtH1D::~SlsQtH1D() {
    if (m_plot)
        m_plot->removeHistogram(this);
}

void SlsQtH1D::initialize() {
    m_ndata = 0;
    m_dx = 0.0;
    m_pen = QPen();
    SetLineColor();
}

void SlsQtH1D::Attach(SlsQt1DPlot *p) {
    if (!p)
        return;
    if (m_plot == p)
        return;
    if (m_plot)
        m_plot->removeHistogram(this);
    m_plot = p;
    p->addHistogram(this);
}

void SlsQtH1D::Detach(SlsQt1DPlot *p) {
    (void)p;
    if (m_plot) {
        SlsQt1DPlot *plot = m_plot;
        m_plot = nullptr;
        plot->removeHistogram(this);
    }
}

int SlsQtH1D::SetLineColor(int c) {
    static int last_color = 1;
    if (c < 0)
        c = (last_color + 1) % 3;
    m_pen.setColor(lineColorForIndex(c));
    last_color = c;
    return c;
}

int SlsQtH1D::SetLineWidth(int w) {
    m_pen.setWidth(w);
    return w;
}

void SlsQtH1D::SetLineStyle(int s) {
    switch (s) {
    case 1:
        m_pen.setStyle(Qt::DashLine);
        break;
    case 2:
        m_pen.setStyle(Qt::DotLine);
        break;
    case 3:
        m_pen.setStyle(Qt::DashDotLine);
        break;
    case 4:
        m_pen.setStyle(Qt::DashDotDotLine);
        break;
    case 5:
        m_pen.setStyle(Qt::CustomDashLine);
        break;
    default:
        m_pen.setStyle(Qt::SolidLine);
        break;
    }
}

void SlsQtH1D::setStyleLinesorDots(bool isLines) { m_linesMode = isLines; }

void SlsQtH1D::setSymbolMarkers(bool isMarker) { m_markerMode = isMarker; }

void SlsQtH1D::SetData(int n, double xmin, double xmax, double *data) {
    setUpArrays(n);
    m_ndata = n;
    if (xmin > xmax)
        std::swap(xmin, xmax);
    // Deliberately divided by n (not n-1): the GUI/ROI mapping depends on
    // the bins being defined this way.
    m_dx = (xmax - xmin) / n;
    m_ymin = m_ymax = data ? data[0] : 0.0;
    m_firstXgt0 = -1.0;
    m_firstYgt0 = -1.0;
    for (int i = 0; i < m_ndata; ++i) {
        const double xv = i ? m_x[static_cast<size_t>(i - 1)] + m_dx : xmin;
        m_x[static_cast<size_t>(i)] = xv;
        m_y[static_cast<size_t>(i)] = data ? data[i] : 0.0;
        const double yv = m_y[static_cast<size_t>(i)];
        if (data && m_ymin > yv)
            m_ymin = yv;
        if (data && m_ymax < yv)
            m_ymax = yv;
        if (xv > 0 && (m_firstXgt0 < 0 || m_firstXgt0 > xv))
            m_firstXgt0 = xv;
        if (yv > 0 && (m_firstYgt0 < 0 || m_firstYgt0 > yv))
            m_firstYgt0 = yv;
    }
}

void SlsQtH1D::SetData(int n, double *data_x, double *data_y) {
    const int reverse =
        (data_x && n > 0 && data_x[0] > data_x[n - 1]) ? 1 : 0;
    setUpArrays(n);
    m_ndata = n;
    m_dx = -1.0; // not regular intervals
    m_ymin = m_ymax = data_y ? data_y[0] : 0.0;
    m_firstXgt0 = -1.0;
    m_firstYgt0 = -1.0;
    for (int i = 0; i < m_ndata; ++i) {
        const int b = reverse ? n - i - 1 : i;
        m_x[static_cast<size_t>(b)] = data_x ? data_x[i] : 0.0;
        m_y[static_cast<size_t>(b)] = data_y ? data_y[i] : 0.0;
        const double xv = m_x[static_cast<size_t>(b)];
        const double yv = m_y[static_cast<size_t>(b)];
        if (data_y && m_ymin > yv)
            m_ymin = yv;
        if (data_y && m_ymax < yv)
            m_ymax = yv;
        if (xv > 0 && (m_firstXgt0 < 0 || m_firstXgt0 > xv))
            m_firstXgt0 = xv;
        if (yv > 0 && (m_firstYgt0 < 0 || m_firstYgt0 > yv))
            m_firstYgt0 = yv;
    }
}

void SlsQtH1D::setUpArrays(int n) {
    n = n < 1 ? 1 : n; // overflow bin
    m_x.assign(static_cast<size_t>(n) + 1, 0.0);
    m_y.assign(static_cast<size_t>(n) + 1, 0.0);
}

int SlsQtH1D::checkIndex(int bx) {
    return (bx < 0 || bx > m_ndata) ? m_ndata : bx;
}

double SlsQtH1D::FillBin(int bx, double v) {
    bx = checkIndex(bx);
    return SetBinContent(bx, m_y[static_cast<size_t>(bx)] + v);
}

double SlsQtH1D::Fill(double x, double v) { return FillBin(FindBinIndex(x), v); }

double SlsQtH1D::SetBinContent(int bx, double v) {
    bx = checkIndex(bx);
    m_y[static_cast<size_t>(bx)] = v;
    if (bx < m_ndata) {
        if (m_y[static_cast<size_t>(bx)] < m_ymin)
            m_ymin = m_y[static_cast<size_t>(bx)];
        if (m_y[static_cast<size_t>(bx)] > 0 &&
            (m_firstYgt0 <= 0 || m_y[static_cast<size_t>(bx)] < m_firstYgt0))
            m_firstYgt0 = m_y[static_cast<size_t>(bx)];
        if (m_y[static_cast<size_t>(bx)] > m_ymax)
            m_ymax = m_y[static_cast<size_t>(bx)];
    }
    return m_y[static_cast<size_t>(bx)];
}

double SlsQtH1D::SetContent(double x, double v) {
    return SetBinContent(FindBinIndex(x), v);
}

int SlsQtH1D::FindBinIndex(double px) {
    int b = 0;
    for (; b < m_ndata; b++) {
        if (m_x[static_cast<size_t>(b)] > px)
            break;
    }
    if (b == 0)
        return 0;
    if (std::fabs(px - m_x[static_cast<size_t>(b - 1)]) <
        std::fabs(px - m_x[static_cast<size_t>(b)]))
        return b - 1;
    return b;
}

SlsQtH1D *SlsQtH1D::Add(double v) {
    for (int bx = 0; bx < m_ndata; bx++)
        FillBin(bx, v);
    return this;
}

// ---------------------------------------------------------------------------
// SlsQt1DPlot - construction / state
// ---------------------------------------------------------------------------

SlsQt1DPlot::SlsQt1DPlot(QWidget *parent, bool gain)
    : QWidget(parent), m_gainPlot(gain) {
    setAttribute(Qt::WA_OpaquePaintEvent, true);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setMinimumSize(40, 30);
    setAutoFillBackground(false);

    m_theme = sls::plot::PlotTheme();
    m_titleFont = QFont();
    m_xTitleFont = QFont();
    m_yTitleFont = QFont();
    m_tickFont = m_theme.tickFont;

    if (m_gainPlot) {
        m_yAuto = false;
        m_disableZoom = true;
        m_title.clear();
        m_yTitle = QStringLiteral("Gain");
    }

    recomputeBase();
    m_zoomStack.clear();
    m_zoomStack.push_back(baseRect());
    m_zoomIndex = 0;
}

SlsQt1DPlot::~SlsQt1DPlot() {
    for (SlsQtH1D *h : m_hists) {
        if (h)
            h->m_plot = nullptr;
    }
    m_hists.clear();
}

void SlsQt1DPlot::addHistogram(SlsQtH1D *h) {
    if (!h)
        return;
    if (std::find(m_hists.begin(), m_hists.end(), h) == m_hists.end())
        m_hists.push_back(h);
    recomputeBase();
    update();
}

void SlsQt1DPlot::removeHistogram(SlsQtH1D *h) {
    const auto it = std::find(m_hists.begin(), m_hists.end(), h);
    if (it != m_hists.end())
        m_hists.erase(it);
    recomputeBase();
    update();
}

void SlsQt1DPlot::recomputeBase() {
    bool have = false;
    double xmin = 0.0, xmax = 0.0, ymin = 0.0, ymax = 0.0;
    double firstX = -1.0, firstY = -1.0;
    for (SlsQtH1D *h : m_hists) {
        if (!h || h->GetNBinsX() <= 0)
            continue;
        const double hx0 = h->GetXMin();
        const double hx1 = h->GetXMax();
        const double hy0 = h->GetYMin();
        const double hy1 = h->GetYMax();
        if (!std::isfinite(hx0) || !std::isfinite(hx1) ||
            !std::isfinite(hy0) || !std::isfinite(hy1))
            continue;
        if (!have) {
            xmin = hx0;
            xmax = hx1;
            ymin = hy0;
            ymax = hy1;
            have = true;
        } else {
            xmin = std::min(xmin, hx0);
            xmax = std::max(xmax, hx1);
            ymin = std::min(ymin, hy0);
            ymax = std::max(ymax, hy1);
        }
        const double fx = h->GetFirstXgtZero();
        if (fx > 0 && (firstX < 0 || fx < firstX))
            firstX = fx;
        const double fy = h->GetFirstYgtZero();
        if (fy > 0 && (firstY < 0 || fy < firstY))
            firstY = fy;
    }

    if (!have) {
        m_bxLo = 0.0;
        m_bxHi = 1.0;
        m_byLo = 0.0;
        m_byHi = 1.0;
    } else {
        const auto padLo = [](double v) { return v < 0 ? v * 1.02 : v / 1.02; };
        const auto padHi = [](double v) { return v < 0 ? v / 1.02 : v * 1.02; };
        m_bxLo = padLo(xmin);
        m_bxHi = padHi(xmax);
        m_byLo = padLo(ymin);
        m_byHi = padHi(ymax);

        if (m_logX) {
            if (!(m_bxLo > 0.0))
                m_bxLo = (firstX > 0) ? firstX * 0.98 : 1e-2;
            if (!(m_bxHi > m_bxLo))
                m_bxHi = m_bxLo * 10.0;
        } else if (!(m_bxHi > m_bxLo)) {
            const double c = m_bxLo;
            m_bxLo = c - 1.0;
            m_bxHi = c + 1.0;
        }

        if (m_gainPlot) {
            m_byLo = 0.0;
            m_byHi = 3.0;
        } else if (m_logY) {
            // The y axis is logarithmic, so the auto-range padding must be
            // done in log10 space.  Padding the linear value (the old
            // ymax * 1.02) only adds ~0.0086 decades, i.e. ~1.5 px on a
            // multi-decade plot, which leaves the peak glued to the top frame
            // (observable on the white curve vs. the white frame).  Pad each
            // side by 2% of the visible decade span instead.
            const double yLo =
                (ymin > 0.0) ? ymin : ((firstY > 0.0) ? firstY : 1e-2);
            double yHi = (ymax > 0.0) ? ymax : yLo;
            double logLo = std::log10(yLo);
            double logHi = std::log10(yHi);
            if (!(logHi > logLo))
                logHi = logLo + 1.0; // flat/degenerate log data: one decade
            const double pad = 0.02 * (logHi - logLo);
            m_byLo = std::pow(10.0, logLo - pad);
            m_byHi = std::pow(10.0, logHi + pad);
            if (!std::isfinite(m_byLo) || !(m_byLo > 0.0))
                m_byLo = yLo * 0.98;
            if (!std::isfinite(m_byHi) || !(m_byHi > m_byLo))
                m_byHi = m_byLo * 10.0;
        } else if (!(m_byHi > m_byLo)) {
            const double c = m_byLo;
            m_byLo = c - 1.0;
            m_byHi = c + 1.0;
        }
    }

    if (m_xAuto) {
        m_xLo = m_bxLo;
        m_xHi = m_bxHi;
    }
    if (m_gainPlot) {
        m_yLo = 0.0;
        m_yHi = 3.0;
    } else if (m_yAuto) {
        m_yLo = m_byLo;
        m_yHi = m_byHi;
    }
    clampView();
    // Keep the bottom of the zoom stack (the "full view") in sync with the
    // data range.
    if (!m_zoomStack.empty())
        m_zoomStack.front() = baseRect();
}

void SlsQt1DPlot::clampView() {
    if (!std::isfinite(m_xLo) || !std::isfinite(m_xHi)) {
        m_xLo = m_bxLo;
        m_xHi = m_bxHi;
    }
    if (m_logX) {
        if (!(m_xLo > 0.0))
            m_xLo = 1e-3;
        if (!(m_xHi > m_xLo))
            m_xHi = m_xLo * 10.0;
    } else if (!(m_xHi > m_xLo)) {
        const double c = m_xLo;
        m_xLo = c - 1.0;
        m_xHi = c + 1.0;
    }

    if (m_gainPlot) {
        m_yLo = 0.0;
        m_yHi = 3.0;
        return;
    }
    if (!std::isfinite(m_yLo) || !std::isfinite(m_yHi)) {
        m_yLo = m_byLo;
        m_yHi = m_byHi;
    }
    if (m_logY) {
        if (!(m_yLo > 0.0))
            m_yLo = 1e-3;
        if (!(m_yHi > m_yLo))
            m_yHi = m_yLo * 10.0;
    } else if (!(m_yHi > m_yLo)) {
        const double c = m_yLo;
        m_yLo = c - 1.0;
        m_yHi = c + 1.0;
    }
}

void SlsQt1DPlot::setView(double xLo, double xHi, double yLo, double yHi) {
    if (xLo > xHi)
        std::swap(xLo, xHi);
    if (yLo > yHi)
        std::swap(yLo, yHi);
    m_xLo = xLo;
    m_xHi = xHi;
    m_yLo = yLo;
    m_yHi = yHi;
    clampView();
}

QRectF SlsQt1DPlot::baseRect() const {
    return rectFromRanges(m_bxLo, m_bxHi, m_byLo, m_byHi);
}

QRectF SlsQt1DPlot::currentRect() const {
    return rectFromRanges(m_xLo, m_xHi, m_yLo, m_yHi);
}

QRectF SlsQt1DPlot::viewRect() const { return currentRect(); }

void SlsQt1DPlot::emitZoomed() { emit PlotZoomedSignal(currentRect()); }

double SlsQt1DPlot::tx(double v) const {
    if (m_logX)
        return std::log10(v > 1e-12 ? v : 1e-12);
    return v;
}

double SlsQt1DPlot::ty(double v) const {
    if (m_logY)
        return std::log10(v > 1e-12 ? v : 1e-12);
    return v;
}

double SlsQt1DPlot::itx(double t) const {
    if (m_logX)
        return std::pow(10.0, t);
    return t;
}

double SlsQt1DPlot::ity(double t) const {
    if (m_logY)
        return std::pow(10.0, t);
    return t;
}

QPointF SlsQt1DPlot::toWidget(double x, double y, const Geometry &g) const {
    const double xl = tx(m_xLo);
    const double xh = tx(m_xHi);
    const double yl = ty(m_yLo);
    const double yh = ty(m_yHi);
    // Map the axis limits onto the first/last INSIDE pixel of the plot
    // rectangle (not one past the right/bottom edge or one above the top
    // edge): a tick exactly at an axis limit must stay inside and be drawn.
    double px = g.plot.left();
    double py = g.plot.bottom();
    if (xh > xl && g.plot.width() > 1)
        px += (tx(x) - xl) / (xh - xl) * (g.plot.width() - 1);
    if (yh > yl && g.plot.height() > 1)
        py -= (ty(y) - yl) / (yh - yl) * (g.plot.height() - 1);
    px = std::max(-kPixelClamp, std::min(kPixelClamp, px));
    py = std::max(-kPixelClamp, std::min(kPixelClamp, py));
    return QPointF(px, py);
}

QPointF SlsQt1DPlot::toData(const QPointF &pos, const Geometry &g) const {
    const double xl = tx(m_xLo);
    const double xh = tx(m_xHi);
    const double yl = ty(m_yLo);
    const double yh = ty(m_yHi);
    const double fx =
        g.plot.width() > 1 ? (pos.x() - g.plot.left()) / (g.plot.width() - 1)
                           : 0.0;
    const double fy =
        g.plot.height() > 1 ? (g.plot.bottom() - pos.y()) / (g.plot.height() - 1)
                            : 0.0;
    return QPointF(itx(xl + (xh - xl) * fx), ity(yl + (yh - yl) * fy));
}

// ---------------------------------------------------------------------------
// SlsQt1DPlot - public API
// ---------------------------------------------------------------------------

void SlsQt1DPlot::SetTitle(QString title) {
    m_title = std::move(title);
    update();
}

void SlsQt1DPlot::SetXTitle(QString title) {
    m_xTitle = std::move(title);
    update();
}

void SlsQt1DPlot::SetYTitle(QString title) {
    m_yTitle = std::move(title);
    update();
}

void SlsQt1DPlot::SetTitleFont(const QFont &f) {
    m_titleFont = f;
    update();
}

void SlsQt1DPlot::SetXFont(const QFont &f) {
    m_xTitleFont = f;
    update();
}

void SlsQt1DPlot::SetYFont(const QFont &f) {
    m_yTitleFont = f;
    update();
}

void SlsQt1DPlot::InsertHLine(double y) {
    m_hasHLine = true;
    m_hLine = y;
    update();
}

void SlsQt1DPlot::RemoveHLine() {
    m_hasHLine = false;
    update();
}

void SlsQt1DPlot::InsertVLine(double v) {
    m_hasVLine = true;
    m_vLine = v;
    update();
}

void SlsQt1DPlot::RemoveVLine() {
    m_hasVLine = false;
    update();
}

void SlsQt1DPlot::DisableZoom(bool disable) {
    if (m_gainPlot)
        disable = true; // the gain plot never zooms
    if (m_disableZoom == disable)
        return;
    m_disableZoom = disable;
    if (disable) {
        m_rubberBand = false;
        m_panning = false;
        unsetCursor();
    }
    update();
}

void SlsQt1DPlot::EnableXAutoScaling() {
    m_xAuto = true;
    recomputeBase();
    update();
}

void SlsQt1DPlot::EnableYAutoScaling() {
    if (m_gainPlot)
        return;
    m_yAuto = true;
    recomputeBase();
    update();
}

void SlsQt1DPlot::SetXMinMax(double min, double max) {
    if (min > max)
        std::swap(min, max);
    m_xAuto = false;
    m_xLo = min;
    m_xHi = max;
    clampView();
    update();
}

void SlsQt1DPlot::SetYMinMax(double min, double max) {
    if (m_gainPlot)
        return;
    if (min > max)
        std::swap(min, max);
    m_yAuto = false;
    m_yLo = min;
    m_yHi = max;
    clampView();
    update();
}

double SlsQt1DPlot::GetXMinimum() const {
    if (!m_hists.empty() && m_hists.front())
        return m_hists.front()->GetXMin();
    return m_bxLo;
}

double SlsQt1DPlot::GetXMaximum() const {
    if (!m_hists.empty() && m_hists.front())
        return m_hists.front()->GetXMax();
    return m_bxHi;
}

double SlsQt1DPlot::GetYMinimum() const {
    if (!m_hists.empty() && m_hists.front())
        return m_hists.front()->GetYMin();
    return m_byLo;
}

double SlsQt1DPlot::GetYMaximum() const {
    if (!m_hists.empty() && m_hists.front())
        return m_hists.front()->GetYMax();
    return m_byHi;
}

void SlsQt1DPlot::SetZoom(double xmin, double ymin, double x_width,
                          double y_width) {
    if (!(x_width > 0.0))
        x_width = 1.0;
    if (!(y_width > 0.0))
        y_width = 1.0;
    m_xAuto = false;
    if (!m_gainPlot)
        m_yAuto = false;
    setView(xmin, xmin + x_width, ymin, ymin + y_width);
    update();
}

void SlsQt1DPlot::SetZoomBase(double xmin, double ymin, double x_width,
                              double y_width) {
    if (!(x_width > 0.0))
        x_width = 1.0;
    if (!(y_width > 0.0))
        y_width = 1.0;
    m_bxLo = xmin;
    m_bxHi = xmin + x_width;
    m_byLo = ymin;
    m_byHi = ymin + y_width;
    if (m_logX) {
        if (!(m_bxLo > 0.0))
            m_bxLo = 1e-2;
        if (!(m_bxHi > m_bxLo))
            m_bxHi = m_bxLo * 10.0;
    }
    if (m_gainPlot) {
        m_byLo = 0.0;
        m_byHi = 3.0;
    } else if (m_logY) {
        if (!(m_byLo > 0.0))
            m_byLo = 1e-2;
        if (!(m_byHi > m_byLo))
            m_byHi = m_byLo * 10.0;
    }
    if (m_xAuto) {
        m_xLo = m_bxLo;
        m_xHi = m_bxHi;
    }
    if (m_gainPlot) {
        m_yLo = 0.0;
        m_yHi = 3.0;
    } else if (m_yAuto) {
        m_yLo = m_byLo;
        m_yHi = m_byHi;
    }
    clampView();
    m_zoomStack.clear();
    m_zoomStack.push_back(baseRect());
    m_zoomIndex = 0;
    update();
}

void SlsQt1DPlot::SetLogX(bool yes) {
    if (m_logX == yes)
        return;
    m_logX = yes;
    if (yes && !(m_xLo > 0.0))
        m_xAuto = true;
    recomputeBase();
    clampView();
    m_zoomStack.clear();
    m_zoomStack.push_back(baseRect());
    m_zoomIndex = 0;
    update();
}

void SlsQt1DPlot::SetLogY(bool yes) {
    if (m_gainPlot || m_logY == yes)
        return;
    m_logY = yes;
    if (yes && !(m_yLo > 0.0))
        m_yAuto = true;
    recomputeBase();
    clampView();
    m_zoomStack.clear();
    m_zoomStack.push_back(baseRect());
    m_zoomIndex = 0;
    update();
}

void SlsQt1DPlot::EnableRoiBoxes(std::vector<slsDetectorDefs::ROI> roi,
                                 int ymin, int ymax) {
    m_rois = std::move(roi);
    m_roiYMin = ymin;
    m_roiYMax = ymax;
    m_roiOn = !m_rois.empty();
    update();
}

void SlsQt1DPlot::DisableRoiBoxes() {
    m_rois.clear();
    m_roiOn = false;
    update();
}

void SlsQt1DPlot::SetZoomX(const QRectF &rect) {
    double x0 = 0.0, y0 = 0.0, x1 = 0.0, y1 = 0.0;
    rect.getCoords(&x0, &y0, &x1, &y1);
    (void)y0;
    (void)y1;
    if (!std::isfinite(x0) || !std::isfinite(x1))
        return;
    if (x0 > x1)
        std::swap(x0, x1);
    m_xAuto = false;
    m_xLo = x0;
    m_xHi = x1;
    clampView();
    update();
}

void SlsQt1DPlot::UnZoom() {
    resetZoom(true);
    update();
}

void SlsQt1DPlot::Update() {
    recomputeBase();
    update();
}

void SlsQt1DPlot::resetZoom(bool emitSignal) {
    m_zoomStack.clear();
    m_zoomStack.push_back(baseRect());
    m_zoomIndex = 0;
    applyZoomRect(baseRect());
    if (m_gainPlot) {
        m_yLo = 0.0;
        m_yHi = 3.0;
    }
    if (emitSignal)
        emitZoomed();
    update();
}

void SlsQt1DPlot::zoomOutOneLevel() {
    if (m_zoomIndex <= 0 || m_zoomStack.empty())
        return;
    --m_zoomIndex;
    applyZoomRect(m_zoomStack[static_cast<size_t>(m_zoomIndex)]);
    if (m_gainPlot) {
        m_yLo = 0.0;
        m_yHi = 3.0;
    }
    emitZoomed();
    update();
}

void SlsQt1DPlot::applyZoomRect(const QRectF &rect) {
    double x0 = 0.0, y0 = 0.0, x1 = 0.0, y1 = 0.0;
    rect.getCoords(&x0, &y0, &x1, &y1);
    if (!std::isfinite(x0) || !std::isfinite(x1) || !std::isfinite(y0) ||
        !std::isfinite(y1))
        return;
    setView(x0, x1, y0, y1);
}

void SlsQt1DPlot::pushZoomRect(const QRectF &rect, bool emitSignal) {
    applyZoomRect(rect);
    if (m_gainPlot) {
        m_yLo = 0.0;
        m_yHi = 3.0;
    }
    const QRectF current = currentRect();
    if (m_zoomStack.empty()) {
        m_zoomStack.push_back(baseRect());
        m_zoomIndex = 0;
    }
    if (m_zoomIndex + 1 < static_cast<int>(m_zoomStack.size()))
        m_zoomStack.resize(static_cast<size_t>(m_zoomIndex) + 1);
    if (m_zoomIndex < 0)
        m_zoomIndex = 0;
    if (m_zoomStack[static_cast<size_t>(m_zoomIndex)] != current) {
        m_zoomStack.push_back(current);
        m_zoomIndex = static_cast<int>(m_zoomStack.size()) - 1;
    }
    m_xAuto = false;
    if (!m_gainPlot)
        m_yAuto = false;
    if (emitSignal)
        emitZoomed();
    update();
}

void SlsQt1DPlot::zoomAbout(const QPointF &pos, double factor, bool push) {
    const Geometry g = computeGeometry();
    if (g.plot.width() <= 0 || g.plot.height() <= 0)
        return;
    if (!g.plot.contains(pos.toPoint()))
        return;
    const double xl = tx(m_xLo);
    const double xh = tx(m_xHi);
    const double yl = ty(m_yLo);
    const double yh = ty(m_yHi);
    const double fx =
        std::max(0.0, std::min(1.0, (pos.x() - g.plot.left()) / g.plot.width()));
    const double fy = std::max(
        0.0, std::min(1.0, (g.plot.bottom() - pos.y()) / g.plot.height()));
    const double cx = xl + (xh - xl) * fx;
    const double cy = yl + (yh - yl) * fy;
    const double nxl = cx - (cx - xl) * factor;
    const double nxh = cx + (xh - cx) * factor;
    const double nyl = cy - (cy - yl) * factor;
    const double nyh = cy + (yh - cy) * factor;
    if (!(nxh - nxl > kMinSpan) || !(nyh - nyl > kMinSpan))
        return;
    QRectF rect = rectFromRanges(itx(nxl), itx(nxh), ity(nyl), ity(nyh));
    if (m_gainPlot) {
        // keep the gain y range fixed in data coordinates
        rect = rectFromRanges(itx(nxl), itx(nxh), m_yLo, m_yHi);
    }
    if (push)
        pushZoomRect(rect, true);
    else {
        applyZoomRect(rect);
        emitZoomed();
        update();
    }
}

// ---------------------------------------------------------------------------
// SlsQt1DPlot - painting
// ---------------------------------------------------------------------------

std::vector<double> SlsQt1DPlot::tickValues(double lo, double hi, bool log,
                                            int maxTicks) const {
    if (log) {
        double l = lo > 0 ? lo : 1e-9;
        double h = hi > l ? hi : l * 10.0;
        std::vector<double> out;
        if (maxTicks < 2)
            maxTicks = 2;
        const auto lt =
            sls::plot::computeLinearTicks(std::log10(l), std::log10(h),
                                          maxTicks);
        out.reserve(lt.size());
        for (double t : lt) {
            const double v = std::pow(10.0, t);
            if (std::isfinite(v) && v > 0)
                out.push_back(v);
        }
        return out;
    }
    return sls::plot::computeLinearTicks(lo, hi, maxTicks);
}

std::vector<QString>
SlsQt1DPlot::tickLabels(const std::vector<double> &ticks, double lo, double hi,
                        bool log) const {
    std::vector<QString> labels;
    labels.reserve(ticks.size());
    if (log) {
        for (double v : ticks) {
            if (v >= 1e5 || v < 1e-2) {
                labels.push_back(QString::number(v, 'g', 3));
            } else if (v >= 1.0) {
                const double r = std::round(v);
                labels.push_back(std::fabs(v - r) < 1e-9
                                     ? QString::number(r, 'f', 0)
                                     : QString::number(v, 'g', 3));
            } else {
                labels.push_back(QString::number(v, 'g', 3));
            }
        }
    } else {
        double step = 1.0;
        if (ticks.size() >= 2)
            step = ticks[1] - ticks[0];
        else
            step = (hi - lo) / 8.0;
        for (double v : ticks)
            labels.push_back(sls::plot::formatTick(v, step));
    }
    return labels;
}

SlsQt1DPlot::Geometry SlsQt1DPlot::computeGeometry() const {
    Geometry g;
    const QFontMetrics fmTick(m_tickFont);
    const QFontMetrics fmAxisX(m_xTitleFont);
    const QFontMetrics fmAxisY(m_yTitleFont);
    const QFontMetrics fmTitle(m_titleFont);

    const int titleH = m_title.isEmpty() ? 0 : fmTitle.height() + 4;
    const int xTitleH = m_xTitle.isEmpty() ? 0 : fmAxisX.height() + 2;
    const int yTitleW = m_yTitle.isEmpty() ? 0 : fmAxisY.height() + 2;

    // Legend: count the curves that ask for an entry and lay them out in one
    // or more rows.  The height is reserved at the very bottom of the widget.
    int legendH = 0;
    {
        std::vector<SlsQtH1D *> entries;
        for (SlsQtH1D *h : m_hists) {
            if (h && h->legendVisible())
                entries.push_back(h);
        }
        if (!entries.empty()) {
            const int rowH = fmTick.height() + 4;
            const int avail = std::max(40, width() - 8);
            int rows = 1;
            int used = 0;
            for (SlsQtH1D *h : entries) {
                const int w =
                    20 + (h->title().isEmpty()
                              ? 0
                              : fmTick.horizontalAdvance(h->title()) + 4);
                if (used > 0 && used + w > avail) {
                    ++rows;
                    used = 0;
                }
                used += w;
            }
            const int maxRows = std::max(1, (height() / 3) / rowH);
            if (rows > maxRows)
                rows = maxRows;
            legendH = rows * rowH + 2;
        }
    }

    if (m_gainPlot) {
        // Exactly the ticks 0, 1, 2, 3 regardless of the widget height.  The
        // generic 1/2/5 chooser would yield 0, 0.5, 1, ... in a taller window.
        g.yTicks = {0.0, 1.0, 2.0, 3.0};
        g.yLabels = {QStringLiteral("0"), QStringLiteral("1"),
                     QStringLiteral("2"), QStringLiteral("3")};
    } else {
        const int availH = std::max(
            20, height() - titleH - xTitleH - legendH - (fmTick.height() + 6));
        const int maxYT =
            std::max(2, std::min(8, availH / (fmTick.height() + 8)));
        g.yTicks = tickValues(m_yLo, m_yHi, m_logY, maxYT);
        g.yLabels = tickLabels(g.yTicks, m_yLo, m_yHi, m_logY);
    }
    int yLabelW = 0;
    for (const QString &s : g.yLabels)
        yLabelW = std::max(yLabelW, fmTick.horizontalAdvance(s));

    int left = 3 + yTitleW + yLabelW + kTickLen + 2;
    const int right = 8;
    int plotW = width() - left - right;
    if (plotW < 20) {
        left = 3;
        plotW = std::max(10, width() - left - right);
    }

    // Choose the number of x ticks so that the labels do not overlap.
    std::vector<double> xt = tickValues(m_xLo, m_xHi, m_logX, 8);
    std::vector<QString> xl = tickLabels(xt, m_xLo, m_xHi, m_logX);
    int maxXLabelW = 12;
    for (const QString &s : xl)
        maxXLabelW = std::max(maxXLabelW, fmTick.horizontalAdvance(s));
    int maxXT = std::max(2, std::min(8, plotW / (maxXLabelW + 12)));
    g.xTicks = tickValues(m_xLo, m_xHi, m_logX, maxXT);
    g.xLabels = tickLabels(g.xTicks, m_xLo, m_xHi, m_logX);

    const int bottom =
        4 + fmTick.height() + 3 + xTitleH + legendH + 1;
    // y tick labels are vertically centred on the tick, so the plot rectangle
    // must not start at y = 0: in gain mode there is no title and the topmost
    // label ("3") would be clipped to its lower half at the widget edge.
    // Reserve at least half a tick-label height at the top (title-less non-gain
    // plots benefit as well; a title already provides the room).
    const int topMargin = std::max(titleH, fmTick.height() / 2 + 1);
    int top = topMargin;
    QRect plot(left, top, width() - left - right, height() - top - bottom);
    if (plot.width() < 20 || plot.height() < 20) {
        left = 3;
        top = topMargin;
        plot = QRect(left, top, std::max(10, width() - left - 3),
                     std::max(10, height() - top - 3));
    }

    if (plot.left() < 0)
        plot.setLeft(0);
    if (plot.top() < 0)
        plot.setTop(0);
    if (plot.right() >= width())
        plot.setRight(std::max(plot.left() + 1, width() - 1));
    if (plot.bottom() >= height())
        plot.setBottom(std::max(plot.top() + 1, height() - 1));

    g.plot = plot;
    g.titleH = titleH;
    g.xTitleH = xTitleH;
    g.yTitleW = yTitleW;
    g.legendH = legendH;
    return g;
}

void SlsQt1DPlot::paintGrid(QPainter &p, const Geometry &g) const {
    p.setRenderHint(QPainter::Antialiasing, false);
    QPen gridPen(m_theme.grid);
    gridPen.setStyle(Qt::SolidLine);
    gridPen.setWidth(1);
    gridPen.setCosmetic(true);
    p.setPen(gridPen);
    for (double v : g.xTicks) {
        const int x = qRound(toWidget(v, m_yLo, g).x());
        if (x > g.plot.left() && x < g.plot.right())
            p.drawLine(x, g.plot.top(), x, g.plot.bottom());
    }
    for (double v : g.yTicks) {
        const int y = qRound(toWidget(m_xLo, v, g).y());
        if (y > g.plot.top() && y < g.plot.bottom())
            p.drawLine(g.plot.left(), y, g.plot.right(), y);
    }
}

void SlsQt1DPlot::paintCurves(QPainter &p, const Geometry &g) const {
    p.save();
    p.setClipRect(g.plot);
    p.setRenderHint(QPainter::Antialiasing, false);
    const QRectF markerRect = QRectF(g.plot).adjusted(-3, -3, 3, 3);
    for (SlsQtH1D *h : m_hists) {
        if (!h || h->GetNBinsX() <= 0)
            continue;
        const double *xs = h->GetX();
        const double *ys = h->GetY();
        const int n = h->GetNBinsX();
        if (!xs || !ys)
            continue;

        QPen pen = h->pen();
        pen.setCosmetic(true);
        if (pen.style() == Qt::NoPen)
            pen.setStyle(Qt::SolidLine);
        p.setPen(pen);
        p.setBrush(Qt::NoBrush);

        const auto valid = [&](int i) {
            const double xv = xs[i];
            const double yv = ys[i];
            if (!std::isfinite(xv) || !std::isfinite(yv))
                return false;
            if (m_logX && !(xv > 0.0))
                return false;
            if (m_logY && !(yv > 0.0))
                return false;
            return true;
        };

        if (h->isLines()) {
            QPolygonF poly;
            poly.reserve(n);
            const auto flush = [&]() {
                if (poly.size() >= 2)
                    p.drawPolyline(poly);
                else if (poly.size() == 1)
                    p.drawPoint(poly[0]);
                poly.clear();
            };
            for (int i = 0; i < n; ++i) {
                if (!valid(i)) {
                    flush();
                    continue;
                }
                poly << toWidget(xs[i], ys[i], g);
            }
            flush();
        } else {
            for (int i = 0; i < n; ++i) {
                if (!valid(i))
                    continue;
                const QPointF pt = toWidget(xs[i], ys[i], g);
                if (markerRect.contains(pt))
                    p.drawPoint(pt);
            }
        }

        if (h->isMarkers()) {
            for (int i = 0; i < n; ++i) {
                if (!valid(i))
                    continue;
                const QPointF pt = toWidget(xs[i], ys[i], g);
                if (!markerRect.contains(pt))
                    continue;
                const double cx = pt.x();
                const double cy = pt.y();
                p.drawLine(QPointF(cx - 2.0, cy), QPointF(cx + 2.0, cy));
                p.drawLine(QPointF(cx, cy - 2.0), QPointF(cx, cy + 2.0));
            }
        }
    }
    p.restore();
}

void SlsQt1DPlot::paintDecorations(QPainter &p, const Geometry &g) const {
    if (!m_roiOn && !m_hasHLine && !m_hasVLine)
        return;
    p.save();
    p.setClipRect(g.plot);
    p.setRenderHint(QPainter::Antialiasing, false);

    if (m_roiOn) {
        QPen roiPen(QColor(255, 235, 59));
        roiPen.setWidthF(1.5);
        roiPen.setCosmetic(true);
        p.setPen(roiPen);
        p.setBrush(Qt::NoBrush);
        for (const slsDetectorDefs::ROI &roi : m_rois) {
            double x0 = roi.xmin;
            double x1 = roi.xmax - 1;
            if (x1 < x0)
                x1 = x0;
            double y0 = m_roiYMin;
            double y1 = m_roiYMax - 1;
            if (y1 < y0)
                y1 = y0;
            const QPointF a = toWidget(x0, y0, g);
            const QPointF b = toWidget(x1, y1, g);
            p.drawRect(QRectF(a, b).normalized());
        }
    }

    QPen hvPen(QColor(255, 215, 0));
    hvPen.setStyle(Qt::SolidLine);
    hvPen.setWidth(1);
    hvPen.setCosmetic(true);
    p.setPen(hvPen);
    if (m_hasHLine) {
        const QPointF a = toWidget(m_xLo, m_hLine, g);
        const QPointF b = toWidget(m_xHi, m_hLine, g);
        p.drawLine(a, b);
    }
    if (m_hasVLine) {
        const QPointF a = toWidget(m_vLine, m_yLo, g);
        const QPointF b = toWidget(m_vLine, m_yHi, g);
        p.drawLine(a, b);
    }
    p.restore();
}

void SlsQt1DPlot::paintFrame(QPainter &p, const Geometry &g) const {
    p.setRenderHint(QPainter::Antialiasing, false);
    const QRect r = g.plot;
    const QFontMetrics fmTick(m_tickFont);

    QPen framePen(m_theme.frame);
    framePen.setWidth(1);
    framePen.setCosmetic(true);
    p.setPen(framePen);
    p.setBrush(Qt::NoBrush);
    p.drawRect(r.adjusted(0, 0, -1, -1));

    QPen tickPen(m_theme.tick);
    tickPen.setWidth(1);
    tickPen.setCosmetic(true);

    // x ticks and labels
    p.setFont(m_tickFont);
    for (int i = 0; i < static_cast<int>(g.xTicks.size()); ++i) {
        const int x = qRound(toWidget(g.xTicks[static_cast<size_t>(i)], m_yLo,
                                       g)
                                 .x());
        if (x < r.left() || x > r.right())
            continue;
        p.setPen(tickPen);
        p.drawLine(x, r.bottom() + 1, x, r.bottom() + kTickLen);
        if (i < static_cast<int>(g.xLabels.size())) {
            const QString &lab = g.xLabels[static_cast<size_t>(i)];
            p.setPen(m_theme.text);
            int w = fmTick.horizontalAdvance(lab) + 8;
            int lx = x - w / 2;
            if (lx < 0)
                lx = 0;
            if (lx + w > width())
                lx = std::max(0, width() - w);
            p.drawText(QRect(lx, r.bottom() + kTickLen + 2, w, fmTick.height()),
                       Qt::AlignHCenter | Qt::AlignTop, lab);
        }
    }

    // y ticks and labels
    for (int i = 0; i < static_cast<int>(g.yTicks.size()); ++i) {
        const int y = qRound(toWidget(m_xLo, g.yTicks[static_cast<size_t>(i)],
                                       g)
                                 .y());
        if (y < r.top() || y > r.bottom())
            continue;
        p.setPen(tickPen);
        p.drawLine(r.left() - kTickLen, y, r.left() - 1, y);
        if (i < static_cast<int>(g.yLabels.size())) {
            const QString &lab = g.yLabels[static_cast<size_t>(i)];
            p.setPen(m_theme.text);
            const int w = fmTick.horizontalAdvance(lab) + 4;
            int lx = r.left() - kTickLen - 2 - w;
            if (lx < 0)
                lx = 0;
            p.drawText(QRect(lx, y - fmTick.height() / 2, w, fmTick.height()),
                       Qt::AlignRight | Qt::AlignVCenter, lab);
        }
    }

    // titles
    p.setPen(m_theme.text);
    if (!m_title.isEmpty()) {
        p.setFont(m_titleFont);
        p.drawText(QRect(0, 0, width(), g.titleH), Qt::AlignCenter, m_title);
    }
    if (!m_xTitle.isEmpty()) {
        const QFontMetrics fmAxisX(m_xTitleFont);
        p.setFont(m_xTitleFont);
        const int y = height() - g.legendH - g.xTitleH;
        p.drawText(QRect(r.left(), y, r.width(),
                         std::max(g.xTitleH, fmAxisX.height())),
                   Qt::AlignHCenter | Qt::AlignTop, m_xTitle);
    }
    if (!m_yTitle.isEmpty()) {
        p.setFont(m_yTitleFont);
        const int axisH = QFontMetrics(m_yTitleFont).height();
        p.save();
        p.translate(1 + g.yTitleW / 2, r.center().y());
        p.rotate(-90.0);
        p.drawText(QRect(-r.height() / 2, -axisH / 2, r.height(), axisH),
                   Qt::AlignCenter, m_yTitle);
        p.restore();
    }
}

void SlsQt1DPlot::paintLegend(QPainter &p, const Geometry &g) const {
    if (g.legendH <= 0)
        return;
    std::vector<SlsQtH1D *> entries;
    for (SlsQtH1D *h : m_hists) {
        if (h && h->legendVisible())
            entries.push_back(h);
    }
    if (entries.empty())
        return;

    const QFontMetrics fm(m_tickFont);
    const int rowH = fm.height() + 4;
    const int avail = std::max(40, width() - 8);
    std::vector<std::vector<SlsQtH1D *>> rows;
    std::vector<int> rowW;
    std::vector<SlsQtH1D *> current;
    int curW = 0;
    for (SlsQtH1D *h : entries) {
        const int w =
            20 + (h->title().isEmpty()
                      ? 0
                      : fm.horizontalAdvance(h->title()) + 4);
        if (!current.empty() && curW + w > avail) {
            rows.push_back(current);
            rowW.push_back(curW);
            current.clear();
            curW = 0;
        }
        current.push_back(h);
        curW += w;
    }
    if (!current.empty()) {
        rows.push_back(current);
        rowW.push_back(curW);
    }
    const int maxRows = std::max(1, g.legendH / rowH);
    if (static_cast<int>(rows.size()) > maxRows) {
        rows.resize(static_cast<size_t>(maxRows));
        rowW.resize(static_cast<size_t>(maxRows));
    }

    p.setRenderHint(QPainter::Antialiasing, false);
    p.setFont(m_tickFont);
    int y = height() - g.legendH + 2;
    for (size_t ri = 0; ri < rows.size(); ++ri) {
        int x = (width() - rowW[ri]) / 2;
        const int cy = y + rowH / 2;
        for (SlsQtH1D *h : rows[ri]) {
            QPen sample(h->pen().color());
            sample.setStyle(Qt::SolidLine);
            sample.setWidth(2);
            sample.setCosmetic(true);
            p.setPen(sample);
            p.drawLine(x, cy, x + 16, cy);
            x += 20;
            if (!h->title().isEmpty()) {
                p.setPen(m_theme.text);
                const int tw = fm.horizontalAdvance(h->title());
                p.drawText(QRect(x, y, tw + 2, rowH),
                           Qt::AlignLeft | Qt::AlignVCenter, h->title());
                x += tw + 4;
            }
        }
        y += rowH;
    }
}

void SlsQt1DPlot::paintTracker(QPainter &p, const Geometry &g) const {
    if (!m_trackValid || !g.plot.contains(m_trackPos.toPoint()))
        return;
    const QPointF d = toData(m_trackPos, g);
    if (!std::isfinite(d.x()) || !std::isfinite(d.y()))
        return;

    const QFontMetrics fm(m_tickFont);
    const QString text =
        QStringLiteral("x: %1  y: %2").arg(formatValue(d.x()), formatValue(d.y()));
    QRect box = fm.boundingRect(text);
    box.adjust(-5, -3, 5, 3);
    int bx = qRound(m_trackPos.x()) + 12;
    int by = qRound(m_trackPos.y()) + 12;
    if (bx + box.width() > width() - 2)
        bx = width() - 2 - box.width();
    if (by + box.height() > height() - 2)
        by = height() - 2 - box.height();
    if (bx < 2)
        bx = 2;
    if (by < 2)
        by = 2;

    p.save();
    p.setRenderHint(QPainter::Antialiasing, false);
    QPen cross(QColor(255, 255, 255, 90));
    cross.setWidth(1);
    cross.setStyle(Qt::DashLine);
    p.setClipRect(g.plot);
    p.setPen(cross);
    const int cx = qRound(m_trackPos.x());
    const int cy = qRound(m_trackPos.y());
    p.drawLine(cx, g.plot.top(), cx, g.plot.bottom());
    p.drawLine(g.plot.left(), cy, g.plot.right(), cy);
    p.restore();

    const QRect textRect(bx, by, box.width(), box.height());
    p.save();
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(255, 255, 255, 205));
    p.drawRect(textRect);
    p.setPen(QColor(20, 20, 20));
    p.setFont(m_tickFont);
    p.drawText(textRect, Qt::AlignCenter, text);
    p.restore();
}

void SlsQt1DPlot::paintRubberBand(QPainter &p, const Geometry &g) const {
    if (!m_rubberBand)
        return;
    QRectF rr = QRectF(m_rubberStart, m_rubberCur).normalized();
    rr = rr.intersected(QRectF(g.plot));
    if (rr.width() < 1.0 && rr.height() < 1.0)
        return;
    QPen pen(QColor(140, 180, 255));
    pen.setStyle(Qt::DashLine);
    pen.setWidth(1);
    pen.setCosmetic(true);
    p.save();
    p.setRenderHint(QPainter::Antialiasing, false);
    p.setPen(pen);
    p.setBrush(QColor(140, 180, 255, 40));
    p.drawRect(rr);
    p.restore();
}

void SlsQt1DPlot::paintEvent(QPaintEvent *event) {
    (void)event;
    QPainter p(this);
    p.fillRect(rect(), m_theme.background);
    if (width() < 8 || height() < 8)
        return;
    const Geometry g = computeGeometry();
    if (g.plot.width() <= 0 || g.plot.height() <= 0)
        return;
    paintGrid(p, g);
    paintCurves(p, g);
    paintDecorations(p, g);
    paintFrame(p, g);
    paintLegend(p, g);
    paintTracker(p, g);
    paintRubberBand(p, g);
}

// ---------------------------------------------------------------------------
// SlsQt1DPlot - interaction
// ---------------------------------------------------------------------------

void SlsQt1DPlot::mousePressEvent(QMouseEvent *event) {
    const QPointF pos = sls::plot::eventPos(event);
    const Geometry g = computeGeometry();
    if (zoomAllowed()) {
        if (event->button() == Qt::LeftButton &&
            g.plot.contains(pos.toPoint())) {
            m_rubberBand = true;
            m_rubberStart = pos;
            m_rubberCur = pos;
            setCursor(Qt::CrossCursor);
            update();
            event->accept();
            return;
        }
        if (event->button() == Qt::MiddleButton) {
            m_panning = true;
            m_panStart = pos;
            m_panXnLo = tx(m_xLo);
            m_panXnHi = tx(m_xHi);
            m_panYnLo = ty(m_yLo);
            m_panYnHi = ty(m_yHi);
            setCursor(Qt::ClosedHandCursor);
            event->accept();
            return;
        }
        if (event->button() == Qt::RightButton) {
            if (event->modifiers() & Qt::ControlModifier)
                resetZoom(true);
            else
                zoomOutOneLevel();
            event->accept();
            return;
        }
    }
    QWidget::mousePressEvent(event);
}

void SlsQt1DPlot::mouseMoveEvent(QMouseEvent *event) {
    const QPointF pos = sls::plot::eventPos(event);
    const Geometry g = computeGeometry();
    const bool inside = g.plot.contains(pos.toPoint());
    if (inside != m_trackValid || pos != m_trackPos) {
        m_trackValid = inside;
        m_trackPos = pos;
        update();
    }

    if (m_rubberBand) {
        m_rubberCur = pos;
        update();
        event->accept();
        return;
    }
    if (m_panning) {
        if (g.plot.width() > 0 && g.plot.height() > 0) {
            const double dtx = (pos.x() - m_panStart.x()) / g.plot.width() *
                               (m_panXnHi - m_panXnLo);
            const double dty = (pos.y() - m_panStart.y()) / g.plot.height() *
                               (m_panYnHi - m_panYnLo);
            setView(itx(m_panXnLo - dtx), itx(m_panXnHi - dtx),
                    ity(m_panYnLo + dty), ity(m_panYnHi + dty));
            if (m_gainPlot) {
                m_yLo = 0.0;
                m_yHi = 3.0;
            }
            m_xAuto = false;
            if (!m_gainPlot)
                m_yAuto = false;
            emitZoomed();
            update();
        }
        event->accept();
        return;
    }
    QWidget::mouseMoveEvent(event);
}

void SlsQt1DPlot::mouseReleaseEvent(QMouseEvent *event) {
    if (m_rubberBand && event->button() == Qt::LeftButton) {
        m_rubberBand = false;
        unsetCursor();
        const QRectF raw = QRectF(m_rubberStart, m_rubberCur).normalized();
        const Geometry g = computeGeometry();
        if (raw.width() >= 5.0 && raw.height() >= 5.0 &&
            g.plot.width() > 0 && g.plot.height() > 0) {
            const QRectF rr = raw.intersected(QRectF(g.plot));
            if (rr.width() >= 5.0 && rr.height() >= 5.0) {
                const QPointF p1 = toData(rr.topLeft(), g);
                const QPointF p2 = toData(rr.bottomRight(), g);
                const double x0 = std::min(p1.x(), p2.x());
                const double x1 = std::max(p1.x(), p2.x());
                const double y0 = std::min(p1.y(), p2.y());
                const double y1 = std::max(p1.y(), p2.y());
                pushZoomRect(rectFromRanges(x0, x1, y0, y1), true);
            }
        }
        update();
        event->accept();
        return;
    }
    if (m_panning && event->button() == Qt::MiddleButton) {
        m_panning = false;
        unsetCursor();
        if (!m_zoomStack.empty() && m_zoomIndex >= 0 &&
            m_zoomIndex < static_cast<int>(m_zoomStack.size()))
            m_zoomStack[static_cast<size_t>(m_zoomIndex)] = currentRect();
        emitZoomed();
        event->accept();
        return;
    }
    QWidget::mouseReleaseEvent(event);
}

void SlsQt1DPlot::wheelEvent(QWheelEvent *event) {
    if (!zoomAllowed()) {
        event->ignore();
        return;
    }
    const QPointF pos = event->position();
    const Geometry g = computeGeometry();
    if (!g.plot.contains(pos.toPoint())) {
        event->ignore();
        return;
    }
    const int dy = event->angleDelta().y();
    if (dy > 0) {
        zoomAbout(pos, 1.0 / 1.18, true);
        event->accept();
    } else if (dy < 0) {
        zoomOutOneLevel();
        event->accept();
    } else {
        event->ignore();
    }
}

void SlsQt1DPlot::leaveEvent(QEvent *event) {
    if (m_trackValid) {
        m_trackValid = false;
        update();
    }
    QWidget::leaveEvent(event);
}

} // namespace sls
