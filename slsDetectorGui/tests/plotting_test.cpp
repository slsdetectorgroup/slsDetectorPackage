// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2026 Contributors to the SLS Detector Package

// Tests of the plotting widgets, through their public interface and through
// the pixels they paint. Run with QT_QPA_PLATFORM=offscreen.

#include "SlsQt1DPlot.h"
#include "SlsQt2DHist.h"
#include "SlsQt2DPlot.h"
#include "SlsQtFastImage.h"
#include "SlsQtPlotCommon.h"
#include "SlsQtScalePanel.h"
#include "sls/sls_detector_defs.h"

#include <QApplication>
#include <QComboBox>
#include <QCoreApplication>
#include <QElapsedTimer>
#include <QHBoxLayout>
#include <QHash>
#include <QImage>
#include <QLabel>
#include <QMouseEvent>
#include <QSet>
#include <QSignalSpy>
#include <QTest>
#include <QVBoxLayout>
#include <QWheelEvent>
#include <QWidget>

#include <algorithm>
#include <cmath>
#include <limits>
#include <numeric>
#include <utility>
#include <vector>

using sls::plot::Palette;

namespace {

constexpr QRgb kBackground = qRgb(28, 28, 28);

bool near(double a, double b, double eps = 1e-6) {
    return std::abs(a - b) <= eps;
}

bool nearRect(const QRectF &a, const QRectF &b) {
    return near(a.x(), b.x()) && near(a.y(), b.y()) &&
           near(a.width(), b.width()) && near(a.height(), b.height());
}

// Repaint synchronously (runs renderImage + paintEvent) and grab the result.
QImage render(QWidget &w) {
    w.repaint();
    return w.grab().toImage();
}

QPointF widgetPos(const sls::SlsQtFastImage &view, double dataX, double dataY) {
    return view.dataToWidget(QPointF(dataX, dataY));
}

// Most common colour of the 7x7 pixels around the data point.  Sampling one
// pixel would sometimes hit an axis grid line or a pixel outline drawn on top
// of the image, which is exactly what the plot is supposed to do; the cell
// colour is the majority of the neighbourhood.
QRgb dominantPixelAtData(const sls::SlsQtFastImage &view, const QImage &img,
                         double dataX, double dataY, int radius = 3) {
    const QPointF p = widgetPos(view, dataX, dataY);
    const int cx = int(std::lround(p.x()));
    const int cy = int(std::lround(p.y()));
    QHash<QRgb, int> counts;
    for (int dy = -radius; dy <= radius; dy++) {
        for (int dx = -radius; dx <= radius; dx++) {
            const int x = cx + dx, y = cy + dy;
            if (x < 0 || y < 0 || x >= img.width() || y >= img.height())
                continue;
            counts[img.pixel(x, y)]++;
        }
    }
    QRgb best = kBackground;
    int bestCount = -1;
    for (auto it = counts.constBegin(); it != counts.constEnd(); ++it) {
        if (it.value() > bestCount) {
            bestCount = it.value();
            best = it.key();
        }
    }
    return best;
}

void sendMouse(QWidget &w, QEvent::Type type, const QPointF &pos,
               Qt::MouseButton button, Qt::MouseButtons buttons) {
    QMouseEvent e(type, pos, QPointF(w.mapToGlobal(pos.toPoint())), button,
                  buttons, Qt::NoModifier);
    QCoreApplication::sendEvent(&w, &e);
}

void sendWheel(QWidget &w, const QPointF &pos, int deltaY) {
    QWheelEvent e(pos, QPointF(w.mapToGlobal(pos.toPoint())), QPoint(),
                  QPoint(0, deltaY), Qt::NoButton, Qt::NoModifier,
                  Qt::NoScrollPhase, false);
    QCoreApplication::sendEvent(&w, &e);
}

// True when a pixel close to (centre +/- radius) is drawn in a yellow tone.
bool yellowNear(const QImage &img, const QPoint &centre, int radius = 2) {
    for (int dy = -radius; dy <= radius; dy++) {
        for (int dx = -radius; dx <= radius; dx++) {
            const int x = centre.x() + dx, y = centre.y() + dy;
            if (x < 0 || y < 0 || x >= img.width() || y >= img.height())
                continue;
            const QRgb c = img.pixel(x, y);
            if (qRed(c) > 180 && qGreen(c) > 180 && qBlue(c) < 90)
                return true;
        }
    }
    return false;
}

bool isYellow(QRgb c) {
    return qRed(c) > 200 && qGreen(c) > 170 && qBlue(c) < 130;
}

bool isBright(QRgb c) {
    return qRed(c) > 120 && qGreen(c) > 120 && qBlue(c) > 120;
}

int countPixels(const QImage &img, bool (*pred)(QRgb)) {
    int n = 0;
    for (int y = 0; y < img.height(); y++) {
        for (int x = 0; x < img.width(); x++) {
            if (pred(img.pixel(x, y)))
                n++;
        }
    }
    return n;
}

// Nice steps must be 1, 2 or 5 times a power of ten.
bool is125Step(double step) {
    if (!(step > 0.0) || !std::isfinite(step))
        return false;
    const double mag = std::pow(10.0, std::floor(std::log10(step) + 1e-9));
    const double norm = step / mag;
    return near(norm, 1.0, 1e-6) || near(norm, 2.0, 1e-6) ||
           near(norm, 5.0, 1e-6);
}

// 1024 x 512 pixels of sin(x/40) + cos(y/30)
std::vector<double> testFrame() {
    std::vector<double> f(size_t(1024) * 512);
    for (int y = 0; y < 512; y++)
        for (int x = 0; x < 1024; x++)
            f[size_t(y) * 1024 + x] = std::sin(x / 40.0) + std::cos(y / 30.0);
    return f;
}

// Every pixel has the value of its x index
std::vector<double> xRamp(int nx, int ny) {
    std::vector<double> f(size_t(nx) * ny);
    for (int y = 0; y < ny; y++)
        for (int x = 0; x < nx; x++)
            f[size_t(y) * nx + x] = x;
    return f;
}

int sum(const std::vector<int> &v) {
    return std::accumulate(v.begin(), v.end(), 0);
}

// Plot area of an image view without titles
QRect plotArea(const sls::SlsQtFastImage &view) {
    return sls::plot::plotRectFor(view.size(), sls::plot::PlotTheme());
}

// Number of matching pixels to the right of the plot area and above it, where
// an image view without titles draws nothing. The corners below and to the
// left are left out: how far the last tick labels reach into them depends on
// the font.
int countOutside(const QImage &img, const QRect &plot, bool (*pred)(QRgb)) {
    int n = 0;
    for (int y = 0; y < img.height(); y++) {
        for (int x = 0; x < img.width(); x++) {
            const bool right = x > plot.right() && y <= plot.bottom();
            const bool above = y < plot.top() && x >= plot.left();
            if ((right || above) && pred(img.pixel(x, y)))
                n++;
        }
    }
    return n;
}

// Bounding box of the pixels that differ
QRect differingArea(const QImage &a, const QImage &b) {
    QRect area;
    for (int y = 0; y < a.height(); y++) {
        for (int x = 0; x < a.width(); x++) {
            if (a.pixel(x, y) != b.pixel(x, y))
                area |= QRect(x, y, 1, 1);
        }
    }
    return area;
}

} // namespace

class PlottingTest : public QObject {
    Q_OBJECT

  private slots:
    void palettesAndLuts();
    void paletteLogClamp();
    void tickMath();
    void tickFormatting();

    void imageIsCopied();
    void imageYUp();
    void imageLogScale();
    void imageHistogram();
    void imageViewport();
    void imageWheelZoom();
    void imageTouchpadZoom();
    void imagePan();
    void imageIsClippedToPlotArea();
    void imageOverlays();
    void imageInterpolation();
    void imageTitles();
    void imagePixelLabels();
    void imageReadout();
    void imageAxisValues();

    void darkPaletteGoesWithThePlots();

    void scaleHistoDrag();
    void scaleHistoLogAxis();
    void scalePanel();

    void h1dDataApi();
    void oneDimensionalRangeZoomLogAndGain();
    void oneDimensionalRoiBoxes();

    void twoDHistogramContainer();
    void plot2dTitles();
    void plot2dZRange();
    void plot2dLogZ();
    void plot2dContours();
    void plot2dRoiAndZoom();
    void plot2dResizeIsReportedLater();
    void plot2dGain();

    void performance1024x512();
};

// ---------------------------------------------------------------------------
// palettes / LUTs
// ---------------------------------------------------------------------------
void PlottingTest::palettesAndLuts() {
    // Gain4 is the fixed categorical palette and is not in the chooser.
    QCOMPARE(sls::plot::paletteCount(), 18);
    QCOMPARE(int(sls::plot::Palette::Gain4), 18);

    QSet<QString> names;
    for (int i = 0; i < sls::plot::paletteCount(); i++) {
        const auto p = static_cast<sls::plot::Palette>(i);
        const QRgb *lut = sls::plot::paletteLut(p);
        QVERIFY2(lut != nullptr, "palette LUT must never be null");
        // The same pointer is handed out every time (built once).
        QVERIFY(sls::plot::paletteLut(p) == lut);
        for (int k = 0; k < 256; k++) {
            QVERIFY2(qAlpha(lut[k]) == 255, "LUT entries must be opaque");
        }
        const char *n = sls::plot::paletteName(p);
        QVERIFY(n != nullptr && n[0] != '\0');
        QVERIFY2(!names.contains(QString::fromLatin1(n)),
                 "palette names must be unique");
        names.insert(QString::fromLatin1(n));
    }

    // Grey ramps: exact endpoints and monotonicity.
    const QRgb *gray = sls::plot::paletteLut(sls::plot::Palette::Gray);
    QCOMPARE(gray[0], qRgb(0, 0, 0));
    QCOMPARE(gray[255], qRgb(255, 255, 255));
    const QRgb *grayR = sls::plot::paletteLut(sls::plot::Palette::Gray_r);
    QCOMPARE(grayR[0], qRgb(255, 255, 255));
    QCOMPARE(grayR[255], qRgb(0, 0, 0));

    // BW1/BW2 are monotonic grey ramps with different gamma.
    for (auto p : {sls::plot::Palette::BW1, sls::plot::Palette::BW2}) {
        const QRgb *lut = sls::plot::paletteLut(p);
        for (int k = 0; k < 256; k++) {
            QVERIFY(qRed(lut[k]) == qGreen(lut[k]) &&
                    qRed(lut[k]) == qBlue(lut[k]));
            if (k > 0)
                QVERIFY(qRed(lut[k]) >= qRed(lut[k - 1]));
        }
    }
    QVERIFY(qRed(sls::plot::paletteLut(sls::plot::Palette::BW1)[128]) !=
            qRed(sls::plot::paletteLut(sls::plot::Palette::BW2)[128]));

    // Color is the jet-like default and differs from the others; Gain4 exists
    // as a fixed categorical map even though paletteCount() hides it.
    QVERIFY(sls::plot::paletteLut(sls::plot::Palette::Color)[10] !=
            sls::plot::paletteLut(sls::plot::Palette::Viridis)[10]);
    const QRgb *gain = sls::plot::paletteLut(sls::plot::Palette::Gain4);
    QVERIFY(gain != nullptr);
    QVERIFY(gain[0] != sls::plot::paletteLut(sls::plot::Palette::Color)[0]);
    QString gainName =
        QString::fromLatin1(sls::plot::paletteName(sls::plot::Palette::Gain4));
    QVERIFY(gainName.contains(QStringLiteral("gain"), Qt::CaseInsensitive));
}

