// SPDX-License-Identifier: LGPL-3.0-or-other
// ===========================================================================
// online_jf.h -- merged online Jungfrau display + event finder
// ===========================================================================
//
// This is a rewrite of TWO separate programs into ONE multithreaded process:
//
//   onlinedisp_zmq.cpp     Qt5 GUI + ROOT plots, subscribed to the filter
//   jf_filter.cpp          ZMQ sub -> pedestal subtract -> threshold gate -> pub
//   jf_event_finder_gui.cpp  Qt control window for that filter
//
// The intermediate ZMQ hop between filter and display is gone.  A single
// subscriber reads slsReceiver's stream, the gate decides pass/drop on every
// frame, and only passing frames are handed to the display/analysis stage.
//
// Thread map
// ----------
//   main / Qt thread : QApplication::exec, all widgets, button handlers
//   acq thread       : zmq_recv(header + data), gate counters, optional ZMQ
//                      republish, hand passing frames to the frame mailbox
//   analysis thread  : pedestal subtraction, ADC/gain images, and the ONLY
//                      thread that FILLS the source ROOT histograms
//   ROOT thread      : canvas painting + gSystem->ProcessEvents; executes the
//                      user ROOT command queue, paints display clones
//
// Invariants carried over from the originals
// ------------------------------------------
//   * ROOT graphics (Draw/Update/ProcessEvents) run ONLY on the ROOT thread.
//     Every other thread posts RootCmd entries.
//   * The source histograms (his*, hchip) are filled ONLY by the analysis
//     thread; the ROOT thread paints display clones whose bins are copied
//     once per redraw.  Never paint an object that is being filled.
//   * Pedestals are immutable once published (shared_ptr swap), so loading a
//     file or acquiring a live pedestal no longer needs stop()/load()/start().
//   * The gate threshold and the display's photon threshold are DIFFERENT
//     quantities (they collided as one global named "threshold" only because
//     they used to live in two processes).
//
// Argument compatibility (superset of both originals)
// ---------------------------------------------------
//   jf_online_display_gui <rx_host> [rx_port] [gate_threshold] [min_pixels]
//                  [pede_file] [--pub PORT] [--contig] [--no-gate]
//   energy < 0  ->  raw ADC map (no photon counting), as onlinedisp_zmq.
#ifndef ONLINE_JF_H
#define ONLINE_JF_H

#define SLS_DETECTOR_JSON_HEADER_VERSION 0x5

/* ------------------------------------------------------------------ std */
#include <algorithm>
#include <array>
#include <atomic>
#include <cctype>
#include <chrono>
#include <cmath>
#include <condition_variable>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <rapidjson/document.h> // json header inside the zmq stream

/* ----------------------------------------------------------------- ROOT */
#include "TCanvas.h"
#include "TH1F.h"
#include "TF1.h"
#include "TH1I.h"
#include "TH2F.h"
#include "TMath.h"
#include "TFile.h"
#include "TStyle.h"
#include "TBox.h"
#include "TSystem.h"
#include "TTimer.h"
#include "TProfile.h"
#include "TColor.h"
#include "TList.h"
#include "TPad.h"
#include <TApplication.h>

/* ------------------------------------------------------------------ Qt5 */
#include <QApplication>
#include <QCheckBox>
#include <QCloseEvent>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QDoubleSpinBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QImage>
#include <QKeySequence>
#include <QLabel>
#include <QLCDNumber>
#include <QLineEdit>
#include <QLocale>
#include <QMainWindow>
#include <QMessageBox>
#include <QMouseEvent>
#include <QMutex>
#include <QPainter>
#include <QPaintEvent>
#include <QPushButton>
#include <QRadioButton>
#include <QShortcut>
#include <QSpinBox>
#include <QAbstractItemView>
#include <QDateTime>
#include <QHeaderView>
#include <QStatusBar>
#include <QTableWidget>
#include <QStringList>
#include <QThread>
#include <QTimer>
#include <QVBoxLayout>
#include <QWheelEvent>
#include <QWidget>

using namespace std; // legacy style of the original sources
using namespace std::chrono;

/* ------------------------------------------------------------- geometry */
#define PI 3.14159265
#define OFFSET 0
#define NBIN 500
#define MIN_POS -500.5 // 400.5
#define MAX_POS 8499.5 //-100.5

// A Jungfrau module frame: 1024 x 512 = 524288 pixels (16 bit).
#define NCH 524288
// The raw module geometry of one UDP port.  This is fixed and independent of
// nx/ny, which under JFSTRX hold the strixel *display* geometry: the data,
// gate, ROI and HDF5 paths always work in these raw coordinates.
constexpr int kRawWidth = 1024;
constexpr int kRawHeight = 512;
static_assert(kRawWidth * kRawHeight == NCH, "raw geometry vs NCH");
// Legacy pedestal-file array size: 3 arrays of 2*NCH floats each
// (the factor 2 is a leftover of an early half-module layout).
#define JF_PEDE_ARRAY_FLOATS (2 * NCH)
#define PEDEFNAME "current_pede.dat"
#define DEFAULT_RX_PORT 30001
#define DEFAULT_N_PEDE_FRAMES 50
// Multi-interface mode: slsReceiver with N UDP interfaces ("threads")
// publishes one ZMQ stream per interface on consecutive ports, each carrying
// one Y-slab of the module:
//   JungfrauData::nPixelsX = 256*4 = 1024
//   JungfrauData::nPixelsY = (256*2)/numUDPInterfaces       -> 256 for N=2
// so 30001+30002 delivers two 1024x256 halves that stack into 1024x512.
#define JF_MAX_PORTS 8

