// SPDX-License-Identifier: LGPL-3.0-or-other
// ===========================================================================
// online_jf_root.cpp -- ROOT canvases, the ROOT redraw thread and the
// command queue.
//
// ROOT is not thread safe: canvas painting and gSystem->ProcessEvents() may
// only run on ONE thread.  That thread is the only one allowed to touch the
// display clones; the source histograms are filled by the analysis thread.
//
// Split out of onlinedisp_zmq.cpp (Plot1D/2DHistos, historeset, axisreset,
// palettes, the ROOT half of MainWindow::rootDrawLoop).
// ===========================================================================
#include "online_jf.h"


// ---------------------------------------------------------------------------
// ROOT objects
// ---------------------------------------------------------------------------
TApplication *rootapp = nullptr;
TCanvas *A2 = nullptr, *A3 = nullptr, *A4 = nullptr, *A5 = nullptr,
        *A6 = nullptr, *A7 = nullptr;
TPad *p1 = nullptr, *p2 = nullptr, *p3 = nullptr, *p4 = nullptr, *p5 = nullptr,
    *p6 = nullptr;
TH1I **hchip = nullptr;
TH1I *hroi = nullptr;  // one spectrum over the gate-ROI pixels only
// the same spectra in keV (canvases A8/A9); filled only when a gain map is
// loaded, and drawn only then too
TH1I **hchipKev = nullptr;
TH1I *hroiKev = nullptr;
TH1I *hproj = nullptr, *hchcum = nullptr;
TH2F *his1000 = nullptr, *his1001 = nullptr, *his1002 = nullptr;
TH2F *his1060 = nullptr, *his1061 = nullptr, *his1062 = nullptr;
TH2F *his2000 = nullptr, *his3000 = nullptr, *his4500 = nullptr,
     *hisLvsR = nullptr;
TBox *box = nullptr;

// display clones painted by the ROOT thread
TH2F *disp3000 = nullptr;
TH2F *disp4500 = nullptr;
TH2F *dispLvsR = nullptr;
TH1I *dispChip[8] = {nullptr, nullptr, nullptr, nullptr,
                     nullptr, nullptr, nullptr, nullptr};
TH1I *dispRoi = nullptr;
TH1I *dispChipKev[8] = {nullptr, nullptr, nullptr, nullptr,
                        nullptr, nullptr, nullptr, nullptr};
TH1I *dispRoiKev = nullptr;
// photon-map helpers (defined further down, ROOT thread only)
void plotPhotonMap();
void zoomPhotonMap(double x0, double x1, double y0, double y1);
void setSpectrumTitles(bool kev);

#ifdef JFSTRX
TH2F *disp1000 = nullptr, *disp1001 = nullptr, *disp1002 = nullptr;
TH2F *disp1060 = nullptr, *disp1061 = nullptr, *disp1062 = nullptr;
#endif

namespace {
// TEMP instrumentation: where the ROOT thread spends its time
double g_rtCopy = 0, g_rtPlot2 = 0, g_rtPlot1 = 0, g_rtPlotRoi = 0,
       g_rtPlotKev = 0, g_rtPlotRoiKev = 0, g_rtPe = 0, g_rtCmd = 0;
uint64_t g_rtRedraws = 0, g_rtLoops = 0;
uint64_t g_rtNPlot1 = 0, g_rtNPlotRoi = 0, g_rtNPlotKev = 0,
         g_rtNPlotRoiKev = 0;
bool g_rootTiming() { return getenv("JF_TIMING") != nullptr; }

std::mutex gCmdMtx;
std::condition_variable gCmdCv; // wakes the ROOT thread for a prompt command
std::vector<RootCmd> gCmds;

std::thread *gRootThread = nullptr;
std::atomic<bool> gRootRun{false};

// copies bin contents + entry count from a source accumulator to its display
// clone (same geometry). ROOT-thread only.
void copyBins(TH1 *dst, TH1 *src) {
    if (!dst || !src)
        return;
    const int bx = dst->GetNbinsX();
    const int by = dst->GetNbinsY();
    dst->Reset();
    if (by <= 1) {
        for (int b = 1; b <= bx; b++)
            dst->SetBinContent(b, src->GetBinContent(b));
    } else {
        for (int y = 1; y <= by; y++)
            for (int x = 1; x <= bx; x++)
                dst->SetBinContent(x, y, src->GetBinContent(x, y));
    }
    dst->SetEntries(src->GetEntries());
}

void rootLoop();
} // namespace