void PlottingTest::paletteLogClamp() {
    // Empty pixels (0 / negative pedestal) must not become -inf.
    QCOMPARE(sls::plot::logClamp(-3.0), -2.0);
    QCOMPARE(sls::plot::logClamp(0.0), -2.0);
    QCOMPARE(sls::plot::logClamp(1e-9), -2.0);
    QCOMPARE(sls::plot::logClamp(0.01), -2.0);
    QVERIFY(near(sls::plot::logClamp(0.02), std::log10(0.02), 1e-12));
    QCOMPARE(sls::plot::logClamp(1.0), 0.0);
    QCOMPARE(sls::plot::logClamp(100.0), 2.0);
    QVERIFY(sls::plot::logClamp(1e6) == 6.0);
}

// ---------------------------------------------------------------------------
// tick math / formatting
// ---------------------------------------------------------------------------
void PlottingTest::tickMath() {
    for (double span : {0.9, 10.0, 1000.0, 3.7, 1e6, 1e-3, 1024.0, 512.0}) {
        QVERIFY2(is125Step(sls::plot::niceStep(span, 8)), "step = 1/2/5*10^n");
    }
    QCOMPARE(sls::plot::niceStep(1000.0, 8), 200.0);
    QVERIFY(near(sls::plot::niceStep(0.9, 8), 0.2));
    QCOMPARE(sls::plot::niceStep(0.0, 8), 1.0);  // degenerate
    QCOMPARE(sls::plot::niceStep(-5.0, 8), 1.0); // degenerate

    // Ticks cover the visible range, never exceed the requested density and
    // are uniformly spaced.
    for (auto range : {std::pair<double, double>{0.0, 1024.0},
                       std::pair<double, double>{-2.0, 2.0},
                       std::pair<double, double>{0.3, 0.7},
                       std::pair<double, double>{-300.0, 1300.0}}) {
        const std::vector<double> ticks =
            sls::plot::computeLinearTicks(range.first, range.second, 8);
        QVERIFY(!ticks.empty());
        QVERIFY2(ticks.size() <= 9, "at most maxTicks+1 tick values");
        QVERIFY(ticks.front() >= range.first - 1e-9);
        QVERIFY(ticks.back() <= range.second + 1e-9);
        const double step = sls::plot::niceStep(range.second - range.first, 8);
        QVERIFY(near(ticks.front(), 0.0, 1e-9) ||
                std::fmod(ticks.front(), step) < 1e-6);
        for (double v : ticks) {
            const double n = v / step;
            QVERIFY2(near(n, std::round(n), 1e-6),
                     "ticks are multiples of the nice step");
        }
    }
    QVERIFY(sls::plot::computeLinearTicks(1.0, 1.0, 8).empty());
    QVERIFY(sls::plot::computeLinearTicks(2.0, 1.0, 8).empty());
    QVERIFY(sls::plot::computeLinearTicks(
                std::numeric_limits<double>::quiet_NaN(), 1.0, 8)
                .empty());

    // Theme values are the contract shared by 1D/2D and the image view.
    const sls::plot::PlotTheme theme;
    QCOMPARE(theme.leftMargin, 52);
    QCOMPARE(theme.bottomMargin, 26);
    QCOMPARE(theme.topMargin, 6);
    QCOMPARE(theme.rightMargin, 6);
    QCOMPARE(theme.tickFont.pixelSize(), 10);
    const QRect r = sls::plot::plotRectFor(QSize(400, 300), theme);
    QCOMPARE(r, QRect(52, 6, 400 - 52 - 6, 300 - 6 - 26));
    QVERIFY(sls::plot::plotRectFor(QSize(10, 10), theme).width() >= 10);
}

void PlottingTest::tickFormatting() {
    QCOMPARE(sls::plot::formatTick(200.0, 200.0), QStringLiteral("200"));
    QCOMPARE(sls::plot::formatTick(0.2, 0.2), QStringLiteral("0.2"));
    QCOMPARE(sls::plot::formatTick(1.0, 0.5), QStringLiteral("1"));
    QCOMPARE(sls::plot::formatTick(3.0, 0.5), QStringLiteral("3"));
    QCOMPARE(sls::plot::formatTick(3.5, 0.5), QStringLiteral("3.5"));
    QCOMPARE(sls::plot::formatTick(0.25, 0.05), QStringLiteral("0.25"));
    QCOMPARE(sls::plot::formatTick(-0.0, 0.5), QStringLiteral("0"));
    QCOMPARE(sls::plot::formatTick(-200.0, 200.0), QStringLiteral("-200"));
    QVERIFY(sls::plot::formatTick(2.5e7, 1e7).contains(QLatin1Char('e')));
    QCOMPARE(
        sls::plot::formatTick(std::numeric_limits<double>::infinity(), 1.0),
        QStringLiteral("?"));
    // No trailing zeros anywhere, and labels stay short.
    for (double v : {0.0, 1.0, 2.0, 5.0, 10.0, 100.0, -100.0, 0.5, 2.5}) {
        const QString s = sls::plot::formatTick(v, 1.0);
        QVERIFY2(!s.contains(QStringLiteral(".0")), qPrintable(s));
        QVERIFY2(s.size() <= 6, qPrintable(s));
    }
}

// ---------------------------------------------------------------------------
// SlsQtFastImage: data and colour mapping
// ---------------------------------------------------------------------------
void PlottingTest::imageIsCopied() {
    sls::SlsQtFastImage view;
    view.resize(200, 200); // pixels are outlined but not labelled
    view.show();
    view.setColorRange(0.0, 2.0);
    std::vector<double> src(16, 1.0);
    view.setImage(src.data(), 4, 4);
    QCOMPARE(dominantPixelAtData(view, render(view), 0.5, 0.5),
             sls::plot::paletteLut(Palette::Color)[127]);

    // the caller may reuse its buffer: a new palette still shows 1.0
    std::fill(src.begin(), src.end(), 2.0);
    view.setPalette(Palette::Gray);
    QCOMPARE(dominantPixelAtData(view, render(view), 0.5, 0.5),
             sls::plot::paletteLut(Palette::Gray)[127]);

    // invalid input is ignored
    view.setImage(nullptr, 4, 4);
    view.setImage(src.data(), 0, 4);
    QCOMPARE(view.imageWidth(), 4);
    QCOMPARE(view.imageHeight(), 4);
}

void PlottingTest::imageYUp() {
    sls::SlsQtFastImage view;
    view.resize(200, 200);
    view.show();
    view.setPalette(Palette::Gray);
    view.setColorRange(0.0, 4.0);
    // row 0 is the bottom row
    const std::vector<double> f{0, 0, 0, 0, 3, 3, 3, 3};
    view.setImage(f.data(), 4, 2);
    const QImage img = render(view);

    QVERIFY(widgetPos(view, 0.5, 0.5).y() > widgetPos(view, 0.5, 1.5).y());
    const QRgb *lut = sls::plot::paletteLut(Palette::Gray);
    QCOMPARE(dominantPixelAtData(view, img, 0.5, 0.5), lut[0]);
    QCOMPARE(dominantPixelAtData(view, img, 0.5, 1.5), lut[3 * 255 / 4]);
    QCOMPARE(dominantPixelAtData(view, img, 2.5, 1.5), lut[3 * 255 / 4]);
}

void PlottingTest::imageLogScale() {
    sls::SlsQtFastImage view;
    view.resize(200, 200);
    view.show();
    view.setLogScale(true);
    view.setColorRange(0.01, 100.0);
    const double nan = std::numeric_limits<double>::quiet_NaN();
    const std::vector<double> f{-100.0, 0.0, 0.01, 0.02, 1000.0, nan, 1.0, 1.0};
    view.setImage(f.data(), 4, 2);
    const QImage img = render(view);
    const QRgb *lut = sls::plot::paletteLut(Palette::Color);

    // everything <= 0 is shown like 1e-2, the lower end of this range
    QCOMPARE(dominantPixelAtData(view, img, 0.5, 0.5), lut[0]);
    QCOMPARE(dominantPixelAtData(view, img, 1.5, 0.5), lut[0]);
    QCOMPARE(dominantPixelAtData(view, img, 2.5, 0.5), lut[0]);
    QVERIFY(dominantPixelAtData(view, img, 3.5, 0.5) != lut[0]);
    // above the range and not finite
    QCOMPARE(dominantPixelAtData(view, img, 0.5, 1.5), lut[255]);
    QCOMPARE(dominantPixelAtData(view, img, 1.5, 1.5), lut[0]);
    // 1 is halfway between 1e-2 and 1e2
    QCOMPARE(dominantPixelAtData(view, img, 2.5, 1.5), lut[127]);
}

