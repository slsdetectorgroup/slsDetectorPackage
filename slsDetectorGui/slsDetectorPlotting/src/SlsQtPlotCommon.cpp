// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2026 Contributors to the SLS Detector Package
#include "SlsQtPlotCommon.h"

#include <algorithm>

namespace sls {
namespace plot {

namespace {

// Piecewise-linear colour definition: up to 24 anchor stops plus an optional
// gamma for the grey ramps (nstops == 0).
struct PaletteDef {
    int nstops = 0;
    double stops[24] = {0, 0, 0, 0, 0};
    double r[24] = {0, 0, 0, 0, 0};
    double g[24] = {0, 0, 0, 0, 0};
    double b[24] = {0, 0, 0, 0, 0};
    double gamma = 1.0;
};

const PaletteDef &paletteDef(Palette p) {
    // "False" colour table, as used by the original display.
    static const PaletteDef color = [] {
        PaletteDef d;
        d.nstops = 5;
        d.stops[0] = 0.00; d.stops[1] = 0.34; d.stops[2] = 0.61;
        d.stops[3] = 0.84; d.stops[4] = 1.00;
        d.r[0] = 0.00; d.r[1] = 0.00; d.r[2] = 0.87; d.r[3] = 1.00;
        d.r[4] = 0.51;
        d.g[0] = 0.00; d.g[1] = 0.81; d.g[2] = 1.00; d.g[3] = 0.20;
        d.g[4] = 0.00;
        d.b[0] = 0.51; d.b[1] = 1.00; d.b[2] = 0.12; d.b[3] = 0.00;
        d.b[4] = 0.00;
        return d;
    }();
    static const PaletteDef bw1 = [] {
        PaletteDef d;
        d.gamma = 1.1111; // same exponent the original 'b' button used
        return d;
    }();
    static const PaletteDef bw2 = [] {
        PaletteDef d;
        d.gamma = 0.9; // same exponent the original 'B' button used
        return d;
    }();
    static const PaletteDef turbo = [] {
        PaletteDef d;
        d.nstops = 15;
        for (int i = 0; i < 15; i++)
            d.stops[i] = i / 14.0;
        const double r[15] = {0.1882, 0.2549, 0.2745, 0.2235, 0.1059,
                              0.1412, 0.3804, 0.6431, 0.8196, 0.9529,
                              0.9961, 0.9529, 0.8510, 0.6941, 0.4784};
        const double g[15] = {0.0706, 0.2706, 0.4588, 0.6353, 0.8118,
                              0.9255, 0.9882, 0.9882, 0.9098, 0.7765,
                              0.6078, 0.3882, 0.2196, 0.0980, 0.0157};
        const double b[15] = {0.2314, 0.6706, 0.9294, 0.9882, 0.8314,
                              0.6510, 0.4235, 0.2314, 0.2039, 0.2275,
                              0.1765, 0.0824, 0.0235, 0.0039, 0.0078};
        for (int i = 0; i < 15; i++) {
            d.r[i] = r[i];
            d.g[i] = g[i];
            d.b[i] = b[i];
        }
        return d;
    }();
    static const PaletteDef viridis = [] {
        PaletteDef d;
        d.nstops = 10;
        for (int i = 0; i < 10; i++)
            d.stops[i] = i / 9.0;
        const double r[10] = {0.2667, 0.2824, 0.2431, 0.1922, 0.1490,
                              0.1216, 0.2078, 0.4314, 0.7098, 0.9922};
        const double g[10] = {0.0039, 0.1569, 0.2863, 0.4078, 0.5098,
                              0.6196, 0.7176, 0.8078, 0.8706, 0.9059};
        const double b[10] = {0.3294, 0.4706, 0.5373, 0.5569, 0.5569,
                              0.5373, 0.4745, 0.3451, 0.1686, 0.1451};
        for (int i = 0; i < 10; i++) {
            d.r[i] = r[i];
            d.g[i] = g[i];
            d.b[i] = b[i];
        }
        return d;
    }();
    static const PaletteDef plasma = [] {
        PaletteDef d;
        d.nstops = 10;
        for (int i = 0; i < 10; i++)
            d.stops[i] = i / 9.0;
        const double r[10] = {0.0510, 0.2745, 0.4471, 0.6118, 0.7412,
                              0.8471, 0.9294, 0.9843, 0.9922, 0.9412};
        const double g[10] = {0.0314, 0.0118, 0.0039, 0.0902, 0.2157,
                              0.3412, 0.4745, 0.6235, 0.7922, 0.9765};
        const double b[10] = {0.5294, 0.6235, 0.6588, 0.6196, 0.5255,
                              0.4196, 0.3255, 0.2275, 0.1490, 0.1294};
        for (int i = 0; i < 10; i++) {
            d.r[i] = r[i];
            d.g[i] = g[i];
            d.b[i] = b[i];
        }
        return d;
    }();
    static const PaletteDef inferno = [] {
        PaletteDef d;
        d.nstops = 10;
        for (int i = 0; i < 10; i++)
            d.stops[i] = i / 9.0;
        const double r[10] = {0.0000, 0.1059, 0.2902, 0.4706, 0.6471,
                              0.8118, 0.9294, 0.9843, 0.9686, 0.9882};
        const double g[10] = {0.0000, 0.0471, 0.0471, 0.1098, 0.1725,
                              0.2667, 0.4118, 0.6078, 0.8196, 1.0000};
        const double b[10] = {0.0157, 0.2549, 0.4196, 0.4275, 0.3765,
                              0.2745, 0.1451, 0.0235, 0.2392, 0.6431};
        for (int i = 0; i < 10; i++) {
            d.r[i] = r[i];
            d.g[i] = g[i];
            d.b[i] = b[i];
        }
        return d;
    }();
    static const PaletteDef magma = [] {
        PaletteDef d;
        d.nstops = 10;
        for (int i = 0; i < 10; i++)
            d.stops[i] = i / 9.0;
        const double r[10] = {0.0000, 0.0941, 0.2667, 0.4471, 0.6196,
                              0.8039, 0.9451, 0.9922, 0.9961, 0.9882};
        const double g[10] = {0.0000, 0.0588, 0.0588, 0.1216, 0.1843,
                              0.2510, 0.3765, 0.5882, 0.7922, 0.9922};
        const double b[10] = {0.0157, 0.2392, 0.4627, 0.5059, 0.4980,
                              0.4431, 0.3647, 0.4078, 0.5529, 0.7490};
        for (int i = 0; i < 10; i++) {
            d.r[i] = r[i];
            d.g[i] = g[i];
            d.b[i] = b[i];
        }
        return d;
    }();
    static const PaletteDef cividis = [] {
        PaletteDef d;
        d.nstops = 10;
        for (int i = 0; i < 10; i++)
            d.stops[i] = i / 9.0;
        const double r[10] = {0.0000, 0.0706, 0.2314, 0.3412, 0.4392,
                              0.5412, 0.6471, 0.7647, 0.8824, 0.9961};
        const double g[10] = {0.1333, 0.2078, 0.2863, 0.3647, 0.4431,
                              0.5255, 0.6118, 0.7020, 0.8000, 0.9098};
        const double b[10] = {0.3059, 0.4392, 0.4235, 0.4275, 0.4510,
                              0.4706, 0.4549, 0.4118, 0.3333, 0.2196};
        for (int i = 0; i < 10; i++) {
            d.r[i] = r[i];
            d.g[i] = g[i];
            d.b[i] = b[i];
        }
        return d;
    }();
    static const PaletteDef hot = [] {
        PaletteDef d;
        d.nstops = 4;
        d.stops[0] = 0.0000; d.stops[1] = 0.3650; d.stops[2] = 0.7460;
        d.stops[3] = 1.0000;
        d.r[0] = 0.0000; d.r[1] = 1.0000; d.r[2] = 1.0000; d.r[3] = 1.0000;
        d.g[0] = 0.0000; d.g[1] = 0.0000; d.g[2] = 1.0000; d.g[3] = 1.0000;
        d.b[0] = 0.0000; d.b[1] = 0.0000; d.b[2] = 0.0000; d.b[3] = 1.0000;
        return d;
    }();
    static const PaletteDef gray = [] {
        PaletteDef d;
        d.nstops = 2;
        d.stops[0] = 0.0000;
        d.stops[1] = 1.0000;
        d.r[0] = 0.0000; d.r[1] = 1.0000;
        d.g[0] = 0.0000; d.g[1] = 1.0000;
        d.b[0] = 0.0000; d.b[1] = 1.0000;
        return d;
    }();
    static const PaletteDef gray_r = [] {
        PaletteDef d;
        d.nstops = 2;
        d.stops[0] = 0.0000;
        d.stops[1] = 1.0000;
        d.r[0] = 1.0000; d.r[1] = 0.0000;
        d.g[0] = 1.0000; d.g[1] = 0.0000;
        d.b[0] = 1.0000; d.b[1] = 0.0000;
        return d;
    }();
    static const PaletteDef coolwarm = [] {
        PaletteDef d;
        d.nstops = 3;
        d.stops[0] = 0.0000; d.stops[1] = 0.5000; d.stops[2] = 1.0000;
        d.r[0] = 0.2314; d.r[1] = 0.8667; d.r[2] = 0.7059;
        d.g[0] = 0.2980; d.g[1] = 0.8667; d.g[2] = 0.0157;
        d.b[0] = 0.7529; d.b[1] = 0.8667; d.b[2] = 0.1490;
        return d;
    }();
    static const PaletteDef bwr = [] {
        PaletteDef d;
        d.nstops = 3;
        d.stops[0] = 0.0000; d.stops[1] = 0.5000; d.stops[2] = 1.0000;
        d.r[0] = 0.0000; d.r[1] = 1.0000; d.r[2] = 1.0000;
        d.g[0] = 0.0000; d.g[1] = 1.0000; d.g[2] = 0.0000;
        d.b[0] = 1.0000; d.b[1] = 1.0000; d.b[2] = 0.0000;
        return d;
    }();
    static const PaletteDef seismic = [] {
        PaletteDef d;
        d.nstops = 5;
        d.stops[0] = 0.0000; d.stops[1] = 0.2500; d.stops[2] = 0.5000;
        d.stops[3] = 0.7500; d.stops[4] = 1.0000;
        d.r[0] = 0.0000; d.r[1] = 0.0000; d.r[2] = 1.0000; d.r[3] = 1.0000;
        d.r[4] = 0.5020;
        d.g[0] = 0.0000; d.g[1] = 0.0000; d.g[2] = 1.0000; d.g[3] = 0.0000;
        d.g[4] = 0.0000;
        d.b[0] = 0.3020; d.b[1] = 1.0000; d.b[2] = 1.0000; d.b[3] = 0.0000;
        d.b[4] = 0.0000;
        return d;
    }();
    static const PaletteDef rdbu = [] {
        PaletteDef d;
        d.nstops = 9;
        for (int i = 0; i < 9; i++)
            d.stops[i] = i / 8.0;
        const double r[9] = {0.6980, 0.8392, 0.9569, 0.9922, 0.9686,
                             0.8196, 0.5725, 0.2627, 0.1294};
        const double g[9] = {0.0941, 0.3765, 0.6471, 0.8588, 0.9686,
                             0.8980, 0.7725, 0.5765, 0.4000};
        const double b[9] = {0.1686, 0.3020, 0.5098, 0.7804, 0.9686,
                             0.9412, 0.8706, 0.7647, 0.6745};
        for (int i = 0; i < 9; i++) {
            d.r[i] = r[i];
            d.g[i] = g[i];
            d.b[i] = b[i];
        }
        return d;
    }();
    static const PaletteDef ylgnbu = [] {
        PaletteDef d;
        d.nstops = 9;
        for (int i = 0; i < 9; i++)
            d.stops[i] = i / 8.0;
        const double r[9] = {0.0314, 0.1451, 0.1333, 0.1137, 0.2549,
                             0.4980, 0.7804, 0.9294, 1.0000};
        const double g[9] = {0.1137, 0.2039, 0.3686, 0.5686, 0.7137,
                             0.8039, 0.9137, 0.9725, 1.0000};
        const double b[9] = {0.3451, 0.5804, 0.6588, 0.7529, 0.7686,
                             0.7333, 0.7059, 0.6941, 0.8510};
        for (int i = 0; i < 9; i++) {
            d.r[i] = r[i];
            d.g[i] = g[i];
            d.b[i] = b[i];
        }
        return d;
    }();
    static const PaletteDef sunset = [] {
        PaletteDef d;
        d.nstops = 11;
        for (int i = 0; i < 11; i++)
            d.stops[i] = i / 10.0;
        const double r[11] = {0.2118, 0.2902, 0.4314, 0.5961, 0.7608, 0.9176,
                              0.9961, 0.9922, 0.9647, 0.8667, 0.6471};
        const double g[11] = {0.2941, 0.4824, 0.6510, 0.7922, 0.8941, 0.9255,
                              0.8549, 0.7020, 0.4941, 0.2392, 0.0000};
        const double b[11] = {0.6039, 0.7176, 0.8039, 0.8824, 0.9373, 0.8000,
                              0.5451, 0.4000, 0.2941, 0.1765, 0.1490};
        for (int i = 0; i < 11; i++) {
            d.r[i] = r[i];
            d.g[i] = g[i];
            d.b[i] = b[i];
        }
        return d;
    }();

    // The gain image holds one of four numbers (the Jungfrau gain bits).  These
    // are deliberately desaturated so a large flat map does not glare.
    static const PaletteDef gain4 = [] {
        PaletteDef d;
        d.nstops = 4;
        d.stops[0] = 0.0000; d.stops[1] = 0.3333; d.stops[2] = 0.6667;
        d.stops[3] = 1.0000;
        d.r[0] = 0.3725; d.r[1] = 0.4353; d.r[2] = 0.7961; d.r[3] = 0.6902;
        d.g[0] = 0.5020; d.g[1] = 0.6275; d.g[2] = 0.7412; d.g[3] = 0.4157;
        d.b[0] = 0.6588; d.b[1] = 0.4353; d.b[2] = 0.4157; d.b[3] = 0.3882;
        return d;
    }();

    switch (p) {
    case Palette::BW1:
        return bw1;
    case Palette::BW2:
        return bw2;
    case Palette::Turbo:
        return turbo;
    case Palette::Viridis:
        return viridis;
    case Palette::Plasma:
        return plasma;
    case Palette::Inferno:
        return inferno;
    case Palette::Magma:
        return magma;
    case Palette::Cividis:
        return cividis;
    case Palette::Hot:
        return hot;
    case Palette::Gray:
        return gray;
    case Palette::Gray_r:
        return gray_r;
    case Palette::Coolwarm:
        return coolwarm;
    case Palette::Bwr:
        return bwr;
    case Palette::Seismic:
        return seismic;
    case Palette::RdBu:
        return rdbu;
    case Palette::YlGnBu:
        return ylgnbu;
    case Palette::Sunset:
        return sunset;
    case Palette::Gain4:
        return gain4;
    case Palette::Color:
    default:
        return color;
    }
}

// 256-entry lookup table sampled from exactly the same definition.
void buildLut(const PaletteDef &d, QRgb *out) {
    for (int i = 0; i < 256; i++) {
        const double v = i / 255.0;
        double rr, gg, bb;
        if (d.nstops <= 0) {
            const double c = std::pow(v, d.gamma);
            rr = gg = bb = c;
        } else {
            int k = 0;
            while (k < d.nstops - 2 && v > d.stops[k + 1])
                k++;
            const double span = d.stops[k + 1] - d.stops[k];
            const double t = (span > 0) ? (v - d.stops[k]) / span : 0.0;
            const double tc = (t < 0) ? 0.0 : (t > 1 ? 1.0 : t);
            rr = d.r[k] + (d.r[k + 1] - d.r[k]) * tc;
            gg = d.g[k] + (d.g[k + 1] - d.g[k]) * tc;
            bb = d.b[k] + (d.b[k + 1] - d.b[k]) * tc;
        }
        out[i] = qRgb(static_cast<int>(rr * 255.0 + 0.5),
                      static_cast<int>(gg * 255.0 + 0.5),
                      static_cast<int>(bb * 255.0 + 0.5));
    }
}

constexpr int kNumPalettes = int(Palette::Gain4) + 1;

// Built once, on first use; const afterwards, so safe from any thread.
const QRgb *paletteTables() {
    static const auto tables = [] {
        auto *t = new QRgb[kNumPalettes][256];
        for (int i = 0; i < kNumPalettes; i++)
            buildLut(paletteDef(static_cast<Palette>(i)), t[i]);
        return t;
    }();
    return &tables[0][0];
}

} // namespace

const QRgb *paletteLut(Palette p) {
    const int i = int(p);
    if (i < 0 || i >= kNumPalettes)
        return paletteTables();
    return paletteTables() + size_t(i) * 256;
}

const char *paletteName(Palette p) {
    switch (p) {
    case Palette::Gain4:
        return "gain 0..3 (fixed)";
    case Palette::Color:
        return "Col (jet)";
    case Palette::BW1:
        return "BW grey 1";
    case Palette::BW2:
        return "BW grey 2";
    case Palette::Turbo:
        return "Turbo";
    case Palette::Viridis:
        return "Viridis";
    case Palette::Plasma:
        return "Plasma";
    case Palette::Inferno:
        return "Inferno";
    case Palette::Magma:
        return "Magma";
    case Palette::Cividis:
        return "Cividis (colorblind)";
    case Palette::Hot:
        return "Hot";
    case Palette::Gray:
        return "Grey (linear)";
    case Palette::Gray_r:
        return "Grey reversed";
    case Palette::Coolwarm:
        return "Coolwarm (diverging)";
    case Palette::Bwr:
        return "BWR (diverging)";
    case Palette::Seismic:
        return "Seismic (diverging)";
    case Palette::RdBu:
        return "RdBu (diverging)";
    case Palette::YlGnBu:
        return "YlGnBu (Brewer)";
    case Palette::Sunset:
        return "Sunset (Tol)";
    }
    return "?";
}

int paletteCount() { return int(Palette::Gain4); }

double niceStep(double span, int maxTicks) {
    if (!(span > 0.0) || !std::isfinite(span))
        return 1.0;
    if (maxTicks < 1)
        maxTicks = 1;
    const double raw = span / double(maxTicks);
    const double mag = std::pow(10.0, std::floor(std::log10(raw)));
    const double norm = raw / mag;
    double mult = 10.0;
    if (norm <= 1.0)
        mult = 1.0;
    else if (norm <= 2.0)
        mult = 2.0;
    else if (norm <= 5.0)
        mult = 5.0;
    return mult * mag;
}

std::vector<double> computeLinearTicks(double lo, double hi, int maxTicks) {
    std::vector<double> out;
    if (!std::isfinite(lo) || !std::isfinite(hi) || !(hi > lo))
        return out;
    if (maxTicks < 1)
        maxTicks = 1;
    const double step = niceStep(hi - lo, maxTicks);
    const double first = std::ceil(lo / step - 1e-9);
    for (long long k = static_cast<long long>(first);; ++k) {
        const double v = double(k) * step;
        if (v > hi + step * 1e-9)
            break;
        out.push_back(std::abs(v) < step * 1e-9 ? 0.0 : v);
        if (static_cast<int>(out.size()) > maxTicks + 1)
            break;
    }
    return out;
}

QString formatTick(double v, double step) {
    if (!std::isfinite(v))
        return QStringLiteral("?");
    if (!(step > 0.0) || !std::isfinite(step))
        step = 1.0;
    if (std::abs(v) < step * 1e-9)
        v = 0.0; // avoid "-0"
    const double a = std::abs(v);
    if (a >= 1e6 || (a > 0.0 && a < 1e-3)) {
        QString s = QString::number(v, 'g', 4);
        return s;
    }
    const int dec =
        std::max(0, static_cast<int>(std::ceil(-std::log10(step) - 1e-9)));
    QString s = QString::number(v, 'f', dec);
    if (s.contains(QLatin1Char('.'))) {
        while (s.endsWith(QLatin1Char('0')))
            s.chop(1);
        if (s.endsWith(QLatin1Char('.')))
            s.chop(1);
    }
    return s;
}

PlotTheme::PlotTheme() { tickFont.setPixelSize(10); }

QRect plotRectFor(const QSize &widgetSize, const PlotTheme &theme) {
    int x0 = theme.leftMargin;
    int y0 = theme.topMargin;
    int x1 = widgetSize.width() - theme.rightMargin;
    int y1 = widgetSize.height() - theme.bottomMargin;
    if (x1 - x0 < 10)
        x1 = x0 + 10;
    if (y1 - y0 < 10)
        y1 = y0 + 10;
    return QRect(x0, y0, x1 - x0, y1 - y0);
}

} // namespace plot
} // namespace sls