/* --------------------------------------------------------------- config */
struct Config {
    // The photon map threshold is a GUI control in ADU (default 150); the
    // photon multiplicity used when filling the map is derived from it
    // (photon peak ~= 2 x threshold), exactly as it used to be derived from
    // the <energy> argument.
    int phene = 300;           // photon peak in ADC = 2 x the ADU threshold
    bool dophotonmap = true;   // false -> his3000 shows raw ADC
    bool rawMap = false;       // --raw-map: show raw ADC instead of counts
    std::string rxHost;        // slsReceiver host
    // ports, ordered BOTTOM -> TOP in the assembled image.  One entry is the
    // classic single-interface mode; "30001+30002" selects two interfaces.
    std::vector<int> rxPorts{DEFAULT_RX_PORT};
    float threshold = 100.0f;  // GATE threshold (pede-subtracted ADC)
    uint64_t minPixels = 5;    // GATE min pixels above threshold
    bool contiguous = false;   // GATE contiguity proxy
    bool gateEnabled = false;  // default: gate off (passthrough); --gate enables
    std::string pedeFile = "./current_pede.dat";
    // calibration for the PH spectra (keV axes); empty = ADU only
    std::string gainMapFile;
    bool gainMapExplicit = false;
    // Gate-ROI list: a single small file in the program folder, rewritten on
    // every save/load/delete.  See roiList*() below.
    std::string roiFile = "./roi_list.dat";
    // directory and file-name base for the HDF5 ROI files + JSON sidecars
    std::string h5Dir = ".";
    std::string h5Base = "jfroi";
    // list of previously used destinations (directory + base), newest first
    std::string saveListFile = "./save_dest_list.dat";
    bool pedeExplicit = false; // user named the file explicitly
    int pubPort = 0;           // 0 -> no republish of gated frames
    int pedeFrames = DEFAULT_N_PEDE_FRAMES;
    bool printCounters = false; // print the old CLI counter line on stdout
    bool acquirePede = false;   // start a live pedestal acquisition at boot
    bool startSave = false;     // press Save at startup (--start-save)
    bool applyGain = false;     // gain correction on at startup (--apply-gain)
    uint64_t saveBudget = 0;    // 0 = keep the GUI default (--save-budget)
    // Analysis worker threads for the per-pixel loops.
    // 0 = auto (cores/4, clamped to 1..8); 1 = single threaded.
    // The loops are pure per-pixel work on disjoint ranges with no ROOT
    // access, so they scale across cores; see the README.
    int analysisWorkers = 0;
    // Receive high-water mark per interface.  Bounded on purpose: the program
    // consumes ~1.2-1.3 kframes/s, so a deep queue would only buy latency and
    // make the catch-up skip expensive.  Frames beyond this are dropped by
    // ZMQ before they cost us anything.
    int rxHwm = 64;
    // Optional storage-cell selection at startup (--sc N, 0..15; -1 = all)
    int scFilter = -1;
    // Optional gate ROI at startup (--gate-roi X Y W H), in DATA coordinates:
    // row 0 is the BOTTOM row, same as the ROI list.  gateRoiX < 0 = unset.
    int gateRoiX = -1, gateRoiY = 0, gateRoiW = 0, gateRoiH = 0;
};

extern Config g_cfg;

/* ------------------------------------------------------ pedestal tracking */
// The pedestals drift a little, so while the beam is off (the signal gate is
// dropping almost everything) a small fraction of each dropped frame is fed
// into a per-pixel running average, and the pedestal actually used is replaced
// when that average has moved far enough.  Defaults:
//   * a sample of 0.2 % of the pixels of every dropped frame,
//   * per-pixel exponential average with a 10 M *pixel-sample* window
//     (10 M / NCH ~ 19 updates per pixel, so the weight per update is
//     NCH / 10 M ~ 0.05),
//   * only while the signal gate drops more than 90 % of the frames reaching
//     it, and while "Gate enabled" is on,
//   * the used pedestal is replaced when the largest per-pixel difference
//     exceeds 5 ADU - at most once per 10 s, and at least once per 10 min,
//   * the HDF5 file gets a pedestal image every 10 min while tracking is on.
#define PEDE_TRACK_FRACTION 0.002
#define PEDE_TRACK_WINDOW 1.0e7
#define PEDE_TRACK_MAXDIFF 5.0f
#define PEDE_TRACK_MIN_PERIOD_MS 10000
#define PEDE_TRACK_MAX_PERIOD_MS 600000
#define PEDE_TRACK_H5_PERIOD_MS 600000
#define PEDE_TRACK_DROP_RATE 0.90

struct PedeSet; // defined with the pedestal store below

extern std::atomic<bool> gPedeTracking; // "Pedestal tracking" button
extern std::atomic<bool> gPedeTracked;  // true once the pedestal was replaced

struct PedeTrackStats {
    bool on = false;
    bool tracked = false;    // the pedestal in use came from the tracker
    uint64_t samples = 0;    // pixel samples fed into the average
    uint64_t updates = 0;    // times the used pedestal was replaced
    double maxDiff = 0.0;    // largest |average - used| in ADU
    int64_t sinceUpdateMs = 0;
};
PedeTrackStats pedeTrackerStats();
// Fraction of the frames reaching the signal gate that it dropped, over the
// last 1 Hz window (falls back to the held value, then the cumulative one, so
// a quiet window does not read as "no drops").
double pedeTrackSignalDropRate(uint64_t passCur, uint64_t dropCur,
                               uint64_t passTotal, uint64_t dropTotal);
// Called by the acq thread for a frame the *signal* gate dropped.  Cheap when
// tracking is off or when the drop rate is below the threshold.
// The 5 ADU test looks only at the gate ROI (roiSet == false -> whole frame);
// the average itself is still accumulated over the whole frame.
void pedeTrackerFeed(const short *frame, uint64_t nPixels, const PedeSet *peds,
                     double signalDropRate, int roiX0, int roiY0, int roiX1,
                     int roiY1);