void PlottingTest::imageHistogram() {
    sls::SlsQtFastImage view;
    view.resize(200, 200);
    view.show();
    const int nx = 64, ny = 32;
    const std::vector<double> ramp = xRamp(nx, ny);
    view.setColorRange(1.0, 3.0);
    view.setImage(ramp.data(), nx, ny);
    QSignalSpy spy(&view, &sls::SlsQtFastImage::histogramChanged);
    render(view);
    QCOMPARE(spy.count(), 1);

    // the colour range widened by half its width on both sides
    double lo = 0.0, hi = 0.0;
    view.histogramRange(lo, hi);
    QVERIFY(near(lo, 0.0) && near(hi, 4.0));
    QCOMPARE(int(view.histogram().size()), 256);
    QCOMPARE(sum(view.histogram()), 5 * ny); // x = 0..4

    // follows the colour range, values outside are not counted
    view.setColorRange(10.2, 19.8);
    render(view);
    view.histogramRange(lo, hi);
    QVERIFY(near(lo, 5.4) && near(hi, 24.6));
    QCOMPARE(sum(view.histogram()), 19 * ny); // x = 6..24
    QCOMPARE(view.histogram().front(), 0);
    QCOMPARE(view.histogram().back(), 0);

    // in log10 of the values with a log scale
    view.setLogScale(true);
    view.setColorRange(1.0, 100.0);
    render(view);
    view.histogramRange(lo, hi);
    QVERIFY(near(lo, -1.0) && near(hi, 3.0));
    QCOMPARE(sum(view.histogram()), 63 * ny); // x = 0 counts as 1e-2
}

// ---------------------------------------------------------------------------
// SlsQtFastImage: viewport, zoom and pan
// ---------------------------------------------------------------------------
void PlottingTest::imageViewport() {
    sls::SlsQtFastImage view;
    view.resize(400, 300);
    view.show();
    const int nx = 64, ny = 32;
    const std::vector<double> ramp = xRamp(nx, ny);
    view.setImage(ramp.data(), nx, ny);
    QSignalSpy spy(&view, &sls::SlsQtFastImage::visibleAreaChanged);

    // the whole image to start with
    QVERIFY(nearRect(view.visibleArea(), QRectF(0, 0, nx, ny)));

    // clamped to the image
    view.setViewport(-100.0, -100.0, 5000.0, 5000.0);
    QVERIFY(nearRect(view.visibleArea(), QRectF(0, 0, nx, ny)));

    // moved back inside
    view.setViewport(60.0, 28.0, 70.0, 38.0);
    QVERIFY(nearRect(view.visibleArea(), QRectF(54, 22, 10, 10)));

    // too small or not finite: ignored
    view.setViewport(10.0, 10.0, 10.5, 10.5);
    view.setViewport(std::numeric_limits<double>::quiet_NaN(), 0.0, 9.0, 9.0);
    QVERIFY(nearRect(view.visibleArea(), QRectF(54, 22, 10, 10)));

    // exactly this part is shown, the rest of the plot area stays empty
    const QImage img = render(view);
    const QRect plot = plotArea(view);
    QVERIFY(near(widgetPos(view, 64, 22).x() - widgetPos(view, 54, 22).x(),
                 plot.height()));
    QCOMPARE(img.pixel(plot.left() + 5, plot.center().y() + 3), kBackground);
    QCOMPARE(img.pixel(plot.right() - 5, plot.center().y() + 3), kBackground);
    QVERIFY(img.pixel(plot.center() + QPoint(3, 3)) != kBackground);

    // only the user and resetView() are reported
    QCOMPARE(spy.count(), 0);
    view.resetView();
    QCOMPARE(spy.count(), 1);
    QVERIFY(nearRect(spy.at(0).at(0).toRectF(), QRectF(0, 0, nx, ny)));

    // a new image size shows the whole image again
    view.setViewport(10.0, 10.0, 20.0, 20.0);
    view.setImage(ramp.data(), ny, nx);
    QVERIFY(nearRect(view.visibleArea(), QRectF(0, 0, ny, nx)));
}

void PlottingTest::imageWheelZoom() {
    sls::SlsQtFastImage view;
    view.resize(400, 300);
    view.show();
    const int n = 64;
    const std::vector<double> ramp = xRamp(n, n);
    view.setImage(ramp.data(), n, n);
    QSignalSpy spy(&view, &sls::SlsQtFastImage::visibleAreaChanged);
    const QRect plot = plotArea(view);
    const double plotShape = double(plot.width()) / plot.height();
    const QPointF cursor(200.0, 150.0);

    // the image grows until it fills the plot area
    for (int i = 0; i < 3; i++)
        sendWheel(view, cursor, 120);
    QCOMPARE(spy.count(), 3);
    QRectF area = view.visibleArea();
    QVERIFY(area.width() < n && area.height() < n);
    QVERIFY(near(area.width() / area.height(), plotShape));
    QVERIFY(nearRect(spy.at(2).at(0).toRectF(), area));

    // and then keeps the point under the cursor
    for (int i = 0; i < 5; i++) {
        const QPointF before = view.widgetToData(cursor);
        sendWheel(view, cursor, 120);
        const QPointF after = view.widgetToData(cursor);
        QVERIFY(near(before.x(), after.x()) && near(before.y(), after.y()));
    }

    // a resize shows more or less of the image, at the same magnification
    area = view.visibleArea();
    view.resize(500, 300);
    QVERIFY(near(view.visibleArea().height(), area.height()));
    QVERIFY(view.visibleArea().width() > area.width());
    view.resize(400, 300);
    QVERIFY(nearRect(view.visibleArea(), area));

    // down to the smallest viewport
    for (int i = 0; i < 40; i++)
        sendWheel(view, cursor, 120);
    area = view.visibleArea();
    QVERIFY(near(area.height(), sls::SlsQtFastImage::kMinimumViewport));
    QVERIFY(near(area.width() / area.height(), plotShape));

    // and back out to the whole image
    for (int i = 0; i < 60; i++)
        sendWheel(view, cursor, -120);
    QVERIFY(nearRect(view.visibleArea(), QRectF(0, 0, n, n)));

    // not when zooming is disabled
    view.setZoomEnabled(false);
    sendWheel(view, cursor, 120);
    QVERIFY(nearRect(view.visibleArea(), QRectF(0, 0, n, n)));
}

void PlottingTest::imageTouchpadZoom() {
    sls::SlsQtFastImage view;
    view.resize(400, 300);
    view.show();
    const int n = 64;
    const std::vector<double> ramp = xRamp(n, n);
    view.setImage(ramp.data(), n, n);
    const QPointF cursor(200.0, 150.0);
    for (int i = 0; i < 4; i++)
        sendWheel(view, cursor, 120);
    const QRectF wheel = view.visibleArea();

    // many small steps add up to the same as the notches of a wheel
    view.resetView();
    for (int i = 0; i < 4 * 120 / 8; i++)
        sendWheel(view, cursor, 8);
    QVERIFY(near(view.visibleArea().width(), wheel.width()));
    QVERIFY(near(view.visibleArea().height(), wheel.height()));

    // a pinch magnifies by 1 + value
    view.resetView();
    for (int i = 0; i < 4; i++) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        QNativeGestureEvent pinch(Qt::ZoomNativeGesture,
                                  QPointingDevice::primaryPointingDevice(), 2,
                                  cursor, cursor, cursor, 0.18, QPointF());
#else
        QNativeGestureEvent pinch(Qt::ZoomNativeGesture, cursor, cursor, cursor,
                                  0.18, 0, 0);
#endif
        QCoreApplication::sendEvent(&view, &pinch);
    }
    QVERIFY(nearRect(view.visibleArea(), wheel));
}

void PlottingTest::imagePan() {
    sls::SlsQtFastImage view;
    view.resize(400, 300);
    view.show();
    const int n = 64;
    const std::vector<double> ramp = xRamp(n, n);
    view.setImage(ramp.data(), n, n);
    for (int i = 0; i < 6; i++)
        sendWheel(view, QPointF(200.0, 150.0), 120);
    const QRectF before = view.visibleArea();
    QSignalSpy spy(&view, &sls::SlsQtFastImage::visibleAreaChanged);

    // the grabbed point follows the cursor
    const QPointF from(200.0, 150.0), to(230.0, 130.0);
    const QPointF grabbed = view.widgetToData(from);
    sendMouse(view, QEvent::MouseButtonPress, from, Qt::LeftButton,
              Qt::LeftButton);
    sendMouse(view, QEvent::MouseMove, to, Qt::NoButton, Qt::LeftButton);
    sendMouse(view, QEvent::MouseButtonRelease, to, Qt::LeftButton,
              Qt::NoButton);
    const QPointF dropped = view.widgetToData(to);
    QVERIFY(near(grabbed.x(), dropped.x()) && near(grabbed.y(), dropped.y()));
    QCOMPARE(spy.count(), 1);
    const QRectF after = view.visibleArea();
    QVERIFY(after.x() < before.x() && after.y() < before.y());
    QVERIFY(near(after.width(), before.width()));
    QVERIFY(near(after.height(), before.height()));

    // not out of the image
    sendMouse(view, QEvent::MouseButtonPress, from, Qt::LeftButton,
              Qt::LeftButton);
    sendMouse(view, QEvent::MouseMove, from + QPointF(5000.0, -5000.0),
              Qt::NoButton, Qt::LeftButton);
    sendMouse(view, QEvent::MouseButtonRelease, from, Qt::LeftButton,
              Qt::NoButton);
    QVERIFY(near(view.visibleArea().x(), 0.0));
    QVERIFY(near(view.visibleArea().y(), 0.0));

    // a double click shows the whole image
    sendMouse(view, QEvent::MouseButtonDblClick, to, Qt::LeftButton,
              Qt::LeftButton);
    QVERIFY(nearRect(view.visibleArea(), QRectF(0, 0, n, n)));
}

// ---------------------------------------------------------------------------
// SlsQtFastImage: painting
// ---------------------------------------------------------------------------
void PlottingTest::imageIsClippedToPlotArea() {
    sls::SlsQtFastImage view;
    view.resize(400, 300);
    view.show();
    const std::vector<double> frame = testFrame();
    view.setColorRange(-2.0, 2.0);
    view.setImage(frame.data(), 1024, 512);

    // zoomed in, the scaled image is much larger than the plot area
    view.setViewport(100.0, 100.0, 164.0, 132.0);
    const QImage img = render(view);
    const QRect plot = plotArea(view);
    QCOMPARE(plot, QRect(52, 6, 400 - 58, 300 - 32));
    QVERIFY(img.pixel(plot.center()) != kBackground);
    QCOMPARE(countOutside(img, plot, [](QRgb c) { return c != kBackground; }),
             0);
}

