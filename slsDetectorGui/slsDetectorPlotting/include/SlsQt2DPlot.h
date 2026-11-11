// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2026 Contributors to the SLS Detector Package
#pragma once

// Qt Widgets-only 2D plot: a sls::SlsQtFastImage canvas (image + axes + zoom /
// pan / ROI overlays) plus the sls::SlsQtScalePanel (histogram strip, palette,
// lin/log and colour range).  It replaces the previous QwtPlot-based class and
// keeps the public API qDrawPlot/qTabPlot/qCloneWidget use; the only call-site
// change is title()/setTitle() instead of the QwtPlot title text.
//
// No qwt/ROOT/OpenGL: painting lives in SlsQtFastImage, contours and the
// bilinear overlay are painted through its clipped onPaintOverlay() hook.

#include "SlsQt2DHist.h"
#include "sls/sls_detector_defs.h"

#include <QFont>
#include <QImage>
#include <QPointF>
#include <QRectF>
#include <QRgb>
#include <QString>
#include <QWidget>

#include <vector>

class QLabel;
class QPainter;
class QTransform;

namespace sls {

class SlsQtFastImage;
class SlsQtScalePanel;

class SlsQt2DPlot : public QWidget {
    Q_OBJECT

  public:
    explicit SlsQt2DPlot(QWidget *parent = nullptr, bool gain = false);
    ~SlsQt2DPlot() override;

    // ---- title / axis titles ---------------------------------------------
    void SetTitle(QString title);
    void SetXTitle(QString title);
    void SetYTitle(QString title);
    void SetZTitle(QString title);
    // qDrawPlot/qCloneWidget use these instead of QwtPlot::title()/setTitle().
    void setTitle(const QString &title);
    QString title() const;
    void SetTitleFont(const QFont &f);
    void SetXFont(const QFont &f);
    void SetYFont(const QFont &f);
    void SetZFont(const QFont &f);

    // ---- interaction / viewport ------------------------------------------
    void UnZoom(bool replot = true);
    // x_width/y_width are the viewport size in data coordinates.
    void SetZoom(double xmin, double ymin, double x_width, double y_width);
    void DisableZoom(bool disable);
    void EnableXAutoScaling();
    void EnableYAutoScaling();
    void SetXMinMax(double min, double max);
    void SetYMinMax(double min, double max);

    // ---- full data ranges (NOT the current viewport: qTabPlot uses these to
    // compute the aspect ratio) ---------------------------------------------
    double GetXMinimum() { return m_hist->GetXMin(); }
    double GetXMaximum() { return m_hist->GetXMax(); }
    double GetYMinimum() { return m_hist->GetYMin(); }
    double GetYMaximum() { return m_hist->GetYMax(); }
    double GetZMinimum() { return m_hist->GetMinimum(); }
    double GetZMaximum() { return m_hist->GetMaximum(); }
    double GetZMean() { return m_hist->GetMean(); }

    // ---- z (colour) range -------------------------------------------------
    void SetZMinMax(double zmin = 0, double zmax = -1);
    void SetZMinimumToFirstGreaterThanZero();

    // ---- data -------------------------------------------------------------
    void SetData(int nbinsx, double xmin, double xmax, int nbinsy, double ymin,
                 double ymax, double *d, double zmin = 0, double zmax = -1);
    double *GetDataPtr() { return m_hist->GetDataPtr(); }
    int GetBinIndex(int bx, int by) { return m_hist->GetBinIndex(bx, by); }
    int FindBinIndex(double x, double y) {
        return m_hist->FindBinIndex(x, y);
    }
    void SetBinValue(int bx, int by, double v);
    double GetBinValue(int bx, int by) { return m_hist->GetBinValue(bx, by); }
    void FillTestPlot(int i = 0);
    // Re-applies the pending view/z state and repaints.
    void Update();

    // ---- rendering options ------------------------------------------------
    void SetInterpolate(bool enable);
    void SetContour(bool enable);
    void SetLogz(bool enable, bool isMin, bool isMax, double min, double max);
    void SetZRange(bool isMin, bool isMax, double min, double max);
    void LogZ(bool on = 1);

    // ---- ROI boxes (all receiver ROIs, yellow overlays) -------------------
    void EnableRoiBoxes(std::vector<slsDetectorDefs::ROI> roi);
    void DisableRoiBoxes();

    // Extra accessors used by the integration/verification code.
    SlsQtFastImage *imageView() const { return m_view; }
    SlsQtScalePanel *scalePanel() const { return m_panel; }
    bool isGainPlot() const { return m_gain; }
    // Compact z-axis (colour scale) title shown above the scale panel; nullptr
    // in gain mode.  Kept as a plain QLabel so the integration tests can check
    // it without touching the foundation ScalePanel.
    QLabel *zTitleLabel() const { return m_zTitleLabel; }

  public slots:
    void showSpectrogram(bool on);
    void SetZoom(const QRectF &rect);

  signals:
    void PlotZoomedSignal(const QRectF &);

  private:
    void setupUi();
    void pushImage();
    void applyPendingView();
    void applyZRange(double lo, double hi);
    void onViewZoomed(double x1, double y1, double x2, double y2);
    void rebuildSmoothImage();
    void updateContours();
    void paintOverlay(QPainter &p, const QTransform &t);
    void drawRois(QPainter &p, const QTransform &t) const;

    // Data (ROOT/qwt bin-centre) coordinates <-> SlsQtFastImage pixel-edge
    // coordinates (pixel ix covers [ix, ix+1), so the conversion is +0.5 for
    // the usual -0.5..nx-0.5 / unit-width bin geometry of qDrawPlot).
    double dataXToImage(double x) const;
    double dataYToImage(double y) const;
    double imageXToData(double u) const;
    double imageYToData(double v) const;

    struct ContourSegment {
        QPointF a;
        QPointF b;
    };

    bool m_gain{false};
    SlsQt2DHist *m_hist{nullptr};
    SlsQtFastImage *m_view{nullptr};
    SlsQtScalePanel *m_panel{nullptr};

    bool m_logZ{false};
    bool m_interp{false};
    bool m_contourOn{false};
    bool m_showImage{true};
    bool m_zoomDisabled{false};

    // Pending programmatic viewport (Enable*AutoScaling/SetXMinMax/...).
    bool m_autoX{true};
    bool m_autoY{true};
    bool m_applyViewPending{false};
    double m_reqX0{0.0}, m_reqX1{1.0}, m_reqY0{0.0}, m_reqY1{1.0};

    // True after the scale panel changed the colour range: the per-frame
    // auto z range must not overwrite the user's choice.
    bool m_panelZOverride{false};
    // True while SetZRange()/SetLogz()/SetZMinMax() pinned an explicit range:
    // Update() must not re-derive the auto range (not even the log minimum)
    // until SetZRange(false, false, ...) clears it.
    bool m_userZOverride{false};
    // Guards the onColorRangeChanged() callback while the plot sets the range.
    bool m_syncingZ{false};

    QRgb m_palette[256];
    QImage m_smooth; // bilinear ("interpolate") overlay, data resolution
    bool m_smoothDirty{true};

    std::vector<ContourSegment> m_contourSegments;
    bool m_contourDirty{true};

    // Yellow ROI boxes in image pixel-edge coordinates; kept so they can be
    // redrawn above the smooth overlay.
    std::vector<QRectF> m_roiRects;

    std::vector<double> m_testData; // storage for FillTestPlot()
    QFont m_zFont;
    QString m_zTitle;
    QLabel *m_zTitleLabel{nullptr};
    int m_lastNx{0};
    int m_lastNy{0};
};

} // namespace sls