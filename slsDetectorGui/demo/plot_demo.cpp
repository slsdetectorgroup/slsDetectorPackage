// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2026 Contributors to the SLS Detector Package

// Shows the plots of the gui with synthetic frames, no detector needed.
//
//   plot_demo                          until the window is closed
//   plot_demo --zoom 500,250,12,8      start with these pixels x,y,width,height
//   QT_QPA_PLATFORM=offscreen plot_demo --png demo.png

#include "SlsQt1DPlot.h"
#include "SlsQt2DPlot.h"

#include <QApplication>
#include <QCommandLineParser>
#include <QTimer>
#include <QVBoxLayout>

#include <cmath>
#include <memory>
#include <vector>

namespace {

constexpr int kNx = 1024;
constexpr int kNy = 512;
constexpr int kFrameIntervalMs = 50;

class PlotDemo : public QWidget {
  public:
    PlotDemo() {
        m_plot2d = new sls::SlsQt2DPlot(this);
        m_plot2d->SetTitle(QStringLiteral("Synthetic frames"));
        m_plot2d->SetXTitle(QStringLiteral("x [pixel]"));
        m_plot2d->SetYTitle(QStringLiteral("y [pixel]"));
        m_plot2d->SetZTitle(QStringLiteral("counts"));

        m_plot1d = new sls::SlsQt1DPlot(this);
        m_plot1d->SetTitle(QStringLiteral("Mean of the columns"));
        m_plot1d->SetXTitle(QStringLiteral("x [pixel]"));
        m_plot1d->SetYTitle(QStringLiteral("counts"));
        m_columns =
            std::make_unique<sls::SlsQtH1D>(QString(), kNx, -0.5, kNx - 0.5);
        m_columns->setLegendVisible(false);
        m_columns->Attach(m_plot1d);

        auto *layout = new QVBoxLayout(this);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->addWidget(m_plot2d, 2);
        layout->addWidget(m_plot1d, 1);
        nextFrame();
    }

    sls::SlsQt2DPlot *plot2d() const { return m_plot2d; }

    // Counts like from a photon counting detector, with a spot moving around
    void nextFrame() {
        m_phase += 0.08;
        const double spotX = kNx / 2 + 220 * std::sin(m_phase);
        const double spotY = kNy / 2 + 120 * std::cos(0.7 * m_phase);
        std::vector<double> frame(static_cast<size_t>(kNx) * kNy);
        std::vector<double> columns(kNx, 0.0);
        for (int y = 0; y < kNy; ++y) {
            for (int x = 0; x < kNx; ++x) {
                const double waves =
                    2 + std::sin(x / 40.0) + std::cos(y / 30.0);
                const double r2 =
                    std::pow(x - spotX, 2) + std::pow(y - spotY, 2);
                const double counts = std::floor(
                    40 * waves + 300 * std::exp(-r2 / (2 * 70.0 * 70.0)));
                frame[static_cast<size_t>(y) * kNx + x] = counts;
                columns[x] += counts / kNy;
            }
        }
        m_plot2d->SetData(kNx, -0.5, kNx - 0.5, kNy, -0.5, kNy - 0.5,
                          frame.data());
        m_columns->SetData(kNx, -0.5, kNx - 0.5, columns.data());
        m_plot1d->Update();
    }

  private:
    sls::SlsQt2DPlot *m_plot2d;
    sls::SlsQt1DPlot *m_plot1d;
    std::unique_ptr<sls::SlsQtH1D> m_columns;
    double m_phase = 0.0;
};

} // namespace

int main(int argc, char **argv) {
    QApplication app(argc, argv);
    QCommandLineParser parser;
    parser.setApplicationDescription(
        QStringLiteral("Shows the plots of the gui with synthetic frames."));
    parser.addHelpOption();
    parser.addOptions({
        {QStringLiteral("zoom"),
         QStringLiteral("Start with these pixels of the image."),
         QStringLiteral("x,y,width,height")},
        {QStringLiteral("png"),
         QStringLiteral("Save the window to <file> and exit."),
         QStringLiteral("file")},
        {QStringLiteral("frames"),
         QStringLiteral("Exit after <n> frames, 5 with --png."),
         QStringLiteral("n")},
    });
    parser.process(app);
    const QString png = parser.value(QStringLiteral("png"));
    int framesLeft = parser.value(QStringLiteral("frames")).toInt();
    if (framesLeft <= 0 && !png.isEmpty())
        framesLeft = 5;

    PlotDemo demo;
    demo.resize(1100, 900);
    demo.show();
    const QStringList zoom = parser.value(QStringLiteral("zoom"))
                                 .split(QLatin1Char(','), Qt::SkipEmptyParts);
    if (zoom.size() == 4) {
        // pixel ix covers ix - 0.5 to ix + 0.5 on the axes
        demo.plot2d()->SetZoom(QRectF(zoom[0].toDouble() - 0.5,
                                      zoom[1].toDouble() - 0.5,
                                      zoom[2].toDouble(), zoom[3].toDouble()));
    }

    QTimer timer;
    QObject::connect(&timer, &QTimer::timeout, &demo, [&] {
        demo.nextFrame();
        if (framesLeft > 0 && --framesLeft == 0) {
            if (!png.isEmpty())
                demo.grab().save(png);
            app.quit();
        }
    });
    timer.start(kFrameIntervalMs);
    return app.exec();
}
