// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2026 Contributors to the SLS Detector Package
#include "SlsQtScalePanel.h"

#include "SlsQtFastImage.h"

#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QObject>
#include <QPainter>
#include <QVBoxLayout>

#include <algorithm>
#include <cmath>

namespace sls {

namespace {
constexpr int kHistoHeight = 38;
constexpr int kPanelHeight = kHistoHeight + 40; // strip + one compact row
} // namespace

// ---------------------------------------------------------------------------
// SlsQtScaleHisto
// ---------------------------------------------------------------------------
SlsQtScaleHisto::SlsQtScaleHisto(QWidget *parent) : QWidget(parent) {
    setFixedHeight(kHistoHeight);
    setMouseTracking(true);
    setToolTip(QStringLiteral(
        "Distribution of the last image (its own value range); the band "
        "between the two markers is the range mapped to the palette. "
        "Markers sit at 1/4 and 3/4 and the axis rescales under them - "
        "drag either one to change that end."));
}

void SlsQtScaleHisto::setData(const std::vector<int> *h, double dlo,
                              double dhi, double clo, double chi, bool logX,
                              bool logY) {
    // Frozen while a marker is being dragged: re-filling the bars from the
    // live image mid-drag fed the drag back into the histogram (the bars
    // moved, which moved the marker, which moved the bars...).  The strip is
    // refreshed once, when the button comes back up.
    if (m_dragging)
        return;
    m_hist = h;
    m_dlo = dlo;
    m_dhi = dhi;
    m_clo = clo;
    m_chi = chi;
    m_logX = logX;
    m_logY = logY;
    update();
}

void SlsQtScaleHisto::clearData() {
    // The view that owned the histogram vector is gone (or detached): never
    // touch m_hist again.  A pending marker drag is abandoned as well.
    m_hist = nullptr;
    m_snap.clear();
    m_dragging = false;
    m_dragMin = -1;
    update();
}

bool SlsQtScaleHisto::isDragging() const { return m_dragging; }

void SlsQtScaleHisto::currentWindow(double &lo, double &hi) const {
    lo = m_clo;
    hi = m_chi;
}

void SlsQtScaleHisto::windowFor(double clo, double chi, double &wlo,
                                double &whi) const {
    double dclo = m_logX ? sls::plot::logClamp(clo) : clo;
    double dchi = m_logX ? sls::plot::logClamp(chi) : chi;
    if (!std::isfinite(dclo) || !std::isfinite(dchi)) {
        dclo = 0.0;
        dchi = 1.0;
    }
    if (!(dchi > dclo))
        dchi = dclo + 1.0;
    const double span = dchi - dclo;
    wlo = dclo - 0.5 * span;
    whi = dchi + 0.5 * span;
    if (!(whi > wlo))
        whi = wlo + 1.0;
}

void SlsQtScaleHisto::displayedWindow(double &lo, double &hi) const {
    // Work in the histogram's own binning space (log10 values when logX).
    if (m_dragging)
        windowFor(m_preClo, m_preChi, lo, hi);
    else
        windowFor(m_clo, m_chi, lo, hi);
}

const std::vector<int> *SlsQtScaleHisto::displayedHistogram() const {
    if (m_dragging && !m_snap.empty())
        return &m_snap;
    return m_hist;
}

int SlsQtScaleHisto::previewX() const { return m_previewX; }

void SlsQtScaleHisto::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.fillRect(rect(), QColor(28, 28, 28));
    const int W = width(), H = height();
    if (W <= 0 || H <= 0)
        return;
    // While a marker is dragged the axis stays exactly as it was when the
    // drag started: the bars (and the two numbers) must not move with the
    // cursor, only the dragged end is previewed as a dashed line.
    const double dclo = m_dragging ? m_preClo : m_clo;
    const double dchi = m_dragging ? m_preChi : m_chi;
    double wlo = 0.0, whi = 1.0;
    windowFor(dclo, dchi, wlo, whi);
    auto xof = [&](double vBin) { return (vBin - wlo) / (whi - wlo) * W; };