// ---------------------------------------------------------------------------
// Command queue (any thread -> ROOT thread)
// ---------------------------------------------------------------------------
void pushRootCmd(const RootCmd &cmd) {
    {
        std::lock_guard<std::mutex> lock(gCmdMtx);
        gCmds.push_back(cmd);
    }
    gCmdCv.notify_one(); // service it now, not at the next scheduled redraw
}

void pushRootCmdCoalesced(const RootCmd &cmd) {
    {
        std::lock_guard<std::mutex> lock(gCmdMtx);
        for (auto &c : gCmds) {
            if (c.type == cmd.type) {
                c = cmd; // keep only the newest viewport
                gCmdCv.notify_one();
                return;
            }
        }
        gCmds.push_back(cmd);
    }
    gCmdCv.notify_one();
}

void requestHistoReset() {
    // Wakes the ROOT thread.  execRootCmd() applies the reset if the analysis
    // thread is between frames, otherwise it leaves the flag for the analysis
    // thread to apply at the top of its next frame.  The ROOT thread must never
    // BLOCK on gHistoMtx: under a heavy stream it can starve, which is what
    // made the reset appear to take tens of seconds.
    RootCmd c;
    c.type = RootCmdType::ResetHis;
    pushRootCmd(c);
}

void forceRootRedraw() {
    gRedrawRequest.store(true);
    gCmdCv.notify_one();
}

void startRootThread() {
    if (gRootThread)
        return;
    gRootRun.store(true);
    gRootThread = new std::thread(rootLoop);
}

void stopRootThread() {
    gRootRun.store(false);
    // only the first call prints (a signal stop and the normal shutdown can
    // both come through here; the second one has nothing left to join)
    if (gRootThread && g_rootTiming()) {
        printf("[root] loops=%llu redraws=%llu | per-redraw us: copy=%.0f "
               "plot2d=%.0f plot1d=%.0f plotroi=%.0f plotkev=%.0f "
               "plotrokev=%.0f | per-loop us: cmd=%.0f pe=%.0f\n",
               (unsigned long long)g_rtLoops,
               (unsigned long long)g_rtRedraws,
               g_rtRedraws ? g_rtCopy / g_rtRedraws : 0.0,
               g_rtRedraws ? g_rtPlot2 / g_rtRedraws : 0.0,
               g_rtRedraws ? g_rtPlot1 / g_rtRedraws : 0.0,
               g_rtRedraws ? g_rtPlotRoi / g_rtRedraws : 0.0,
               g_rtRedraws ? g_rtPlotKev / g_rtRedraws : 0.0,
               g_rtRedraws ? g_rtPlotRoiKev / g_rtRedraws : 0.0,
               g_rtLoops ? g_rtCmd / g_rtLoops : 0.0,
               g_rtLoops ? g_rtPe / g_rtLoops : 0.0);
        printf("[root] spectrum paints: chipsADU=%llu roiADU=%llu "
               "chipsKeV=%llu roiKeV=%llu | per paint us: %.0f %.0f %.0f %.0f\n",
               (unsigned long long)g_rtNPlot1, (unsigned long long)g_rtNPlotRoi,
               (unsigned long long)g_rtNPlotKev,
               (unsigned long long)g_rtNPlotRoiKev,
               g_rtNPlot1 ? g_rtPlot1 / g_rtNPlot1 : 0.0,
               g_rtNPlotRoi ? g_rtPlotRoi / g_rtNPlotRoi : 0.0,
               g_rtNPlotKev ? g_rtPlotKev / g_rtNPlotKev : 0.0,
               g_rtNPlotRoiKev ? g_rtPlotRoiKev / g_rtNPlotRoiKev : 0.0);
        // spectrum totals (the ROI ones must be a subset of the chip ones);
        // read under the histogram lock, the analysis thread may still be
        // alive when this thread is stopped
        {
            std::lock_guard<std::mutex> lock(gHistoMtx);
            char chipTxt[8 * 20] = "";
            double chipTot = 0;
            for (int k = 0; k < 8; k++) {
                char one[24];
                const double c = hchip && hchip[k] ? hchip[k]->GetEntries() : 0;
                chipTot += c;
                snprintf(one, sizeof(one), "%.0f ", c);
                strncat(chipTxt, one, sizeof(chipTxt) - strlen(chipTxt) - 1);
            }
            const double roiTot = hroi ? hroi->GetEntries() : 0;
            double kevTot = 0;
            if (hchipKev) {
                for (int k = 0; k < 8; k++)
                    if (hchipKev[k])
                        kevTot += hchipKev[k]->GetEntries();
            }
            const double roiKevTot = hroiKev ? hroiKev->GetEntries() : 0;
            printf("[root] spectra entries: hchip=[%s] total=%.0f | hroi=%.0f "
                   "(roi/chip=%.4f)\n",
                   chipTxt, chipTot, roiTot,
                   chipTot > 0 ? roiTot / chipTot : 0.0);
            printf("[root] keV entries: hchipKev=%.0f hroiKev=%.0f "
                   "(kev/adu=%.4f, gainmap=%s)\n",
                   kevTot, roiKevTot,
                   chipTot > 0 ? kevTot / chipTot : 0.0,
                   gainMapLoaded() ? "loaded"
                                   : ((kevSpectra.load() && gain_flag.load())
                                          ? "nominal"
                                          : "none"));
            // the conversion is right when keV_mean == ADU_mean / (ADU per
            // keV): with a g0 of 40 that ratio is exactly 40
            if (hroiKev && roiKevTot > 0) {
                const TAxis *kx = hroiKev->GetXaxis();
                const int kmax = hroiKev->GetMaximumBin();
                printf("[root] hroi_keV axis %.4f..%.4f nbins %d window "
                       "bins %d..%d | maxbin %d (%.4f) content %.0f | "
                       "mean %.4f | under %0.f over %.0f\n",
                       kx->GetXmin(), kx->GetXmax(), kx->GetNbins(),
                       kx->GetFirst(), kx->GetLast(), kmax,
                       kx->GetBinCenter(kmax), hroiKev->GetBinContent(kmax),
                       hroiKev->GetMean(), hroiKev->GetBinContent(0),
                       hroiKev->GetBinContent(kx->GetNbins() + 1));
            }
            if (hroiKev && roiTot > 0 && roiKevTot > 0) {
                const double aduMean = hroi->GetMean();
                const double kevMean = hroiKev->GetMean();
                printf("[root] mean of the ROI spectra: ADU %.4f -> keV %.4f "
                       "(ADU/keV = %.4f)\n",
                       aduMean, kevMean,
                       kevMean != 0 ? aduMean / kevMean : 0.0);
            }
            // the axis windows actually in force
            auto lo = [](TAxis *a) { return a->GetBinLowEdge(a->GetFirst()); };
            auto hi = [](TAxis *a) { return a->GetBinUpEdge(a->GetLast()); };
            if (dispChip[0])
                printf("[root] ranges: PH x %.0f..%.0f",
                       lo(dispChip[0]->GetXaxis()), hi(dispChip[0]->GetXaxis()));
            if (dispRoi)
                printf(" | ROI x %.0f..%.0f", lo(dispRoi->GetXaxis()),
                       hi(dispRoi->GetXaxis()));
            printf(" | PH set %s",
                   (kevSpectra.load() && gain_flag.load()) ? "keV" : "ADU");
            if (disp3000) // the colour scale lives in min/max, not in a range
                printf(" | main z %.0f..%.0f (%s)", disp3000->GetMinimum(),
                       disp3000->GetMaximum(),
                       dophotonmap.load() ? "photon counts" : "raw ADC");
            printf("\n");
        }
        fflush(stdout);
    }
    if (gRootThread) {
        if (gRootThread->joinable())
            gRootThread->join();
        delete gRootThread;
        gRootThread = nullptr;
    }
}

