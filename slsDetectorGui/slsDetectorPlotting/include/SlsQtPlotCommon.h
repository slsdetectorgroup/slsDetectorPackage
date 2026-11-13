// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2026 Contributors to the SLS Detector Package
#pragma once

// Palettes, axis ticks and colours shared by the 1D plot, the 2D plot and the
// scale panel, so that they all draw the same axes with the same colours.

#include <QColor>
#include <QFont>
#include <QPalette>
#include <QRect>
#include <QRgb>
#include <QSize>
#include <QString>

#include <cmath>
#include <vector>

namespace sls {
namespace plot {

// Palettes offered by the image scale panel.  Color / BW1 / BW2 are the
// original three and stay first (the b / B / c shortcuts use those indices).
// Gain4 is the fixed, muted palette for the 0..3 gain bits and is *not* in the
// chooser (see paletteCount()).
enum class Palette {
    Color,
    BW1,
    BW2,
    Turbo,
    Viridis,
    Plasma,
    Inferno,
    Magma,
    Cividis,
    Hot,
    Gray,
    Gray_r,
    Coolwarm,
    Bwr,
    Seismic,
    RdBu,
    YlGnBu,
    Sunset,
    Gain4
};

// 256-entry RGB lookup table for one palette.  All tables are built once, the
// first time they are needed, and are safe to use from any thread afterwards.
const QRgb *paletteLut(Palette p);

// Human-readable name (label of the scale panel's palette chooser).
const char *paletteName(Palette p);

// Number of palettes offered in the chooser: everything except Gain4.
int paletteCount();

// Log10 mapping that clamps values <= 0 to 1e-2, so that empty pixels (or
// negative pedestal-corrected values) stay visible instead of becoming -inf.
inline double logClamp(double v) {
    return std::log10(v > 1e-2 ? v : 1e-2);
}

// Nice axis step: 1, 2 or 5 times a power of ten, at most maxTicks intervals
// across span.  Returns 1.0 for a degenerate span.
double niceStep(double span, int maxTicks = 8);

// All nice ticks in [lo, hi] (multiples of niceStep()).  Empty when the range
// is degenerate.  At most maxTicks + 1 values are returned.
std::vector<double> computeLinearTicks(double lo, double hi, int maxTicks = 8);

// Formats one tick with the precision implied by the step: no trailing zeros,
// no "-0", scientific notation only for very large / very small values.
QString formatTick(double v, double step);

// The dark plot style shared by the 1D plot, the 2D plot and the histogram
// strip of the scale panel.  The image view keeps its own margins because it
// also has axis titles, but uses the same colours and font size.
struct PlotTheme {
    PlotTheme();

    QColor background{28, 28, 28};
    QColor frame{225, 225, 225};
    QColor grid{95, 95, 95};
    QColor tick{215, 215, 215};
    QColor text{230, 230, 230};
    QFont tickFont;
    int leftMargin = 52;
    int bottomMargin = 26;
    int topMargin = 6;
    int rightMargin = 6;
};

// Inner plot rectangle of a widget, after the theme margins have been
// subtracted.  Never smaller than 10x10 so a tiny widget still paints.
QRect plotRectFor(const QSize &widgetSize, const PlotTheme &theme);

// Widget palette of the gui, made from the colours of PlotTheme so that the
// tabs, docks and dialogs around the plots are as dark as the plots. Text
// entry fields get the plot background, the windows are a little lighter.
// QPalette::Link is the accent colour, for text that has to stand out.
QPalette darkPalette();

} // namespace plot
} // namespace sls
