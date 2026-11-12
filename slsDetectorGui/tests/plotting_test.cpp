// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2026 Contributors to the SLS Detector Package
//
// QtTest suite for the Qt5/Qt6 plotting stack (verification task).
//
// Everything is checked through the public API or through rendered pixels; the
// widgets are shown on the offscreen QPA platform and rendered synchronously
// with QWidget::grab(), so the assertions exercise the same paint path the GUI
// uses.  The performance slot is the acceptance benchmark for 1024x512 frames.

#include "SlsQt1DPlot.h"
#include "SlsQt2DHist.h"
#include "SlsQt2DPlot.h"
#include "SlsQtFastImage.h"
#include "SlsQtPlotCommon.h"
#include "SlsQtScalePanel.h"
#include "sls/sls_detector_defs.h"

#include <QApplication>
#include <QCoreApplication>
#include <QElapsedTimer>
#include <QHash>
#include <QSet>
#include <QImage>
#include <QLabel>
#include <QMouseEvent>
#include <QSignalSpy>
#include <QTest>
#include <QWheelEvent>
#include <QWidget>

#include <algorithm>
#include <cmath>
#include <limits>
#include <utility>
#include <vector>

namespace {

constexpr QRgb kBackground = qRgb(28, 28, 28);

bool near(double a, double b, double eps = 1e-6) {
    return std::abs(a - b) <= eps;
}

// Repaint synchronously (runs renderImage + paintEvent) and grab the result.
QImage render(QWidget &w) {
    w.repaint();
    return w.grab().toImage();
}

QPointF widgetPos(const sls::SlsQtFastImage &view, double dataX,
                  double dataY) {
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
#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
    QWheelEvent e(pos, QPointF(w.mapToGlobal(pos.toPoint())), QPoint(),
                  QPoint(0, deltaY), Qt::NoButton, Qt::NoModifier,
                  Qt::NoScrollPhase, false);
#else
    // Qt 5.9 (RHEL 7): the scroll-phase constructor does not exist yet.
    QWheelEvent e(pos, QPointF(w.mapToGlobal(pos.toPoint())), QPoint(),
                  QPoint(0, deltaY), 0, Qt::Vertical, Qt::NoButton,
                  Qt::NoModifier);
#endif
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

// A 1024x512 acceptance frame: sin(x/40) + cos(y/30).
std::vector<float> acceptanceFrame(double phase = 0.0) {
    std::vector<float> f(size_t(1024) * 512);
    for (int y = 0; y < 512; y++) {
        for (int x = 0; x < 1024; x++) {
            f[size_t(y) * 1024 + x] =
                float(std::sin(x / 40.0 + phase) + std::cos(y / 30.0));
        }
    }
    return f;
}

} // namespace

class PlottingTest : public QObject {
    Q_OBJECT

  private slots:
    // ---- foundation: palettes / ticks / formatting ------------------------
    void palettesAndLuts();
    void paletteLogClamp();
    void tickMath();
    void tickFormatting();

    // ---- FastImageView ----------------------------------------------------
    void fastImageOwnsFrame();
    void fastImageYUp();
    void fastImageHistogramWindow();
    void fastImageHistogramLogWindow();
    void fastImageLogClamp();
    void fastImageZeroWhite();
    void fastImageZoomCursorInvariance();
    void fastImageViewportClamping();
    void fastImageClipsToPlotRect();
    void fastImageOverlaysInDataCoordinates();
    void fastImageTitlesAndAspect();

    // ---- ScaleHisto / ScalePanel -----------------------------------------
    void scaleHistoThreeWayFreeze();
    void scaleHistoMaxMarker();
    void scalePanelReentrancy();
    void scalePanelFreezeAndPushRules();
    void scalePanelDetachLifetime();

    // ---- 1D plot ----------------------------------------------------------
    void h1dDataApi();
    void oneDimensionalRangeZoomLogAndGain();
    void oneDimensionalRoiBoxes();

    // ---- 2D plot ----------------------------------------------------------
    void twoDHistogramContainer();
    void twoDimensionalDataZRangeAndPanel();
    void twoDimensionalLogInterpolateAndContour();
    void twoDimensionalRoiAndClipping();
    void twoDimensionalGainMode();