// ---------------------------------------------------------------------------
// Command execution (ROOT thread only)
// ---------------------------------------------------------------------------
namespace {

void execRootCmd(const RootCmd &cmd) {
    switch (cmd.type) {
    case RootCmdType::ResetHis:
        gResetHisPending.store(true);
        gRedrawRequest.store(true);
        // If the analysis thread is between frames it does not hold gHistoMtx,
        // so apply the reset here (makes a reset while idle/paused immediate).
        // Otherwise leave the flag; the analysis thread performs it at the top
        // of its next frame, where the lock is already held.  try_lock never
        // blocks, so the ROOT thread can never starve on gHistoMtx.
        if (gHistoMtx.try_lock()) {
            if (gResetHisPending.exchange(false))
                historesetSourcesLocked();
            gHistoMtx.unlock();
        }
        break;
    case RootCmdType::ResetAxes:
        axisreset();
        break;
    case RootCmdType::PaletteColor:
        loadRootPalette(Palette::Color);
        Plot2DHistos();
        Plot1DHistos();
        break;
    case RootCmdType::PaletteBW1:
        loadRootPalette(Palette::BW1);
        Plot2DHistos();
        Plot1DHistos();
        break;
    case RootCmdType::PaletteBW2:
        loadRootPalette(Palette::BW2);
        Plot2DHistos();
        Plot1DHistos();
        break;
    case RootCmdType::Zoom:
        if (disp3000 && A5) {
            // a drag produces a stream of viewports; only repaint the last one
            // (the ranges are applied below, so nothing is lost by skipping)
            {
                bool newer = false;
                std::lock_guard<std::mutex> lock(gCmdMtx);
                for (const RootCmd &c : gCmds) {
                    if (c.type == RootCmdType::Zoom) {
                        newer = true;
                        break;
                    }
                }
                if (newer)
                    break;
            }
            zoomPhotonMap(cmd.a, cmd.c, cmd.b, cmd.d);
        }
        break;
    case RootCmdType::SetMapRange:
        // colour scale of the ROOT 2D map: applied once, on the ROOT thread,
        // so a scale the operator drags on the canvas is NOT reset by the
        // periodic redraw (TH1::Reset() does not touch it either)
        if (disp3000) {
            disp3000->GetZaxis()->SetRangeUser(cmd.a, cmd.b);
            Plot2DHistos();
        }
        break;
    case RootCmdType::SetChipRange:
        for (int ih = 0; ih < 8; ih++) {
            if (dispChip[ih])
                dispChip[ih]->GetXaxis()->SetRangeUser(cmd.a, cmd.b);
            // the ROI and keV spectra share A4's x window; the keV axis is
            // the ADU axis / 40, so the same bin indices are selected
            if (dispRoi)
                dispRoi->GetXaxis()->SetRangeUser(cmd.a, cmd.b);
            if (dispRoiKev)
                dispRoiKev->GetXaxis()->SetRangeUser(cmd.a / KEV_NOMINAL_ADU,
                                                     cmd.b / KEV_NOMINAL_ADU);
            for (int ik = 0; ik < 8; ik++) {
                if (dispChipKev[ik])
                    dispChipKev[ik]->GetXaxis()->SetRangeUser(
                        cmd.a / KEV_NOMINAL_ADU, cmd.b / KEV_NOMINAL_ADU);
            }
        }
        Plot1DHistos();
        break;
    default:
        break;
    }
}

// ---------------------------------------------------------------------------
// ROOT redraw loop: the single owner of ROOT graphics
// ---------------------------------------------------------------------------
void rootLoop() {
    auto next = steady_clock::now();
    while (gRootRun.load()) {
      try { // an escaping exception would abort the process
        // Wait for the scheduled redraw, but wake immediately for a queued
        // command or a forced redraw: with ROOT update set to tens of seconds
        // a button must not wait for the next tick.
        {
            std::unique_lock<std::mutex> lk(gCmdMtx);
            gCmdCv.wait_until(lk, next, [&] {
                return !gCmds.empty() || gRedrawRequest.load() ||
                       !gRootRun.load();
            });
        }
        if (!gRootRun.load())
            break;

        g_rtLoops++;
        const auto tLoop0 = steady_clock::now();
        // 1) execute queued user commands (posted by the Qt thread)
        for (;;) {
            RootCmd cmd;
            {
                std::lock_guard<std::mutex> lock(gCmdMtx);
                if (gCmds.empty())
                    break;
                cmd = gCmds.front();
                gCmds.erase(gCmds.begin());
            }
            execRootCmd(cmd);
        }

        g_rtCmd += duration<double, std::micro>(steady_clock::now() - tLoop0)
                       .count();
        // 2) ROOT GUI event processing lives HERE (single owner thread),
        //    keeping the canvases interactive (zoom etc.)
        {
            const auto t0 = steady_clock::now();
            gSystem->ProcessEvents();
            g_rtPe += duration<double, std::micro>(steady_clock::now() - t0)
                          .count();
        }

        // 3) periodic plot
        int ms = rootUpdateMs.load();
        if (ms < 50)
            ms = 50;
        next = steady_clock::now() + std::chrono::milliseconds(ms);

        const bool forced = gRedrawRequest.exchange(false);
        if (gFramesForRoot.exchange(0) == 0 && !forced)
            continue;

        // snapshot the source accumulators into the display clones that the
        // canvases paint (never paint the objects being filled)
        {
            const auto t0 = steady_clock::now();
            copyBins(disp3000, his3000);
            copyBins(disp4500, his4500);
            copyBins(dispLvsR, hisLvsR);
            for (int ih = 0; ih < 8; ih++)
                copyBins(dispChip[ih], hchip ? hchip[ih] : nullptr);
            copyBins(dispRoi, hroi);
            for (int ih = 0; ih < 8; ih++)
                copyBins(dispChipKev[ih], hchipKev ? hchipKev[ih] : nullptr);
            copyBins(dispRoiKev, hroiKev);
            g_rtCopy += duration<double, std::micro>(steady_clock::now() - t0)
                            .count();
        }
#ifdef JFSTRX
        copyBins(disp1000, his1000);
        copyBins(disp1001, his1001);
        copyBins(disp1002, his1002);
        copyBins(disp1060, his1060);
        copyBins(disp1061, his1061);
        copyBins(disp1062, his1062);
#endif

        {
            const auto t0 = steady_clock::now();
            Plot2DHistos();
            g_rtPlot2 += duration<double, std::micro>(steady_clock::now() - t0)
                             .count();
        }
        serviceRootEvents(); // the 2D canvases are painted: service input now

        // The spectrum canvases are the most expensive ones, so they run at a
        // slower cadence than the images, one per tick in round-robin: a
        // single redraw never pays for both.  Only ONE set is on screen - the
        // ADU pair or the keV pair, selected by the "keV PH" checkbox.
        int every = chipUpdateEvery.load();
        if (every < 1)
            every = 1;
        static int chipTick = 0;
        static int rot = 0;
        if ((chipTick++ % every) == 0) {
            // the keV view follows the gain stage; no map -> nominal 40 ADU/keV
            const bool kev = kevSpectra.load() && gain_flag.load();
            setSpectrumTitles(kev);
            const auto t0 = steady_clock::now();
            if (kev) {
                if ((rot % 2) == 0)
                    Plot1DHistosKev();
                else
                    PlotRoiHistosKev();
            } else {
                if ((rot % 2) == 0)
                    Plot1DHistos();
                else
                    PlotRoiHistos();
            }
            const double us =
                duration<double, std::micro>(steady_clock::now() - t0).count();
            if (kev && (rot % 2) == 0) {
                g_rtPlotKev += us;
                g_rtNPlotKev++;
            } else if (kev) {
                g_rtPlotRoiKev += us;
                g_rtNPlotRoiKev++;
            } else if ((rot % 2) == 0) {
                g_rtPlot1 += us;
                g_rtNPlot1++;
            } else {
                g_rtPlotRoi += us;
                g_rtNPlotRoi++;
            }
            rot++;
            serviceRootEvents();
        }
        g_rtRedraws++;
        // keep the TvsB/LvsR scatter windows fresh: the analysis thread
        // resets them before the first fill of the next period (the photon
        // map stays cumulative)
        gSoftResetPending.store(true);
      } catch (const std::exception &e) {
        fprintf(stderr, "[root] exception: %s (ROOT drawing stopped)\n",
                e.what());
        fflush(stderr);
        break;
      } catch (...) {
        fprintf(stderr, "[root] unknown exception (ROOT drawing stopped)\n");
        fflush(stderr);
        break;
      }
    }
}

} // namespace