// Re-primes the running average from the pedestal now in use (called whenever a
// pedestal is installed, so a fresh file starts from itself).
void pedeTrackerReset();

/* ------------------------------------------------------------ gain map */
// Calibration: the gain-map file holds SIX 1024x512 images of doubles, in the
// order g0, g1, g2, hg0, hg1, hg2.  In normal mode g0 holds ADU/keV per
// pixel, so a pedestal-corrected ADC value becomes
//     keV = ADU / g0[pixel]
// Only the PH spectra are converted; the ADC image, the photon map and the
// HDF5 writer keep working in ADU.  KEV_NOMINAL_ADU gives the keV axes the
// same span as the ADU ones (the operator's PH window divided by 40), so the
// two sets of spectra show the same physical region.
#define JF_GAINMAP_IMAGES 6
#define KEV_NOMINAL_ADU 40.0
#define JF_GAINMAP_PIXELS (1024 * 512)
struct GainMap {
    std::vector<double> img[JF_GAINMAP_IMAGES];
    GainMap() {
        for (int i = 0; i < JF_GAINMAP_IMAGES; i++)
            img[i].assign(JF_GAINMAP_PIXELS, 0.0);
    }
    // the map used for the conversion (normal mode)
    const double *g0() const { return img[0].data(); }
};
class GainMapStore {
  public:
    static GainMapStore &instance();
    // snapshot; empty shared_ptr means "no calibration, spectra stay in ADU"
    std::shared_ptr<const GainMap> get() const;
    void install(const std::shared_ptr<GainMap> &m, const std::string &path);
    bool loaded() const;
    std::string path() const;

  private:
    mutable std::mutex mtx_;
    std::shared_ptr<const GainMap> cur_;
    std::string path_;
};
// Reads the 6 x 1024 x 512 doubles and installs them.  Returns false + error
// when the file is missing or has the wrong size.
bool gainMapLoadFile(const std::string &path, std::string &error);
// true once a usable map is installed (the keV spectra are filled and drawn)
bool gainMapLoaded();
// Which of the two PH sets the ROOT thread paints: false = ADU (A4/A7),
// true = keV (A8/A9).  Both sets are always accumulated; this only selects
// what is on screen, and it falls back to ADU when no gain map is loaded.
extern std::atomic<bool> kevSpectra;

/* ------------------------------------------------------- display state */
// defined in online_jf_core.cpp
extern short *hchptr; // cumulative photon counted map
extern float factor;
#define PHOTON_THRESHOLD_DEFAULT 150.0f // ADU, Virtual photon counting control
#define PHOTON_THRESHOLD_KEV_DEFAULT 4.0f // keV, same control with gain on
// Photon map for its own Qt window: how many analysed frames between publishes
// (default 2000), and whether it keeps adding up (off by default, so each image
// shows the photons counted since the previous one).
extern std::atomic<int> gPmapEvery;
extern std::atomic<bool> gPmapCumulative;
// Saving mode: 0 = gate-passed frames, 1 = the virtual photon counting maps.
// gPmapSaveEvery is their cadence in analysed frames (minimum 200).
extern std::atomic<int> gSaveMode;
extern std::atomic<int> gPmapSaveEvery;
void pmapSaveReset();

// One photon map on its way from the analysis (which counts the photons) to the
// acq thread (which owns the saver).
struct PmapSlot {
    std::mutex mtx;
    std::vector<float> map; // NCH, photons per pixel
    uint64_t frames = 0;    // analysed frames counted into this map
    uint64_t seq = 0;
    bool ready = false;
};
// Set when the operator confirms a gate-criterion change while a run is
// writing: the saved file keeps the criterion it started with, so its JSON has
// to say that it was moved.
extern std::atomic<bool> gGateChangedDuringSave;
extern float g_pheneThreshold; // photon-map threshold in ADU (GUI, default 150)
// Same control while the gain correction is on: the counting value is then
// adcpedecorr / ADU-per-keV, so the threshold means keV (default 4).  Both are
// kept, and switching the gain swaps which one the spinbox shows.
extern float g_pheneThresholdKev;

extern std::atomic<bool> gain_flag;
extern std::atomic<bool> dophotonmap;
extern std::atomic<int> goout;        // 1 = process, 0 = paused
extern std::atomic<int> HDraw_every;
// While the gate passes frames slowly, "every N frames" would freeze the image,
// so a frame is also displayed when the previous one is older than this.
#define DISPLAY_FLOOR_MS 250
extern std::atomic<uint64_t> gImagesShown; // images handed to the Qt view  // main image every N frames
// ROOT redraws of the chip spectra every N-th redraw (they are the most
// expensive canvas; 1 = every redraw)
extern std::atomic<int> chipUpdateEvery;
extern std::atomic<int> rootUpdateMs; // ROOT redraw period
// Gain-1 / gain-3 pedestal offsets used by the correction.  The GUI controls
// for them were removed on request, so they are constant 0 for now; they are
// kept because the planned AFC/gain-split work needs per-gain offsets.
extern std::atomic<int> G1Poffset;
extern std::atomic<int> G2Poffset;
// Axis ranges (Set Ranges dialog), one pair per display:
//   adcmin/adcmax   colour scale of the ADC image (the Qt "main plot")
//   pmmin/pmmax     colour scale of the ROOT 2D map (canvas A5)
//   chipxmin/max    shared x window of the spectra (A4 and the ROI spectrum A7)
// The dialog opens on the scale each display is using right now (it can be
// dragged on the image colour bar or on the ROOT canvas) and applies only what
// the operator actually changed.
extern std::atomic<int> adcmin, adcmax, pmmin, pmmax, chipxmin, chipxmax;
extern std::atomic<bool> fixranges; // a fixed ROOT range is in force (Reset
                                    // Axis releases it and it auto-scales)