void PlottingTest::imageOverlays() {
    sls::SlsQtFastImage view;
    view.resize(400, 300);
    view.show();
    const std::vector<double> frame = testFrame();
    view.setColorRange(-2.0, 2.0);
    view.setImage(frame.data(), 1024, 512);
    const QImage plain = render(view);

    // lower left corner (120, 110), 20 x 10 pixels
    view.setOverlayRects({QRectF(120.0, 110.0, 20.0, 10.0)});
    QCOMPARE(int(view.overlayRects().size()), 1);
    QImage img = render(view);
    QVERIFY(yellowNear(img, widgetPos(view, 120.0, 120.0).toPoint()));
    QVERIFY(yellowNear(img, widgetPos(view, 140.0, 110.0).toPoint()));
    QVERIFY(!yellowNear(img, widgetPos(view, 130.0, 115.0).toPoint(), 0));

    // sticking out of the image: clipped
    view.setOverlayRects({QRectF(1000.0, 400.0, 400.0, 400.0)});
    img = render(view);
    QVERIFY(countPixels(img, isYellow) > 0);
    QCOMPARE(countOutside(img, plotArea(view), isYellow), 0);
    view.setOverlayRects({});
    QCOMPARE(render(view), plain);

    // from corner to corner and beyond
    view.setOverlayLines({QLineF(0.0, 0.0, 2048.0, 1024.0)});
    img = render(view);
    QVERIFY(isBright(img.pixel(widgetPos(view, 512.0, 256.0).toPoint())));
    const QRect changed = differingArea(plain, img);
    QVERIFY(changed.width() > 300 && plotArea(view).contains(changed));
    view.setOverlayLines({});
    QCOMPARE(render(view), plain);
}

void PlottingTest::imageInterpolation() {
    sls::SlsQtFastImage view;
    view.resize(400, 300);
    view.show();
    // one bright pixel, shown at about 8 widget pixels per image pixel
    std::vector<double> f(32 * 32, 0.0);
    f[16 * 32 + 16] = 1.0;
    view.setPalette(Palette::Gray);
    view.setColorRange(0.0, 1.0);
    view.setImage(f.data(), 32, 32);
    const QPoint neighbour = widgetPos(view, 15.9, 16.5).toPoint();

    QCOMPARE(render(view).pixel(neighbour), qRgb(0, 0, 0));
    view.setInterpolate(true);
    QVERIFY(qGray(render(view).pixel(neighbour)) > 0);

    // not once the pixels are outlined
    view.setViewport(12.0, 12.0, 20.0, 20.0);
    QCOMPARE(dominantPixelAtData(view, render(view), 15.5, 16.5),
             qRgb(0, 0, 0));
}

void PlottingTest::imageTitles() {
    sls::SlsQtFastImage view;
    view.resize(400, 300);
    view.show();
    const std::vector<double> ramp = xRamp(64, 32);
    view.setImage(ramp.data(), 64, 32);
    const QPointF before = widgetPos(view, 0.0, 32.0);

    view.setTitle(QStringLiteral("Detector 0"));
    view.setXAxisTitle(QStringLiteral("x [px]"));
    view.setYAxisTitle(QStringLiteral("y [px]"));
    QCOMPARE(view.title(), QStringLiteral("Detector 0"));
    QCOMPARE(view.xAxisTitle(), QStringLiteral("x [px]"));
    QCOMPARE(view.yAxisTitle(), QStringLiteral("y [px]"));

    // the titles take their space from the image
    const QPointF after = widgetPos(view, 0.0, 32.0);
    QVERIFY(after.x() > before.x() && after.y() > before.y());
    render(view);

    // nothing but the image without axes
    view.setTitle(QString());
    view.setXAxisTitle(QString());
    view.setYAxisTitle(QString());
    view.setAxesVisible(false);
    QVERIFY(!view.axesVisible());
    QCOMPARE(widgetPos(view, 0.0, 0.0).x(), 0.0);
}

void PlottingTest::imagePixelLabels() {
    sls::SlsQtFastImage view;
    view.resize(400, 300);
    view.show();
    view.setPalette(Palette::Gray);
    view.setColorRange(0.0, 1e9);
    std::vector<double> f(64 * 64, 0.0);
    view.setImage(f.data(), 64, 64);

    // what is drawn on the black pixels of the lower left size x size pixels,
    // without the frame of the plot area around them
    int outlines = 0, labels = 0;
    const auto show = [&](double size) {
        view.setViewport(0.0, 0.0, size, size);
        const QRectF shown(widgetPos(view, 0.0, size),
                           widgetPos(view, size, 0.0));
        const QImage img =
            render(view).copy(shown.toRect().adjusted(2, 2, -2, -2));
        outlines =
            countPixels(img, [](QRgb c) { return c == qRgb(55, 55, 55); });
        labels = countPixels(img, isBright);
    };

    // the plot area is 268 pixels high
    show(20); // 13 widget pixels per pixel
    QCOMPARE(outlines, 0);
    QCOMPARE(labels, 0);
    show(12); // 22
    QVERIFY(outlines > 0);
    QCOMPARE(labels, 0);
    show(8); // 33
    QVERIFY(outlines > 0);
    QVERIFY(labels > 0);

    // longer labels need larger pixels: 1.2346e+06. The sizes leave room for
    // fonts of different width.
    std::fill(f.begin(), f.end(), 1234567.5);
    view.setImage(f.data(), 64, 64);
    show(8);
    QCOMPARE(labels, 0);
    show(3); // 89
    QVERIFY(labels > 0);

    // integers are written out: 1234567
    std::fill(f.begin(), f.end(), 1234567.0);
    view.setImage(f.data(), 64, 64);
    show(4); // 67
    QVERIFY(labels > 0);
}

void PlottingTest::imageReadout() {
    sls::SlsQtFastImage view;
    view.resize(400, 300);
    view.show();
    const std::vector<double> ramp = xRamp(64, 64);
    view.setImage(ramp.data(), 64, 64);
    const QPointF center(200.0, 150.0);
    for (int i = 0; i < 4; i++)
        sendWheel(view, center, 120); // the image fills the plot area
    const QRect plot = plotArea(view);
    QEvent leave(QEvent::Leave);
    QCoreApplication::sendEvent(&view, &leave);
    const QImage plain = render(view);

    // in the upper right corner of the plot area
    sendMouse(view, QEvent::MouseMove, center, Qt::NoButton, Qt::NoButton);
    QRect box = differingArea(plain, render(view));
    QVERIFY(plot.contains(box));
    QVERIFY(box.right() > plot.right() - 10 && box.top() < plot.top() + 10);
    QVERIFY(box.width() > 50 && box.width() < 200 && box.height() < 30);

    // or in the left one if the cursor is there
    sendMouse(view, QEvent::MouseMove, plot.topRight() + QPointF(-20, 15),
              Qt::NoButton, Qt::NoButton);
    box = differingArea(plain, render(view));
    QVERIFY(plot.contains(box));
    QVERIFY(box.left() < plot.left() + 10 && box.top() < plot.top() + 10);

    // gone with the cursor
    QCoreApplication::sendEvent(&view, &leave);
    QCOMPARE(render(view), plain);
    sendMouse(view, QEvent::MouseMove, QPointF(5.0, 150.0), Qt::NoButton,
              Qt::NoButton);
    QCOMPARE(render(view), plain);
}

void PlottingTest::imageAxisValues() {
    sls::SlsQtFastImage view;
    view.resize(400, 300);
    view.show();
    const std::vector<double> ramp = xRamp(8, 8);
    view.setImage(ramp.data(), 8, 8);
    const QRect plot = plotArea(view);
    // is there a tick below the plot area at this image coordinate
    const auto tickAt = [&](double x) {
        const QImage img = render(view);
        const int wx = int(widgetPos(view, x, 0.0).x());
        for (int dx = -1; dx <= 1; dx++)
            if (img.pixel(wx + dx, plot.bottom() + 2) != kBackground)
                return true;
        return false;
    };

    // the pixel index, at the centre of the pixels
    QVERIFY(tickAt(0.5) && tickAt(3.5) && tickAt(7.5));
    QVERIFY(!tickAt(1.0) && !tickAt(4.0) && !tickAt(7.0));

    // or the values given for the edges of the image
    view.setAxisRange(0.0, 8.0, 0.0, 8.0);
    QVERIFY(tickAt(1.0) && tickAt(4.0) && tickAt(7.0));
    QVERIFY(!tickAt(0.5) && !tickAt(3.5) && !tickAt(7.5));
    view.setAxisRange(100.0, 500.0, 0.0, 8.0); // ticks every 50
    QVERIFY(tickAt(1.0) && tickAt(4.0) && tickAt(7.0));
    QVERIFY(!tickAt(0.5) && !tickAt(3.5) && !tickAt(7.5));

    // an empty range is ignored
    view.setAxisRange(5.0, 5.0, 0.0, 8.0);
    QVERIFY(tickAt(1.0) && !tickAt(0.5));
}