// ---------------------------------------------------------------------------
// Plotting helpers (ROOT thread only, except where noted)
// ---------------------------------------------------------------------------
void serviceRootEvents() {
    if (gSystem)
        gSystem->ProcessEvents();
}

// ---------------------------------------------------------------------------
// Palettes: a single definition per palette feeds both ROOT (gradient colour
// table) and the fast image widgets (256-entry RGB lookup table), so the
// photon map and the ADC image always look the same.
// ---------------------------------------------------------------------------
namespace {

struct PaletteDef {
    // piecewise-linear colour stops (used when nstops > 0)
    int nstops = 0;
    double stops[5] = {0, 0, 0, 0, 0};
    double r[5] = {0, 0, 0, 0, 0};
    double g[5] = {0, 0, 0, 0, 0};
    double b[5] = {0, 0, 0, 0, 0};
    // gray ramp exponent (used when nstops == 0)
    double gamma = 1.0;
};

const PaletteDef &paletteDef(Palette p) {
    // "False" colour table, as used by the original display
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
    switch (p) {
    case Palette::BW1:
        return bw1;
    case Palette::BW2:
        return bw2;
    case Palette::Color:
    default:
        return color;
    }
}

// 256-entry lookup table sampled from exactly the same definition
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

QRgb g_lutColor[256];
QRgb g_lutBW1[256];
QRgb g_lutBW2[256];
struct LutInit {
    LutInit() {
        buildLut(paletteDef(Palette::Color), g_lutColor);
        buildLut(paletteDef(Palette::BW1), g_lutBW1);
        buildLut(paletteDef(Palette::BW2), g_lutBW2);
    }
} g_lutInit;

} // namespace

