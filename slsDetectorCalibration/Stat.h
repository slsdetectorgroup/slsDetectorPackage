// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
class Stat {
  public:
    Stat() : n(0), m(0.), m2(0.) {}

    void Clear() {
        n = 0;
        m = 0;
        m2 = 0;
    }

    void Push(double x) {

        m += x;
        m2 += x * x;
        n++;
    }

    int NumDataValues() const { return n; }

    double Mean() const { return (n > 0) ? m / n : 0.0; }

    double Variance() const {
        return ((n > 0) ? (m2 / n - m * m / (n * n)) : 0.0);
    }

    double StandardDeviation() const { return sqrt(Variance()); }

  private:
    int n;
    double m, m2;
};