// ---------------------------------------------------------------------------
// SlsQtScaleHisto and SlsQtScalePanel
// ---------------------------------------------------------------------------
void PlottingTest::scaleHistoDrag() {
    sls::SlsQtScaleHisto histo;
    histo.resize(400, 38);
    histo.show();
    // colour range 10..20: the axis shows 5..25, markers at x = 100 and 300
    histo.setData(std::vector<int>(256, 5), 5.0, 25.0, 10.0, 20.0, false,
                  false);
    QSignalSpy spy(&histo, &sls::SlsQtScaleHisto::rangeChanged);
    const QImage before = render(histo);

    sendMouse(histo, QEvent::MouseButtonPress, QPointF(100, 19), Qt::LeftButton,
              Qt::LeftButton);
    QVERIFY(histo.isDragging());
    sendMouse(histo, QEvent::MouseMove, QPointF(160, 19), Qt::NoButton,
              Qt::LeftButton);

    // only a preview line moves and new data has to wait
    std::vector<int> other(256, 1);
    other[60] = 100;
    histo.setData(other, 0.0, 100.0, 50.0, 60.0, false, false);
    const QImage during = render(histo);
    QCOMPARE(spy.count(), 0);
    QVERIFY(during != before);
    for (const int x : {50, 100, 250, 300, 350})
        QCOMPARE(during.copy(x, 0, 1, 38), before.copy(x, 0, 1, 38));

    // x = 160 is at 40% of the axis 5..25
    sendMouse(histo, QEvent::MouseButtonRelease, QPointF(160, 19),
              Qt::LeftButton, Qt::NoButton);
    QVERIFY(!histo.isDragging());
    QCOMPARE(spy.count(), 1);
    QVERIFY(near(spy.at(0).at(0).toDouble(), 13.0));
    QVERIFY(near(spy.at(0).at(1).toDouble(), 20.0));

    // a click away from the markers does nothing
    sendMouse(histo, QEvent::MouseButtonPress, QPointF(200, 19), Qt::LeftButton,
              Qt::LeftButton);
    QVERIFY(!histo.isDragging());
    sendMouse(histo, QEvent::MouseButtonRelease, QPointF(200, 19),
              Qt::LeftButton, Qt::NoButton);
    QCOMPARE(spy.count(), 1);
}

void PlottingTest::scaleHistoLogAxis() {
    sls::SlsQtScaleHisto histo;
    histo.resize(400, 38);
    histo.show();
    // colour range 1..100: the axis shows 1e-1..1e3
    histo.setData(std::vector<int>(256, 5), -1.0, 3.0, 1.0, 100.0, true, true);
    QSignalSpy spy(&histo, &sls::SlsQtScaleHisto::rangeChanged);
    const auto drag = [&histo](double from, double to) {
        sendMouse(histo, QEvent::MouseButtonPress, QPointF(from, 19),
                  Qt::LeftButton, Qt::LeftButton);
        sendMouse(histo, QEvent::MouseMove, QPointF(to, 19), Qt::NoButton,
                  Qt::LeftButton);
        sendMouse(histo, QEvent::MouseButtonRelease, QPointF(to, 19),
                  Qt::LeftButton, Qt::NoButton);
    };

    // upper marker to the middle of the axis
    drag(300, 200);
    QCOMPARE(spy.count(), 1);
    QVERIFY(near(spy.at(0).at(0).toDouble(), 1.0));
    QVERIFY(near(spy.at(0).at(1).toDouble(), 10.0));

    // the lower marker cannot pass the upper one
    drag(100, 390);
    QCOMPARE(spy.count(), 2);
    QVERIFY(spy.at(1).at(0).toDouble() < 100.0);
    QVERIFY(near(spy.at(1).at(0).toDouble(), 100.0));
    QVERIFY(near(spy.at(1).at(1).toDouble(), 100.0));
}

void PlottingTest::scalePanel() {
    QWidget window;
    auto *view = new sls::SlsQtFastImage(&window);
    auto *panel = new sls::SlsQtScalePanel(view, &window);
    auto *layout = new QVBoxLayout(&window);
    layout->addWidget(view);
    layout->addWidget(panel);
    window.resize(500, 500);
    window.show();
    std::vector<double> ramp = xRamp(64, 32);
    std::fill_n(ramp.begin(), 64 * 8, 12.0); // not the same count per bin
    view->setColorRange(10.0, 20.0);
    view->setImage(ramp.data(), 64, 32);
    render(window);

    // in the order they are created: palette, image lin/log, histogram lin/log
    const QList<QComboBox *> combos = panel->findChildren<QComboBox *>();
    QCOMPARE(combos.size(), 3);
    QCOMPARE(combos[0]->count(), sls::plot::paletteCount());

    // the controls show the view
    view->setPalette(Palette::Viridis);
    view->setLogScale(true);
    QCOMPARE(combos[0]->currentIndex(), int(Palette::Viridis));
    QCOMPARE(combos[1]->currentIndex(), 1);

    // and change it
    combos[0]->setCurrentIndex(int(Palette::Gray));
    combos[1]->setCurrentIndex(0);
    QCOMPARE(view->palette(), Palette::Gray);
    QVERIFY(!view->logScale());

    const QImage linear = render(*panel->histogramWidget());
    combos[2]->setCurrentIndex(1);
    QVERIFY(render(*panel->histogramWidget()) != linear);

    // dragging the lower marker from 25% to 40% of the strip
    QSignalSpy spy(panel, &sls::SlsQtScalePanel::colorRangeEdited);
    sls::SlsQtScaleHisto *histo = panel->histogramWidget();
    const QPointF from(0.25 * histo->width(), 19), to(0.4 * histo->width(), 19);
    sendMouse(*histo, QEvent::MouseButtonPress, from, Qt::LeftButton,
              Qt::LeftButton);
    sendMouse(*histo, QEvent::MouseMove, to, Qt::NoButton, Qt::LeftButton);
    sendMouse(*histo, QEvent::MouseButtonRelease, to, Qt::LeftButton,
              Qt::NoButton);
    double lo = 0.0, hi = 0.0;
    view->colorRange(lo, hi);
    QVERIFY(near(lo, 13.0) && near(hi, 20.0));
    QCOMPARE(spy.count(), 1);

    // a range set by the program is not an edit
    view->setColorRange(0.0, 5.0);
    QCOMPARE(spy.count(), 1);
}

// ---------------------------------------------------------------------------
// 1D: SlsQtH1D data container
// ---------------------------------------------------------------------------
void PlottingTest::h1dDataApi() {
    std::vector<double> data(10);
    for (int i = 0; i < 10; i++)
        data[i] = double(i + 1);
    sls::SlsQtH1D h(QStringLiteral("hist"), 10, 0.0, 10.0, data.data());
    QCOMPARE(h.GetNBinsX(), 10);
    QCOMPARE(h.title(), QStringLiteral("hist"));
    QCOMPARE(h.GetXMin(), 0.0);
    // Bin width is (xmax - xmin) / n, so for n=10 the last bin starts at 9.
    QCOMPARE(h.GetXMax(), 9.0);
    QCOMPARE(h.GetYMin(), 1.0);
    QCOMPARE(h.GetYMax(), 10.0);
    QCOMPARE(h.GetFirstXgtZero(), 1.0);
    QCOMPARE(h.GetFirstYgtZero(), 1.0);

    QCOMPARE(h.FillBin(0, 5.0), 6.0);
    QCOMPARE(h.GetY()[0], 6.0);
    QCOMPARE(h.Fill(5.0, 2.0), 8.0); // y[5] was 6, FindBinIndex(5) == 5
    QCOMPARE(h.SetContent(0.0, 42.0), 42.0);
    QCOMPARE(h.SetBinContent(9, 100.0), 100.0);
    QCOMPARE(h.GetYMax(), 100.0);
    h.Add(1.0);
    QCOMPARE(h.GetY()[9], 101.0);

    // Irregular x arrays: decreasing input is stored increasing.
    double xs[3] = {2.0, 1.0, 0.0};
    double ys[3] = {7.0, 8.0, 9.0};
    h.SetData(3, xs, ys);
    QCOMPARE(h.GetNBinsX(), 3);
    QCOMPARE(h.GetX()[0], 0.0);
    QCOMPARE(h.GetX()[2], 2.0);
    QCOMPARE(h.GetY()[0], 9.0);
    QCOMPARE(h.GetY()[2], 7.0);

    // Curve style API used by qDrawPlot.
    QCOMPARE(h.SetLineWidth(2), 2);
    h.SetLineStyle(1);
    QCOMPARE(h.pen().style(), Qt::DashLine);
    QCOMPARE(h.SetLineColor(1), 1);
    QCOMPARE(h.pen().color(), QColor(Qt::red));
    h.setStyleLinesorDots(false);
    QVERIFY(!h.isLines());
    h.setSymbolMarkers(true);
    QVERIFY(h.isMarkers());
    QVERIFY(h.legendVisible());
    h.setLegendVisible(false);
    QVERIFY(!h.legendVisible());
}