extern std::atomic<uint64_t> framesinstream; // frames reaching the display
extern std::atomic<uint64_t> gFramesForRoot; // frames since last ROOT redraw

// analysis-thread accumulators (single owner: the analysis thread)
extern short image_data[JF_PEDE_ARRAY_FLOATS];
extern float adc_image[NCH];
extern float gain_image[NCH];
// legacy gain map: never initialised to anything but 0 in the original
// sources, so the "gain correction" toggle multiplies by zero for gain 0.
extern float fgaind[JF_PEDE_ARRAY_FLOATS];
extern int nx, ny;

// The GATE ROI currently shown on screen, in DATA coordinates: x grows to the
// right, y grows upwards, (0,0) is the BOTTOM-LEFT pixel.  This is the one and
// only rectangle the operator sees: the yellow box on the ADC image, the box
// on the ROOT photon map and the ROI spectra all read it.  Mirrored once per
// analysed frame by the analysis thread from the GateEngine snapshot.
// ROIactive == false means "no ROI set"; the rectangle is then the full frame
// and nothing is drawn.
extern std::atomic<uint16_t> ROIxmin, ROIxmax, ROIymin, ROIymax;
extern std::atomic<bool> ROIactive;
// The ROI the RECEIVER is configured with, from the "rx_roi" array in the ZMQ
// JSON header ([xmin, xmax, ymin, ymax], raw port coordinates).  It is NOT the
// gate ROI and is drawn as a separate RED box on the ADC image; rxRoiValid is
// false until a non-degenerate one has been seen.
extern std::atomic<uint16_t> rxRoiXmin, rxRoiXmax, rxRoiYmin, rxRoiYmax;
extern std::atomic<bool> rxRoiValid;

// cross-thread reset / redraw requests
extern std::atomic<bool> gSoftResetPending; // windowed TvsB/LvsR reset
extern std::atomic<bool> gRedrawRequest;    // force a ROOT redraw
extern std::atomic<bool> gFrameCountReset;  // restart display frame count
// "Reset His": the GUI sets this and the ANALYSIS thread performs the reset at
// the top of its next frame, while it already holds gHistoMtx.  The ROOT thread
// must never wait for that lock - under a heavy stream (JFSTRX fills) it can
// starve, which is what made the reset take tens of seconds.
extern std::atomic<bool> gResetHisPending;
// "No pede": ignore the loaded pedestal everywhere - the display, the photon
// map, the spectra, the event finder and the ROI save all work on raw ADC.
// Set by the button in the Pedestal panel; drawn as a bypass in the flow.
extern std::atomic<bool> gNoPede;
// Protects the source histograms against a Reset() racing the analysis
// thread's per-frame Fill().  copyBins() is deliberately NOT protected: the
// original sources accepted a torn read during a redraw, and taking the lock
// for a 524288-bin copy would stall the analysis thread every redraw.
extern std::mutex gHistoMtx;

/* ------------------------------------------------------- ROOT objects */
// defined in online_jf_root.cpp
extern TApplication *rootapp;
extern TCanvas *A2, *A3, *A4, *A5, *A6, *A7;
extern TPad *p1, *p2, *p3, *p4, *p5, *p6;
extern TH1I **hchip; // 8 chip / gain-group spectra
extern TH1I *hroi;   // one spectrum over the gate-ROI pixels only
// the same two spectra again in keV (canvases A8/A9); they are filled and
// drawn only when a gain map is loaded
extern TH1I **hchipKev;
extern TH1I *hroiKev;
extern TH1I *hproj, *hchcum;
extern TH2F *his1000, *his1001, *his1002, *his1060, *his1061, *his1062;
extern TH2F *his2000, *his3000, *his4500, *hisLvsR;
extern TBox *box;

// display clones painted by the ROOT thread (single-owner objects)
extern TH2F *disp3000, *disp4500, *dispLvsR;
extern TH1I *dispChip[8];
extern TH1I *dispRoi;
extern TH1I *dispChipKev[8];
extern TH1I *dispRoiKev;
extern TH2F *disp1000, *disp1001, *disp1002, *disp1060, *disp1061, *disp1062;

// ROOT thread control + command queue
enum class RootCmdType {
    None,
    ResetHis,
    ResetAxes,
    // palette changes are applied to the ROOT gradient table AND to the fast
    // image widgets, so both show the same colours
    PaletteColor,
    PaletteBW1,
    PaletteBW2,
    Zoom,
    SetChipRange,
    // colour scale of the ROOT 2D map (canvas A5): a, b = min, max
    SetMapRange
};

// One palette definition drives both the ROOT colour table and the 256-entry
// lookup table used by the fast ADC/photon image widgets.
// The palettes offered by the image scale panel.  Color / BW1 / BW2 are the
// original three (kept first: the b / Shift+B / c shortcuts use those indices);
// the rest are the matplotlib family that is useful for detector images -
// perceptually uniform (viridis, plasma, inferno, magma, cividis), turbo,
// hot, grey, and the diverging maps for pedestal-corrected data that can go
// negative.
enum class Palette {
    Color, BW1, BW2,
    Turbo, Viridis, Plasma, Inferno, Magma, Cividis,
    Hot, Gray, Gray_r,
    Coolwarm, Bwr, Seismic, RdBu, YlGnBu, Sunset,
    // not in the chooser: the gain map has four valid values (the gain bits
    // 0..3) and gets a fixed, calm blue/green/yellow/red code
    Gain4
};
const char *paletteName(Palette p); // label for the scale panel
void loadRootPalette(Palette p);   // ROOT thread only
const QRgb *paletteLut(Palette p); // static table, safe from any thread
struct RootCmd {
    RootCmdType type = RootCmdType::None;
    double a = 0, b = 0, c = 0, d = 0; // zoom coords / chip range
};
void pushRootCmd(const RootCmd &cmd);
// like pushRootCmd, but replaces a pending command of the same type instead
// of queueing another one (used for the continuous zoom stream)
void pushRootCmdCoalesced(const RootCmd &cmd);
void requestHistoReset(); // safe "Reset His": analysis thread clears sources
void forceRootRedraw();
void startRootThread();
void stopRootThread();