const QRgb *paletteLut(Palette p) {
    switch (p) {
    case Palette::BW1:
        return g_lutBW1;
    case Palette::BW2:
        return g_lutBW2;
    case Palette::Color:
    default:
        return g_lutColor;
    }
}

// ROOT-thread only: rebuild the ROOT gradient colour table
void loadRootPalette(Palette p) {
    const PaletteDef &d = paletteDef(p);
    const Int_t NCont = 90;
    Double_t stops[99], red[99], green[99], blue[99];
    Int_t n;
    if (d.nstops <= 0) {
        n = 99;
        for (int i = 0; i < n; i++) {
            const double s = static_cast<double>(i) / (n - 1);
            stops[i] = s;
            const double c = std::pow(s, d.gamma);
            red[i] = green[i] = blue[i] = c;
        }
    } else {
        n = d.nstops;
        for (int i = 0; i < n; i++) {
            stops[i] = d.stops[i];
            red[i] = d.r[i];
            green[i] = d.g[i];
            blue[i] = d.b[i];
        }
    }
    TColor::CreateGradientColorTable(n, stops, red, green, blue, NCont);
    gStyle->SetNumberContours(NCont);
}

// The 8 chip spectra live in a 4x2 pad grid whose TOP row is the TOP half of
// the detector (chips 4..7 = data rows 256..511).  Both spectrum canvases must
// use the same mapping so they can be compared pad by pad.
int chipPadToHist(int ipad) { return ipad % 4 + (1 - int(ipad / 4)) * 4; }