    // The image keeps arriving while the marker is dragged, so paint from the
    // snapshot taken when the button went down.
    const std::vector<int> *hv = displayedHistogram();
    if (hv && !hv->empty() && m_dhi > m_dlo) {
        int mx = 1;
        for (int v : *hv)
            if (v > mx)
                mx = v;
        const double lmx = std::log10(double(mx) + 1.0);
        const double binW = (m_dhi - m_dlo) / double(hv->size());
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(120, 190, 255));
        for (size_t i = 0; i < hv->size(); i++) {
            if ((*hv)[i] <= 0)
                continue;
            // solid: the bar is as wide as the bin spacing on screen
            const double x0 = xof(m_dlo + double(i) * binW);
            const double x1 = xof(m_dlo + double(i + 1) * binW);
            const double bh =
                m_logY ? (std::log10(double((*hv)[i]) + 1.0) / lmx)
                       : (double((*hv)[i]) / double(mx));
            const double w = std::max(1.0, x1 - x0);
            if (x0 > W || x0 + w < 0)
                continue;
            p.drawRect(QRectF(x0, H - 3 - bh * (H - 6), w,
                              std::max(1.0, bh * (H - 6))));
        }
    }

    // the two markers at their fixed places (they are what the axis follows)
    const double xa = 0.25 * W, xb = 0.75 * W;
    if (m_dragging) { // the dragged end, previewed only
        QPen pv(QColor(255, 205, 110, 180));
        pv.setWidth(1);
        pv.setStyle(Qt::DashLine);
        p.setPen(pv);
        const double x =
            std::max(0.0, std::min(double(W - 1), double(m_previewX)));
        p.drawLine(QPointF(x, 0), QPointF(x, H));
    }
    p.fillRect(QRectF(xa, 0, xb - xa, H), QColor(255, 255, 255, 26));
    QPen mk(QColor(255, 205, 110));
    mk.setWidth(2);
    p.setPen(mk);
    p.setBrush(Qt::NoBrush);
    p.drawLine(QPointF(xa, 0), QPointF(xa, H));
    p.drawLine(QPointF(xb, 0), QPointF(xb, H));
    QFont f = p.font();
    f.setPixelSize(9);
    p.setFont(f);
    p.setPen(QColor(255, 225, 160));
    p.drawText(QRectF(xa + 3, 1, std::max(0.0, W - xa - 6), 11),
               Qt::AlignLeft | Qt::AlignTop,
               QString::number(dclo, 'g', 4));
    p.drawText(QRectF(3, 1, std::max(0.0, xb - 3), 11),
               Qt::AlignRight | Qt::AlignTop,
               QString::number(dchi, 'g', 4));
}

void SlsQtScaleHisto::mousePressEvent(QMouseEvent *e) {
    const double xa = 0.25 * width(), xb = 0.75 * width();
    const double d = std::abs(sls::plot::eventPosition(e).x() - xa);
    const double d2 = std::abs(sls::plot::eventPosition(e).x() - xb);
    if (std::min(d, d2) > 10) {
        m_dragMin = -1; // too far from either marker: ignore
        return;
    }
    m_dragMin = (d <= d2) ? 1 : 0;
    // Dragging only moves this strip: re-rendering a megapixel image on every
    // mouse move is what made it crawl, so the new range is applied when the
    // button comes back up (see mouseReleaseEvent).
    m_dragging = true;
    m_snap.clear();
    if (m_hist)
        m_snap = *m_hist; // the bars: frozen as well (the image keeps updating)
    m_preClo = m_clo;     // the axis basis: frozen until the button is released
    m_preChi = m_chi;
    m_previewX = sls::plot::eventPosition(e).toPoint().x();
    mouseMoveEvent(e);
}

void SlsQtScaleHisto::mouseMoveEvent(QMouseEvent *e) {
    if (m_dragMin < 0)
        return;
    const int W = width();
    if (W <= 0)
        return;
    // The cursor -> value mapping uses the axis FROZEN at press time.  Using
    // the live clo_/chi_ here was the positive feedback: the move changed the
    // range, the range changed the window, and the same cursor position then
    // mapped to a different value, so the marker chased itself.
    double wlo = 0.0, whi = 1.0;
    windowFor(m_preClo, m_preChi, wlo, whi);
    const double t = std::max(0.0, std::min(1.0, double(sls::plot::eventPosition(e).x()) / W));
    const double vBin = wlo + t * (whi - wlo);
    const double v = m_logX ? std::pow(10.0, vBin) : vBin;
    if (m_dragMin == 1)
        m_clo = std::min(v, m_chi - 1e-9);
    else
        m_chi = std::max(v, m_clo + 1e-9);
    m_previewX = sls::plot::eventPosition(e).toPoint().x();
    update(); // only the preview line moves; bars and axis stay frozen
}

void SlsQtScaleHisto::mouseReleaseEvent(QMouseEvent *) {
    if (m_dragMin >= 0 && m_dragging) {
        m_dragging = false; // unfreeze first, so the refresh is applied
        m_snap.clear();     // back to the live histogram
        if (onRangeChanged)
            onRangeChanged(m_clo, m_chi); // one update, as soon as released
        update();
    }
    m_dragMin = -1;
}

