// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2026 Contributors to the SLS Detector Package
#pragma once

// 2D data container used by SlsQt2DPlot: the bin geometry, the data, the z
// range and box/bilinear sampling.
//
// Data layout: the values are stored x fastest (index = bx + by * nbinsx),
// which is the layout qDrawPlot fills (data2d[py * nPixelsX + px]) and the one
// SlsQtFastImage::setImage() expects.
//
// Data coordinates: bin (bx, by) covers
//   [xmin + bx*xwidth, xmin + (bx+1)*xwidth) x
//   [ymin + by*ywidth, ymin + (by+1)*ywidth)
// and its centre is at (xmin + (bx+0.5)*xwidth, ...).  y is up (row 0 is the
// bottom row), like in SlsQtFastImage and in the ROI list.

#include <vector>

namespace sls {

class SlsQt2DHist {
  public:
    SlsQt2DHist(int nbinsx = 10, double xmin = 0, double xmax = 10,
                int nbinsy = 10, double ymin = 0, double ymax = 10,
                const double *d = nullptr, double zmin = 0, double zmax = -1);
    ~SlsQt2DHist() = default;

    // ---- bin geometry (data coordinates) ---------------------------------
    double GetXMin() const { return x_min; }
    double GetXMax() const { return x_max; }
    double GetXBinWidth() const { return x_width; }
    double GetYMin() const { return y_min; }
    double GetYMax() const { return y_max; }
    double GetYBinWidth() const { return y_width; }
    int GetNBinsX() const { return nx; }
    int GetNBinsY() const { return ny; }

    // ---- z range ----------------------------------------------------------
    double GetMinimum() const { return z_min; }
    double GetMaximum() const { return z_max; }
    // Mean of all bins (lazily computed; invalidated by the setters).
    double GetMean();
    void SetMinimum(double zmin) { z_min = zmin; }
    void SetMaximum(double zmax) { z_max = zmax; }
    // zmax < 0: compute the auto range from the data (+/-2% padding); a zmin
    // is ignored in that case.
    void SetMinMax(double zmin = 0, double zmax = -1);
    // Smallest positive value, used as the lower end of a log z scale.
    // Returns z_min; when there is no positive bin it keeps the previous
    // (larger than z_max) fallback so the log mapping stays valid.
    double SetMinimumToFirstGreaterThanZero();

    // ---- data access ------------------------------------------------------
    int GetBinIndex(int bx, int by) const;
    int FindBinIndex(double x, double y) const;
    double GetBinValue(int bx, int by) const;
    void SetBinValue(int bx, int by, double v);
    double *GetDataPtr() { return data.data(); }
    const double *GetDataPtr() const { return data.data(); }

    // ---- sampling ---------------------------------------------------------
    void Interpolate(bool on = true) { interp = on; }
    bool IsInterpolated() const { return interp; }

    // Box sampling (containing bin) or, when Interpolate(true) is set,
    // bilinear interpolation between the four surrounding bin centres.
    // Points outside the data sample the under/overflow bin (which SetData
    // keeps at 0).
    double value(double x, double y) const;

    // ---- bulk input -------------------------------------------------------
    // Copies nbinsx*nbinsy values from d (x fastest).  d may be nullptr: the
    // call then only re-allocates the buffer when the geometry changed, and a
    // new allocation is zeroed; with an unchanged geometry the previous values
    // are kept (nullptr does not clear the frame).  Invalid (non-positive) bin
    // counts are ignored and the previous data is kept.
    void SetData(int nbinsx, double xmin, double xmax, int nbinsy, double ymin,
                 double ymax, const double *d, double zmin = 0, double zmax = -1);

  private:
    double x_min{0.0}, x_max{0.0}, y_min{0.0}, y_max{0.0};
    double x_width{1.0}, y_width{1.0};
    int nx{0}, ny{0}, nb{0};
    // nb + 1 values: the last entry is the under/overflow bin (kept at 0), so
    // an out-of-range GetBinIndex() still returns a valid index.
    std::vector<double> data;
    double z_min{0.0}, z_mean{0.0}, z_max{0.0};
    bool z_mean_has_been_calculated{false};
    bool interp{false};
};

} // namespace sls