/* root plotting helpers (ROOT thread only) */
void historesetSources();       // takes gHistoMtx itself
void historesetSourcesLocked(); // gHistoMtx already held (analysis thread)
void axisreset();

void Plot1DHistos();
void Plot2DHistos();
// the same two spectra canvases in keV (A8/A9), no-ops without a gain map
void Plot1DHistosKev();
void PlotRoiHistosKev();
// the per-chip spectra of the gate-ROI pixels only (canvas A7); same pad
// layout as Plot1DHistos so the two can be compared pad by pad
void PlotRoiHistos();
// runs the ROOT GUI event loop once; called inside long redraws so the
// canvases stay interactive even while a redraw is in progress
void serviceRootEvents();
void drawRoiBox();
void printhelp();

/* ------------------------------------------------------------ pedestal */
// Immutable pedestal snapshot.  g0 = gain 0 (also the fallback for every
// gain when hasAll is false); g1/g2 = gain 1 / gain 3 arrays.
struct PedeSet {
    std::vector<float> g0, g1, g2;
    bool hasAll = false; // true only when all three arrays came from a file
    PedeSet()
        : g0(JF_PEDE_ARRAY_FLOATS, 0.0f), g1(JF_PEDE_ARRAY_FLOATS, 0.0f),
          g2(JF_PEDE_ARRAY_FLOATS, 0.0f) {}
};

class PedestalStore {
  public:
    static PedestalStore &instance();
    // snapshot; empty shared_ptr means "no pedestals, run on raw values"
    std::shared_ptr<const PedeSet> get() const;
    void install(const std::shared_ptr<PedeSet> &set);
    bool loaded() const;

  private:
    mutable std::mutex mtx_;
    std::shared_ptr<const PedeSet> cur_;
};

// Reads the 3-array legacy layout into a new set and installs it.
// Returns false + error when the file is missing / has the wrong size.
// ---- gate-ROI list -------------------------------------------------------
// One entry of the saved gate-ROI list.  Half-open rectangle in data
// coordinates (row 0 at the bottom), w/h are the size in pixels.
struct RoiEntry {
    int x = 0, y = 0, w = 0, h = 0;
    int64_t usedMs = 0; // epoch ms of the last save/load (ordering key)
};
// Reads the list, sorted most-recently-used first.
std::vector<RoiEntry> roiListLoad();
// Adds the rectangle to the list (or refreshes its timestamp if it is already
// there) and writes the file back.  Returns false on I/O failure.
bool roiListUpsert(int x, int y, int w, int h);
// Removes the rectangle and writes the file back.
bool roiListRemove(int x, int y, int w, int h);

// ---- previously used save destinations -----------------------------------
struct SaveDest {
    std::string dir;  // output directory
    std::string base; // file-name base
    int64_t usedMs = 0;
};
// Same idea as the ROI list: one small file, newest used first.
std::vector<SaveDest> saveDestListLoad();
bool saveDestListUpsert(const std::string &dir, const std::string &base);
bool saveDestListRemove(const std::string &dir, const std::string &base);

bool pedeLoadFile(const std::string &path, std::string &error);
// Writes the currently installed set in the legacy 3-array layout.
bool pedeSaveFile(const std::string &path, std::string &error);
bool pathExists(const std::string &path);

/* ---------------------------------------------------------------- gate */
// Event-finder gate.  One instance; the acq thread calls evaluate() for every
// received frame, the Qt thread reads snapshot() and changes parameters.
class GateEngine {
  public:
    struct Counters {
        uint64_t framesIn = 0;
        uint64_t framesPass = 0;
        uint64_t framesDrop = 0;
        uint64_t framesScDrop = 0; // rejected by the storage-cell filter
        uint64_t pixelsAbove = 0;
        uint64_t framesInCur = 0;
        uint64_t framesPassCur = 0;
        uint64_t framesDropCur = 0;
        uint64_t framesScDropCur = 0;
        uint64_t pixelsAboveCur = 0;
        int lastSc = -1;   // storage cell of the last frame seen (-1 unknown)
        int scFilter = -1; // -1 = all storage cells
        double rateIn = 0;
        double ratePass = 0;
        double rateInMBs = 0;
        double ratePassMBs = 0;
        double dropFrac = 0;
        bool connected = false;
        bool pedeLoaded = false;
        std::string lastError;
    };

    void setEnabled(bool on);
    bool enabled() const { return enabled_.load(); }
    void setThreshold(float adc);
    void setMinPixels(uint64_t n);
    void setContiguous(bool on);
    // Storage-cell selection: only frames acquired in cell 'sc' (0..15, from
    // bits 8-11 of the header's detSpec3) are passed to the display and the
    // analysis; -1 = all of them.  Checked before the pixel work, so filtered
    // frames cost nothing.
    void setScFilter(int sc);
    int scFilter() const { return scFilter_.load(); }
    // Restrict the gate search to a rectangle of the assembled frame, in
    // pixel coordinates with row 0 at the BOTTOM (the display's data order).
    // Half-open [x0,x1) x [y0,y1); an unusable rectangle clears the ROI.
    // This is the "gate ROI" chosen from the ADC zoom, not the receiver ROI
    // (detSpec1) that the photon map draws in red.
    void setRoi(int x0, int y0, int x1, int y1);
    void clearRoi();
    bool roiActive() const { return roiOn_.load(); }
    // The ROI rectangle and the decision to gate on it are separate: selecting
    // a ROI ("Select ROI" / Load ROI) only stores it, "ROI only" switches the
    // restriction on, and the gate then uses whatever is current.
    bool roiSet() const { return roiSet_.load(); }
    // live settings - used when a file records what the gate is doing
    float threshold() const { return threshold_.load(); }
    uint64_t minPixels() const { return minPixels_.load(); }
    bool contiguous() const { return contiguous_.load(); }
    void setRoiOnly(bool on) { roiOn_.store(on && roiSet_.load()); }
    void roiSnapshot(int &x0, int &y0, int &x1, int &y1) const;
    void resetCounters();
    Counters snapshot() const;