// ---------------------------------------------------------------------------
// 1D: auto range / explicit range / zoom / log axes / gain mode / markers
// ---------------------------------------------------------------------------
void PlottingTest::oneDimensionalRangeZoomLogAndGain() {
    sls::SlsQt1DPlot plot;
    plot.resize(600, 400);
    plot.show();
    QTest::qWait(5);
    QVERIFY(!plot.isGainPlot());
    QVERIFY(!plot.isZoomDisabled());

    std::vector<double> y(101);
    for (int i = 0; i < 101; i++)
        y[i] = 200.0 + 100.0 * std::sin(i / 10.0);
    sls::SlsQtH1D curve(QStringLiteral("curve"), 101, 0.0, 101.0, y.data());
    curve.SetLineColor(1);
    curve.Attach(&plot);
    plot.SetTitle(QStringLiteral("1D plot"));
    plot.SetXTitle(QStringLiteral("x"));
    plot.SetYTitle(QStringLiteral("counts"));
    plot.Update();

    // Auto range covers the data with the documented 2 % padding.
    // viewRect() is QRectF(xLo, yLo, width, height): top() is the lower data
    // bound and bottom() the upper one.
    const QRectF base = plot.viewRect();
    QVERIFY(base.left() <= 0.0);
    QVERIFY(base.right() >= 100.0);
    QVERIFY(base.top() <= curve.GetYMin());
    QVERIFY(base.bottom() >= curve.GetYMax());

    QImage img = render(plot);
    QVERIFY2(countPixels(img, [](QRgb c) { return c != kBackground; }) > 500,
             "1D plot must paint frame, grid, labels and curve");

    // Explicit ranges.
    plot.SetXMinMax(20.0, 40.0);
    QRectF v = plot.viewRect();
    QVERIFY(near(v.left(), 20.0) && near(v.width(), 20.0));
    plot.SetYMinMax(0.0, 100.0);
    v = plot.viewRect();
    QVERIFY(near(v.top(), 0.0) && near(v.bottom(), 100.0));
    plot.SetZoom(10.0, 50.0, 20.0, 100.0);
    v = plot.viewRect();
    QVERIFY(near(v.left(), 10.0) && near(v.top(), 50.0));
    QVERIFY(near(v.width(), 20.0) && near(v.height(), 100.0));

    // UnZoom restores the auto range.
    plot.UnZoom();
    v = plot.viewRect();
    QVERIFY(near(v.left(), base.left()) && near(v.width(), base.width()));
    QVERIFY(near(v.top(), base.top()) && near(v.height(), base.height()));

    // Wheel zoom shrinks the view and reports the viewport.
    QSignalSpy spy(&plot, &sls::SlsQt1DPlot::PlotZoomedSignal);
    const double w0 = plot.viewRect().width();
    sendWheel(plot, QPointF(300.0, 200.0), 120);
    QVERIFY(plot.viewRect().width() < w0);
    QVERIFY(spy.count() >= 1);
    QVERIFY(spy.takeLast().at(0).toRectF().width() > 0.0);

    // DisableZoom freezes the view.
    plot.DisableZoom(true);
    QVERIFY(plot.isZoomDisabled());
    const QRectF frozen = plot.viewRect();
    sendWheel(plot, QPointF(300.0, 200.0), 120);
    QVERIFY(near(plot.viewRect().width(), frozen.width()));
    plot.DisableZoom(false);
    QVERIFY(!plot.isZoomDisabled());

    // Log axes keep the ranges positive.
    plot.SetXMinMax(1.0, 100.0);
    plot.SetYMinMax(1.0, 100.0);
    plot.SetLogX(true);
    plot.SetLogY(true);
    QRectF lv = plot.viewRect();
    QVERIFY(lv.left() > 0.0 && lv.top() > 0.0);
    QVERIFY(lv.right() > lv.left() * 50.0);
    QVERIFY(lv.bottom() > lv.top() * 50.0);
    (void)render(plot); // must not crash in log mode
    plot.SetLogX(false);
    plot.SetLogY(false);

    // SetZoomX is the gain-plot x synchronisation path.
    plot.SetZoomX(QRectF(5.0, 0.0, 10.0, 1.0));
    v = plot.viewRect();
    QVERIFY(near(v.left(), 5.0) && near(v.width(), 10.0));
    plot.EnableXAutoScaling();
    QVERIFY(plot.viewRect().width() > 15.0);

    // G/H marker lines and SetYStep.
    plot.SetYMinMax(0.0, 300.0);
    plot.InsertHLine(150.0);
    plot.InsertVLine(50.0);
    plot.SetYStep(2);
    img = render(plot);
    QVERIFY2(countPixels(img, isYellow) > 20,
             "H/V marker lines are painted in the plot");
    plot.RemoveHLine();
    plot.RemoveVLine();
    plot.EnableYAutoScaling();

    // On a log-y axis the auto-range padding is applied in
    // log10 space (2 % of the decade span on each side), not to the linear
    // value, so the top point is not glued to the frame.
    plot.SetLogY(true);
    const QRectF logBase = plot.viewRect();
    const double logYMin = std::log10(curve.GetYMin());
    const double logYMax = std::log10(curve.GetYMax());
    const double padY = 0.02 * (logYMax - logYMin);
    QVERIFY(near(std::log10(logBase.top()), logYMin - padY, 1e-9));
    QVERIFY(near(std::log10(logBase.bottom()), logYMax + padY, 1e-9));
    plot.SetLogY(false);

    // Gain mode: fixed 0..3 y range, no zoom, log ignored.
    sls::SlsQt1DPlot gain(nullptr, true);
    gain.resize(400, 120);
    gain.show();
    QTest::qWait(5);
    QVERIFY(gain.isGainPlot());
    QVERIFY(gain.isZoomDisabled());
    std::vector<double> g(64, 1.0);
    g[10] = 2.0;
    g[30] = 3.0;
    sls::SlsQtH1D gcurve(QStringLiteral("gain"), 64, 0.0, 64.0, g.data());
    gcurve.Attach(&gain);
    gain.Update();
    QRectF gv = gain.viewRect();
    QVERIFY(near(gv.top(), 0.0) && near(gv.bottom(), 3.0));
    gain.SetLogY(true); // ignored for gain plots
    QVERIFY(near(gain.viewRect().top(), 0.0));
    QVERIFY(near(gain.viewRect().bottom(), 3.0));
    gain.SetYMinMax(0.0, 100.0); // ignored for gain plots
    QVERIFY(near(gain.viewRect().bottom(), 3.0));
    const QImage gshort = render(gain);
    QVERIFY(countPixels(gshort, [](QRgb c) { return c != kBackground; }) > 100);
    // The topmost gain tick label ("3") must be fully inside the
    // widget in both a short and a tall gain plot.  The top-left label area
    // (x < plot.left) contains no frame/grid pixels, so it isolates the label.
    auto topLabelVisible = [](const QImage &img) {
        int count = 0, maxY = -1;
        for (int y = 0; y < 18; y++) {
            for (int x = 0; x < 28; x++) {
                if (!isBright(img.pixel(x, y)))
                    continue;
                count++;
                maxY = std::max(maxY, y);
            }
        }
        return count > 5 && maxY >= 8;
    };
    QVERIFY2(topLabelVisible(gshort),
             "gain top tick label (3) visible at the widget top (O2)");
    gain.resize(400, 340);
    QTest::qWait(5);
    const QImage gtall = render(gain);
    QVERIFY2(topLabelVisible(gtall),
             "gain top tick label (3) visible in a tall gain plot (O2)");
}

void PlottingTest::oneDimensionalRoiBoxes() {
    sls::SlsQt1DPlot plot;
    plot.resize(600, 400);
    plot.show();
    QTest::qWait(5);
    std::vector<double> y(100);
    for (int i = 0; i < 100; i++)
        y[i] = double(i);
    sls::SlsQtH1D curve(QStringLiteral("curve"), 100, 0.0, 100.0, y.data());
    curve.SetLineColor(1);
    curve.Attach(&plot);
    plot.Update();

    std::vector<slsDetectorDefs::ROI> rois{slsDetectorDefs::ROI(10, 30),
                                           slsDetectorDefs::ROI(50, 70)};
    plot.EnableRoiBoxes(rois, 0, 100);
    QVERIFY(countPixels(render(plot), isYellow) > 100);

    // ROI boxes live in data coordinates and are clipped to the view: a view
    // that does not contain them paints no yellow at all.
    plot.SetXMinMax(0.0, 5.0);
    QCOMPARE(countPixels(render(plot), isYellow), 0);
    plot.EnableXAutoScaling();
    QVERIFY(countPixels(render(plot), isYellow) > 100);

    plot.DisableRoiBoxes();
    QCOMPARE(countPixels(render(plot), isYellow), 0);
}

// ---------------------------------------------------------------------------
// 2D: SlsQt2DHist data container
// ---------------------------------------------------------------------------
void PlottingTest::twoDHistogramContainer() {
    sls::SlsQt2DHist hist(4, 0.0, 4.0, 3, 0.0, 3.0);
    QCOMPARE(hist.GetNBinsX(), 4);
    QCOMPARE(hist.GetNBinsY(), 3);
    QCOMPARE(hist.GetXBinWidth(), 1.0);
    QCOMPARE(hist.GetYBinWidth(), 1.0);
    QCOMPARE(hist.GetXMin(), 0.0);
    QCOMPARE(hist.GetXMax(), 4.0);

    std::vector<double> d(12);
    for (int by = 0; by < 3; by++)
        for (int bx = 0; bx < 4; bx++)
            d[size_t(bx) + size_t(by) * 4] = double(bx + by * 4);
    hist.SetData(4, 0.0, 4.0, 3, 0.0, 3.0, d.data());
    QCOMPARE(hist.GetBinIndex(1, 1), 5);
    QCOMPARE(hist.GetBinValue(1, 1), 5.0);
    QCOMPARE(hist.FindBinIndex(0.5, 0.5), 0);
    QCOMPARE(hist.FindBinIndex(3.9, 2.9), 11);
    QCOMPARE(hist.GetBinIndex(-1, 0), 12); // under/overflow bin
    // Auto z range with 2 % padding.
    QCOMPARE(hist.GetMinimum(), 0.0);
    QVERIFY(near(hist.GetMaximum(), 11.0 * 1.02));
    QVERIFY(near(hist.GetMean(), 5.5));

    hist.SetMinMax(2.0, 5.0);
    QCOMPARE(hist.GetMinimum(), 2.0);
    QCOMPARE(hist.GetMaximum(), 5.0);
    QCOMPARE(hist.SetMinimumToFirstGreaterThanZero(), 1.0);
    QCOMPARE(hist.GetMinimum(), 1.0);

    // Sampling: box vs bilinear.
    QVERIFY(!hist.IsInterpolated());
    QCOMPARE(hist.value(1.5, 1.5), 5.0);
    QCOMPARE(hist.value(-5.0, -5.0), 0.0); // under/overflow bin
    hist.Interpolate(true);
    QVERIFY(hist.IsInterpolated());
    QCOMPARE(hist.value(1.5, 1.5), 5.0); // bin centre is exact
    QVERIFY(hist.value(1.0, 1.5) > 4.0 && hist.value(1.0, 1.5) < 6.0);

    // SetBinValue never touches the under/overflow bin.
    hist.SetBinValue(2, 1, 42.0);
    QCOMPARE(hist.GetBinValue(2, 1), 42.0);
    hist.SetBinValue(-1, 0, 99.0);
    QCOMPARE(hist.GetBinValue(-1, 0), 0.0);

    // A null buffer with the same geometry keeps the previous frame (the
    // QImage contract only documents zeroing when the buffer is reallocated);
    // a *different* geometry reallocates and zeroes it.  Neutral accessors.
    hist.SetData(4, 0.0, 4.0, 3, 0.0, 3.0, nullptr);
    QCOMPARE(hist.GetBinValue(2, 1), 42.0);
    hist.SetData(5, 0.0, 5.0, 2, 0.0, 2.0, nullptr);
    QCOMPARE(hist.GetNBinsX(), 5);
    QCOMPARE(hist.GetNBinsY(), 2);
    for (int by = 0; by < 2; by++)
        for (int bx = 0; bx < 5; bx++)
            QCOMPARE(hist.GetBinValue(bx, by), 0.0);
    hist.SetData(0, 0.0, 4.0, 3, 0.0, 3.0, nullptr);
    QCOMPARE(hist.GetNBinsX(), 5); // invalid geometry ignored

    // NaN/Inf bins are skipped by the automatic z range/mean.
    std::vector<double> nf(12);
    for (int i = 0; i < 12; i++)
        nf[size_t(i)] = double(i + 1);                // 1..12
    nf[0] = std::numeric_limits<double>::quiet_NaN(); // 1
    nf[5] = std::numeric_limits<double>::infinity();  // 6
    nf[7] = -std::numeric_limits<double>::infinity(); // 8
    hist.SetData(4, 0.0, 4.0, 3, 0.0, 3.0, nf.data());
    QVERIFY(std::isfinite(hist.GetMinimum()));
    QVERIFY(std::isfinite(hist.GetMaximum()));
    QVERIFY(std::isfinite(hist.GetMean()));
    QVERIFY(near(hist.GetMinimum(), 2.0 / 1.02)); // smallest finite bin
    QVERIFY(near(hist.GetMaximum(), 12.0 * 1.02));
    QVERIFY(near(hist.GetMean(), 7.0)); // (sum finite 63) / 9

    // No finite bin at all: a harmless 0..1 range and a zero mean.
    std::vector<double> allnf(12, std::numeric_limits<double>::quiet_NaN());
    hist.SetData(4, 0.0, 4.0, 3, 0.0, 3.0, allnf.data());
    QCOMPARE(hist.GetMinimum(), 0.0);
    QCOMPARE(hist.GetMaximum(), 1.0);
    QCOMPARE(hist.GetMean(), 0.0);

    // All-negative data is padded OUTWARD on both ends; the
    // high end must stay above the data maximum (the brightest bin must not be
    // clipped - multiplying a negative max by 1.02 would move it below).
    std::vector<double> neg(12);
    for (int i = 0; i < 12; i++)
        neg[size_t(i)] = (i % 2) ? -10.0 : -20.0;
    hist.SetData(4, 0.0, 4.0, 3, 0.0, 3.0, neg.data());
    QVERIFY(hist.GetMinimum() < -20.0);
    QVERIFY(near(hist.GetMinimum(), -20.4));
    QVERIFY(hist.GetMaximum() > -10.0);
    QVERIFY(near(hist.GetMaximum(), -9.8));
}

