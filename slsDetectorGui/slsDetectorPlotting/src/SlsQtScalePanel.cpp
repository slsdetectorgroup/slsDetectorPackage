// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2026 Contributors to the SLS Detector Package
#include "SlsQtScalePanel.h"

#include "SlsQtFastImage.h"
#include "SlsQtPlotCommon.h"

#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QSignalBlocker>
#include <QVBoxLayout>

#include <algorithm>
#include <cmath>

namespace sls {

namespace {

constexpr int kHistoHeight = 38;
// fraction of the width where the markers sit
constexpr double kLoMarker = 0.25;
constexpr double kHiMarker = 0.75;
// how close to a marker a click has to be, in widget pixels
constexpr double kGrabDistance = 10.0;
// smallest colour range a drag can produce
constexpr double kMinRange = 1e-9;

} // namespace

// ---------------------------------------------------------------------------
// SlsQtScaleHisto
// ---------------------------------------------------------------------------
SlsQtScaleHisto::SlsQtScaleHisto(QWidget *parent) : QWidget(parent) {
    setFixedHeight(kHistoHeight);
    setToolTip(tr("Histogram of the image. The band between the two markers "
                  "is the colour range, drag a marker to change it."));
}

void SlsQtScaleHisto::setData(const std::vector<int> &counts, double binLo,
                              double binHi, double colorLo, double colorHi,
                              bool logX, bool logY) {
    if (isDragging())
        return;
    m_counts = counts;
    m_binLo = binLo;
    m_binHi = binHi;
    m_colorLo = colorLo;
    m_colorHi = colorHi;
    m_logX = logX;
    m_logY = logY;
    update();
}

bool SlsQtScaleHisto::isDragging() const { return m_dragged != Marker::None; }

// Value axis shown by the strip, in log10 of the values if m_logX
void SlsQtScaleHisto::axisRange(double &lo, double &hi) const {
    double a = m_logX ? plot::logClamp(m_colorLo) : m_colorLo;
    double b = m_logX ? plot::logClamp(m_colorHi) : m_colorHi;
    if (!std::isfinite(a) || !std::isfinite(b)) {
        a = 0.0;
        b = 1.0;
    }
    if (!(b > a))
        b = a + 1.0;
    // puts a at kLoMarker and b at kHiMarker
    const double perWidth = (b - a) / (kHiMarker - kLoMarker);
    lo = a - kLoMarker * perWidth;
    hi = lo + perWidth;
}

void SlsQtScaleHisto::paintEvent(QPaintEvent *) {
    const plot::PlotTheme theme;
    QPainter p(this);
    p.fillRect(rect(), theme.background);
    const double w = width();
    const double h = height();

    double axisLo = 0.0, axisHi = 1.0;
    axisRange(axisLo, axisHi);
    const auto toX = [&](double v) {
        return (v - axisLo) / (axisHi - axisLo) * w;
    };

    const int maxCount =
        m_counts.empty() ? 0
                         : *std::max_element(m_counts.begin(), m_counts.end());
    if (maxCount > 0 && m_binHi > m_binLo) {
        const double binWidth = (m_binHi - m_binLo) / m_counts.size();
        const double full = m_logY ? std::log10(maxCount + 1.0) : maxCount;
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(120, 190, 255));
        for (size_t i = 0; i < m_counts.size(); ++i) {
            if (m_counts[i] <= 0)
                continue;
            const double x0 = toX(m_binLo + i * binWidth);
            const double x1 = toX(m_binLo + (i + 1) * binWidth);
            const double count =
                m_logY ? std::log10(m_counts[i] + 1.0) : m_counts[i];
            const double barHeight = std::max(1.0, count / full * (h - 6));
            p.drawRect(QRectF(x0, h - 3 - barHeight, std::max(1.0, x1 - x0),
                              barHeight));
        }
    }

    if (isDragging()) {
        p.setPen(QPen(QColor(255, 205, 110, 180), 1, Qt::DashLine));
        p.drawLine(QPointF(m_dragX, 0), QPointF(m_dragX, h));
    }

    const double xLo = kLoMarker * w;
    const double xHi = kHiMarker * w;
    p.fillRect(QRectF(xLo, 0, xHi - xLo, h), QColor(255, 255, 255, 26));
    p.setPen(QPen(QColor(255, 205, 110), 2));
    p.drawLine(QPointF(xLo, 0), QPointF(xLo, h));
    p.drawLine(QPointF(xHi, 0), QPointF(xHi, h));

    QFont f = font();
    f.setPixelSize(9);
    p.setFont(f);
    p.setPen(QColor(255, 225, 160));
    p.drawText(QRectF(xLo + 3, 1, w, 11), Qt::AlignLeft | Qt::AlignTop,
               QString::number(m_colorLo, 'g', 4));
    p.drawText(QRectF(0, 1, xHi - 3, 11), Qt::AlignRight | Qt::AlignTop,
               QString::number(m_colorHi, 'g', 4));
}