    // ---- acq-thread side ------------------------------------------------
    // Decides whether a frame passes.  Returns true when it must be
    // forwarded; 'above' receives the number of pixels above threshold
    // (only computed when the gate is enabled, else 0).
    // 'sc' is the storage cell of this frame (-1 when the header did not
    // carry one); it is filtered before anything else is looked at.
    // scDropped (optional) tells the caller that the frame was rejected by the
    // storage-cell filter rather than by the signal gate
    bool evaluate(const short *frame, uint64_t nPixels, int sc,
                  const PedeSet *peds, int g1off, int g2off, uint64_t &above,
                  bool *scDropped = nullptr);
    void noteReceived(uint64_t bytes, bool broken);
    void noteForwarded(uint64_t bytes);
    void setConnected(bool on);
    void setLastError(const std::string &msg);

  private:
    uint64_t countPixelsAbove(const short *frame, uint64_t nPixels,
                              const PedeSet *peds, int g1off,
                              int g2off) const;
    bool hitsClustered(const short *frame, uint64_t nPixels,
                       const PedeSet *peds, int g1off, int g2off) const;

    std::atomic<bool> enabled_{true};
    std::atomic<bool> contiguous_{false};
    // storage-cell filter (-1 = all) and the cell of the last frame seen
    std::atomic<int> scFilter_{-1};
    std::atomic<int> lastSc_{-1};
    // gate ROI (see setRoi); read once per frame by the acq thread
    std::atomic<bool> roiOn_{false};
    std::atomic<bool> roiSet_{false}; // a ROI rectangle exists
    std::atomic<int> roiX0_{0}, roiY0_{0}, roiX1_{1024}, roiY1_{512};
    std::atomic<float> threshold_{100.0f};
    std::atomic<uint64_t> minPixels_{5};

    struct AtomCounters {
        std::atomic<uint64_t> framesIn{0};
        std::atomic<uint64_t> framesPass{0};
        std::atomic<uint64_t> framesDrop{0};
        std::atomic<uint64_t> framesScDrop{0}; // storage-cell filter
        std::atomic<uint64_t> pixelsAbove{0};
        std::atomic<uint64_t> bytesIn{0};
        std::atomic<uint64_t> bytesOut{0};
    };
    AtomCounters c_, cCur_;
    // 1 Hz smoothed rates (written by the acq loop, read by the GUI)
    struct RateAtoms {
        std::atomic<double> rateIn{0.0};
        std::atomic<double> ratePass{0.0};
        std::atomic<double> rateInMBs{0.0};
        std::atomic<double> ratePassMBs{0.0};
    };
    RateAtoms r_;
    std::atomic<bool> connected_{false};
    mutable std::mutex errMtx_;
    std::string lastError_;

    friend class Pipeline;
};

/* ------------------------------------------------------------- mailbox */
// Single-slot "latest frame wins" mailbox: acq -> analysis.  Overwriting the
// slot is the merged equivalent of the ZMQ receive-high-water-mark drops of
// the old chain (the display always works on the freshest frame, and the gate
// counters stay exact because they are updated before the hand-off).
struct FrameMailbox {
    std::mutex mtx;
    std::condition_variable cv;
    std::vector<short> buf;
    size_t len = 0; // valid shorts in buf
    bool have = false;
    uint64_t seq = 0;
    uint64_t pushed = 0;
    uint64_t dropped = 0;

    void publish(const short *data, size_t nBytes);
    // waits up to 100 ms; false when nothing was available
    bool waitTake(std::vector<short> &out, size_t &lenOut, uint64_t &seqOut,
                  const std::atomic<bool> &running);
};

// Image hand-off: analysis -> Qt timer.  The Qt thread copies the buffers
// into the widgets' own storage, so the widgets never read a live buffer.
#ifdef JFSTRX
// Strixel ADC panels, index order:
//   0 = chip1 group1, 1 = chip1 group2, 2 = chip1 group3,
//   3 = chip6 group1, 4 = chip6 group2, 5 = chip6 group3.
// The GUI arranges them like the original ROOT ADC canvas (left column
// chip1 g3/g2/g1, right column chip6 g1/g2/g3).
constexpr int kNumAdcPanels = 6;
constexpr int kAdcPanelW[kNumAdcPanels] = {79, 47, 59, 79, 47, 59};
constexpr int kAdcPanelH[kNumAdcPanels] = {165, 320, 476, 165, 320, 476};
#endif

struct ImageMailbox {
    std::mutex mtx;
    std::vector<float> adc, gain; // always raw 1024x512 (ROI selection uses it)
    // the photon map, published every gPmapEvery frames
    std::vector<float> photon;
    bool photonReady = false;
    uint64_t photonSeq = 0;
    bool ready = false;
    uint64_t seq = 0;
#ifdef JFSTRX
    // mapped strixel ADC groups for the separate six-panel window
    std::array<std::vector<float>, kNumAdcPanels> panels;
    bool panelsReady = false;
#endif
};

