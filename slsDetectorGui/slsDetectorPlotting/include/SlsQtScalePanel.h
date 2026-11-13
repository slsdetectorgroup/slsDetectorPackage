// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2026 Contributors to the SLS Detector Package
#pragma once

#include <QWidget>

#include <vector>

class QComboBox;

namespace sls {

class SlsQtFastImage;

/**
 * Histogram of the image with the two ends of the colour range as markers.
 *
 * The markers stay at 1/4 and 3/4 of the width and the value axis rescales
 * under them, so the strip shows the colour range plus half a range on either
 * side. Dragging a marker moves that end of the range.
 *
 * Only a preview line follows the cursor during a drag and the new range is
 * reported on release. Rescaling the axis under a dragged marker would make
 * the marker run away from the cursor.
 */
class SlsQtScaleHisto : public QWidget {
    Q_OBJECT

  public:
    explicit SlsQtScaleHisto(QWidget *parent = nullptr);

    /**
     * Ignored while a marker is dragged.
     * @param counts histogram of the image
     * @param binLo lower edge of the first bin
     * @param binHi upper edge of the last bin
     * @param colorLo lower end of the colour range
     * @param colorHi upper end of the colour range
     * @param logX the colour scale is logarithmic, the bins are in log10
     * @param logY draw log10(count + 1)
     */
    void setData(const std::vector<int> &counts, double binLo, double binHi,
                 double colorLo, double colorHi, bool logX, bool logY);
    bool isDragging() const;

  signals:
    /** A marker was dragged and released */
    void rangeChanged(double lo, double hi);

  protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

  private:
    enum class Marker { None, Lo, Hi };

    void axisRange(double &lo, double &hi) const;

    std::vector<int> m_counts;
    double m_binLo = 0.0;
    double m_binHi = 1.0;
    double m_colorLo = 0.0;
    double m_colorHi = 1.0;
    bool m_logX = false;
    bool m_logY = false;

    Marker m_dragged = Marker::None;
    double m_dragX = 0.0; // widget x of the cursor
};

/**
 * Colour controls of an image view: the histogram strip, the palette and
 * lin/log switches for the image and for the histogram.
 */
class SlsQtScalePanel : public QWidget {
    Q_OBJECT

  public:
    explicit SlsQtScalePanel(SlsQtFastImage *view, QWidget *parent = nullptr);

    SlsQtScaleHisto *histogramWidget() const;

  signals:
    /** The user dragged a marker of the histogram strip */
    void colorRangeEdited(double lo, double hi);

  private:
    void refresh();

    SlsQtFastImage *m_view;
    SlsQtScaleHisto *m_histo;
    QComboBox *m_paletteCombo;
    QComboBox *m_imageLogCombo;
    QComboBox *m_histoLogCombo;
};

} // namespace sls