// shares the x axis of every spectrum of one canvas with its first pad (the
// operator zooms one pad and all of them follow, as in the original display).
// Each canvas keeps its own state so the two cannot interact.
struct ChipXSync {
    int first = -1, last = -1;
    bool have = false;
};
ChipXSync g_aduXSync, g_kevXSync;
void syncChipXRange(TH1I **h, ChipXSync &st) {
    if (!h || !h[0])
        return;
    if (!st.have || h[0]->GetXaxis()->GetLast() != st.last) {
        st.first = h[0]->GetXaxis()->GetFirst();
        st.last = h[0]->GetXaxis()->GetLast();
        st.have = true;
        for (int ipad = 1; ipad < 8; ipad++) {
            if (h[ipad])
                h[ipad]->GetXaxis()->SetRange(st.first, st.last);
        }
    }
}

// the ROI spectrum is read against the chip spectra, so it always mirrors the
// x range of the chip canvas (and only A4's: the two sync states must not
// interact)
void mirrorChipXRangeToRoi() {
    if (!dispChip[0])
        return;
    const int first = dispChip[0]->GetXaxis()->GetFirst();
    const int last = dispChip[0]->GetXaxis()->GetLast();
    // the keV canvases share the same window: their axes are the ADU axes
    // divided by KEV_NOMINAL_ADU, so the bin indices are identical
    TH1I *dst[10] = {dispRoi,     dispRoiKev,     dispChipKev[0],
                     dispChipKev[1], dispChipKev[2], dispChipKev[3],
                     dispChipKev[4], dispChipKev[5], dispChipKev[6],
                     dispChipKev[7]};
    for (TH1I *h : dst) {
        if (!h)
            continue;
        if (h->GetXaxis()->GetFirst() != first ||
            h->GetXaxis()->GetLast() != last)
            h->GetXaxis()->SetRange(first, last);
    }
}

// paints one 4x2 canvas of 8 chip spectra (canvas A4 in ADU, A8 in keV)
void plotChipCanvas(TH1I **h, TCanvas *c, ChipXSync &st) {
    if (!h || !h[0] || !c)
        return;
    syncChipXRange(h, st); // all pads follow the first one
    gStyle->SetOptStat(1);
    for (int ipad = 0; ipad < 8; ipad++) {
        c->cd(ipad + 1);
        if (gPad && !gPad->GetLogy())
            gPad->SetLogy();
        // "L" draws one polyline through the bin contents.  The default
        // (markers + error bars) is ~2000 X11 operations per pad, which costs
        // hundreds of ms per redraw and starves the event loop.
        h[chipPadToHist(ipad)]->Draw("L");
        if (ipad == 3)
            serviceRootEvents(); // keep the canvases interactive mid-redraw
    }
    c->cd();
    c->Update();
}

// paints the canvas holding the single ROI spectrum (A7 in ADU, A9 in keV).
// The window title carries the ROI rectangle it was filled with.
void plotRoiCanvas(TH1I *h, TCanvas *c, const char *what) {
    if (!h || !c)
        return;
    static int lx0 = -1, ly0 = -1, lx1 = -1, ly1 = -1;
    static bool lastOn = false, have = false, titled = false;
    const bool on = ROIactive.load();
    const int x0 = ROIxmin.load(), y0 = ROIymin.load();
    const int x1 = ROIxmax.load(), y1 = ROIymax.load();
    if (!have || !titled || on != lastOn || x0 != lx0 || y0 != ly0 ||
        x1 != lx1 || y1 != ly1) {
        have = true;
        titled = true;
        lastOn = on;
        lx0 = x0;
        ly0 = y0;
        lx1 = x1;
        ly1 = y1;
        char t[160];
        if (on)
            snprintf(t, sizeof(t),
                     "%s - gate ROI x %d..%d, y %d..%d (%dx%d px, data rows, "
                     "row 0 = bottom)",
                     what, x0, x1, y0, y1, x1 - x0, y1 - y0);
        else
            snprintf(t, sizeof(t), "%s - no gate ROI (use \"ROI only\")",
                     what);
        c->SetTitle(t); // TCanvas::SetTitle also renames the window
    }
    gStyle->SetOptStat(1);
    c->cd();
    if (gPad && !gPad->GetLogy())
        gPad->SetLogy();
    h->Draw(); // normal histogram rendering (markers + error bars)
    c->cd();
    c->Update();
}

