// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2026 Contributors to the SLS Detector Package
#include "SlsQt2DHist.h"

#include <algorithm>
#include <cmath>
#include <cstring>

namespace sls {

SlsQt2DHist::SlsQt2DHist(int nbinsx, double xmin, double xmax, int nbinsy,
                         double ymin, double ymax, double *d, double zmin,
                         double zmax) {
    SetData(nbinsx, xmin, xmax, nbinsy, ymin, ymax, d, zmin, zmax);
}

namespace {
// Bins are addressed with their lower edge; the ranges are descending/invalid
// only if a caller passes nonsense - fall back to a unit width so the
// conversions below never divide by zero.
double safeBinWidth(int nbins, double lo, double hi) {
    if (nbins <= 0)
        return 1.0;
    const double w = (hi - lo) / nbins;
    if (!std::isfinite(w) || w <= 0.0)
        return 1.0;
    return w;
}
} // namespace

void SlsQt2DHist::SetData(int nbinsx, double xmin, double xmax, int nbinsy,
                          double ymin, double ymax, double *d, double zmin,
                          double zmax) {
    if (nbinsx < 1 || nbinsy < 1)
        return; // invalid geometry: keep the previous data

    z_mean_has_been_calculated = false;

    x_min = xmin;
    x_max = xmax;
    y_min = ymin;
    y_max = ymax;
    x_width = safeBinWidth(nbinsx, xmin, xmax);
    y_width = safeBinWidth(nbinsy, ymin, ymax);

    nx = nbinsx;
    ny = nbinsy;
    nb = nx * ny;
    // one extra entry for the under/overflow bin; keep the previous allocation
    // when the frame geometry is unchanged (the GUI feeds a frame every few ms)
    const size_t needed = static_cast<size_t>(nb) + 1;
    if (data.size() != needed)
        data.assign(needed, 0.0);
    data[static_cast<size_t>(nb)] = 0.0;

    if (d) {
        std::memcpy(data.data(), d, static_cast<size_t>(nb) * sizeof(double));
        SetMinMax(zmin, zmax);
    }
}

int SlsQt2DHist::GetBinIndex(int bx, int by) const {
    if (bx < 0 || bx >= nx || by < 0 || by >= ny)
        return nb; // under/overflow bin
    return bx + by * nx; // x fastest, the layout of the image copy
}

int SlsQt2DHist::FindBinIndex(double x, double y) const {
    if (!std::isfinite(x) || !std::isfinite(y))
        return nb;
    const int bx = static_cast<int>(std::floor((x - x_min) / x_width));
    const int by = static_cast<int>(std::floor((y - y_min) / y_width));
    return GetBinIndex(bx, by);
}

double SlsQt2DHist::GetBinValue(int bx, int by) const {
    return data[static_cast<size_t>(GetBinIndex(bx, by))];
}

void SlsQt2DHist::SetBinValue(int bx, int by, double v) {
    const int index = GetBinIndex(bx, by);
    if (index >= nb)
        return; // never modify the under/overflow bin through this API
    z_mean_has_been_calculated = false;
    data[static_cast<size_t>(index)] = v;
}

void SlsQt2DHist::SetMinMax(double zmin, double zmax) {
    if (zmax != -1.0) {
        z_min = zmin;
        z_max = zmax;
        z_mean_has_been_calculated = false;
        return;
    }

    z_mean_has_been_calculated = true;
    z_min = 0.0;
    z_max = 1.0;
    z_mean = 0.0;
    if (nb < 1 || data.empty())
        return;

    // Skip non-finite bins: a single NaN used to poison the whole auto range
    // (min/max became NaN, the colour LUT index degenerated) and an Inf made
    // it unbounded.  The mean is taken over the finite bins only.
    bool haveFinite = false;
    double sum = 0.0;
    int finiteCount = 0;
    for (int i = 0; i < nb; i++) {
        const double v = data[static_cast<size_t>(i)];
        if (!std::isfinite(v))
            continue;
        if (!haveFinite) {
            z_min = v;
            z_max = v;
            haveFinite = true;
        } else {
            if (v < z_min)
                z_min = v;
            if (v > z_max)
                z_max = v;
        }
        sum += v;
        finiteCount++;
    }
    if (!haveFinite) {
        // No usable value at all: keep a valid, harmless range so the colour
        // mapping and the scale-panel control range stay finite.
        z_min = 0.0;
        z_max = 1.0;
        z_mean = 0.0;
        return;
    }
    z_mean = sum / finiteCount;

    // Keep a tiny margin so the extreme bins are not clipped by rounding.
    // The padding must always be OUTWARD:
    //  - low end:  divide a positive value / multiply a negative one;
    //  - high end: multiply a positive value / add 2% of the magnitude to a
    //    negative one, i.e. move it up towards zero.  (Multiplying a negative
    //    z_max by 1.02 would move it BELOW the data maximum and clip the
    //    brightest bin - that was the direction bug reported as R5.)
    // Positive-value padding is unchanged (the old qwt formulas).
    if (z_min > 0)
        z_min /= 1.02;
    else
        z_min -= 0.02 * std::abs(z_min);
    if (z_max > 0)
        z_max *= 1.02;
    else
        z_max += 0.02 * std::abs(z_max);
}

double SlsQt2DHist::GetMean() {
    if (!z_mean_has_been_calculated) {
        z_mean_has_been_calculated = true;
        z_mean = 0.0;
        double sum = 0.0;
        int finiteCount = 0;
        for (int i = 0; i < nb; i++) {
            const double v = data[static_cast<size_t>(i)];
            if (!std::isfinite(v))
                continue;
            sum += v;
            finiteCount++;
        }
        z_mean = (finiteCount > 0) ? sum / finiteCount : 0.0;
    }
    return z_mean;
}

double SlsQt2DHist::SetMinimumToFirstGreaterThanZero() {
    double first = std::fabs(z_max) + 1.0;
    if (!std::isfinite(first) || first <= 0.0)
        first = 1.0;
    for (int i = 0; i < nb; i++) {
        const double v = data[static_cast<size_t>(i)];
        if (v > 0.0 && v < first)
            first = v;
    }
    z_min = first;
    return z_min;
}

double SlsQt2DHist::value(double x, double y) const {
    if (nb < 1 || data.empty())
        return 0.0;
    if (!std::isfinite(x) || !std::isfinite(y))
        return data[static_cast<size_t>(nb)];

    if (interp && nx >= 2 && ny >= 2) {
        // Continuous bin-centre coordinates: bin centre i is at i.
        const double cu = (x - x_min) / x_width - 0.5;
        const double cv = (y - y_min) / y_width - 0.5;
        if (!std::isfinite(cu) || !std::isfinite(cv))
            return data[static_cast<size_t>(nb)];
        // Outside the grid the border value is extended (no extrapolation).
        const double u = std::min(std::max(cu, 0.0), double(nx - 1));
        const double v = std::min(std::max(cv, 0.0), double(ny - 1));
        int i0 = static_cast<int>(std::floor(u));
        int j0 = static_cast<int>(std::floor(v));
        i0 = std::min(std::max(i0, 0), nx - 2);
        j0 = std::min(std::max(j0, 0), ny - 2);
        const double fx = u - i0;
        const double fy = v - j0;
        const size_t i1 = static_cast<size_t>(i0 + 1);
        const size_t j1 = static_cast<size_t>(j0 + 1);
        const size_t i0s = static_cast<size_t>(i0);
        const size_t j0s = static_cast<size_t>(j0);
        const double v00 = data[i0s + j0s * nx];
        const double v10 = data[i1 + j0s * nx];
        const double v01 = data[i0s + j1 * nx];
        const double v11 = data[i1 + j1 * nx];
        return (1.0 - fx) * (1.0 - fy) * v00 + fx * (1.0 - fy) * v10 +
               (1.0 - fx) * fy * v01 + fx * fy * v11;
    }

    // Box sampling: the value of the bin containing (x, y).
    const double bx = (x - x_min) / x_width;
    const double by = (y - y_min) / y_width;
    if (!std::isfinite(bx) || !std::isfinite(by))
        return data[static_cast<size_t>(nb)];
    const int ix = static_cast<int>(std::floor(bx));
    const int iy = static_cast<int>(std::floor(by));
    if (ix < 0 || ix >= nx || iy < 0 || iy >= ny)
        return data[static_cast<size_t>(nb)];
    return data[static_cast<size_t>(ix + iy * nx)];
}

} // namespace sls