void SlsQtScaleHisto::mousePressEvent(QMouseEvent *e) {
    if (e->button() != Qt::LeftButton)
        return;
    const double x = e->position().x();
    const double toLo = std::abs(x - kLoMarker * width());
    const double toHi = std::abs(x - kHiMarker * width());
    if (std::min(toLo, toHi) > kGrabDistance)
        return;
    m_dragged = (toLo <= toHi) ? Marker::Lo : Marker::Hi;
    m_dragX = x;
    update();
}

void SlsQtScaleHisto::mouseMoveEvent(QMouseEvent *e) {
    if (!isDragging())
        return;
    m_dragX = std::clamp(e->position().x(), 0.0, double(width()));
    update();
}

void SlsQtScaleHisto::mouseReleaseEvent(QMouseEvent *e) {
    if (e->button() != Qt::LeftButton || !isDragging())
        return;
    double axisLo = 0.0, axisHi = 1.0;
    axisRange(axisLo, axisHi);
    const double onAxis = axisLo + m_dragX / width() * (axisHi - axisLo);
    const double v = m_logX ? std::pow(10.0, onAxis) : onAxis;

    double lo = m_colorLo;
    double hi = m_colorHi;
    if (m_dragged == Marker::Lo)
        lo = std::min(v, hi - kMinRange);
    else
        hi = std::max(v, lo + kMinRange);
    m_dragged = Marker::None;
    update();
    emit rangeChanged(lo, hi);
}

// ---------------------------------------------------------------------------
// SlsQtScalePanel
// ---------------------------------------------------------------------------
SlsQtScalePanel::SlsQtScalePanel(SlsQtFastImage *view, QWidget *parent)
    : QWidget(parent), m_view(view) {
    m_histo = new SlsQtScaleHisto(this);

    m_paletteCombo = new QComboBox(this);
    for (int i = 0; i < plot::paletteCount(); ++i)
        m_paletteCombo->addItem(
            plot::paletteName(static_cast<plot::Palette>(i)));
    m_paletteCombo->setMinimumContentsLength(12);

    m_imageLogCombo = new QComboBox(this);
    m_imageLogCombo->addItems({tr("lin"), tr("log")});
    m_imageLogCombo->setToolTip(
        tr("Colour scale of the image. With log, values <= 0 are shown as "
           "1e-2 so that empty pixels stay visible."));

    m_histoLogCombo = new QComboBox(this);
    m_histoLogCombo->addItems({tr("lin"), tr("log")});
    m_histoLogCombo->setToolTip(
        tr("Height of the histogram bars: counts or log10(counts + 1)."));

    auto *row = new QHBoxLayout();
    row->setSpacing(4);
    row->addWidget(new QLabel(tr("palette:"), this));
    row->addWidget(m_paletteCombo);
    row->addWidget(new QLabel(tr("image"), this));
    row->addWidget(m_imageLogCombo);
    row->addWidget(new QLabel(tr("histo"), this));
    row->addWidget(m_histoLogCombo);
    row->addStretch(1);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(4, 2, 4, 4);
    layout->setSpacing(3);
    layout->addWidget(m_histo);
    layout->addLayout(row);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    connect(
        m_paletteCombo, &QComboBox::currentIndexChanged, this,
        [this](int i) { m_view->setPalette(static_cast<plot::Palette>(i)); });
    connect(m_imageLogCombo, &QComboBox::currentIndexChanged, this,
            [this](int i) { m_view->setLogScale(i == 1); });
    connect(m_histoLogCombo, &QComboBox::currentIndexChanged, this,
            &SlsQtScalePanel::refresh);
    connect(m_histo, &SlsQtScaleHisto::rangeChanged, this,
            [this](double lo, double hi) {
                m_view->setColorRange(lo, hi);
                emit colorRangeEdited(lo, hi);
            });
    connect(m_view, &SlsQtFastImage::colorMapChanged, this,
            &SlsQtScalePanel::refresh);
    connect(m_view, &SlsQtFastImage::histogramChanged, this,
            &SlsQtScalePanel::refresh);
    refresh();
}

SlsQtScaleHisto *SlsQtScalePanel::histogramWidget() const { return m_histo; }

// Shows the state of the view
void SlsQtScalePanel::refresh() {
    {
        const QSignalBlocker blockPalette(m_paletteCombo);
        const QSignalBlocker blockLog(m_imageLogCombo);
        m_paletteCombo->setCurrentIndex(static_cast<int>(m_view->palette()));
        m_imageLogCombo->setCurrentIndex(m_view->logScale() ? 1 : 0);
    }
    double colorLo = 0.0, colorHi = 1.0;
    m_view->colorRange(colorLo, colorHi);
    double binLo = 0.0, binHi = 1.0;
    m_view->histogramRange(binLo, binHi);
    m_histo->setData(m_view->histogram(), binLo, binHi, colorLo, colorHi,
                     m_view->logScale(), m_histoLogCombo->currentIndex() == 1);
}

} // namespace sls
