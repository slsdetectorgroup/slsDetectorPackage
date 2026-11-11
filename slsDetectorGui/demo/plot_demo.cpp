// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2026 Contributors to the SLS Detector Package
//
// Standalone offscreen demo / benchmark for the Qt6-only plotting stack.
//
// It builds a synthetic 1024x512 float frame (sin(x/40)+cos(y/30) plus a moving
// hotspot) at ~20 Hz, feeds it to SlsQtFastImage, keeps SlsQtScalePanel in sync
// and shows a 1D plot with three live curves below it.  It prints the
// measured timer rate plus the unthrottled render throughput.  --png saves a
// screenshot; --shot prepares one of the acceptance views:
//
//   2d-default    2D image + scale panel
//   2d-zoom-roi   64x30 pixel zoom with gate/overlay/receiver ROIs
//   scale-drag    mid-drag ScaleHisto strip (frozen bars, dashed preview)
//   1d-log        the real SlsQt1DPlot with three curves on a log y axis
//   gain          the real SlsQt2DPlot gain-mode widget (Gain4, 0..3)
//
//   QT_QPA_PLATFORM=offscreen ./bin/plot_demo --frames 120
//   QT_QPA_PLATFORM=offscreen ./bin/plot_demo --shot 1d-log \
//       --png ../artifacts/1d-log.png
//
// CLI: --frames N (0 = run until closed), --png PATH, --shot MODE.

#include "SlsQt1DPlot.h"
#include "SlsQt2DPlot.h"
#include "SlsQtFastImage.h"
#include "SlsQtPlotCommon.h"
#include "SlsQtScalePanel.h"

#include <QApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QElapsedTimer>
#include <QMouseEvent>
#include <QPixmap>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <vector>

using sls::SlsQt1DPlot;
using sls::SlsQt2DPlot;
using sls::SlsQtFastImage;
using sls::SlsQtH1D;
using sls::SlsQtScalePanel;

namespace {

constexpr int kNx = 1024;
constexpr int kNy = 512;
constexpr int kTimerMs = 50; // ~20 Hz
constexpr int kShotFrames = 8;
constexpr int kCurveBins = 256;

void sendMouse(QWidget &w, QEvent::Type type, const QPointF &pos,
               Qt::MouseButton button, Qt::MouseButtons buttons) {
    QMouseEvent e(type, pos, QPointF(w.mapToGlobal(pos.toPoint())), button,
                  buttons, Qt::NoModifier);
    QCoreApplication::sendEvent(&w, &e);
}

struct Options {
    int frames = 120;
    QString png;
    QString shot;
};

} // namespace

class PlotDemo : public QWidget {
  public:
    explicit PlotDemo(const Options &opt) : QWidget(nullptr), m_opt(opt) {
        m_frame.resize(size_t(kNx) * size_t(kNy));
        m_frameD.resize(size_t(kNx) * size_t(kNy));
        m_c1.resize(kCurveBins);
        m_c2.resize(kCurveBins);
        m_c3.resize(kCurveBins);
        buildUi();
    }

    ~PlotDemo() override {
        // The curves must go before the QWidget base class destroys the plot
        // they are attached to.
        for (SlsQtH1D *c : m_curves)
            delete c;
        m_curves.clear();
    }

    int run() {
        resize(1100, 900);
        setWindowTitle(QStringLiteral("slsDetectorPlotting Qt6 demo"));
        show();

        m_targetFrames = m_opt.frames;
        connect(&m_timer, &QTimer::timeout, this, [this] { tick(); });
        connect(qApp, &QCoreApplication::aboutToQuit, this,
                [this] { printStats(); });
        m_timer.start(kTimerMs);
        return QApplication::exec();
    }

  private:
    void buildUi() {
        auto *root = new QVBoxLayout(this);
        root->setContentsMargins(0, 0, 0, 0);
        root->setSpacing(0);

        // 2D image + scale panel (the acceptance demo path).
        m_2dRow = new QWidget(this);
        auto *row = new QVBoxLayout(m_2dRow);
        row->setContentsMargins(0, 0, 0, 0);
        row->setSpacing(0);
        m_view = new SlsQtFastImage(m_2dRow);
        m_panel = new SlsQtScalePanel(m_view, m_2dRow);
        row->addWidget(m_view, 1);
        row->addWidget(m_panel);

        // 1D plot with three curves (the real widget).
        m_1d = new SlsQt1DPlot(this);
        m_1d->SetTitle(QStringLiteral("1D histograms"));
        m_1d->SetXTitle(QStringLiteral("bin"));
        m_1d->SetYTitle(QStringLiteral("counts"));
        for (int i = 0; i < 3; i++) {
            auto *h = new SlsQtH1D(QStringLiteral("curve %1").arg(i + 1),
                                   kCurveBins, 0.0, double(kCurveBins),
                                   nullptr);
            h->Attach(m_1d);
            h->SetLineColor(i);
            h->SetLineWidth(2);
            m_curves.push_back(h);
        }

        // Real SlsQt2DPlot used for the 2D acceptance screenshots (title,
        // axis titles, z-axis caption, scale panel): hidden in the live demo.
        m_2d = new SlsQt2DPlot(this);
        m_2d->hide();

        // Real SlsQt2DPlot gain-mode widget (hidden until --shot gain).
        m_gain = new SlsQt2DPlot(this, true);
        m_gain->hide();
        m_gainData.resize(size_t(kNx) * size_t(kNy));

        root->addWidget(m_2dRow, 2);
        root->addWidget(m_1d, 1);
        root->addWidget(m_2d, 2);
        root->addWidget(m_gain, 2);
    }

