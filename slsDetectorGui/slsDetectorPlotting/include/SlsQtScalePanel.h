// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2026 Contributors to the SLS Detector Package
#pragma once

// Thin histogram strip + scale panel of the plotting stack: the
// strip shows the distribution of the last frame over the window the colour
// mapping currently shows and carries the two draggable range ends; the panel
// owns the palette chooser and the image/histogram lin-log switches.  The
// colour min/max live in the Plot tab, not here.  See AGENT_INSTRUCTIONS.md:
// "the three-way freeze rule".

#include "SlsQtPlotCommon.h"

#include <QMetaObject>
#include <QPointer>
#include <QWidget>

#include <functional>
#include <vector>

class QComboBox;

namespace sls {

class SlsQtFastImage;

// The histogram strip (fixed height ~38 px).  The two markers sit at 1/4 and
// 3/4 of the width and the value axis rescales under them: the window shown is
// the mapped range widened by half a range on either side (in log space when
// logX is on).
//
// FREEZE RULE: while a marker is dragged the strip does not change at all.
//  * the axis window is the one captured at press (see displayedWindow()),
//  * the painted bars are a snapshot of the histogram taken at press
//    (see displayedHistogram()),
//  * the cursor->value mapping goes through that pressed-time window, so the
//    marker cannot chase itself.
// Only the dashed preview line moves.  The new range is applied once, on
// release (onRangeChanged), and the strip is refreshed afterwards.
class SlsQtScaleHisto : public QWidget {
    Q_OBJECT

  public:
    explicit SlsQtScaleHisto(QWidget *parent = nullptr);

    // h: live 256-bin histogram (owned by the image view); dlo/dhi: bounds of
    // those bins in their own binning space (log10 space when logX); clo/chi:
    // colour range in value space; logX: the value axis is log; logY: the bar
    // heights are log10(count + 1).
    void setData(const std::vector<int> *h, double dlo, double dhi, double clo,
                 double chi, bool logX, bool logY);

    // Drops the raw pointer to the view's histogram vector (and any frozen
    // snapshot).  Called when the panel detaches from a view or when the view
    // is destroyed, so a later repaint cannot read freed memory.
    void clearData();

    // Reports the new mapped range (value space) once, when the button is
    // released.
    std::function<void(double, double)> onRangeChanged;

    // ---- test accessors ---------------------------------------------------
    bool isDragging() const;
    // The live mapped colour range (value space).
    void currentWindow(double &lo, double &hi) const;
    // The axis window currently shown/painted, in bin space.  Frozen while
    // dragging.
    void displayedWindow(double &lo, double &hi) const;
    // The bars currently painted: the press-time snapshot while dragging.
    const std::vector<int> *displayedHistogram() const;
    int previewX() const;

  protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

  private:
    void windowFor(double clo, double chi, double &wlo, double &whi) const;

    const std::vector<int> *m_hist = nullptr;
    double m_dlo = 0.0, m_dhi = 1.0;
    double m_clo = 0.0, m_chi = 1.0;
    bool m_logX = false, m_logY = false;

    int m_dragMin = -1; // -1 none, 1 min, 0 max
    bool m_dragging = false;
    double m_preClo = 0.0, m_preChi = 1.0; // axis basis at press
    std::vector<int> m_snap;               // bars snapshot at press
    int m_previewX = 0;
};

// Histogram strip + palette combo + image lin/log + histogram lin/log.
// push() = controls -> view, refresh() = view -> controls, with a
// re-entrancy guard; refresh() is skipped while the strip is being dragged and
// the range is applied on release.  The z min/max are edited in the Plot tab.
class SlsQtScalePanel : public QWidget {
    Q_OBJECT

  public:
    explicit SlsQtScalePanel(SlsQtFastImage *view = nullptr,
                             QWidget *parent = nullptr);
    ~SlsQtScalePanel() override;

    // Attaches to a view; preserves callbacks that were already installed on
    // it (the panel chains onImageChanged / onScaleReleased).
    void setView(SlsQtFastImage *view);
    SlsQtFastImage *view() const;

    void refresh();      // view -> controls
    void syncFromView(); // alias of refresh()
    void push();         // controls -> view

    // Convenience API for the 2D plot: keep the controls in sync while the
    // plot changes the view programmatically.
    void setPalette(plot::Palette p); // also handles Gain4 (not in the chooser)
    void setPaletteChoice(int index);
    int paletteChoice() const;
    void setImageLog(bool on);
    void setColorRange(double lo, double hi);
    void setHistoLog(bool on);
    bool histoLog() const;
    void setPaletteEnabled(bool on);

    SlsQtScaleHisto *histogramWidget() const;
    bool isDragging() const; // strip drag in progress

    // ---- test / programmatic hooks ---------------------------------------
    // The colour range last read from / written to the view.  Stays frozen
    // while the strip marker is dragged (R1).
    void controlRange(double &lo, double &hi) const;
    // Applies a colour range as a user edit would: this is the only path that
    // pushes the colour range from the panel controls, so a test can trigger
    // one without synthesising events.
    void setControlRange(double lo, double hi);
    // Range last pushed to the view by a control edit; false if no edit was
    // pushed yet.  Palette/log changes never update this (R2).
    bool lastPushedRange(double &lo, double &hi) const;

  private:
    void attachView(SlsQtFastImage *view);
    void detachView();
    void updateHisto();

    QPointer<SlsQtFastImage> m_view;
    SlsQtScaleHisto *m_histo = nullptr;
    QComboBox *m_paletteCombo = nullptr;
    QComboBox *m_imageLogCombo = nullptr;
    QComboBox *m_histoLogCombo = nullptr;
    // The colour range last read from the view (kept for the R1 freeze rule
    // and for programmatic/test edits; there are no min/max spin boxes here).
    double m_controlLo = 0.0;
    double m_controlHi = 1.0;
    int m_suppress = 0;

    // R2: only an explicit control edit may push the colour range.  Refreshes
    // (automatic z range) and palette/log changes only update the controls.
    bool m_rangeEdited = false;
    bool m_lastPushedValid = false;
    double m_lastPushedLo = 0.0;
    double m_lastPushedHi = 1.0;

    std::function<void()> m_prevImageChanged;
    std::function<void()> m_prevScaleReleased;
    bool m_installed = false;
    QMetaObject::Connection m_destroyedConn;
};

} // namespace sls