    // ---- acceptance benchmark --------------------------------------------
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
    QString gainName = QString::fromLatin1(
        sls::plot::paletteName(sls::plot::Palette::Gain4));
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
    QCOMPARE(sls::plot::niceStep(0.0, 8), 1.0);   // degenerate
    QCOMPARE(sls::plot::niceStep(-5.0, 8), 1.0);  // degenerate

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
    QCOMPARE(sls::plot::formatTick(
                 std::numeric_limits<double>::infinity(), 1.0),
             QStringLiteral("?"));
    // No trailing zeros anywhere, and labels stay short.
    for (double v : {0.0, 1.0, 2.0, 5.0, 10.0, 100.0, -100.0, 0.5, 2.5}) {
        const QString s = sls::plot::formatTick(v, 1.0);
        QVERIFY2(!s.contains(QStringLiteral(".0")), qPrintable(s));
        QVERIFY2(s.size() <= 6, qPrintable(s));
    }
}

// ---------------------------------------------------------------------------
// FastImageView: buffer ownership, orientation, histogram, colour mapping
// ---------------------------------------------------------------------------
void PlottingTest::fastImageOwnsFrame() {
    sls::SlsQtFastImage view;
    view.resize(200, 200); // sx ~ 35 px => no in-pixel text over cell centres
    view.show();
    QTest::qWait(5);

    view.setPaletteLut(sls::plot::paletteLut(sls::plot::Palette::Color));
    view.setColorRange(0.0, 2.0);
    std::vector<float> src(16, 1.0f);
    view.setImageCopy(src.data(), 4, 4);
    const QRgb expected = sls::plot::paletteLut(sls::plot::Palette::Color)[127];
    QCOMPARE(dominantPixelAtData(view, render(view), 0.5, 0.5), expected);

    // The producer owns and recycles its buffer: the view must have copied it.
    std::fill(src.begin(), src.end(), 2.0f);
    // Force a recomposition from the widget-owned data (no new frame pushed).
    view.setColorRange(0.0, 2.0);
    const QImage after = render(view);
    QCOMPARE(dominantPixelAtData(view, after, 0.5, 0.5), expected);

    // Same for the double overload (the GUI feeds double frames).
    std::vector<double> dsrc(16, 1.0);
    view.setImageCopy(dsrc.data(), 4, 4);
    QCOMPARE(dominantPixelAtData(view, render(view), 1.5, 1.5), expected);
    std::fill(dsrc.begin(), dsrc.end(), 2.0);
    view.setColorRange(0.0, 2.0);
    QCOMPARE(dominantPixelAtData(view, render(view), 1.5, 1.5), expected);

    // Invalid input is ignored, the previous frame stays valid.
    view.setImageCopy(static_cast<const float *>(nullptr), 4, 4);
    view.setImageCopy(src.data(), 0, 4);
    QCOMPARE(view.imageWidth(), 4);
    QCOMPARE(view.imageHeight(), 4);
}

void PlottingTest::fastImageYUp() {
    sls::SlsQtFastImage view;
    view.resize(200, 200);
    view.show();
    QTest::qWait(5);
    view.setPaletteLut(sls::plot::paletteLut(sls::plot::Palette::Gray));
    view.setColorRange(0.0, 4.0);
    // row 0 = bottom = 0, row 1 = top = 3
    std::vector<float> f{0, 0, 0, 0, 3, 3, 3, 3};
    view.setImageCopy(f.data(), 4, 2);
    const QImage img = render(view);

    // Data y is up: y=0.5 (bottom row) is drawn below y=1.5 (top row).
    QVERIFY(widgetPos(view, 0.5, 0.5).y() > widgetPos(view, 0.5, 1.5).y());
    const QRgb *lut = sls::plot::paletteLut(sls::plot::Palette::Gray);
    QCOMPARE(dominantPixelAtData(view, img, 0.5, 0.5), lut[0]);
    QCOMPARE(dominantPixelAtData(view, img, 0.5, 1.5),
             lut[int((3.0 - 0.0) * 255.0 / 4.0)]);
    // Exactly one flip: both quarters of the top row match, no mirrored mix.
    QCOMPARE(dominantPixelAtData(view, img, 2.5, 1.5),
             dominantPixelAtData(view, img, 0.5, 1.5));
}

void PlottingTest::fastImageHistogramWindow() {
    sls::SlsQtFastImage view;
    view.resize(200, 200);
    view.show();
    QTest::qWait(5);
    view.setLogScale(false);

    // 64x32 ramp, value == x: only x in [0,4] is inside the 1..3 window.
    const int nx = 64, ny = 32;
    std::vector<float> ramp(size_t(nx) * ny);
    for (int y = 0; y < ny; y++)
        for (int x = 0; x < nx; x++)
            ramp[size_t(y) * nx + x] = float(x);
    view.setColorRange(1.0, 3.0);
    view.setImageCopy(ramp.data(), nx, ny);
    render(view);

    double lo = 0.0, hi = 0.0;
    view.histogramRange(lo, hi);
    QVERIFY(near(lo, 0.0) && near(hi, 4.0));
    QCOMPARE(int(view.histogram().size()), 256);
    long long sum = 0;
    for (int v : view.histogram())
        sum += v;
    QCOMPARE(sum, 5LL * ny); // 0,1,2,3,4 in-window; 5..63 skipped

    // The histogram follows a colour-range change (invalidated, not stale).
    // Range 10.2..19.8 -> window 5.1..24.9 (not aligned with the integer ramp).
    view.setColorRange(10.2, 19.8);
    render(view);
    view.histogramRange(lo, hi);
    QVERIFY(near(lo, 5.4) && near(hi, 24.6));
    sum = 0;
    for (int v : view.histogram())
        sum += v;
    QCOMPARE(sum, 19LL * ny); // x = 6..24

    // Outside values are skipped instead of piling into the edge bins.
    QCOMPARE(view.histogram().front(), 0);
    QCOMPARE(view.histogram().back(), 0);
}

void PlottingTest::fastImageHistogramLogWindow() {
    sls::SlsQtFastImage view;
    view.resize(200, 200);
    view.show();
    QTest::qWait(5);
    const int nx = 64, ny = 32;
    std::vector<float> ramp(size_t(nx) * ny);
    for (int y = 0; y < ny; y++)
        for (int x = 0; x < nx; x++)
            ramp[size_t(y) * nx + x] = float(x);

    view.setLogScale(true);
    QVERIFY(view.logScale());
    view.setColorRange(1.0, 100.0);
    view.setImageCopy(ramp.data(), nx, ny);
    render(view);

    double lo = 0.0, hi = 0.0;
    view.histogramRange(lo, hi);
    // log10(1) - 1 .. log10(100) + 1
    QVERIFY(near(lo, -1.0) && near(hi, 3.0));
    long long sum = 0;
    for (int v : view.histogram())
        sum += v;
    // x=0 is clamped to 1e-2 (log -2) and is outside; x=1..63 are inside.
    QCOMPARE(sum, 63LL * ny);
}

void PlottingTest::fastImageLogClamp() {
    sls::SlsQtFastImage view;
    view.resize(200, 200);
    view.show();
    QTest::qWait(5);
    view.setPaletteLut(sls::plot::paletteLut(sls::plot::Palette::Color));
    view.setLogScale(true);
    // Lower end is exactly the 1e-2 log clamp, so everything <= 0 maps to the
    // first LUT entry while 0.02 (just above the clamp) does not.
    view.setColorRange(0.01, 100.0);
    std::vector<float> f{-100.0f, 0.0f, 0.01f, 0.02f, 1000.0f, 1000.0f, 1.0f,
                         1.0f};
    view.setImageCopy(f.data(), 4, 2);
    const QImage img = render(view);
    const QRgb *lut = sls::plot::paletteLut(sls::plot::Palette::Color);

    const QRgb clamped0 = dominantPixelAtData(view, img, 0.5, 0.5);
    QCOMPARE(clamped0, lut[0]);
    QCOMPARE(dominantPixelAtData(view, img, 1.5, 0.5), clamped0);
    QCOMPARE(dominantPixelAtData(view, img, 2.5, 0.5), clamped0);
    // 0.02 is > 1e-2, so it is not clamped to the bottom colour.
    QVERIFY(dominantPixelAtData(view, img, 3.5, 0.5) != clamped0);
    // Above the range: top colour, no wraparound.
    QCOMPARE(dominantPixelAtData(view, img, 0.5, 1.5), lut[255]);
}

void PlottingTest::fastImageZeroWhite() {
    sls::SlsQtFastImage view;
    view.resize(200, 200);
    view.show();
    QTest::qWait(5);
    view.setPaletteLut(sls::plot::paletteLut(sls::plot::Palette::Color));
    view.setColorRange(0.0, 2.0);

    std::vector<float> f(16, 1.0f);
    f[0] = 0.0f; // data pixel (0,0), bottom-left
    view.setImageCopy(f.data(), 4, 4);
    const QRgb *lut = sls::plot::paletteLut(sls::plot::Palette::Color);

    QVERIFY(!view.zeroWhite());
    QCOMPARE(dominantPixelAtData(view, render(view), 0.5, 0.5), lut[0]);

    view.setZeroWhite(true);
    QVERIFY(view.zeroWhite());
    QCOMPARE(dominantPixelAtData(view, render(view), 0.5, 0.5), qRgb(255, 255, 255));
    // A real value is not affected.
    QCOMPARE(dominantPixelAtData(view, render(view), 1.5, 1.5), lut[127]);

    // Non-finite pixels use the lowest colour, even with zero-white on.
    f[0] = std::numeric_limits<float>::quiet_NaN();
    view.setImageCopy(f.data(), 4, 4);
    QCOMPARE(dominantPixelAtData(view, render(view), 0.5, 0.5), lut[0]);

    view.setIntegerValues(true);
    QVERIFY(view.integerValues());
    view.setIntegerValues(false);
    QVERIFY(!view.integerValues());
}

// ---------------------------------------------------------------------------
// FastImageView: zoom / viewport / clipping
// ---------------------------------------------------------------------------
void PlottingTest::fastImageZoomCursorInvariance() {
    sls::SlsQtFastImage view;
    view.resize(400, 300);
    view.show();
    QTest::qWait(5);
    // Square image: the whole-image fit is height-limited with a constant
    // horizontal letterbox, so the cursor-fixed rule can hold exactly even when
    // the viewport is clamped at the minimum size during deep zoom.
    const int nx = 64, ny = 64;
    std::vector<float> ramp(size_t(nx) * ny, 1.0f);
    view.setColorRange(0.0, 2.0);
    view.setImageCopy(ramp.data(), nx, ny);
    view.resetView();

    int zoomCalls = 0;
    view.onZoomChanged = [&](double, double, double, double) { zoomCalls++; };
    view.setViewport(10.0, 5.0, 30.0, 15.0); // programmatic: no callback
    QCOMPARE(zoomCalls, 0);
    view.resetView();
    QCOMPARE(zoomCalls, 1);

    const QPointF cursor(200.0, 150.0);
    for (int i = 0; i < 5; i++) {
        const QPointF before = view.widgetToData(cursor);
        sendWheel(view, cursor, 120); // zoom in, factor 1/1.18
        const QPointF after = view.widgetToData(cursor);
        QVERIFY2(near(before.x(), after.x(), 1e-6),
                 "wheel zoom must keep the cursor x fixed");
        QVERIFY2(near(before.y(), after.y(), 1e-6),
                 "wheel zoom must keep the cursor y fixed");
    }
    double vx, vy, vw, vh;
    view.viewport(vx, vy, vw, vh);
    QVERIFY(vw < nx && vh < ny);
    QVERIFY(zoomCalls >= 6); // 1 reset + 5 wheels

    // Deep zoom clamps to the minimum viewport instead of collapsing.
    for (int i = 0; i < 40; i++)
        sendWheel(view, cursor, 120);
    view.viewport(vx, vy, vw, vh);
    QVERIFY2(vw >= sls::SlsQtFastImage::kMinimumViewport - 1e-9,
             "viewport clamps to the minimum width");
    QVERIFY2(vh >= sls::SlsQtFastImage::kMinimumViewport - 1e-9,
             "viewport clamps to the minimum height");

    // Zooming back out stops at the full frame and keeps the cursor point.
    for (int i = 0; i < 60; i++) {
        const QPointF before = view.widgetToData(cursor);
        sendWheel(view, cursor, -120);
        const QPointF after = view.widgetToData(cursor);
        QVERIFY(near(before.x(), after.x(), 1e-6));
        QVERIFY(near(before.y(), after.y(), 1e-6));
    }
    view.viewport(vx, vy, vw, vh);
    QVERIFY(near(vw, nx) && near(vh, ny));
    QVERIFY(near(vx, 0.0) && near(vy, 0.0));
}

void PlottingTest::fastImageViewportClamping() {
    sls::SlsQtFastImage view;
    view.resize(400, 300);
    view.show();
    QTest::qWait(5);
    const int nx = 64, ny = 32;
    std::vector<float> ramp(size_t(nx) * ny, 1.0f);
    view.setImageCopy(ramp.data(), nx, ny);
    view.resetView();
    double x, y, w, h;

    // Requested viewports are clamped to the image bounds.
    view.setViewport(-100.0, -100.0, 5000.0, 5000.0);
    view.viewport(x, y, w, h);
    QVERIFY(near(x, 0.0) && near(y, 0.0));
    QVERIFY(near(w, nx) && near(h, ny));

    // A viewport smaller than the minimum is rejected (state unchanged).
    view.setViewport(10.0, 10.0, 10.5, 10.5);
    view.viewport(x, y, w, h);
    QVERIFY(near(w, nx) && near(h, ny));

    // A viewport that would stick out on the right/top is slid back inside.
    view.setViewport(60.0, 28.0, 70.0, 38.0);
    view.viewport(x, y, w, h);
    QVERIFY(near(w, 10.0) && near(h, 10.0));
    QVERIFY(x >= 0.0 && x + w <= nx + 1e-9);
    QVERIFY(y >= 0.0 && y + h <= ny + 1e-9);

    // Non-finite values are ignored.
    const double nan = std::numeric_limits<double>::quiet_NaN();
    view.setViewport(nan, 0.0, 10.0, 10.0);
    view.viewport(x, y, w, h);
    QVERIFY(near(w, 10.0) && near(h, 10.0));

    view.resetView();
    view.viewport(x, y, w, h);
    QVERIFY(near(x, 0.0) && near(y, 0.0) && near(w, nx) && near(h, ny));

    // Pixel aspect stretches x only and is validated.
    view.setPixelAspectX(2.0);
    QCOMPARE(view.pixelAspectX(), 2.0);
    view.setPixelAspectX(-1.0);
    QCOMPARE(view.pixelAspectX(), 1.0);
}

void PlottingTest::fastImageClipsToPlotRect() {
    sls::SlsQtFastImage view;
    view.resize(400, 300);
    view.show();
    QTest::qWait(5);
    const int nx = 1024, ny = 512;
    std::vector<float> frame = acceptanceFrame();
    view.setColorRange(-2.0, 2.0);
    view.setImageCopy(frame.data(), nx, ny);
    view.resetView();

    // Margins of the widget: the plot rect is QRect(52, 6, w-58, h-32).
    const QRect ir = sls::plot::plotRectFor(view.size(), sls::plot::PlotTheme());
    QCOMPARE(ir, QRect(52, 6, 400 - 58, 300 - 32));

    // Zoom so the scaled image is much larger than the plot area; without the
    // clip it would paint over the right/top margins.
    view.setViewport(100.0, 100.0, 164.0, 132.0);
    const QImage img = render(view);
    QVERIFY(img.width() == 400 && img.height() == 300);

    for (int y = ir.top(); y <= ir.bottom(); y++) {
        for (int x = ir.right() + 1; x < img.width(); x++) {
            QVERIFY2(img.pixel(x, y) == kBackground,
                     "zoomed image must be clipped to the plot rect (right "
                     "margin)");
        }
    }
    for (int x = ir.left(); x <= img.width() - 1; x++) {
        for (int y = 0; y < ir.top(); y++) {
            QVERIFY2(img.pixel(x, y) == kBackground,
                     "zoomed image must be clipped to the plot rect (top "
                     "margin)");
        }
    }
    // ... while the image itself is definitely painted inside.
    QVERIFY(img.pixel(ir.center()) != kBackground);
}

void PlottingTest::fastImageOverlaysInDataCoordinates() {
    sls::SlsQtFastImage view;
    view.resize(400, 300);
    view.show();
    QTest::qWait(5);
    const int nx = 1024, ny = 512;
    std::vector<float> frame = acceptanceFrame();
    view.setColorRange(-2.0, 2.0);
    view.setImageCopy(frame.data(), nx, ny);
    view.resetView();

    // (120,110) lower-left + 20x10, entirely inside the view.
    view.setOverlayRects({QRectF(120.0, 110.0, 20.0, 10.0)});
    QCOMPARE(int(view.overlayRects().size()), 1);
    QImage img = render(view);
    const QPointF topLeft = view.dataToWidget(QPointF(120.0, 120.0));
    QVERIFY2(yellowNear(img, topLeft.toPoint(), 2),
             "overlay must be drawn at its data coordinates (yellow)");
    const QPointF bottomRight = view.dataToWidget(QPointF(140.0, 110.0));
    QVERIFY(yellowNear(img, bottomRight.toPoint(), 2));

    // A rectangle crossing the plot edge is clipped: no yellow outside.
    view.setOverlayRects({QRectF(1000.0, 400.0, 400.0, 400.0)});
    img = render(view);
    const QRect ir = sls::plot::plotRectFor(view.size(), sls::plot::PlotTheme());
    for (int x = ir.right() + 1; x < img.width(); x++) {
        for (int y = 0; y < img.height(); y++) {
            const QRgb c = img.pixel(x, y);
            QVERIFY2(!(qRed(c) > 180 && qGreen(c) > 180 && qBlue(c) < 90),
                     "overlays must be clipped to the plot rect");
        }
    }
    view.clearOverlayRects();
    QVERIFY(view.overlayRects().empty());

    // Editable gate ROI and receiver ROI are reported back in data coords.
    view.setRoiRect(10.0, 20.0, 110.0, 220.0);
    double x0, y0, x1, y1;
    QVERIFY(view.roiRect(x0, y0, x1, y1));
    QVERIFY(near(x0, 10.0) && near(y1, 220.0));
    view.setRoiRectVisible(false);
    QVERIFY(!view.roiRect(x0, y0, x1, y1));
    view.setReceiverRoiRect(5.0, 6.0, 7.0, 8.0);
    view.clearReceiverRoiRect();
}

void PlottingTest::fastImageTitlesAndAspect() {
    sls::SlsQtFastImage view;
    view.resize(400, 300);
    view.show();
    QTest::qWait(5);
    std::vector<float> ramp(64 * 32, 1.0f);
    view.setImageCopy(ramp.data(), 64, 32);

    QVERIFY(view.title().isEmpty());
    view.setTitle(QStringLiteral("Detector 0"));
    view.setXAxisTitle(QStringLiteral("x [px]"));
    view.setYAxisTitle(QStringLiteral("y [px]"));
    QCOMPARE(view.title(), QStringLiteral("Detector 0"));
    QCOMPARE(view.xAxisTitle(), QStringLiteral("x [px]"));
    QCOMPARE(view.yAxisTitle(), QStringLiteral("y [px]"));
    // A title consumes a row at the top; the image must still render.
    const QImage img = render(view);
    const QRect ir = sls::plot::plotRectFor(view.size(), sls::plot::PlotTheme());
    QVERIFY(img.pixel(ir.center()) != kBackground);

    QFont f;
    f.setPixelSize(17);
    view.setTitleFont(f);
    QCOMPARE(view.titleFont().pixelSize(), 17);
    view.setAxisTitleFont(f);
    QCOMPARE(view.axisTitleFont().pixelSize(), 17);
    view.setTickFont(f);
    QCOMPARE(view.tickFont().pixelSize(), 17);
}

// ---------------------------------------------------------------------------
// ScaleHisto: three-way freeze rule
// ---------------------------------------------------------------------------
void PlottingTest::scaleHistoThreeWayFreeze() {
    sls::SlsQtScaleHisto histo;
    histo.resize(400, 38);
    histo.show();
    QTest::qWait(5);

    std::vector<int> bins(256, 0);
    bins[100] = 10;
    bins[200] = 300;
    histo.setData(&bins, 0.0, 1.0, 0.2, 0.8, false, false);
    double dw0 = 0.0, dw1 = 0.0;
    histo.displayedWindow(dw0, dw1);
    QVERIFY(near(dw0, -0.1, 1e-9) && near(dw1, 1.1, 1e-9));
    double cw0 = 0.0, cw1 = 0.0;
    histo.currentWindow(cw0, cw1);
    QVERIFY(near(cw0, 0.2) && near(cw1, 0.8));

    int rangeCalls = 0;
    double newLo = 0.0, newHi = 0.0;
    histo.onRangeChanged = [&](double a, double b) {
        rangeCalls++;
        newLo = a;
        newHi = b;
    };

    const int W = histo.width();
    const QPointF marker(W * 0.25, 19.0);
    sendMouse(histo, QEvent::MouseButtonPress, marker, Qt::LeftButton,
              Qt::LeftButton);
    QVERIFY(histo.isDragging());

    // Freeze #1 (bars): a live histogram update must be ignored.
    const std::vector<int> *frozen = histo.displayedHistogram();
    QVERIFY(frozen != nullptr);
    QVERIFY(frozen != &bins);
    QCOMPARE(int(frozen->size()), 256);
    QCOMPARE((*frozen)[100], 10);
    std::vector<int> live2(256, 0);
    live2[50] = 999;
    histo.setData(&live2, 0.0, 1.0, 0.0, 1.0, false, false);
    QVERIFY2(histo.displayedHistogram() == frozen,
             "histogram bars must be frozen while dragging");
    QCOMPARE((*histo.displayedHistogram())[100], 10);
    QCOMPARE((*histo.displayedHistogram())[200], 300);

    // Freeze #2 (axis window): still the press-time window after a move.
    const int previewAtPress = histo.previewX();
    sendMouse(histo, QEvent::MouseMove, QPointF(W * 0.40, 19.0),
              Qt::NoButton, Qt::LeftButton);
    QVERIFY(histo.isDragging());
    double fw0 = 0.0, fw1 = 0.0;
    histo.displayedWindow(fw0, fw1);
    QVERIFY2(near(fw0, dw0) && near(fw1, dw1),
             "axis window must stay frozen while dragging");

    // Freeze #3 (cursor mapping): the pre-move range is used, so these must be
    // stable no matter how often the mouse moves.
    double l0 = 0.0, l1 = 0.0;
    histo.currentWindow(l0, l1);
    QVERIFY(l0 > 0.2); // preview moved the low end
    for (int i = 0; i < 3; i++)
        sendMouse(histo, QEvent::MouseMove, QPointF(W * 0.40, 19.0),
                  Qt::NoButton, Qt::LeftButton);
    double l2 = 0.0, l3 = 0.0;
    histo.currentWindow(l2, l3);
    QVERIFY(near(l0, l2) && near(l1, l3));
    QVERIFY2(histo.previewX() != previewAtPress,
             "only the dashed preview line moves");
    QCOMPARE(rangeCalls, 0); // nothing applied before release

    // Painting mid-drag must not mutate the frozen state either.
    (void)histo.grab();
    histo.displayedWindow(fw0, fw1);
    QVERIFY(near(fw0, dw0) && near(fw1, dw1));
    QVERIFY(histo.displayedHistogram() == frozen);

    sendMouse(histo, QEvent::MouseButtonRelease, QPointF(W * 0.40, 19.0),
              Qt::LeftButton, Qt::NoButton);
    QVERIFY(!histo.isDragging());
    QCOMPARE(rangeCalls, 1); // applied exactly once, on release
    QVERIFY(near(newLo, l2) && near(newHi, l3));
    QVERIFY2(histo.displayedHistogram() == &bins,
             "live histogram is restored after release");
    double nw0 = 0.0, nw1 = 0.0;
    histo.displayedWindow(nw0, nw1);
    QVERIFY2(!near(nw0, dw0) || !near(nw1, dw1),
             "the axis rebuilds from the new range after release");

    // A press far away from both markers is ignored.
    sendMouse(histo, QEvent::MouseButtonPress, QPointF(W * 0.5, 19.0),
              Qt::LeftButton, Qt::LeftButton);
    QVERIFY(!histo.isDragging());
    sendMouse(histo, QEvent::MouseButtonRelease, QPointF(W * 0.5, 19.0),
              Qt::LeftButton, Qt::NoButton);
    QCOMPARE(rangeCalls, 1);
}

void PlottingTest::scaleHistoMaxMarker() {
    sls::SlsQtScaleHisto histo;
    histo.resize(400, 38);
    histo.show();
    QTest::qWait(5);
    std::vector<int> bins(256, 1);
    histo.setData(&bins, 0.0, 1.0, 0.2, 0.8, false, false);

    int rangeCalls = 0;
    double newLo = 0.0, newHi = 0.0;
    histo.onRangeChanged = [&](double a, double b) {
        rangeCalls++;
        newLo = a;
        newHi = b;
    };
    const int W = histo.width();
    sendMouse(histo, QEvent::MouseButtonPress, QPointF(W * 0.75, 19.0),
              Qt::LeftButton, Qt::LeftButton);
    QVERIFY(histo.isDragging());
    sendMouse(histo, QEvent::MouseMove, QPointF(W * 0.60, 19.0),
              Qt::NoButton, Qt::LeftButton);
    sendMouse(histo, QEvent::MouseButtonRelease, QPointF(W * 0.60, 19.0),
              Qt::LeftButton, Qt::NoButton);
    QCOMPARE(rangeCalls, 1);
    QVERIFY(newLo < newHi);
    QVERIFY(newHi < 0.8); // max marker dragged down
}

// ---------------------------------------------------------------------------
// ScalePanel: re-entrancy, control synchronisation, strip freeze
// ---------------------------------------------------------------------------
void PlottingTest::scalePanelReentrancy() {
    sls::SlsQtFastImage view;
    view.resize(300, 300);
    view.show();
    QTest::qWait(5);
    std::vector<float> ramp(64 * 32);
    for (int i = 0; i < 64 * 32; i++)
        ramp[i] = float(i % 100) / 10.0f;
    view.setColorRange(0.0, 10.0);
    view.setImageCopy(ramp.data(), 64, 32);
    view.repaint();

    sls::SlsQtScalePanel panel(&view);
    panel.resize(760, 78);
    panel.show();
    QTest::qWait(5);
    QVERIFY(panel.histogramWidget() != nullptr);
    QVERIFY(panel.view() == &view);
    QVERIFY(!panel.isDragging());

    // view -> controls -> view must not recurse (sympass guard): one change,
    // one control update, no push-back loop.  The min/max spin boxes were
    // removed from the plot window; the range is owned by the Plot tab, so
    // the panel only keeps the last range it read from/wrote to the view
    // (controlRange(), not spin-box widgets).
    int rangeCalls = 0;
    view.onColorRangeChanged = [&](double, double) { rangeCalls++; };
    panel.setColorRange(1.0, 5.0);
    QCOMPARE(rangeCalls, 1);
    double a = 0.0, b = 0.0;
    view.colorRange(a, b);
    QVERIFY(near(a, 1.0) && near(b, 5.0));
    panel.controlRange(a, b);
    QVERIFY(near(a, 1.0) && near(b, 5.0));

    // refresh()/push() round trip is stable (idempotent, no re-entrancy).
    panel.refresh();
    panel.push();
    view.colorRange(a, b);
    QVERIFY(near(a, 1.0) && near(b, 5.0));
    panel.refresh();
    panel.refresh();
    view.colorRange(a, b);
    QVERIFY(near(a, 1.0) && near(b, 5.0));

    // Palette and image-log controls stay in sync with the view.
    panel.setPaletteChoice(3);
    QCOMPARE(view.paletteIndex(), 3);
    QCOMPARE(view.paletteIndex(), int(sls::plot::Palette::Turbo));
    QCOMPARE(panel.paletteChoice(), 3);
    panel.setImageLog(true);
    QVERIFY(view.logScale());
    QVERIFY(!view.isScaleDragging());
    panel.setImageLog(false);
    QVERIFY(!view.logScale());
    panel.setHistoLog(true);
    QVERIFY(panel.histoLog());

    // A strip drag freezes the strip; refresh() during the drag (the image
    // keeps arriving in the real GUI) must not feed back into it.
    sls::SlsQtScaleHisto *strip = panel.histogramWidget();
    double sw0 = 0.0, sw1 = 0.0;
    strip->displayedWindow(sw0, sw1);
    const std::vector<int> *live = strip->displayedHistogram();
    QVERIFY(live != nullptr);
    const int W = strip->width();
    sendMouse(*strip, QEvent::MouseButtonPress, QPointF(W * 0.25, 10.0),
              Qt::LeftButton, Qt::LeftButton);
    QVERIFY(strip->isDragging());
    QVERIFY(panel.isDragging());
    const std::vector<int> *frozen = strip->displayedHistogram();
    QVERIFY2(frozen != live, "drag paints a snapshot, not the live bars");
    panel.refresh();
    sendMouse(*strip, QEvent::MouseMove, QPointF(W * 0.40, 10.0),
              Qt::NoButton, Qt::LeftButton);
    panel.refresh();
    double dw0 = 0.0, dw1 = 0.0;
    strip->displayedWindow(dw0, dw1);
    QVERIFY2(near(dw0, sw0) && near(dw1, sw1),
             "panel refresh must not move the frozen strip");
    QVERIFY2(strip->displayedHistogram() == frozen,
             "panel refresh must not replace the frozen bars");

    sendMouse(*strip, QEvent::MouseButtonRelease, QPointF(W * 0.40, 10.0),
              Qt::LeftButton, Qt::NoButton);
    QVERIFY(!strip->isDragging());
    QVERIFY(!panel.isDragging());
    // Release applied the previewed range to the view.
    view.colorRange(a, b);
    QVERIFY(b > a);
    QVERIFY(!near(a, 1.0) || !near(b, 5.0));

    // Detach/reattach keeps working and preserves the view callbacks.
    panel.setView(nullptr);
    QVERIFY(panel.view() == nullptr);
    panel.setView(&view);
    QVERIFY(panel.view() == &view);
    panel.refresh(); // must not crash with a fresh attachment
}

// ---------------------------------------------------------------------------
// ScalePanel R1/R2 (task-11): strip freeze and the controls->view push rules
// ---------------------------------------------------------------------------
void PlottingTest::scalePanelFreezeAndPushRules() {
    sls::SlsQtFastImage view;
    view.resize(400, 400);
    view.show();
    QTest::qWait(5);
    std::vector<float> data(64 * 64);
    for (int i = 0; i < 64 * 64; i++)
        data[size_t(i)] = float(i % 100) / 10.0f;
    view.setColorRange(1.0, 5.0);
    view.setImageCopy(data.data(), 64, 64);
    view.repaint();

    sls::SlsQtScalePanel panel(&view);
    panel.resize(760, 78);
    panel.show();
    QTest::qWait(5);

    double lo = 0.0, hi = 0.0;
    panel.controlRange(lo, hi);
    QVERIFY(near(lo, 1.0) && near(hi, 5.0));
    int rangeCalls = 0;
    view.onColorRangeChanged = [&](double, double) { rangeCalls++; };

    // R1: while a strip marker is dragged, incoming frames may change the
    // view's colour range, but refresh() must keep the controls frozen at the
    // press-time values (and the strip axis/bars frozen as well).
    auto *strip = panel.histogramWidget();
    double frozenW0 = 0.0, frozenW1 = 0.0;
    strip->displayedWindow(frozenW0, frozenW1);
    const std::vector<int> *liveBars = strip->displayedHistogram();
    sendMouse(*strip, QEvent::MouseButtonPress,
              QPointF(strip->width() * 0.25, strip->height() / 2.0),
              Qt::LeftButton, Qt::LeftButton);
    QVERIFY(strip->isDragging());
    const std::vector<int> *frozenBars = strip->displayedHistogram();
    QVERIFY2(frozenBars != nullptr && frozenBars != liveBars,
             "drag paints a press-time histogram snapshot");
    view.setColorRange(2.0, 7.0);
    panel.refresh();
    panel.controlRange(lo, hi);
    QVERIFY2(near(lo, 1.0) && near(hi, 5.0),
             "control range frozen while the strip marker is dragged (R1)");
    view.setColorRange(3.0, 9.0);
    panel.refresh();
    panel.controlRange(lo, hi);
    QVERIFY2(near(lo, 1.0) && near(hi, 5.0),
             "control range still frozen after a second live range change (R1)");
    double dw0 = 0.0, dw1 = 0.0;
    strip->displayedWindow(dw0, dw1);
    QVERIFY(near(dw0, frozenW0) && near(dw1, frozenW1));
    QVERIFY(strip->displayedHistogram() == frozenBars);
    // Move the low marker from its 1/4 place towards 2/5 (the preview value).
    sendMouse(*strip, QEvent::MouseMove,
              QPointF(strip->width() * 0.40, strip->height() / 2.0),
              Qt::NoButton, Qt::LeftButton);
    double cw0 = 0.0, cw1 = 0.0;
    strip->currentWindow(cw0, cw1);
    QVERIFY(cw0 > 2.0);
    sendMouse(*strip, QEvent::MouseButtonRelease,
              QPointF(strip->width() * 0.40, strip->height() / 2.0),
              Qt::LeftButton, Qt::NoButton);
    QVERIFY(!strip->isDragging());
    // On release the strip range is applied and the controls follow it.
    double vlo = 0.0, vhi = 0.0;
    view.colorRange(vlo, vhi);
    QVERIFY2(near(vlo, cw0) && near(vhi, cw1),
             "strip range applied to the view on release (R1)");
    panel.controlRange(lo, hi);
    QVERIFY(std::abs(lo - vlo) <= 0.051);
    QVERIFY(std::abs(hi - vhi) <= 0.051);

    // R2: palette/log changes must NOT push the control range into the view
    // (that would fire onColorRangeChanged and pin the 2D panel override).
    // Only an explicit control-range edit may change the range.
    view.setColorRange(0.408939, 40.8); // e.g. SlsQt2DPlot's auto z range
    view.repaint();
    const int callsBefore = rangeCalls;
    panel.setPaletteChoice(3);
    QCOMPARE(rangeCalls, callsBefore);
    double a = 0.0, b = 0.0;
    view.colorRange(a, b);
    QVERIFY2(near(a, 0.408939) && near(b, 40.8),
             "palette change leaves the auto z range bit-exact (R2)");
    double plo = 0.0, phi = 0.0;
    QVERIFY(!panel.lastPushedRange(plo, phi));
    panel.setImageLog(true);
    panel.setImageLog(false);
    QCOMPARE(rangeCalls, callsBefore);
    view.colorRange(a, b);
    QVERIFY(near(a, 0.408939) && near(b, 40.8));
    QVERIFY(!panel.lastPushedRange(plo, phi));
    panel.controlRange(lo, hi);
    QVERIFY(near(lo, 0.4, 0.051) && near(hi, 40.8, 0.051));

    // A real operator edit is the only path that pushes a range, exactly once.
    panel.setControlRange(2.5, 8.0);
    QCOMPARE(rangeCalls, callsBefore + 1);
    view.colorRange(a, b);
    QVERIFY(near(a, 2.5) && near(b, 8.0));
    QVERIFY(panel.lastPushedRange(plo, phi));
    QVERIFY(near(plo, 2.5) && near(phi, 8.0));
    // A later palette change still does not push the (now edited) range.
    panel.setPaletteChoice(5);
    QCOMPARE(rangeCalls, callsBefore + 1);
    view.colorRange(a, b);
    QVERIFY(near(a, 2.5) && near(b, 8.0));
}

// ---------------------------------------------------------------------------
// ScalePanel R6 (task-11): the strip never keeps a pointer to a dead histogram
// ---------------------------------------------------------------------------
void PlottingTest::scalePanelDetachLifetime() {
    std::vector<float> ramp(64 * 32, 1.0f);

    // Detaching the panel clears the strip's raw histogram pointer, and the
    // orphaned strip can still be repainted/grab'ed safely.
    auto *view = new sls::SlsQtFastImage;
    view->resize(300, 300);
    view->show();
    QTest::qWait(5);
    view->setImageCopy(ramp.data(), 64, 32);
    view->repaint();
    auto *panel = new sls::SlsQtScalePanel(view);
    panel->resize(600, 78);
    panel->show();
    QTest::qWait(5);
    panel->histogramWidget()->repaint();
    QVERIFY(panel->histogramWidget()->displayedHistogram() != nullptr);
    panel->setView(nullptr);
    QVERIFY(panel->view() == nullptr);
    QVERIFY2(panel->histogramWidget()->displayedHistogram() == nullptr,
             "detach drops the live histogram pointer (R6)");
    (void)panel->histogramWidget()->grab(); // must not read freed memory
    panel->refresh();
    delete panel;

    // Destroying the view clears the strip through the destroyed connection;
    // repainting the orphaned panel afterwards is safe.
    auto *view2 = new sls::SlsQtFastImage;
    view2->resize(300, 300);
    view2->show();
    QTest::qWait(5);
    view2->setImageCopy(ramp.data(), 64, 32);
    view2->repaint();
    auto *panel2 = new sls::SlsQtScalePanel(view2);
    panel2->resize(600, 78);
    panel2->show();
    QTest::qWait(5);
    panel2->histogramWidget()->repaint();
    QVERIFY(panel2->histogramWidget()->displayedHistogram() != nullptr);
    delete view2;
    QVERIFY(panel2->view() == nullptr);
    QVERIFY2(panel2->histogramWidget()->displayedHistogram() == nullptr,
             "destroyed view drops the live histogram pointer (R6)");
    (void)panel2->histogramWidget()->grab();
    panel2->refresh();
    delete panel2;
    delete view;
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

    // Fill/FillBin/SetContent/SetBinContent follow the old qwt data contract.
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

    // G/H marker lines, SetYStep and alignScales (API compatibility).
    plot.SetYMinMax(0.0, 300.0);
    plot.InsertHLine(150.0);
    plot.InsertVLine(50.0);
    plot.SetYStep(2);
    plot.alignScales();
    img = render(plot);
    QVERIFY2(countPixels(img, isYellow) > 20,
             "H/V marker lines are painted in the plot");
    plot.RemoveHLine();
    plot.RemoveVLine();
    plot.EnableYAutoScaling();

    // O1 (task-10): on a log-y axis the auto-range padding is applied in
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
    // O2/task-14: the topmost gain tick label ("3") must be fully inside the
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

    // R3 (task-12): NaN/Inf bins are skipped by the automatic z range/mean.
    std::vector<double> nf(12);
    for (int i = 0; i < 12; i++)
        nf[size_t(i)] = double(i + 1); // 1..12
    nf[0] = std::numeric_limits<double>::quiet_NaN();  // 1
    nf[5] = std::numeric_limits<double>::infinity();   // 6
    nf[7] = -std::numeric_limits<double>::infinity();  // 8
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

    // R5 (task-12): all-negative data is padded OUTWARD on both ends; the
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
// 2D: SetData / z range / scale panel integration
// ---------------------------------------------------------------------------
void PlottingTest::twoDimensionalDataZRangeAndPanel() {
    sls::SlsQt2DPlot plot;
    plot.resize(700, 600);
    plot.show();
    QTest::qWait(5);
    QVERIFY(!plot.isGainPlot());
    QVERIFY(plot.imageView() != nullptr);
    QVERIFY(plot.scalePanel() != nullptr);

    plot.SetTitle(QStringLiteral("2D detector"));
    QCOMPARE(plot.title(), QStringLiteral("2D detector"));
    plot.SetXTitle(QStringLiteral("x"));
    plot.SetYTitle(QStringLiteral("y"));
    // The colour-scale title is a compact caption row above the scale panel.
    plot.SetZTitle(QStringLiteral("counts [a.u.]"));
    QVERIFY(plot.zTitleLabel() != nullptr);
    QTest::qWait(5);
    QVERIFY2(plot.zTitleLabel()->isVisible(),
             "SetZTitle() shows the z-axis caption");
    QCOMPARE(plot.zTitleLabel()->text(), QStringLiteral("counts [a.u.]"));
    plot.SetZTitle(QString());
    QTest::qWait(5);
    QVERIFY2(!plot.zTitleLabel()->isVisible(),
             "an empty z title hides the caption row again");

    plot.FillTestPlot();
    QCOMPARE(plot.GetXMinimum(), 200.0);
    QCOMPARE(plot.GetXMaximum(), 822.0);
    QCOMPARE(plot.GetYMinimum(), -0.5);
    QCOMPARE(plot.GetYMaximum(), 49.5);
    QVERIFY(plot.GetZMaximum() > plot.GetZMinimum());
    QVERIFY(plot.imageView()->hasImage());
    QCOMPARE(plot.imageView()->imageWidth(), 50);
    QCOMPARE(plot.imageView()->imageHeight(), 50);
    QVERIFY(plot.imageView()->hasImage());

    // Z range is pushed to the canvas and the scale panel stays in sync.
    plot.SetZMinMax(2.0, 8.0);
    QCOMPARE(plot.GetZMinimum(), 2.0);
    QCOMPARE(plot.GetZMaximum(), 8.0);
    double lo = 0.0, hi = 0.0;
    plot.imageView()->colorRange(lo, hi);
    QVERIFY(near(lo, 2.0) && near(hi, 8.0));
    QVERIFY(plot.scalePanel()->view() == plot.imageView());

    // Panel -> plot direction.
    plot.scalePanel()->setColorRange(3.0, 6.0);
    plot.imageView()->colorRange(lo, hi);
    QVERIFY(near(lo, 3.0) && near(hi, 6.0));

    // Fresh data with an explicit z range.
    std::vector<double> d(4 * 3);
    for (int i = 0; i < 12; i++)
        d[size_t(i)] = double(i);
    plot.SetData(4, 0.0, 4.0, 3, 0.0, 3.0, d.data(), 1.0, 9.0);
    QCOMPARE(plot.imageView()->imageWidth(), 4);
    QCOMPARE(plot.imageView()->imageHeight(), 3);
    QCOMPARE(plot.GetZMinimum(), 1.0);
    QCOMPARE(plot.GetZMaximum(), 9.0);
    QCOMPARE(plot.GetBinValue(1, 1), 5.0);
    QCOMPARE(plot.GetBinIndex(1, 1), 5);
    QCOMPARE(plot.FindBinIndex(0.5, 0.5), 0);

    plot.SetBinValue(1, 1, 42.0);
    QCOMPARE(plot.GetBinValue(1, 1), 42.0);
    QCOMPARE(*(plot.GetDataPtr() + 1 + 1 * 4), 42.0);

    // Whole-image fit: no paint outside the plot rectangle of the canvas.
    plot.UnZoom();
    QImage img = render(*plot.imageView());
    const QRect ir =
        sls::plot::plotRectFor(plot.imageView()->size(), sls::plot::PlotTheme());
    for (int x = ir.right() + 1; x < img.width(); x++) {
        for (int y = 0; y < img.height(); y++) {
            QVERIFY2(img.pixel(x, y) == kBackground,
                     "2D canvas must not paint in the right margin");
        }
    }
    QVERIFY(img.pixel(ir.center()) != kBackground);

    plot.EnableXAutoScaling();
    plot.EnableYAutoScaling();
    plot.SetXMinMax(1.0, 3.0);
    plot.SetYMinMax(0.5, 2.5);
    QVERIFY(plot.imageView()->hasImage());
    plot.UnZoom(false);
}

// ---------------------------------------------------------------------------
// 2D: log z, interpolation, contours
// ---------------------------------------------------------------------------
void PlottingTest::twoDimensionalLogInterpolateAndContour() {
    sls::SlsQt2DPlot plot;
    plot.resize(800, 600);
    plot.show();
    QTest::qWait(5);
    plot.FillTestPlot();
    plot.UnZoom();

    // Log z reaches the canvas and the panel chooser.
    plot.LogZ(true);
    QVERIFY(plot.imageView()->logScale());
    plot.LogZ(false);
    QVERIFY(!plot.imageView()->logScale());

    // SetLogz(enable, isMin, isMax, min, max) sets the range and the scale.
    // An explicit Plot-tab / SetLogz() range is honoured in log mode too (the
    // old qwt behaviour): the requested zmin/zmax reach the data and the
    // canvas.
    plot.SetLogz(true, true, true, 0.1, 9.0);
    QVERIFY(plot.imageView()->logScale());
    QVERIFY(near(plot.GetZMinimum(), 0.1));
    QVERIFY(near(plot.GetZMaximum(), 9.0));
    double zlo = 0.0, zhi = 0.0;
    plot.imageView()->colorRange(zlo, zhi);
    QVERIFY(near(zlo, 0.1) && near(zhi, 9.0));

    // The explicit range survives the Update() calls of streaming frames ...
    plot.Update();
    QVERIFY(near(plot.GetZMinimum(), 0.1));
    QVERIFY(near(plot.GetZMaximum(), 9.0));
    plot.imageView()->colorRange(zlo, zhi);
    QVERIFY(near(zlo, 0.1) && near(zhi, 9.0));

    // ... while SetZRange(false, false) (qDrawPlot's per-frame call while the
    // z-range check boxes are off) drops the explicit pin and returns to the
    // automatic log range.
    plot.SetZRange(false, false, 0.0, 0.0);
    QVERIFY(plot.GetZMinimum() > 0.0);
    QVERIFY(!(near(plot.GetZMinimum(), 0.1) && near(plot.GetZMaximum(), 9.0)));
    plot.imageView()->colorRange(zlo, zhi);
    QVERIFY(near(zlo, plot.GetZMinimum()) && near(zhi, plot.GetZMaximum()));

    // Contours are painted through the clipped overlay hook.
    const QImage blocky = render(*plot.imageView());
    plot.SetContour(true);
    const QImage contours = render(*plot.imageView());
    QVERIFY2(blocky != contours, "SetContour(true) changes the rendered image");
    const QRect ir =
        sls::plot::plotRectFor(plot.imageView()->size(), sls::plot::PlotTheme());
    for (int x = ir.right() + 1; x < contours.width(); x++) {
        for (int y = 0; y < contours.height(); y++) {
            QVERIFY2(contours.pixel(x, y) == kBackground,
                     "contours must be clipped to the plot rect");
        }
    }
    plot.SetContour(false);
    QVERIFY(render(*plot.imageView()) == blocky);

    // Bilinear interpolation overlay.
    plot.SetInterpolate(true);
    const QImage smooth = render(*plot.imageView());
    QVERIFY2(smooth != blocky, "SetInterpolate(true) changes the rendered image");
    plot.SetInterpolate(false);
    QVERIFY(render(*plot.imageView()) == blocky);

    // Spectrogram image can be hidden without losing contours/ROI paint.
    plot.showSpectrogram(false);
    QVERIFY(render(*plot.imageView()) != blocky);
    plot.showSpectrogram(true);
    QVERIFY(render(*plot.imageView()) == blocky);

    // R8 (task-12): on a >256-wide frame the marching-squares grid is
    // subsampled; the last row/column must be included so the contours reach
    // the image edges (pre-fix an outer band up to step-1 bins wide was empty).
    plot.LogZ(false);
    {
        std::vector<double> big(size_t(1024) * 1024);
        for (int y = 0; y < 1024; y++)
            for (int x = 0; x < 1024; x++)
                big[size_t(y) * 1024 + size_t(x)] = double(x + y);
        plot.SetData(1024, -0.5, 1023.5, 1024, -0.5, 1023.5, big.data());
        plot.UnZoom();
        plot.SetContour(true);
        plot.showSpectrogram(false); // contour-only render on the dark canvas
        const QImage edgeImg = render(*plot.imageView());
        const QPointF imgRight =
            plot.imageView()->dataToWidget(QPointF(1024.0, 0.0));
        const QPointF imgTop =
            plot.imageView()->dataToWidget(QPointF(0.0, 1024.0));
        int maxX = -1;
        int minY = std::numeric_limits<int>::max();
        for (int y = ir.top(); y <= ir.bottom(); y++) {
            for (int x = ir.left(); x <= ir.right(); x++) {
                if (!isBright(edgeImg.pixel(x, y)))
                    continue;
                maxX = std::max(maxX, x);
                minY = std::min(minY, y);
            }
        }
        QVERIFY2(maxX >= int(std::lround(imgRight.x())) - 1,
                 "contours reach the image right edge (R8)");
        QVERIFY2(minY <= int(std::lround(imgTop.y())) + 1,
                 "contours reach the image top edge (R8)");
        plot.showSpectrogram(true);
        plot.SetContour(false);
        plot.FillTestPlot(); // restore the small test frame
    }

    // R3 (task-12): a frame with NaN/Inf bins keeps the z range and the canvas
    // colour range finite (auto range skips the non-finite bins).
    plot.SetZRange(false, false, 0.0, 0.0);
    {
        std::vector<double> nf(50 * 50);
        for (int i = 0; i < 50 * 50; i++)
            nf[size_t(i)] = double(i % 100);
        nf[0] = std::numeric_limits<double>::quiet_NaN();
        nf[1] = std::numeric_limits<double>::infinity();
        nf[2] = -std::numeric_limits<double>::infinity();
        plot.SetData(50, -0.5, 49.5, 50, -0.5, 49.5, nf.data());
        double qlo = 0.0, qhi = 0.0;
        plot.imageView()->colorRange(qlo, qhi);
        QVERIFY(std::isfinite(qlo) && std::isfinite(qhi));
        QVERIFY(qhi > qlo);
        QVERIFY(std::isfinite(plot.GetZMinimum()));
        QVERIFY(std::isfinite(plot.GetZMaximum()));
        QVERIFY(std::isfinite(plot.GetZMean()));
        (void)render(*plot.imageView());

        std::vector<double> allnf(50 * 50,
                                  std::numeric_limits<double>::quiet_NaN());
        plot.SetData(50, -0.5, 49.5, 50, -0.5, 49.5, allnf.data());
        plot.imageView()->colorRange(qlo, qhi);
        QVERIFY(near(qlo, 0.0) && near(qhi, 1.0));
    }

    plot.LogZ(false);
}

// ---------------------------------------------------------------------------
// 2D: ROI boxes, viewport/zoom, no paint outside the plot rect
// ---------------------------------------------------------------------------
void PlottingTest::twoDimensionalRoiAndClipping() {
    sls::SlsQt2DPlot plot;
    plot.resize(700, 600);
    plot.show();
    QTest::qWait(5);
    std::vector<double> d(100 * 80);
    for (int y = 0; y < 80; y++)
        for (int x = 0; x < 100; x++)
            d[size_t(y) * 100 + x] = double(x + y);
    plot.SetData(100, 0.0, 100.0, 80, 0.0, 80.0, d.data());
    plot.UnZoom();

    std::vector<slsDetectorDefs::ROI> rois{slsDetectorDefs::ROI(10, 29, 20, 39)};
    plot.EnableRoiBoxes(rois);
    const std::vector<QRectF> &ov = plot.imageView()->overlayRects();
    QCOMPARE(int(ov.size()), 1);
    QVERIFY(near(ov[0].left(), 10.0) && near(ov[0].top(), 20.0));
    QVERIFY(near(ov[0].width(), 20.0) && near(ov[0].height(), 20.0));
    QImage img = render(*plot.imageView());
    const QPointF roiCorner = plot.imageView()->dataToWidget(QPointF(10.0, 20.0));
    QVERIFY2(yellowNear(img, roiCorner.toPoint(), 3),
             "ROI box is painted in data coordinates");
    plot.DisableRoiBoxes();
    QVERIFY(plot.imageView()->overlayRects().empty());

    // Programmatic zoom: data rect -> image pixel-edge viewport.
    plot.SetZoom(QRectF(20.0, 20.0, 20.0, 20.0));
    double x = 0.0, y = 0.0, w = 0.0, h = 0.0;
    plot.imageView()->viewport(x, y, w, h);
    QVERIFY(near(x, 20.0) && near(y, 20.0));
    QVERIFY(near(w, 20.0) && near(h, 20.0));
    QVERIFY(plot.imageView()->hasImage());

    // Deep zoom: the canvas must still be clipped to its plot rect.
    img = render(*plot.imageView());
    const QRect ir =
        sls::plot::plotRectFor(plot.imageView()->size(), sls::plot::PlotTheme());
    for (int xx = ir.right() + 1; xx < img.width(); xx++) {
        for (int yy = 0; yy < img.height(); yy++) {
            QVERIFY2(img.pixel(xx, yy) == kBackground,
                     "zoomed 2D image must be clipped to the plot rect");
        }
    }

    // User zoom emits the data-coordinate viewport for the gain-plot sync.
    QSignalSpy spy(&plot, &sls::SlsQt2DPlot::PlotZoomedSignal);
    sendWheel(*plot.imageView(),
              QPointF(plot.imageView()->width() / 2.0,
                      plot.imageView()->height() / 2.0),
              120);
    QVERIFY(spy.count() >= 1);
    const QRectF zr = spy.takeLast().at(0).toRectF();
    QVERIFY(zr.width() > 0.0 && zr.height() > 0.0);
    QVERIFY(zr.width() <= 100.0 && zr.height() <= 80.0);
    plot.UnZoom();
}

// ---------------------------------------------------------------------------
// 2D: gain mode
// ---------------------------------------------------------------------------
void PlottingTest::twoDimensionalGainMode() {
    sls::SlsQt2DPlot gain(nullptr, true);
    gain.resize(200, 200);
    gain.show();
    QTest::qWait(5);
    QVERIFY(gain.isGainPlot());
    QVERIFY(gain.scalePanel() == nullptr);
    QVERIFY(gain.zTitleLabel() == nullptr);
    QVERIFY(!gain.imageView()->axesVisible());
    QVERIFY(!gain.imageView()->zoomEnabled());
    QCOMPARE(gain.imageView()->paletteIndex(),
             int(sls::plot::Palette::Gain4));
    double lo = 0.0, hi = 0.0;
    gain.imageView()->colorRange(lo, hi);
    QVERIFY(near(lo, 0.0) && near(hi, 3.0));

    // Four quadrants, one per gain bit (data layout: x fastest, y up).
    std::vector<double> d(8 * 8);
    for (int by = 0; by < 8; by++)
        for (int bx = 0; bx < 8; bx++) {
            const int g = (bx < 4 ? 0 : 1) + (by < 4 ? 2 : 0);
            d[size_t(bx) + size_t(by) * 8] = double(g);
        }
    gain.SetData(8, 0.0, 8.0, 8, 0.0, 8.0, d.data());
    QCOMPARE(gain.imageView()->imageWidth(), 8);
    QCOMPARE(gain.imageView()->imageHeight(), 8);
    const QImage img = render(*gain.imageView());
    const QRgb c0 = img.pixel(int(gain.imageView()->dataToWidget(QPointF(1.5, 1.5)).x()),
                              int(gain.imageView()->dataToWidget(QPointF(1.5, 1.5)).y()));
    const QRgb c1 = img.pixel(int(gain.imageView()->dataToWidget(QPointF(5.5, 1.5)).x()),
                              int(gain.imageView()->dataToWidget(QPointF(5.5, 1.5)).y()));
    const QRgb c2 = img.pixel(int(gain.imageView()->dataToWidget(QPointF(1.5, 5.5)).x()),
                              int(gain.imageView()->dataToWidget(QPointF(1.5, 5.5)).y()));
    const QRgb c3 = img.pixel(int(gain.imageView()->dataToWidget(QPointF(5.5, 5.5)).x()),
                              int(gain.imageView()->dataToWidget(QPointF(5.5, 5.5)).y()));
    // Data value = (x>=4 ? 1 : 0) + (y>=4 ? 2 : 0); the bottom half (small y)
    // adds 2, so the bottom-left cell is gain bit 2.
    const QRgb *gainLut = sls::plot::paletteLut(sls::plot::Palette::Gain4);
    QCOMPARE(c0, gainLut[170]); // value 2
    QCOMPARE(c1, gainLut[255]); // value 3
    QCOMPARE(c2, gainLut[0]);   // value 0
    QCOMPARE(c3, gainLut[85]);  // value 1
    // All four categorical colours are actually painted somewhere.
    QSet<QRgb> present;
    for (int yy = 0; yy < img.height(); yy++)
        for (int xx = 0; xx < img.width(); xx++)
            present.insert(img.pixel(xx, yy));
    for (int k : {0, 85, 170, 255})
        QVERIFY2(present.contains(gainLut[k]),
                 "all four gain categories are painted");
}

// ---------------------------------------------------------------------------
// acceptance benchmark: 1024x512 float frames at >= 20 Hz
// ---------------------------------------------------------------------------
void PlottingTest::performance1024x512() {
    sls::SlsQtFastImage view;
    view.resize(1100, 600);
    view.show();
    QTest::qWait(20);
    std::vector<float> frame = acceptanceFrame();
    view.setColorRange(-2.0, 2.0);
    view.setImageCopy(frame.data(), 1024, 512);
    view.repaint();

    // Warm up: first paints allocate the cached QImage and font paths.
    for (int i = 0; i < 5; i++) {
        frame[0] = float(i);
        view.setImageCopy(frame.data(), 1024, 512);
        view.repaint();
    }

    const int frames = 60;
    QElapsedTimer timer;
    timer.start();
    for (int i = 0; i < frames; i++) {
        frame[0] = float(i);
        view.setImageCopy(frame.data(), 1024, 512);
        view.repaint(); // synchronous: copy + compose + paint
    }
    const double seconds = double(timer.nsecsElapsed()) / 1e9;
    const double fps = double(frames) / seconds;
    qInfo("1024x512 benchmark: %.1f fps (%.2f ms/frame)", fps,
          1000.0 * seconds / frames);
    QVERIFY2(fps >= 20.0, "1024x512 frames must render at >= 20 fps");
}

QTEST_MAIN(PlottingTest)
#include "plotting_test.moc"