// ---------------------------------------------------------------------------
// SlsQtScalePanel
// ---------------------------------------------------------------------------
SlsQtScalePanel::SlsQtScalePanel(SlsQtFastImage *view, QWidget *parent)
    : QWidget(parent) {
    auto *col = new QVBoxLayout(this);
    col->setContentsMargins(4, 2, 4, 4);
    col->setSpacing(3);

    m_histo = new SlsQtScaleHisto(this);
    col->addWidget(m_histo);

    auto *row = new QHBoxLayout();
    row->setSpacing(4);
    m_paletteCombo = new QComboBox(this);
    for (int i = 0; i < sls::plot::paletteCount(); i++)
        m_paletteCombo->addItem(
            sls::plot::paletteName(static_cast<sls::plot::Palette>(i)));
    m_imageLogCombo = new QComboBox(this);
    m_imageLogCombo->addItem(QStringLiteral("lin"));
    m_imageLogCombo->addItem(QStringLiteral("log"));
    m_imageLogCombo->setToolTip(
        QStringLiteral("image: log10(value), values <= 0 are capped at\n"
                       "1e-2 so empty pixels stay visible"));
    m_histoLogCombo = new QComboBox(this);
    m_histoLogCombo->addItem(QStringLiteral("lin"));
    m_histoLogCombo->addItem(QStringLiteral("log"));
    m_histoLogCombo->setToolTip(
        QStringLiteral("histogram bars: linear counts, or log10(count+1).\n"
                       "The histogram's value axis always follows the image\n"
                       "mapping above."));
    m_paletteCombo->setMinimumContentsLength(12);

    row->addWidget(new QLabel(QStringLiteral("palette:"), this));
    row->addWidget(m_paletteCombo);
    row->addWidget(new QLabel(QStringLiteral("image"), this));
    row->addWidget(m_imageLogCombo);
    row->addWidget(new QLabel(QStringLiteral("histo"), this));
    row->addWidget(m_histoLogCombo);
    row->addStretch(1);
    col->addLayout(row);

    connect(m_paletteCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int) { push(); });
    connect(m_imageLogCombo,
            QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            [this](int i) {
                if (m_suppress || !m_view)
                    return;
                m_view->setLogScale(i == 1);
                refresh();
            });
    connect(m_histoLogCombo,
            QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            [this](int) { updateHisto(); });
    m_histo->onRangeChanged = [this](double a, double b) {
        if (m_suppress || !m_view)
            return;
        m_view->setColorRange(a, b);
        m_controlLo = a;
        m_controlHi = b;
        refresh();
    };

    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    setFixedHeight(kPanelHeight);
    setView(view);
}

SlsQtScalePanel::~SlsQtScalePanel() { detachView(); }

void SlsQtScalePanel::attachView(SlsQtFastImage *view) {
    if (!view)
        return;
    m_prevImageChanged = view->onImageChanged;
    m_prevScaleReleased = view->onScaleReleased;
    view->onImageChanged = [this]() {
        if (m_prevImageChanged)
            m_prevImageChanged();
        refresh();
    };
    view->onScaleReleased = [this]() {
        if (m_prevScaleReleased)
            m_prevScaleReleased();
        refresh();
    };
    m_installed = true;
    // R6 lifetime: the strip holds a raw pointer to view->histogram().  When
    // the view is destroyed, drop it before any repaint can read freed memory.
    m_destroyedConn = QObject::connect(
        view, &QObject::destroyed, this, [this]() {
            m_installed = false;
            m_prevImageChanged = nullptr;
            m_prevScaleReleased = nullptr;
            if (m_histo)
                m_histo->clearData();
        });
}

void SlsQtScalePanel::detachView() {
    if (m_destroyedConn) {
        QObject::disconnect(m_destroyedConn);
        m_destroyedConn = QMetaObject::Connection();
    }
    if (m_view && m_installed) {
        m_view->onImageChanged = m_prevImageChanged;
        m_view->onScaleReleased = m_prevScaleReleased;
    }
    m_installed = false;
    m_prevImageChanged = nullptr;
    m_prevScaleReleased = nullptr;
    m_rangeEdited = false; // a pending edit belongs to the old view
    // R6: the strip must not keep pointing at the old view's histogram.
    if (m_histo)
        m_histo->clearData();
}

void SlsQtScalePanel::setView(SlsQtFastImage *view) {
    if (m_view == view) {
        refresh();
        return;
    }
    detachView();
    m_view = view;
    attachView(view);
    refresh();
}

SlsQtFastImage *SlsQtScalePanel::view() const { return m_view; }

void SlsQtScalePanel::updateHisto() {
    if (!m_view || !m_histo)
        return;
    double a = 0.0, b = 1.0;
    m_view->colorRange(a, b);
    double hlo = 0.0, hhi = 1.0;
    m_view->histogramRange(hlo, hhi);
    m_histo->setData(&m_view->histogram(), hlo, hhi, a, b,
                     m_view->logScale(),
                     m_histoLogCombo &&
                         m_histoLogCombo->currentIndex() == 1);
}