// the operator's PH window is shared by every spectrum canvas
void Plot1DHistos(void) {
    mirrorChipXRangeToRoi();
    plotChipCanvas(dispChip, A4, g_aduXSync);
}

// title of the shared canvases, so it is obvious which set is on screen
void setSpectrumTitles(bool kev) {
    static int shown = -1;
    if (shown == (kev ? 1 : 0))
        return;
    shown = kev ? 1 : 0;
    if (A4)
        A4->SetTitle(kev ? "PH spectra [keV]" : "PH spectra [ADU]");
    if (A7)
        A7->SetTitle(kev ? "ROI PH [keV]" : "ROI PH [ADU]");
}

// ONE spectrum of the gate-ROI pixels (canvas A7): same binning and same
// border trim as the chip spectra, but summed over the whole selection, so it
// answers "what do the pixels I selected look like?" and can be compared with
// the sum of the eight pads of A4.
void PlotRoiHistos(void) {
    if (!dispRoi || !A7)
        return;
    plotRoiCanvas(dispRoi, A7, "ROI PH");
}

// the same two canvases in keV, filled and drawn only when a gain map is
// loaded (canvases A8/A9)
// the same canvas, filled from the keV histograms instead of the ADU ones
void Plot1DHistosKev(void) {
    if (!kevSpectra.load() || !gain_flag.load())
        return;
    // the ROI canvas mirrors the chips, exactly as in the ADU set
    if (dispChipKev[0] && dispRoiKev) {
        const int f = dispChipKev[0]->GetXaxis()->GetFirst();
        const int l = dispChipKev[0]->GetXaxis()->GetLast();
        if (dispRoiKev->GetXaxis()->GetFirst() != f ||
            dispRoiKev->GetXaxis()->GetLast() != l)
            dispRoiKev->GetXaxis()->SetRange(f, l);
    }
    plotChipCanvas(dispChipKev, A4, g_kevXSync);
}

void PlotRoiHistosKev(void) {
    if (!kevSpectra.load() || !gain_flag.load())
        return;
    plotRoiCanvas(dispRoiKev, A7, "ROI PH [keV]");
}

// ROOT maps an explicit axis range onto its pad frame, so a range whose shape
// differs from the frame is stretched: a narrow/tall ROI came out 5x wider than
// tall.  Widen the shorter side of the range to the frame's shape - centred on
// the ROI and shifted (never resized) to stay on the sensor - so the region
// keeps the shape of the ADC window, the ROI box keeps its shape and the
// pixels stay square.  The extra margin around the ROI is visible, so the box
// can be seen sitting inside the plot.
void zoomPhotonMap(double x0, double x1, double y0, double y1) {
    if (!A5 || !disp3000)
        return;
    double dx = x1 - x0, dy = y1 - y0;
    if (dx <= 0 || dy <= 0)
        return;
    double W = A5->GetWw(), H = A5->GetWh();
    if (W < 10 || H < 10) { // not mapped yet: use the booked size
        W = 1000;
        H = 600;
    }
    // frame area in pixels (the z palette lives in the right margin)
    const double fw =
        1.0 - gStyle->GetPadLeftMargin() - gStyle->GetPadRightMargin();
    const double fh =
        1.0 - gStyle->GetPadBottomMargin() - gStyle->GetPadTopMargin();
    const double fx = W * fw, fy = H * fh;
    if (fx / dx > fy / dy)
        dx = dy * fx / fy; // x is magnified more: widen it
    else
        dy = dx * fy / fx;
    // centre on the requested window, then shift it inside the sensors range
    const double cx = 0.5 * (x0 + x1), cy = 0.5 * (y0 + y1);
    const double ax0 = disp3000->GetXaxis()->GetXmin();
    const double ax1 = disp3000->GetXaxis()->GetXmax();
    const double ay0 = disp3000->GetYaxis()->GetXmin();
    const double ay1 = disp3000->GetYaxis()->GetXmax();
    double nx = cx - dx / 2.0, ny = cy - dy / 2.0;
    if (nx < ax0)
        nx = ax0;
    if (nx + dx > ax1)
        nx = ax1 - dx;
    if (nx < ax0)
        nx = ax0; // wider than the sensor: ROOT clamps the selection
    if (ny < ay0)
        ny = ay0;
    if (ny + dy > ay1)
        ny = ay1 - dy;
    if (ny < ay0)
        ny = ay0;
    disp3000->GetXaxis()->SetRangeUser(nx, nx + dx);
    disp3000->GetYaxis()->SetRangeUser(ny, ny + dy);
}

