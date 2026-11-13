// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2026 Contributors to the SLS Detector Package
#pragma once

#include "SlsQt2DHist.h"
#include "sls/sls_detector_defs.h"

#include <QRectF>
#include <QString>
#include <QWidget>

#include <vector>

class QLabel;

namespace sls {

class SlsQtFastImage;
class SlsQtScalePanel;

/**
 * 2D detector image: an SlsQtFastImage with an SlsQtScalePanel below it.
 *
 * Positions and ranges are in the coordinates given to SetData(), while the
 * image view itself works in pixels.
 *
 * With gain = true the plot shows the gain bits (0..3) in a fixed palette,
 * without axes, zoom and scale panel.
 */
class SlsQt2DPlot : public QWidget {
    Q_OBJECT

  public:
    explicit SlsQt2DPlot(QWidget *parent = nullptr, bool gain = false);

    void SetTitle(const QString &title);
    QString title() const;
    void SetXTitle(const QString &title);
    void SetYTitle(const QString &title);
    void SetZTitle(const QString &title);

    /** Copies nbinsx * nbinsy values from d, x fastest */
    void SetData(int nbinsx, double xmin, double xmax, int nbinsy, double ymin,
                 double ymax, const double *d);
    /** Applies pending axis and z ranges and repaints */
    void Update();

    /** Range of the data, not of the visible part */
    double GetXMinimum() const { return m_hist.GetXMin(); }
    double GetXMaximum() const { return m_hist.GetXMax(); }
    double GetYMinimum() const { return m_hist.GetYMin(); }
    double GetYMaximum() const { return m_hist.GetYMax(); }

    void DisableZoom(bool disable);
    void EnableXAutoScaling();
    void EnableYAutoScaling();
    void SetXMinMax(double min, double max);
    void SetYMinMax(double min, double max);

    void SetInterpolate(bool enable);
    void SetContour(bool enable);
    void SetLogz(bool enable, bool isMin, bool isMax, double min, double max);
    /** Fixes the enabled ends of the z range, the other ends follow the
     * data. A range picked on the scale panel is kept until an end is
     * fixed here or the image size changes. */
    void SetZRange(bool isMin, bool isMax, double min, double max);

    void EnableRoiBoxes(const std::vector<slsDetectorDefs::ROI> &roi);
    void DisableRoiBoxes();

    SlsQtFastImage *imageView() const { return m_view; }
    /** nullptr for a gain plot */
    SlsQtScalePanel *scalePanel() const { return m_panel; }

  public slots:
    void SetZoom(const QRectF &rect);

  signals:
    /** The user zoomed or panned */
    void PlotZoomedSignal(const QRectF &);

  private:
    void fillTestPlot();
    void applyPendingView();
    void updateContours();

    double dataXToImage(double x) const;
    double dataYToImage(double y) const;

    const bool m_gain;
    SlsQt2DHist m_hist;
    SlsQtFastImage *m_view{nullptr};
    SlsQtScalePanel *m_panel{nullptr};
    QLabel *m_zTitleLabel{nullptr};

    bool m_contourOn{false};
    bool m_contourDirty{false};

    // axis ranges requested through SetXMinMax() etc.
    bool m_autoX{true};
    bool m_autoY{true};
    bool m_applyViewPending{true};
    double m_reqX0{0.0}, m_reqX1{1.0}, m_reqY0{0.0}, m_reqY1{1.0};

    // who decides the z range, if not the data
    bool m_panelZOverride{false}; // picked on the scale panel
    bool m_userZOverride{false};  // fixed with SetZRange()
};

} // namespace sls