// ---- HDF5 ROI save -------------------------------------------------------
// Per-frame header values, written into the same parameter datasets as the
// receiver (see online_jf_h5.cpp).
struct SavedFrameHeader {
    uint64_t frameNumber = 0;
    uint32_t expLength = 0;
    uint32_t packetNumber = 0;
    uint64_t detSpec1 = 0;
    uint64_t timestamp = 0;
    uint16_t modId = 0, row = 0, column = 0, detSpec2 = 0, detSpec4 = 0;
    uint32_t detSpec3 = 0;
    uint8_t detType = 0, version = 0;
};

struct GateCriteria {
    bool enabled = true;
    float threshold = 0.0f;
    uint64_t minPixels = 0;
    bool contiguous = false;
    bool roiActive = false;
    bool noPede = false; // pedestal subtraction disabled (raw ADC)
    int roiX = 0, roiY = 0, roiW = 0, roiH = 0;
};

struct RunStats {
    uint64_t resync = 0, behind = 0, unpaired = 0;
    uint64_t displayFrames = 0, displayDropped = 0;
};

struct StreamInfo {
    int port = 0;
    std::string state;
    uint64_t frames = 0, lastFrame = 0;
};

// Decimated ROI writer: one HDF5 file per run (receiver-compatible layout,
// ROI crop, gate-passed frames only, slice 0 = pedestal) plus a JSON sidecar.
// A bounded queue decouples the acq thread from the writer; overflow is
// counted as dropped and reported.
class RoiSaver {
  public:
    struct StartInfo {
        int x = 0, y = 0, w = 0, h = 0; // ROI in data coordinates
        uint64_t maxFrames = 0;         // 0 = unlimited
        std::string dir = ".";          // output directory
        std::string baseName = "jfroi"; // file-name base (receiver "prefix")
        // false: raw 16-bit words in /data with the pedestal image as slice 0
        // true : pedestal-corrected float32 in /data (no slice 0) and the
        //        pedestal image in the separate "pedestal data" dataset
        bool corrected = false;
        std::vector<float> pedestal;    // w*h gain-0 pedestal (may be empty)
        // full pedestal set + gain offsets used for the correction
        std::shared_ptr<const PedeSet> peds;
        bool useGain = false;
        int g1off = 0, g2off = 0;
        // Refresh the on-disk HDF5 (exact extents + flush) and rewrite the JSON
        // sidecar atomically every this many ms while the save runs, so a
        // snapshot on disk is always clean and a crash loses at most one
        // interval.  0 disables the periodic checkpoint (final state at stop).
        int checkpointMs = 30000;
        // virtual photon counting save: write the maps instead of the frames
        bool photonMode = false;
        int photonEvery = 2000;      // analysed frames per saved map
        double photonThreshold = 150.0; // photon threshold, in photonKev units
        bool photonKev = false;         // true: photonThreshold is keV
        GateCriteria gate;              // criteria in force at start
        GateEngine::Counters gateAtEnd; // fallback values for the JSON
        RunStats stats;
        std::vector<StreamInfo> streams;
        std::string gateJson;           // gate criteria as a JSON object
        // Called by the writer thread just before the JSON sidecar is written,
        // so the reported statistics are the exact end-of-run ones.
        std::function<void(GateEngine::Counters &, RunStats &,
                           std::vector<StreamInfo> &)>
            statsProvider;
    };
    struct Status {
        bool running = false;
        uint64_t saved = 0, dropped = 0;
        uint64_t pmaps = 0;        // maps written (photon-map mode)
        bool photonMode = false;   // which of the two counters applies
        std::string h5Path, jsonPath, stopReason, lastError;
    };

    RoiSaver();
    ~RoiSaver();
    RoiSaver(const RoiSaver &) = delete;
    RoiSaver &operator=(const RoiSaver &) = delete;

    bool start(const StartInfo &info, std::string &error);
    // acq thread, for a frame that passed the gate
    // Virtual photon counting mode: one map (NCH floats, photons per pixel)
    // plus the window it covers.  The writer appends it to
    // "/virtual photon counting" and records the first frame's number and
    // timestamp of the window.
    void offerPhotonMap(const std::vector<float> &map, uint64_t frames,
                        uint64_t firstFrame, uint64_t firstStamp);
    void offer(const short *frame, int frameW, int frameH,
               const SavedFrameHeader &hdr);
    void requestStop(const std::string &reason);
    bool running() const;
    Status status() const;
    // waits for the writer, shrinks the datasets and writes the JSON sidecar
    void finish();

  private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

/* ------------------------------------------------------------- pipeline */
class Pipeline {
  public:
    // Stream state derived from ZMQ socket monitoring, NOT from receive
    // timeouts.  An idle-but-alive receiver is "Idle"/"Connected", never
    // "stopped" (the old jf_filter behaviour).
    enum class StreamState {
        Disconnected, // no peer connected
        Connecting,   // peer not up yet / reconnecting
        Connected,    // peer connected, frames arriving
        Idle,         // peer connected but no frame for a while
        EndOfAcq      // peer sent the end-of-acquisition marker
    };

    Pipeline();
    ~Pipeline();

    bool init(std::string &error); // create the ZMQ sockets
    void start();
    void stop();
    bool running() const { return running_.load(); }

    GateEngine &gate() { return gate_; }
    RoiSaver &saver() { return saver_; }
    ImageMailbox &images() { return images_; }
    FrameMailbox &frames() { return frames_; }

    bool multiPort() const { return rxPorts_.size() > 1; }
    size_t portCount() const { return rxPorts_.size(); }
    const std::vector<int> &portList() const { return rxPorts_; }