    void tick() {
        const bool first = !m_started;
        if (first) {
            // Benchmark while the 2D view is visible and laid out.
            (void)benchmark();
            m_started = true;
            setupShot();
        }
        m_phase += 0.08;
        buildFrame();
        if (m_2dRow->isVisible()) {
            m_view->setImageCopy(m_frame.data(), kNx, kNy);
            m_view->repaint(); // compose + paint, like the GUI acquisition loop
        }
        if (m_2d->isVisible()) {
            // Real SlsQt2DPlot path: the GUI feeds double frames.
            for (size_t i = 0; i < m_frame.size(); i++)
                m_frameD[i] = double(m_frame[i]);
            m_2d->SetData(kNx, -0.5, kNx - 0.5, kNy, -0.5, kNy - 0.5,
                          m_frameD.data());
            m_2d->imageView()->repaint(); // sync compose: histogram/panel too
        }
        if (m_1d->isVisible())
            update1d();
        if (first) {
            applyShotGeometry();
            m_clock.start();
            m_frames = 0; // measure the timer rate from here on
        }
        m_frames++;

        if (m_opt.shot == QLatin1String("scale-drag") && !m_shotPrepared &&
            m_frames >= 4) {
            startStripDrag();
            m_shotPrepared = true;
        }
        if (m_targetFrames > 0 && m_frames >= m_targetFrames)
            finish();
    }

    // One-shot unthrottled measurement: copy + compose + paint per frame.
    double benchmark() {
        const int frames = 40;
        QElapsedTimer t;
        t.start();
        for (int i = 0; i < frames; i++) {
            m_frame[0] = float(i);
            m_view->setImageCopy(m_frame.data(), kNx, kNy);
            m_view->repaint();
        }
        const double seconds = double(t.nsecsElapsed()) / 1e9;
        const double fps = double(frames) / seconds;
        std::printf(
            "plot_demo: render throughput %.1f fps (%.2f ms/frame, %d frames, "
            "%dx%d)\n",
            fps, 1000.0 * seconds / double(frames), frames, kNx, kNy);
        std::fflush(stdout);
        return fps;
    }

    void setupShot() {
        m_view->setTitle(QStringLiteral("sin(x/40) + cos(y/30)  |  1024x512"));
        m_view->setXAxisTitle(QStringLiteral("x [px]"));
        m_view->setYAxisTitle(QStringLiteral("y [px]"));
        m_view->setColorRange(-2.0, 2.0);

        if (m_opt.shot == QLatin1String("1d-log")) {
            m_2dRow->hide();
            m_gain->hide();
            m_1d->show();
            resize(1100, 640);
            m_1d->SetTitle(QStringLiteral("1D histograms  |  log y"));
            m_1d->SetLogY(true);
            m_1d->SetLogX(false);
        } else if (m_opt.shot == QLatin1String("gain")) {
            m_2dRow->hide();
            m_1d->hide();
            m_gain->show();
            resize(kNx, kNy);
        } else if (m_opt.shot == QLatin1String("2d-default") ||
                   m_opt.shot == QLatin1String("2d-zoom-roi") ||
                   m_opt.shot == QLatin1String("scale-drag")) {
            // Real SlsQt2DPlot: canvas + scale panel + z-axis caption row.
            m_2dRow->hide();
            m_1d->hide();
            m_gain->hide();
            m_2d->show();
            resize(1100, 660);
            m_2d->SetTitle(
                QStringLiteral("sin(x/40) + cos(y/30)  |  1024x512"));
            m_2d->SetXTitle(QStringLiteral("x [px]"));
            m_2d->SetYTitle(QStringLiteral("y [px]"));
            m_2d->SetZTitle(QStringLiteral("z (colour scale)"));
        } else {
            m_2dRow->show();
            m_1d->hide();
            m_2d->hide();
            m_gain->hide();
            resize(1100, 640);
            if (!m_opt.shot.isEmpty()) {
                std::fprintf(stderr, "plot_demo: unknown --shot '%s'\n",
                             qPrintable(m_opt.shot));
            }
        }
    }