// redraws the photon map (used by Plot2DHistos and by the zoom command)
void plotPhotonMap() {
    if (!A5 || !disp3000)
        return;
    A5->cd();
    disp3000->Draw("colz");
    drawRoiBox();
    A5->Update();
}

void drawRoiBox() {
    if (box) {
        if (gPad && gPad->GetListOfPrimitives())
            gPad->GetListOfPrimitives()->Remove(box);
        delete box;
        box = nullptr;
    }
    if (!ROIactive.load())
        return; // no ROI: nothing to outline
    const double x1 = ROIxmin.load();
    const double x2 = ROIxmax.load();
    const double y1 = ROIymin.load();
    const double y2 = ROIymax.load();
    if (x2 <= x1 || y2 <= y1)
        return;
    box = new TBox(x1, y1, x2, y2);
    box->SetLineColor(kYellow); // same colour as the ADC overlay
    box->SetFillStyle(0);
    box->SetLineWidth(2);
    box->Draw();
}

void Plot2DHistos(void) {
    gStyle->SetOptStat(0);
#ifdef JFSTRX
    if (p5 && disp1000) {
        p5->cd();
        disp1000->Draw("colz");
    }
    if (p3 && disp1001) {
        p3->cd();
        disp1001->Draw("colz");
    }
    if (p1 && disp1002) {
        p1->cd();
        disp1002->Draw("colz");
    }
    if (p2 && disp1060) {
        p2->cd();
        disp1060->Draw("colz");
    }
    if (p4 && disp1061) {
        p4->cd();
        disp1061->Draw("colz");
    }
    if (p6 && disp1062) {
        p6->cd();
        disp1062->Draw("colz");
    }
#endif
    if (A5 && disp3000)
        plotPhotonMap();
    serviceRootEvents(); // the photon map is painted: service input
    if (A6 && disp4500 && dispLvsR) {
        A6->cd();
        if (!A6->GetListOfPrimitives() ||
            !A6->GetListOfPrimitives()->GetSize()) {
            A6->Divide(1, 2);
        }
        A6->cd(1);
        disp4500->Draw("colz");
        A6->cd(2);
        dispLvsR->Draw("colz");
        A6->Update();
    }
}

void axisreset() { // ROOT-thread only (executed via the command queue)
    fixranges.store(false);
#ifdef JFSTRX
    TH2F *jf[6] = {disp1000, disp1001, disp1002, disp1060, disp1061, disp1062};
    for (int k = 0; k < 6; k++) {
        if (!jf[k])
            continue;
        jf[k]->GetXaxis()->UnZoom();
        jf[k]->GetYaxis()->UnZoom();
        jf[k]->GetZaxis()->UnZoom();
    }
#endif
    if (disp3000) {
        disp3000->GetXaxis()->UnZoom();
        disp3000->GetYaxis()->UnZoom();
        disp3000->GetZaxis()->UnZoom();
    }
    for (int i = 0; i < 8; i++) {
        if (dispChip[i]) {
            dispChip[i]->GetXaxis()->UnZoom();
            dispChip[i]->GetYaxis()->UnZoom();
        }
    }
    if (dispRoi) {
        dispRoi->GetXaxis()->UnZoom();
        dispRoi->GetYaxis()->UnZoom();
    }
    if (dispRoiKev) {
        dispRoiKev->GetXaxis()->UnZoom();
        dispRoiKev->GetYaxis()->UnZoom();
    }
    for (int i = 0; i < 8; i++) {
        if (dispChipKev[i]) {
            dispChipKev[i]->GetXaxis()->UnZoom();
            dispChipKev[i]->GetYaxis()->UnZoom();
        }
    }
    if (disp4500) {
        disp4500->GetXaxis()->UnZoom();
        disp4500->GetYaxis()->UnZoom();
        disp4500->GetZaxis()->UnZoom();
    }
    if (dispLvsR) {
        dispLvsR->GetXaxis()->UnZoom();
        dispLvsR->GetYaxis()->UnZoom();
        dispLvsR->GetZaxis()->UnZoom();
    }
    Plot2DHistos();
    Plot1DHistos();
}