    bool connected() const;
    // overall state (worst of all ports) and per-port state
    StreamState streamState(double *sinceSec = nullptr) const;
    StreamState streamState(int portIdx, double *sinceSec = nullptr) const;
    // short token for --stats: connected|idle|eoa|connecting|disconnected
    const char *streamStateName() const;
    static const char *streamStateName(StreamState st);
    // "connected,idle" for N ports
    std::string streamStateSummary() const;
    uint64_t portFrames(int portIdx) const;
    int portWidth(int portIdx) const;  // detected per-port width  (0 unknown)
    int portHeight(int portIdx) const; // detected per-port height (0 unknown)
    // ---- frame-alignment audit (see "Keeping the interfaces aligned") -----
    uint64_t portLastFrame(int portIdx) const; // newest frame number seen
    uint64_t resyncDrops() const;  // stale slabs discarded to restore alignment
    uint64_t behindDrops() const;  // frames discarded while catching up
    uint64_t incompleteFrames() const; // assembled while an interface was absent

    uint64_t displayFrames() const;
    uint64_t displayDropped() const;
    PmapSlot &pmapSave() { return pmapSave_; }
    PmapSlot pmapSave_;
    // window bookkeeping for the photon-map save (acq thread)
    uint64_t pmapWinFirstFrame = 0, pmapWinFirstStamp = 0, pmapWinCount = 0;
    // route a gate-passed frame to the saver: the frame itself, or the photon
    // map bookkeeping when saving virtual photon counting
    void offerToSaver(const short *frame, int w, int h,
                      const SavedFrameHeader &hdr);
    uint64_t imagesShown() const { return gImagesShown.load(); }

    // Ask the acq thread to average the next N frames into a new gain-0
    // pedestal set (the old "Load Pede (p)" blocking loop, made async).
    void requestPedeAcquire(int nFrames);
    bool pedeAcquireActive() const;
    int pedeAcquireProgress() const; // frames accumulated so far

  private:
    void acqLoop();
    void analysisLoop();
    void acquirePede(const short *frame); // acq thread, during acquisition
    void drainMonitor(int portIdx);       // acq thread: consume monitor events
    bool readOne(int portIdx);            // acq thread: one header+data pair
    void readStream(int portIdx);         // drain a socket, newest frame wins
    void drainTo(int portIdx, uint64_t targetKey); // drop until caught up
    bool tryAssemble(bool force);         // acq thread: pair + stack the slabs
    void refreshStreamState();            // acq thread: status + messages
    void refreshStreamState(int portIdx);

    // per-port receive slot: the latest slab waiting to be paired
    struct StreamSlot {
        std::vector<short> data;
        size_t len = 0;       // valid shorts
        int w = 0, h = 0;     // geometry as reported by that stream's header
        int rowBase = 0;      // first destination row in the assembled frame
        uint64_t frameKey = 0; // absolute frame number (pairing key)
        SavedFrameHeader hdr;  // header fields for the ROI-save datasets
        int64_t stampMs = 0;
        bool valid = false;
        bool logged = false;   // first-frame geometry already reported
    };

    // per-port connection tracking (atomics: fixed size, one per port)
    struct StreamTrack {
        std::atomic<bool> peerConnected{false};
        std::atomic<bool> everConnected{false};
        std::atomic<bool> endOfAcq{false};
        std::atomic<int64_t> lastHeaderMs{0};
        std::atomic<int64_t> connectedSinceMs{0};
        std::atomic<uint64_t> frames{0};
        std::atomic<uint64_t> lastFrame{0};
        std::atomic<uint64_t> errors{0};
        std::atomic<int> w{0};
        std::atomic<int> h{0};
        std::atomic<int> sc{-1}; // storage cell, detSpec3 bits 8-11
    };

    std::atomic<bool> running_{false};
    std::thread *acqThread_ = nullptr;
    std::thread *analysisThread_ = nullptr;

    GateEngine gate_;
    RoiSaver saver_;
    FrameMailbox frames_;
    ImageMailbox images_;

    std::vector<int> rxPorts_;
    void *zmqCtx_ = nullptr;
    std::vector<void *> zmqSubs_; // one SUB per interface
    std::vector<void *> zmqMons_; // one monitor PAIR per interface (may be null)
    void *zmqPub_ = nullptr;      // optional republish socket

    std::array<StreamTrack, JF_MAX_PORTS> track_;
    std::vector<StreamSlot> slots_;
    std::vector<char> hdrBytes_; // raw header of the last received slab
    int hdrLen_ = 0;
    std::vector<short> assembled_; // full-frame assembly buffer
    // acq-thread only pairing bookkeeping:
    //   pairWaitStartMs_ : when the current pairing attempt started (0 = none)
    //   lastProgressMs_  : per interface, when its frame number last advanced
    //                      (a stalled interface must not freeze the display)
    //   firstDataMs_     : when the first slab ever arrived
    //   acqStartMs_      : when acquisition started (start-up tolerance)
    int64_t pairWaitStartMs_ = 0;
    std::array<int64_t, JF_MAX_PORTS> lastProgressMs_{};
    int64_t firstDataMs_ = 0;
    int64_t acqStartMs_ = 0;
    std::atomic<int> fullW_{0};
    std::atomic<int> fullH_{0};
    std::atomic<uint64_t> resyncDrops_{0};
    std::atomic<uint64_t> behindDrops_{0};
    std::atomic<uint64_t> incomplete_{0};
    std::atomic<int64_t> lastPairWarnMs_{0};

    std::atomic<bool> monitorActive_{false};
    std::atomic<int> stateShown_{-1}; // overall StreamState last printed
    std::array<std::atomic<int>, JF_MAX_PORTS> portStateShown_;

    std::atomic<bool> pedeAcqActive_{false};
    std::atomic<int> pedeAcqWant_{0};
    std::atomic<int> pedeAcqDone_{0};
    std::shared_ptr<PedeSet> pedeAcq_;
};

/* ---------------------------------------------------------------- misc */
bool parseCommandLine(int argc, char **argv, Config &cfg, bool &help,
                      std::string &error);
void printUsage(const char *prog);
void initGeometry(); // sets nx/ny (JFSTRX decoder or the 1024x512 default)
#endif // ONLINE_JF_H