// ---------------------------------------------------------------------------
// SlsQt2DPlot
// ---------------------------------------------------------------------------
void PlottingTest::plot2dTitles() {
    sls::SlsQt2DPlot plot;
    plot.resize(700, 600);
    plot.show();

    // a test image is shown until the first data arrives
    QCOMPARE(plot.GetXMinimum(), 200.0);
    QCOMPARE(plot.GetXMaximum(), 822.0);
    QCOMPARE(plot.GetYMinimum(), -0.5);
    QCOMPARE(plot.GetYMaximum(), 49.5);
    QCOMPARE(plot.imageView()->imageWidth(), 50);
    QCOMPARE(plot.imageView()->imageHeight(), 50);

    plot.SetTitle(QStringLiteral("2D detector"));
    plot.SetXTitle(QStringLiteral("x"));
    plot.SetYTitle(QStringLiteral("y"));
    QCOMPARE(plot.title(), QStringLiteral("2D detector"));
    QCOMPARE(plot.imageView()->xAxisTitle(), QStringLiteral("x"));
    QCOMPARE(plot.imageView()->yAxisTitle(), QStringLiteral("y"));

    // the z title has its own row above the scale panel
    const QList<QLabel *> labels =
        plot.findChildren<QLabel *>(QString(), Qt::FindDirectChildrenOnly);
    QCOMPARE(labels.size(), 1);
    QVERIFY(!labels[0]->isVisible());
    plot.SetZTitle(QStringLiteral("counts"));
    QVERIFY(labels[0]->isVisible());
    QCOMPARE(labels[0]->text(), QStringLiteral("counts"));
    plot.SetZTitle(QString());
    QVERIFY(!labels[0]->isVisible());
}

void PlottingTest::plot2dZRange() {
    sls::SlsQt2DPlot plot;
    plot.resize(700, 600);
    plot.show();
    sls::SlsQtFastImage *view = plot.imageView();
    std::vector<double> d(64 * 32);
    for (size_t i = 0; i < d.size(); i++)
        d[i] = double(i % 12);
    double lo = 0.0, hi = 0.0;

    // follows the data
    plot.SetData(64, 0.0, 64.0, 32, 0.0, 32.0, d.data());
    view->colorRange(lo, hi);
    QVERIFY(lo <= 0.0 && lo > -1.0 && hi >= 11.0 && hi < 12.0);

    // fixed ends stay for the following frames
    plot.SetZRange(true, true, 2.0, 8.0);
    plot.SetData(64, 0.0, 64.0, 32, 0.0, 32.0, d.data());
    view->colorRange(lo, hi);
    QVERIFY(near(lo, 2.0) && near(hi, 8.0));

    // one end fixed, the other from the data
    plot.SetZRange(true, false, 2.0, 0.0);
    view->colorRange(lo, hi);
    QVERIFY(near(lo, 2.0) && hi >= 11.0 && hi < 12.0);

    plot.SetZRange(false, false, 0.0, 0.0);
    view->colorRange(lo, hi);
    QVERIFY(lo <= 0.0 && hi >= 11.0);

    // a range picked on the scale panel stays as well, qDrawPlot calls
    // SetZRange() without fixed ends for every frame
    sls::SlsQtScaleHisto *histo = plot.scalePanel()->histogramWidget();
    const QPointF from(0.75 * histo->width(), 19), to(0.5 * histo->width(), 19);
    sendMouse(*histo, QEvent::MouseButtonPress, from, Qt::LeftButton,
              Qt::LeftButton);
    sendMouse(*histo, QEvent::MouseMove, to, Qt::NoButton, Qt::LeftButton);
    sendMouse(*histo, QEvent::MouseButtonRelease, to, Qt::LeftButton,
              Qt::NoButton);
    double pickedLo = 0.0, pickedHi = 0.0;
    view->colorRange(pickedLo, pickedHi);
    QVERIFY(near(pickedLo, lo) && pickedHi < hi - 1.0);
    plot.SetData(64, 0.0, 64.0, 32, 0.0, 32.0, d.data());
    plot.SetZRange(false, false, 0.0, 0.0);
    view->colorRange(lo, hi);
    QVERIFY(near(lo, pickedLo) && near(hi, pickedHi));

    // until an end is fixed
    plot.SetZRange(false, true, 0.0, 5.0);
    view->colorRange(lo, hi);
    QVERIFY(lo <= 0.0 && near(hi, 5.0));
    plot.SetZRange(false, false, 0.0, 0.0);
    view->colorRange(lo, hi);
    QVERIFY(lo <= 0.0 && hi >= 11.0);

    // or the image size changes
    sendMouse(*histo, QEvent::MouseButtonPress, from, Qt::LeftButton,
              Qt::LeftButton);
    sendMouse(*histo, QEvent::MouseButtonRelease, to, Qt::LeftButton,
              Qt::NoButton);
    plot.SetData(32, 0.0, 32.0, 64, 0.0, 64.0, d.data());
    view->colorRange(lo, hi);
    QVERIFY(lo <= 0.0 && hi >= 11.0);
}

void PlottingTest::plot2dLogZ() {
    sls::SlsQt2DPlot plot;
    plot.resize(700, 600);
    plot.show();
    sls::SlsQtFastImage *view = plot.imageView();
    std::vector<double> d(64 * 32);
    for (size_t i = 0; i < d.size(); i++)
        d[i] = double(i % 12) * 0.5;
    plot.SetData(64, 0.0, 64.0, 32, 0.0, 32.0, d.data());
    double lo = 0.0, hi = 0.0;

    // starts at the smallest value above zero
    plot.SetLogz(true, false, false, 0.0, 0.0);
    QVERIFY(view->logScale());
    view->colorRange(lo, hi);
    QVERIFY(near(lo, 0.5) && hi >= 5.5);

    plot.SetLogz(true, true, true, 0.1, 9.0);
    view->colorRange(lo, hi);
    QVERIFY(near(lo, 0.1) && near(hi, 9.0));

    plot.SetLogz(false, false, false, 0.0, 0.0);
    QVERIFY(!view->logScale());

    // values that are not finite do not end up in the range
    d[0] = std::numeric_limits<double>::quiet_NaN();
    d[1] = std::numeric_limits<double>::infinity();
    d[2] = -std::numeric_limits<double>::infinity();
    plot.SetData(64, 0.0, 64.0, 32, 0.0, 32.0, d.data());
    view->colorRange(lo, hi);
    QVERIFY(lo <= 0.0 && lo > -1.0 && hi >= 5.5 && hi < 6.5);
    std::fill(d.begin(), d.end(), std::numeric_limits<double>::quiet_NaN());
    plot.SetData(64, 0.0, 64.0, 32, 0.0, 32.0, d.data());
    view->colorRange(lo, hi);
    QVERIFY(near(lo, 0.0) && near(hi, 1.0));
    render(plot);
}

void PlottingTest::plot2dContours() {
    sls::SlsQt2DPlot plot;
    plot.resize(800, 600);
    plot.show();
    sls::SlsQtFastImage *view = plot.imageView();
    const int n = 1024; // more pixels than the contour grid has cells
    std::vector<double> d(size_t(n) * n);
    for (int y = 0; y < n; y++)
        for (int x = 0; x < n; x++)
            d[size_t(y) * n + x] = double(x + y);
    plot.SetData(n, -0.5, n - 0.5, n, -0.5, n - 0.5, d.data());
    const QImage plain = render(*view);

    // the diagonal lines reach the right and the upper edge of the image
    plot.SetContour(true);
    const QRect changed = differingArea(plain, render(*view));
    QVERIFY(plotArea(*view).contains(changed));
    QVERIFY(changed.right() >= int(widgetPos(*view, n, 0).x()) - 2);
    QVERIFY(changed.top() <= int(widgetPos(*view, 0, n).y()) + 2);
    plot.SetContour(false);
    QCOMPARE(render(*view), plain);

    // the levels follow the colour range: x + y stays below 500
    plot.SetZRange(true, true, 0.0, 500.0);
    const QImage plainLow = render(*view);
    plot.SetContour(true);
    const QRect low = differingArea(plainLow, render(*view));
    QVERIFY(low.right() < int(widgetPos(*view, 500, 0).x()));
    QVERIFY(low.top() > int(widgetPos(*view, 0, 500).y()));
    plot.SetContour(false);

    plot.SetInterpolate(true);
    QVERIFY(view->interpolate());
    plot.SetInterpolate(false);
    QVERIFY(!view->interpolate());
}