    // Applied after the first frame: setImageCopy() of a new image resets the
    // viewport, so zoom/ROI/gain data must be set once an image exists.
    void applyShotGeometry() {
        if (m_opt.shot == QLatin1String("2d-zoom-roi")) {
            // Data coords of SlsQt2DPlot are bin centres: -0.5..nx-0.5 with
            // unit bins, i.e. image pixel edges 400..460 / 200..230.
            m_2d->SetZoom(QRectF(399.5, 199.5, 60.0, 30.0));
            m_2d->EnableRoiBoxes({slsDetectorDefs::ROI(410, 429, 210, 224),
                                  slsDetectorDefs::ROI(446, 451, 218, 225)});
            m_2d->imageView()->repaint();
        } else if (m_opt.shot == QLatin1String("gain")) {
            // Real gain widget: 0..3 gain bits in four quadrants plus a
            // gain-switching block (as on a Jungfrau chip).
            for (int y = 0; y < kNy; y++) {
                for (int x = 0; x < kNx; x++) {
                    int g = (x < kNx / 2 ? 0 : 1) + (y < kNy / 2 ? 2 : 0);
                    if (x > 3 * kNx / 4 && y > 7 * kNy / 8)
                        g = (g + 1) % 4;
                    m_gainData[size_t(y) * kNx + x] = double(g);
                }
            }
            m_gain->SetData(kNx, -0.5, kNx - 0.5, kNy, -0.5, kNy - 0.5,
                            m_gainData.data());
            m_gain->UnZoom();
            m_gain->Update();
        }
    }

    void update1d() {
        const double centre1 =
            kCurveBins / 2.0 + 40.0 * std::sin(m_phase * 0.6);
        const double centre2 =
            kCurveBins / 2.0 - 40.0 * std::sin(m_phase * 0.6);
        for (int i = 0; i < kCurveBins; i++) {
            const double x = i;
            m_c1[size_t(i)] =
                800.0 * std::exp(-std::pow(x - centre1, 2.0) / (2 * 18 * 18)) +
                5.0;
            m_c2[size_t(i)] =
                500.0 * std::exp(-std::pow(x - centre2, 2.0) / (2 * 28 * 28)) +
                15.0;
            m_c3[size_t(i)] = 20.0 + 8.0 * std::sin(x / 7.0 + m_phase) + 1.0;
        }
        m_curves[0]->SetData(kCurveBins, 0.0, double(kCurveBins), m_c1.data());
        m_curves[1]->SetData(kCurveBins, 0.0, double(kCurveBins), m_c2.data());
        m_curves[2]->SetData(kCurveBins, 0.0, double(kCurveBins), m_c3.data());
        m_1d->Update();
    }

    void buildFrame() {
        const double cx = 512.0 + 220.0 * std::sin(m_phase);
        const double cy = 256.0 + 120.0 * std::cos(m_phase * 0.7);
        for (int y = 0; y < kNy; y++) {
            for (int x = 0; x < kNx; x++) {
                const double base = std::sin(x / 40.0) + std::cos(y / 30.0);
                const double dx = x - cx, dy = y - cy;
                const double hotspot =
                    1.2 * std::exp(-(dx * dx + dy * dy) / (2.0 * 70.0 * 70.0));
                m_frame[size_t(y) * kNx + x] = float(base + hotspot);
            }
        }
    }

    SlsQtScalePanel *scalePanel() const {
        return m_2d && m_2d->isVisible() ? m_2d->scalePanel() : m_panel;
    }

    void startStripDrag() {
        SlsQtScalePanel *panel = scalePanel();
        if (!panel)
            return;
        auto *strip = panel->histogramWidget();
        if (!strip || strip->width() <= 0)
            return;
        const double w = strip->width();
        const QPointF press(w * 0.25, strip->height() * 0.5);
        const QPointF move(w * 0.42, strip->height() * 0.5);
        sendMouse(*strip, QEvent::MouseButtonPress, press, Qt::LeftButton,
                  Qt::LeftButton);
        sendMouse(*strip, QEvent::MouseMove, move, Qt::NoButton,
                  Qt::LeftButton);
        m_stripDragging = strip->isDragging();
        std::printf("plot_demo: ScaleHisto drag prepared (dragging=%d, "
                    "previewX=%d)\n",
                    m_stripDragging ? 1 : 0, strip->previewX());
    }

    void endStripDrag() {
        if (!m_stripDragging)
            return;
        SlsQtScalePanel *panel = scalePanel();
        if (!panel)
            return;
        auto *strip = panel->histogramWidget();
        if (!strip)
            return;
        const double w = strip->width();
        sendMouse(*strip, QEvent::MouseButtonRelease,
                  QPointF(w * 0.42, strip->height() * 0.5), Qt::LeftButton,
                  Qt::NoButton);
        m_stripDragging = false;
    }