void SlsQtScalePanel::refresh() {
    if (!m_view)
        return;
    // Dragging the Z scale on the image changes the range on every mouse
    // move; re-filling the histogram from the image with that half-applied
    // range fed the drag back into the bars.  Frozen until release, when
    // onScaleReleased() calls this again.
    // R1: the same holds while one of the strip's own markers is dragged -
    // otherwise the incoming frames keep moving the control range (and the
    // strip labels) under the frozen marker.
    if (m_view->isScaleDragging() ||
        (m_histo && m_histo->isDragging()))
        return;
    m_suppress++;
    double a = 0.0, b = 1.0;
    m_view->colorRange(a, b);
    m_controlLo = a;
    m_controlHi = b;
    if (m_imageLogCombo)
        m_imageLogCombo->setCurrentIndex(m_view->logScale() ? 1 : 0);
    const int pi = m_view->paletteIndex();
    if (m_paletteCombo && pi >= 0 && pi < m_paletteCombo->count())
        m_paletteCombo->setCurrentIndex(pi);
    updateHisto();
    m_suppress--;
}

void SlsQtScalePanel::syncFromView() { refresh(); }

void SlsQtScalePanel::push() { // controls -> view
    if (m_suppress || !m_view)
        return;
    // Palette changes are always applied (cheap and lossless).
    const int pi = m_paletteCombo ? m_paletteCombo->currentIndex() : 0;
    if (pi >= 0)
        m_view->setPaletteLut(
            sls::plot::paletteLut(static_cast<sls::plot::Palette>(pi)));
    // R2: only push the colour range on an explicit control edit (the Plot
    // tab owns min/max; the panel only synchronises it).  Palette/log changes
    // must not perturb the automatic z range.  A strip-marker drag applies its
    // range directly in the onRangeChanged handler, not here.
    if (m_rangeEdited) {
        m_rangeEdited = false;
        double a = m_controlLo;
        double b = m_controlHi;
        if (!(b > a)) {
            b = a + 1.0;
            m_controlHi = b;
        }
        m_lastPushedValid = true;
        m_lastPushedLo = a;
        m_lastPushedHi = b;
        m_view->setColorRange(a, b);
    }
    refresh();
}

void SlsQtScalePanel::controlRange(double &lo, double &hi) const {
    lo = m_controlLo;
    hi = m_controlHi;
}

void SlsQtScalePanel::setControlRange(double lo, double hi) {
    // Mimic the operator applying both ends from the Plot tab.
    m_controlLo = lo;
    m_controlHi = hi;
    m_rangeEdited = true;
    push();
}

bool SlsQtScalePanel::lastPushedRange(double &lo, double &hi) const {
    lo = m_lastPushedLo;
    hi = m_lastPushedHi;
    return m_lastPushedValid;
}

void SlsQtScalePanel::setPalette(sls::plot::Palette p) {
    const int idx = int(p);
    if (m_paletteCombo && idx >= 0 && idx < m_paletteCombo->count())
        setPaletteChoice(idx);
    else if (m_view)
        m_view->setPaletteLut(sls::plot::paletteLut(p));
    refresh();
}

void SlsQtScalePanel::setPaletteChoice(int index) {
    if (!m_paletteCombo || index < 0 || index >= m_paletteCombo->count())
        return;
    if (m_paletteCombo->currentIndex() == index)
        push();
    else
        m_paletteCombo->setCurrentIndex(index); // triggers push()
}

int SlsQtScalePanel::paletteChoice() const {
    return m_paletteCombo ? m_paletteCombo->currentIndex() : -1;
}

void SlsQtScalePanel::setImageLog(bool on) {
    if (m_view)
        m_view->setLogScale(on);
    refresh();
}

void SlsQtScalePanel::setColorRange(double lo, double hi) {
    if (m_view)
        m_view->setColorRange(lo, hi);
    refresh();
}

void SlsQtScalePanel::setHistoLog(bool on) {
    if (m_histoLogCombo)
        m_histoLogCombo->setCurrentIndex(on ? 1 : 0);
}

bool SlsQtScalePanel::histoLog() const {
    return m_histoLogCombo && m_histoLogCombo->currentIndex() == 1;
}

void SlsQtScalePanel::setPaletteEnabled(bool on) {
    if (m_paletteCombo)
        m_paletteCombo->setEnabled(on);
}

SlsQtScaleHisto *SlsQtScalePanel::histogramWidget() const { return m_histo; }

bool SlsQtScalePanel::isDragging() const {
    return m_histo && m_histo->isDragging();
}

} // namespace sls