void PlottingTest::plot2dRoiAndZoom() {
    sls::SlsQt2DPlot plot;
    plot.resize(700, 600);
    plot.show();
    sls::SlsQtFastImage *view = plot.imageView();
    const std::vector<double> d = xRamp(100, 80);
    // the data coordinates are the pixels shifted by 1000 and 2000
    plot.SetData(100, 1000.0, 1100.0, 80, 2000.0, 2080.0, d.data());

    // pixels 10..29 x 20..39
    plot.EnableRoiBoxes({slsDetectorDefs::ROI(10, 29, 20, 39)});
    QCOMPARE(int(view->overlayRects().size()), 1);
    QCOMPARE(view->overlayRects()[0], QRectF(10.0, 20.0, 20.0, 20.0));
    QVERIFY(yellowNear(render(*view), widgetPos(*view, 10.0, 20.0).toPoint()));
    plot.DisableRoiBoxes();
    QVERIFY(view->overlayRects().empty());

    plot.SetZoom(QRectF(1020.0, 2020.0, 20.0, 20.0));
    QVERIFY(nearRect(view->visibleArea(), QRectF(20, 20, 20, 20)));

    // the user zooming is reported in data coordinates
    QSignalSpy spy(&plot, &sls::SlsQt2DPlot::PlotZoomedSignal);
    sendWheel(*view, widgetPos(*view, 30.0, 30.0), 120);
    QCOMPARE(spy.count(), 1);
    const QRectF zoomed = spy.at(0).at(0).toRectF();
    QVERIFY(zoomed.height() < 20.0);
    QVERIFY(zoomed.contains(QPointF(1030.0, 2030.0)));
    QVERIFY(QRectF(1000.0, 2000.0, 100.0, 80.0).contains(zoomed));

    // axis ranges from the plot tab
    plot.SetXMinMax(1040.0, 1060.0);
    QVERIFY(nearRect(view->visibleArea(), QRectF(40, 0, 20, 80)));
    plot.SetYMinMax(2030.0, 2010.0);
    QVERIFY(nearRect(view->visibleArea(), QRectF(40, 10, 20, 20)));
    plot.EnableXAutoScaling();
    plot.EnableYAutoScaling();
    QVERIFY(nearRect(view->visibleArea(), QRectF(0, 0, 100, 80)));

    plot.DisableZoom(true);
    QVERIFY(!view->zoomEnabled());
    plot.DisableZoom(false);
    QVERIFY(view->zoomEnabled());
}

// qDrawPlot shows the gain plot next to the data plot while it holds a mutex,
// and its slot for PlotZoomedSignal takes the same mutex. Showing a widget
// resizes its neighbours on the spot, so the plot may not report its new
// visible area from within that resize.
void PlottingTest::plot2dResizeIsReportedLater() {
    QWidget window;
    auto *plot = new sls::SlsQt2DPlot(&window);
    auto *gain = new sls::SlsQt2DPlot(&window, true);
    gain->setFixedSize(180, 120);
    gain->hide();
    auto *layout = new QHBoxLayout(&window);
    layout->addWidget(plot);
    layout->addWidget(gain);
    window.resize(900, 600);
    window.show();
    for (int i = 0; i < 3; i++) // the visible area now depends on the size
        sendWheel(*plot->imageView(), QPointF(300.0, 250.0), 120);
    const QSize before = plot->size();
    const QRectF areaBefore = plot->imageView()->visibleArea();

    bool showing = false;
    int reportedWhileShowing = 0;
    QSignalSpy spy(plot, &sls::SlsQt2DPlot::PlotZoomedSignal);
    connect(plot, &sls::SlsQt2DPlot::PlotZoomedSignal, &window, [&] {
        if (showing)
            reportedWhileShowing++;
    });
    showing = true;
    gain->show();
    showing = false;
    QVERIFY(plot->width() < before.width()); // resized inside show()
    QVERIFY(plot->imageView()->visibleArea() != areaBefore);
    QCOMPARE(reportedWhileShowing, 0);

    // but once that is over, for the gain plot to follow
    QCoreApplication::processEvents();
    QCOMPARE(spy.count(), 1);

    // the gain plot is small, all of it has to be visible
    QVERIFY(gain->rect().contains(gain->imageView()->geometry()));
    if (!qEnvironmentVariableIsEmpty("PLOTTING_TEST_IMAGES"))
        window.grab().save(qEnvironmentVariable("PLOTTING_TEST_IMAGES") +
                           QStringLiteral("/gain-next-to-data.png"));
}

// The widgets around the plots use the plot colours, and every text colour is
// readable on the background it is drawn on.
void PlottingTest::darkPaletteGoesWithThePlots() {
    const sls::plot::PlotTheme theme;
    const QPalette p = sls::plot::darkPalette();
    QCOMPARE(p.color(QPalette::Base), theme.background);
    QCOMPARE(p.color(QPalette::Text), theme.text);
    QCOMPARE(p.color(QPalette::WindowText), theme.text);

    auto luminance = [](const QColor &c) {
        auto lin = [](double v) {
            return v <= 0.03928 ? v / 12.92
                                : std::pow((v + 0.055) / 1.055, 2.4);
        };
        return 0.2126 * lin(c.redF()) + 0.7152 * lin(c.greenF()) +
               0.0722 * lin(c.blueF());
    };
    auto contrast = [&](QPalette::ColorRole fg, QPalette::ColorRole bg) {
        const double a = luminance(p.color(fg));
        const double b = luminance(p.color(bg));
        return (std::max(a, b) + 0.05) / (std::min(a, b) + 0.05);
    };
    QVERIFY(contrast(QPalette::WindowText, QPalette::Window) > 7.0);
    QVERIFY(contrast(QPalette::Text, QPalette::Base) > 7.0);
    QVERIFY(contrast(QPalette::ButtonText, QPalette::Button) > 7.0);
    QVERIFY(contrast(QPalette::ToolTipText, QPalette::ToolTipBase) > 7.0);
    QVERIFY(contrast(QPalette::Link, QPalette::Window) > 4.5);
    QVERIFY(contrast(QPalette::Link, QPalette::Base) > 4.5);
    QVERIFY(contrast(QPalette::HighlightedText, QPalette::Highlight) > 4.5);

    // disabled text is dimmer, but still there
    QVERIFY(p.color(QPalette::Disabled, QPalette::WindowText) !=
            p.color(QPalette::Active, QPalette::WindowText));
    QVERIFY(luminance(p.color(QPalette::Disabled, QPalette::WindowText)) >
            2.0 * luminance(p.color(QPalette::Window)));
}

void PlottingTest::plot2dGain() {
    sls::SlsQt2DPlot gain(nullptr, true);
    gain.resize(200, 200);
    gain.show();
    sls::SlsQtFastImage *view = gain.imageView();
    QVERIFY(gain.scalePanel() == nullptr);
    QVERIFY(!view->axesVisible());
    QVERIFY(!view->zoomEnabled());
    QCOMPARE(view->palette(), Palette::Gain4);

    // one quadrant per gain: 0 1 above 2 3
    std::vector<double> d(8 * 8);
    for (int y = 0; y < 8; y++)
        for (int x = 0; x < 8; x++)
            d[size_t(y) * 8 + x] = (x < 4 ? 0 : 1) + (y < 4 ? 2 : 0);
    gain.SetData(8, 0.0, 8.0, 8, 0.0, 8.0, d.data());
    const QImage img = render(*view);
    const QRgb *lut = sls::plot::paletteLut(Palette::Gain4);
    QCOMPARE(dominantPixelAtData(*view, img, 2.0, 6.0), lut[0]);
    QCOMPARE(dominantPixelAtData(*view, img, 6.0, 6.0), lut[85]);
    QCOMPARE(dominantPixelAtData(*view, img, 2.0, 2.0), lut[170]);
    QCOMPARE(dominantPixelAtData(*view, img, 6.0, 2.0), lut[255]);

    // the range is fixed and the plot only follows the zoom of the data plot
    gain.SetZRange(true, true, 0.0, 100.0);
    gain.SetLogz(true, false, false, 0.0, 0.0);
    double lo = 0.0, hi = 0.0;
    view->colorRange(lo, hi);
    QVERIFY(near(lo, 0.0) && near(hi, 3.0));
    QVERIFY(!view->logScale());
    gain.SetZoom(QRectF(2.0, 2.0, 4.0, 4.0));
    QVERIFY(nearRect(view->visibleArea(), QRectF(2, 2, 4, 4)));
}

// ---------------------------------------------------------------------------
// the gui has to keep up with 1024 x 512 pixels at 20 Hz
// ---------------------------------------------------------------------------
void PlottingTest::performance1024x512() {
    sls::SlsQtFastImage view;
    view.resize(1100, 600);
    view.show();
    QVERIFY(QTest::qWaitForWindowExposed(&view)); // or repaint() does nothing
    std::vector<double> frame = testFrame();
    view.setColorRange(-2.0, 2.0);
    const auto showFrame = [&](int i) {
        frame[0] = i;
        view.setImage(frame.data(), 1024, 512);
        view.repaint();
    };
    for (int i = 0; i < 5; i++)
        showFrame(i);

    const int frames = 60;
    QElapsedTimer timer;
    timer.start();
    for (int i = 0; i < frames; i++)
        showFrame(i);
    const double seconds = timer.nsecsElapsed() / 1e9;
    qInfo("1024x512: %.1f fps (%.2f ms/frame)", frames / seconds,
          1000.0 * seconds / frames);
    QVERIFY(frames / seconds >= 20.0);
}

QTEST_MAIN(PlottingTest)
#include "plotting_test.moc"