    void finish() {
        // The drag screenshot must be taken before the release applies it.
        if (!m_opt.png.isEmpty())
            savePng();
        endStripDrag();
        printStats();
        m_timer.stop();
        QApplication::quit();
    }

    void savePng() {
        const QPixmap shot = grab();
        if (shot.isNull()) {
            std::fprintf(stderr, "plot_demo: could not grab the window\n");
            return;
        }
        if (!shot.save(m_opt.png, "PNG")) {
            std::fprintf(stderr, "plot_demo: failed to save %s\n",
                         qPrintable(m_opt.png));
            return;
        }
        std::printf("plot_demo: saved %s (%dx%d)\n", qPrintable(m_opt.png),
                    shot.width(), shot.height());
        std::fflush(stdout);
        m_pngSaved = true;
    }

    void printStats() {
        if (m_statsPrinted)
            return;
        m_statsPrinted = true;
        if (!m_started || m_frames <= 0) {
            std::printf("plot_demo: no frames rendered\n");
            return;
        }
        // m_clock starts after the first frame, so (m_frames - 1) intervals
        // were measured.
        const double seconds = double(m_clock.nsecsElapsed()) / 1e9;
        const double fps =
            (m_frames > 1 && seconds > 0.0) ? double(m_frames - 1) / seconds : 0.0;
        std::printf("plot_demo: %d timer frames, %.3f s between first/last = "
                    "%.1f fps (QTimer target %d ms)\n",
                    m_frames, seconds, fps, kTimerMs);
        std::fflush(stdout);
    }

    Options m_opt;
    QWidget *m_2dRow = nullptr;
    SlsQtFastImage *m_view = nullptr;
    SlsQtScalePanel *m_panel = nullptr;
    SlsQt1DPlot *m_1d = nullptr;
    SlsQt2DPlot *m_2d = nullptr;
    SlsQt2DPlot *m_gain = nullptr;
    QTimer m_timer;

    QElapsedTimer m_clock;
    std::vector<float> m_frame;
    std::vector<double> m_frameD;
    std::vector<double> m_gainData;
    std::vector<double> m_c1, m_c2, m_c3;
    std::vector<SlsQtH1D *> m_curves;
    double m_phase = 0.0;
    int m_frames = 0;
    int m_targetFrames = 120;
    bool m_started = false;
    bool m_shotPrepared = false;
    bool m_stripDragging = false;
    bool m_statsPrinted = false;
    bool m_pngSaved = false;
};

int main(int argc, char **argv) {
    QApplication app(argc, argv);
    QCoreApplication::setApplicationName(QStringLiteral("plot_demo"));
    QCoreApplication::setApplicationVersion(QStringLiteral("1.0"));

    QCommandLineParser parser;
    parser.setApplicationDescription(
        QStringLiteral("Qt6-only plotting stack demo / offline benchmark"));
    parser.addHelpOption();
    parser.addVersionOption();
    const QCommandLineOption framesOpt(
        QStringList{QStringLiteral("f"), QStringLiteral("frames")},
        QStringLiteral("number of timer frames before exiting (0 = run until "
                       "the window is closed)"),
        QStringLiteral("N"), QStringLiteral("120"));
    const QCommandLineOption pngOpt(
        QStringLiteral("png"),
        QStringLiteral("save a screenshot to PATH when the run finishes"),
        QStringLiteral("PATH"));
    const QCommandLineOption shotOpt(
        QStringLiteral("shot"),
        QStringLiteral("prepare an acceptance view: 2d-default, 2d-zoom-roi, "
                       "scale-drag, 1d-log, gain"),
        QStringLiteral("MODE"));
    parser.addOption(framesOpt);
    parser.addOption(pngOpt);
    parser.addOption(shotOpt);
    parser.process(app);

    Options opt;
    opt.frames = parser.value(framesOpt).toInt();
    opt.png = parser.value(pngOpt);
    opt.shot = parser.value(shotOpt);
    if (opt.frames < 0)
        opt.frames = 0;
    // A shot needs a few frames for the histogram/panel to settle.
    if (!opt.shot.isEmpty() && opt.frames > 0 && !parser.isSet(framesOpt))
        opt.frames = kShotFrames;
    if (opt.frames == 0 && !opt.png.isEmpty()) {
        std::fprintf(stderr,
                     "plot_demo: --png needs a finite --frames value\n");
        return 2;
    }

    std::printf("plot_demo: Qt %s, platform '%s', frame %dx%d\n",
                qVersion(), qPrintable(QGuiApplication::platformName()), kNx,
                kNy);
    std::fflush(stdout);

    PlotDemo demo(opt);
    const int rc = demo.run();
    return rc;
}
