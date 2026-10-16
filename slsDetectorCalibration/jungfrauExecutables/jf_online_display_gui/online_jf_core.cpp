// SPDX-License-Identifier: LGPL-3.0-or-other
// ===========================================================================
// online_jf_core.cpp -- acquisition + gate + analysis (no ROOT graphics here
// except the histogram FILLING, which is the analysis thread's job).
//
// Merged from:
//   jf_filter.cpp            (gate, counters, pedestal loading)
//   onlinedisp_zmq.cpp       receiverWorker()/processData() (images, histos)
// ===========================================================================
#include "online_jf.h"

#include <zmq.h>

#ifdef JFSTRX
#include "jungfrauLGADStrixelsData_new.h"
#include "sls/sls_detector_defs.h"
#include "slsDetectorData.h"
using header = sls::defs::sls_receiver_header;
#endif

// ---------------------------------------------------------------------------
// Global state (see online_jf.h for the ownership rules)
// ---------------------------------------------------------------------------
Config g_cfg;

short *hchptr = nullptr;
float factor = 1.84f;
std::atomic<bool> gGateChangedDuringSave{false};
float g_pheneThreshold = 0.0f;
float g_pheneThresholdKev = 0.0f;

std::atomic<bool> gain_flag{false};
std::atomic<bool> dophotonmap{true};
std::atomic<int> goout{1};
std::atomic<int> HDraw_every{20};
std::atomic<uint64_t> gImagesShown{0};
// when the Qt view last got an image (analysis thread only)
int64_t g_lastShownMs = 0;
std::atomic<int> rootUpdateMs{1000};
// one spectrum canvas is painted per tick, in round-robin; with the keV
// set enabled there are four of them, so the default is one per redraw
std::atomic<int> chipUpdateEvery{1};
std::atomic<int> G1Poffset{0};
std::atomic<int> G2Poffset{0};
// defaults from the operator: single-photon ADC window for the main
// plot (-200..1000), photon counts 0..256, spectra -200..3000
std::atomic<int> adcmin{-200}, adcmax{1000}, pmmin{0}, pmmax{256};
std::atomic<int> chipxmin{-200}, chipxmax{3000};
std::atomic<bool> fixranges{true};
std::atomic<uint64_t> framesinstream{0};
std::atomic<uint64_t> gFramesForRoot{0};

// analysis-thread accumulators
short image_data[JF_PEDE_ARRAY_FLOATS];
float adc_image[NCH];
float gain_image[NCH];
#ifdef JFSTRX
// Strixel ADC groups for the separate six-panel window.  The Qt ADC/gain
// images stay raw 1024x512 (that is what the ROI selection works on), and the
// mapped groups are only used by this display path.
static std::array<std::vector<float>, kNumAdcPanels> g_adcPanel;
#endif
float fgaind[JF_PEDE_ARRAY_FLOATS];
int nx = 1024, ny = 512;

std::atomic<uint16_t> ROIxmin{0}, ROIxmax{0}, ROIymin{0}, ROIymax{0};
std::atomic<bool> ROIactive{false};
std::atomic<bool> kevSpectra{false}; // show keV PH instead of ADU PH
std::atomic<uint16_t> rxRoiXmin{0}, rxRoiXmax{0}, rxRoiYmin{0}, rxRoiYmax{0};
std::atomic<bool> rxRoiValid{false};

std::atomic<bool> gSoftResetPending{false};
std::atomic<bool> gRedrawRequest{false};
std::atomic<bool> gResetHisPending{false};
std::atomic<bool> gFrameCountReset{false};
std::atomic<bool> gNoPede{true}; // startup default: raw ADC (--use-pede clears)
std::atomic<int> gPmapEvery{2000};
std::atomic<int> gSaveMode{0};
std::atomic<int> gPmapSaveEvery{2000};
std::atomic<bool> gPmapCumulative{false};
std::atomic<bool> gPedeTracking{false};
std::atomic<bool> gPedeTracked{false};
std::mutex gHistoMtx;

namespace {
int processedf = 0; // analysis thread only
constexpr size_t kHeaderBytes = 16384;

// ---------------------------------------------------------------------------
// Deferred histogram accumulation (analysis thread, under gHistoMtx)
// ---------------------------------------------------------------------------
// Calling TH1::Fill() per pixel per frame dominated the analysis stage
// (~0.6-1 M fills/frame -> ~4-6 ms) and ROOT histograms are not thread safe,
// so the per-frame work accumulates into plain arrays here and is pushed into
// the ROOT histograms only at the redraw cadence.  The histograms are painted
// once per rootUpdateMs, so syncing faster than that is pure waste.
// TEMPORARY timing instrumentation (JF_TIMING)
bool g_timing = false;
double tTake = 0, tCopy = 0, tStep1 = 0, tStep3 = 0, tStep4 = 0,
       tSync = 0, tStep5 = 0;
uint64_t tFrames = 0;
struct TT { // scoped timer; zero overhead when disabled (acc == nullptr)
    double *acc;
    std::chrono::steady_clock::time_point t0;
    explicit TT(double *a) : acc(a) {
        if (acc)
            t0 = std::chrono::steady_clock::now();
    }
    ~TT() {
        if (!acc)
            return;
        *acc += std::chrono::duration<double, std::micro>(
                    std::chrono::steady_clock::now() - t0)
                    .count();
    }
};

constexpr int kChipOverflowBins = NBIN + 2; // + underflow + overflow
std::vector<float> g_pmapAccum;            // NCH, photon map / raw ADC map
std::vector<float> g_pmapSaveAccum;        // NCH, the map being saved
uint64_t g_pmapSaveCount = 0;              // analysed frames in that map
uint32_t g_chipAccum[8][kChipOverflowBins];
uint32_t g_chipSynced[8][kChipOverflowBins]; // already pushed into hchip
uint64_t g_chipSyncedTotal[8];               // entries already pushed
// ONE spectrum over the gate-ROI pixels (canvas A7): same binning, same
// border trim, but no chip split - it is the sum over the selected pixels
uint32_t g_roiAccum[kChipOverflowBins];
uint32_t g_roiSynced[kChipOverflowBins];
uint64_t g_roiSyncedTotal = 0;
// the same two sets again in keV (canvases A8/A9); their axes are the ADU
// axes divided by KEV_NOMINAL_ADU, so the bin index is identical and only the
// value that is filled differs
uint32_t g_kevAccum[8][kChipOverflowBins];
uint32_t g_kevSynced[8][kChipOverflowBins];
uint64_t g_kevSyncedTotal[8];
uint32_t g_roiKevAccum[kChipOverflowBins];
uint32_t g_roiKevSynced[kChipOverflowBins];
uint64_t g_roiKevSyncedTotal = 0;
double g_pmapFills = 0;
double g_pmapFillsSynced = 0;
int64_t g_lastAccumSyncMs = 0;
bool g_accumReady = false;

// Bin index exactly as ROOT's TAxis::FindBin computes it for the uniform
// NBIN-bin axis [MIN_POS, MAX_POS] (verified against FindBin over all bin
// edges and 20M random values, 0 mismatches).  0 = underflow, NBIN+1 = over.
inline int chipBin(double x) {
    const double inv = static_cast<double>(NBIN) / (MAX_POS - MIN_POS);
    if (x < MIN_POS)
        return 0;
    if (x >= MAX_POS)
        return NBIN + 1;
    return 1 + static_cast<int>((x - MIN_POS) * inv);
}

void ensureAccumulators() {
    if (g_accumReady)
        return;
    g_pmapAccum.assign(static_cast<size_t>(NCH), 0.0f);
    memset(g_chipAccum, 0, sizeof(g_chipAccum));
    memset(g_chipSynced, 0, sizeof(g_chipSynced));
    memset(g_chipSyncedTotal, 0, sizeof(g_chipSyncedTotal));
    memset(g_roiAccum, 0, sizeof(g_roiAccum));
    memset(g_roiSynced, 0, sizeof(g_roiSynced));
    g_roiSyncedTotal = 0;
    memset(g_kevAccum, 0, sizeof(g_kevAccum));
    memset(g_kevSynced, 0, sizeof(g_kevSynced));
    memset(g_kevSyncedTotal, 0, sizeof(g_kevSyncedTotal));
    memset(g_roiKevAccum, 0, sizeof(g_roiKevAccum));
    memset(g_roiKevSynced, 0, sizeof(g_roiKevSynced));
    g_roiKevSyncedTotal = 0;
    g_pmapFills = 0;
    g_pmapFillsSynced = 0;
    g_lastAccumSyncMs = 0;
    g_accumReady = true;
}

// Zeroes the accumulators so they match the freshly Reset() ROOT histograms.
// Called under gHistoMtx from historesetSources().
void resetAccumulators() {
    if (!g_accumReady)
        return;
    std::fill(g_pmapAccum.begin(), g_pmapAccum.end(), 0.0f);
    memset(g_chipAccum, 0, sizeof(g_chipAccum));
    memset(g_chipSynced, 0, sizeof(g_chipSynced));
    memset(g_chipSyncedTotal, 0, sizeof(g_chipSyncedTotal));
    memset(g_roiAccum, 0, sizeof(g_roiAccum));
    memset(g_roiSynced, 0, sizeof(g_roiSynced));
    g_roiSyncedTotal = 0;
    memset(g_kevAccum, 0, sizeof(g_kevAccum));
    memset(g_kevSynced, 0, sizeof(g_kevSynced));
    memset(g_kevSyncedTotal, 0, sizeof(g_kevSyncedTotal));
    memset(g_roiKevAccum, 0, sizeof(g_roiKevAccum));
    memset(g_roiKevSynced, 0, sizeof(g_roiKevSynced));
    g_roiKevSyncedTotal = 0;
    g_pmapFills = 0;
    g_pmapFillsSynced = 0;
    g_lastAccumSyncMs = 0; // force a sync on the next frame
}

// Pushes the accumulators into his3000 / hchip.  Analysis thread, gHistoMtx
// held.  The chip spectra are filled with the DELTA so ROOT's own statistics
// (mean/RMS in the stats box) stay exactly as if Fill() had been called.
// Pushes one 8-chip accumulator into its ROOT histograms.  Only changed bins
// are pushed, using Fill(x, delta), so a sync costs O(changed bins) instead of
// O(8*NBIN).  fEntries is restored explicitly: ROOT's Fill(x,w) bumps it once
// per *call*, not per weight, so the delta fills would otherwise report the
// number of syncs instead of the number of pixels.
// scale = 1 for the ADU histograms, 1/KEV_NOMINAL_ADU for the keV ones (their
// axes are the ADU axes divided by that constant, same bin count)
void syncOneAccum(TH1I *dst, const uint32_t *accum, uint32_t *synced,
                  uint64_t &syncedTotal, double scale = 1.0) {
    if (!dst)
        return;
    const double w = (MAX_POS - MIN_POS) / NBIN;
    uint64_t total = 0;
    for (int b = 0; b < kChipOverflowBins; b++) {
        const uint32_t cur = accum[b];
        total += cur;
        if (cur == synced[b])
            continue;
        const double delta = static_cast<double>(cur - synced[b]);
        if (b == 0)
            dst->Fill((MIN_POS - 1.0) * scale, delta);
        else if (b == NBIN + 1)
            dst->Fill((MAX_POS + 1.0) * scale, delta);
        else
            dst->Fill((MIN_POS + (b - 0.5) * w) * scale, delta);
        synced[b] = cur;
    }
    // every pixel lands in exactly one bin (including under/overflow)
    if (total != syncedTotal) {
        dst->SetEntries(static_cast<double>(total));
        syncedTotal = total;
    }
}

void syncChipAccum(TH1I **dst, uint32_t accum[8][kChipOverflowBins],
                   uint32_t synced[8][kChipOverflowBins],
                   uint64_t syncedTotal[8], double scale = 1.0) {
    if (!dst)
        return;
    for (int ic = 0; ic < 8; ic++)
        syncOneAccum(dst[ic], accum[ic], synced[ic], syncedTotal[ic], scale);
}

void syncAccumulators(int64_t now, bool force = false) {
    if (!g_accumReady)
        return;
    int64_t period = rootUpdateMs.load() / 2;
    if (period < 50)
        period = 50;
    if (period > 500)
        period = 500;
    if (!force && g_lastAccumSyncMs != 0 &&
        now - g_lastAccumSyncMs < period)
        return;
    g_lastAccumSyncMs = now;

    if (his3000 && his3000->GetNbinsX() * his3000->GetNbinsY() == NCH) {
        const int nbx = his3000->GetNbinsX();
        for (int iy = 0; iy < his3000->GetNbinsY(); iy++) {
            for (int ix = 0; ix < nbx; ix++) {
                his3000->SetBinContent(ix + 1, iy + 1,
                                       g_pmapAccum[ix + iy * nbx]);
            }
        }
        if (g_pmapFills != g_pmapFillsSynced) {
            his3000->SetEntries(g_pmapFills);
            g_pmapFillsSynced = g_pmapFills;
        }
    }

    if (hchip)
        syncChipAccum(hchip, g_chipAccum, g_chipSynced, g_chipSyncedTotal);
    if (hroi)
        syncOneAccum(hroi, g_roiAccum, g_roiSynced, g_roiSyncedTotal);
    if (hchipKev)
        syncChipAccum(hchipKev, g_kevAccum, g_kevSynced, g_kevSyncedTotal,
                      1.0 / KEV_NOMINAL_ADU);
    if (hroiKev)
        syncOneAccum(hroiKev, g_roiKevAccum, g_roiKevSynced,
                     g_roiKevSyncedTotal, 1.0 / KEV_NOMINAL_ADU);
}
// after this long without a header the stream is reported as "idle"
constexpr int64_t kIdleWarnMs = 2000;
// fallback staleness for when socket monitoring is unavailable
constexpr int64_t kNoMonitorStaleMs = 3000;
// how long to wait for the partner slab of the same frame before assembling
// from what is available (keeps a dead interface from freezing the display)
constexpr int64_t kPairTimeoutMs = 100;
// ... but while an interface has never delivered a single frame (start-up,
// or it comes up later) wait much longer, otherwise the first frames are
// assembled from one slab only
constexpr int64_t kInitialPairTimeoutMs = 2000;
// how long after the first data we keep using the long "wait for the partner
// to come up" timeout (covers a second interface that starts late)
constexpr int64_t kSyncWindowMs = 5000;
// how long a given-up interface is ignored before it is retried
constexpr int64_t kRelatchMs = 1000;
// max messages consumed from one socket per catch-up burst
constexpr int kMaxDrainPerCall = 256;
// an interface whose frame number has not advanced for this long is stalled
// (dead, or not in this acquisition) and must not block the display
constexpr int64_t kStallMs = 500;

int64_t nowMs() {
    return duration_cast<milliseconds>(steady_clock::now().time_since_epoch())
        .count();
}
} // namespace

void initGeometry() {
#ifdef JFSTRX
    cout << "JFSTRX" << endl;
    jungfrauLGADStrixelsData *decoder = new jungfrauLGADStrixelsData();
    decoder->getDetectorSize(nx, ny);
    cout << decoder->dataMap[22][22] << endl;
    delete decoder;
#else
    nx = 1024;
    ny = 512;
#endif
}

void printhelp() {
    cout << endl
         << "s=start/pause| p/n=getnewpede/raw | o/O=read/save pede | n=nopede(raw)"
            " | r/R=rst His/Axis | +/- = faster/slower ref. |q=exit | U/C sUrf2/Colz"
         << endl;
}

// ---------------------------------------------------------------------------
// Pedestal store
// ---------------------------------------------------------------------------
PedestalStore &PedestalStore::instance() {
    static PedestalStore store;
    return store;
}

// ---- gain map (calibration for the PH spectra) --------------------------
GainMapStore &GainMapStore::instance() {
    static GainMapStore store;
    return store;
}

std::shared_ptr<const GainMap> GainMapStore::get() const {
    std::lock_guard<std::mutex> lock(mtx_);
    return cur_;
}

void GainMapStore::install(const std::shared_ptr<GainMap> &m,
                           const std::string &path) {
    std::lock_guard<std::mutex> lock(mtx_);
    cur_ = m;
    path_ = path;
}

bool GainMapStore::loaded() const {
    std::lock_guard<std::mutex> lock(mtx_);
    return cur_ != nullptr;
}

bool gainMapLoaded() { return GainMapStore::instance().loaded(); }

std::string GainMapStore::path() const {
    std::lock_guard<std::mutex> lock(mtx_);
    return path_;
}

// Six 1024x512 images of doubles, in file order g0, g1, g2, hg0, hg1, hg2.
bool gainMapLoadFile(const std::string &path, std::string &error) {
    const long long expected = static_cast<long long>(JF_GAINMAP_IMAGES) *
                               JF_GAINMAP_PIXELS * (long long)sizeof(double);
    std::ifstream f(path.c_str(), std::ios::binary);
    if (!f.is_open()) {
        error = "cannot open gain map '" + path + "'";
        return false;
    }
    f.seekg(0, std::ios::end);
    const long long bytes = static_cast<long long>(f.tellg());
    f.seekg(0, std::ios::beg);
    if (bytes != expected) {
        std::ostringstream oss;
        oss << "'" << path << "' has " << bytes << " bytes, expected "
            << expected << " (" << JF_GAINMAP_IMAGES << " x " << 1024 << "x"
            << 512 << " doubles)";
        error = oss.str();
        return false;
    }
    std::shared_ptr<GainMap> m = std::make_shared<GainMap>();
    for (int i = 0; i < JF_GAINMAP_IMAGES; i++) {
        f.read(reinterpret_cast<char *>(m->img[i].data()),
               static_cast<std::streamsize>(JF_GAINMAP_PIXELS *
                                            sizeof(double)));
        if (!f.good()) {
            error = "short read from '" + path + "'";
            return false;
        }
    }
    // sanity: the normal-mode map must be usable
    uint64_t bad = 0;
    double g0sum = 0, hg0sum = 0;
    for (int i = 0; i < JF_GAINMAP_PIXELS; i++) {
        if (!(m->img[0][i] > 0.0) || !std::isfinite(m->img[0][i]))
            bad++;
        g0sum += m->img[0][i];
        hg0sum += m->img[3][i];
    }
    GainMapStore::instance().install(m, path);
    printf("[gain] loaded %s: g0 ADU/keV mean %.2f (%llu unusable pixels), "
           "hg0 mean %.2f\n",
           path.c_str(), g0sum / JF_GAINMAP_PIXELS, (unsigned long long)bad,
           hg0sum / JF_GAINMAP_PIXELS);
    fflush(stdout);
    return true;
}

std::shared_ptr<const PedeSet> PedestalStore::get() const {
    std::lock_guard<std::mutex> lock(mtx_);
    return cur_;
}

void PedestalStore::install(const std::shared_ptr<PedeSet> &set) {
    pedeTrackerReset(); // a fresh pedestal re-primes the running average
    std::lock_guard<std::mutex> lock(mtx_);
    cur_ = set;
}

bool PedestalStore::loaded() const {
    std::lock_guard<std::mutex> lock(mtx_);
    return static_cast<bool>(cur_);
}

bool pathExists(const std::string &path) {
    struct stat st;
    return (::stat(path.c_str(), &st) == 0) && S_ISREG(st.st_mode);
}

// Legacy layout written by onlinedisp_zmq "Save Pede (O)":
//   3 arrays (fpeded, fpedeG1d, fpedeG2d), each 2*NCH floats.
bool pedeLoadFile(const std::string &path, std::string &error) {
    const long long expected =
        (long long)3 * JF_PEDE_ARRAY_FLOATS * (long long)sizeof(float);

    std::ifstream f(path.c_str(), std::ios::binary);
    if (!f.is_open()) {
        error = "cannot open pedestal file '" + path + "'";
        return false;
    }
    f.seekg(0, std::ios::end);
    const long long fileBytes = static_cast<long long>(f.tellg());
    f.seekg(0, std::ios::beg);
    if (fileBytes != expected) {
        std::ostringstream oss;
        oss << "'" << path << "' has " << fileBytes << " bytes, expected "
            << expected << " (3 x " << JF_PEDE_ARRAY_FLOATS << " floats)";
        error = oss.str();
        return false;
    }

    std::shared_ptr<PedeSet> set = std::make_shared<PedeSet>();
    const size_t arrayBytes =
        static_cast<size_t>(JF_PEDE_ARRAY_FLOATS) * sizeof(float);
    f.read(reinterpret_cast<char *>(set->g0.data()),
           static_cast<std::streamsize>(arrayBytes));
    f.read(reinterpret_cast<char *>(set->g1.data()),
           static_cast<std::streamsize>(arrayBytes));
    f.read(reinterpret_cast<char *>(set->g2.data()),
           static_cast<std::streamsize>(arrayBytes));
    if (!f.good()) {
        error = "short read from '" + path + "'";
        return false;
    }
    set->hasAll = true;
    PedestalStore::instance().install(set);
    return true;
}

// ---------------------------------------------------------------------------
// Gate-ROI list (single file, rewritten on every change, newest first)
// ---------------------------------------------------------------------------
namespace {
constexpr size_t kMaxRoiEntries = 200; // keeps the file at a few kB forever

bool roiListWrite(const std::vector<RoiEntry> &v) {
    const std::string path = g_cfg.roiFile;
    std::ofstream f(path.c_str(), std::ios::trunc);
    if (!f.is_open())
        return false;
    f << "# gate ROI list: x y w h lastUsedEpochMillis\n";
    for (const RoiEntry &e : v)
        f << e.x << ' ' << e.y << ' ' << e.w << ' ' << e.h << ' '
          << e.usedMs << '\n';
    return f.good();
}
} // namespace

std::vector<RoiEntry> roiListLoad() {
    std::vector<RoiEntry> v;
    std::ifstream f(g_cfg.roiFile.c_str());
    if (!f.is_open())
        return v;
    std::string line;
    while (std::getline(f, line)) {
        if (line.empty() || line[0] == '#')
            continue;
        std::istringstream is(line);
        RoiEntry e;
        if (is >> e.x >> e.y >> e.w >> e.h >> e.usedMs)
            v.push_back(e);
    }
    std::sort(v.begin(), v.end(), [](const RoiEntry &a, const RoiEntry &b) {
        return a.usedMs > b.usedMs; // last saved/loaded first
    });
    return v;
}

bool roiListUpsert(int x, int y, int w, int h) {
    std::vector<RoiEntry> v = roiListLoad();
    const int64_t now = duration_cast<milliseconds>(
                            system_clock::now().time_since_epoch())
                            .count();
    bool found = false;
    for (RoiEntry &e : v) {
        if (e.x == x && e.y == y && e.w == w && e.h == h) {
            e.usedMs = now;
            found = true;
            break;
        }
    }
    if (!found) {
        RoiEntry e;
        e.x = x; e.y = y; e.w = w; e.h = h;
        e.usedMs = now;
        v.push_back(e);
    }
    std::sort(v.begin(), v.end(), [](const RoiEntry &a, const RoiEntry &b) {
        return a.usedMs > b.usedMs;
    });
    if (v.size() > kMaxRoiEntries)
        v.resize(kMaxRoiEntries); // drop the least recently used
    return roiListWrite(v);
}

bool roiListRemove(int x, int y, int w, int h) {
    std::vector<RoiEntry> v = roiListLoad();
    const size_t before = v.size();
    v.erase(std::remove_if(v.begin(), v.end(), [&](const RoiEntry &e) {
                return e.x == x && e.y == y && e.w == w && e.h == h;
            }),
            v.end());
    if (v.size() == before)
        return true; // nothing to do
    return roiListWrite(v);
}

// ---------------------------------------------------------------------------
// Previously used save destinations (single tab-separated file, newest first)
// ---------------------------------------------------------------------------
namespace {
constexpr size_t kMaxSaveDests = 100;

int64_t epochMsNow() {
    return duration_cast<milliseconds>(system_clock::now().time_since_epoch())
        .count();
}

bool saveDestListWrite(const std::vector<SaveDest> &v) {
    std::ofstream f(g_cfg.saveListFile.c_str(), std::ios::trunc);
    if (!f.is_open())
        return false;
    f << "# save destinations: <directory>\t<base name>\t<lastUsedEpochMillis>\n";
    for (const SaveDest &d : v)
        f << d.dir << '\t' << d.base << '\t' << d.usedMs << '\n';
    return f.good();
}
} // namespace

std::vector<SaveDest> saveDestListLoad() {
    std::vector<SaveDest> v;
    std::ifstream f(g_cfg.saveListFile.c_str());
    if (!f.is_open())
        return v;
    std::string line;
    while (std::getline(f, line)) {
        if (line.empty() || line[0] == '#')
            continue;
        const size_t t1 = line.find('\t');
        if (t1 == std::string::npos)
            continue;
        const size_t t2 = line.find('\t', t1 + 1);
        if (t2 == std::string::npos)
            continue;
        SaveDest d;
        d.dir = line.substr(0, t1);
        d.base = line.substr(t1 + 1, t2 - t1 - 1);
        d.usedMs = std::strtoll(line.c_str() + t2 + 1, nullptr, 10);
        if (!d.base.empty())
            v.push_back(d);
    }
    std::sort(v.begin(), v.end(), [](const SaveDest &a, const SaveDest &b) {
        return a.usedMs > b.usedMs; // last used first
    });
    return v;
}

bool saveDestListUpsert(const std::string &dir, const std::string &base) {
    if (base.empty())
        return false;
    std::vector<SaveDest> v = saveDestListLoad();
    const int64_t now = epochMsNow();
    bool found = false;
    for (SaveDest &d : v) {
        if (d.dir == dir && d.base == base) {
            d.usedMs = now;
            found = true;
            break;
        }
    }
    if (!found) {
        SaveDest d;
        d.dir = dir;
        d.base = base;
        d.usedMs = now;
        v.push_back(d);
    }
    std::sort(v.begin(), v.end(), [](const SaveDest &a, const SaveDest &b) {
        return a.usedMs > b.usedMs;
    });
    if (v.size() > kMaxSaveDests)
        v.resize(kMaxSaveDests);
    return saveDestListWrite(v);
}

bool saveDestListRemove(const std::string &dir, const std::string &base) {
    std::vector<SaveDest> v = saveDestListLoad();
    const size_t before = v.size();
    v.erase(std::remove_if(v.begin(), v.end(),
                           [&](const SaveDest &d) {
                               return d.dir == dir && d.base == base;
                           }),
            v.end());
    if (v.size() == before)
        return true;
    return saveDestListWrite(v);
}

bool pedeSaveFile(const std::string &path, std::string &error) {
    std::shared_ptr<const PedeSet> set = PedestalStore::instance().get();
    if (!set) {
        error = "no pedestals loaded";
        return false;
    }
    const int pfd = ::open(path.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0666);
    if (pfd == -1) {
        error = std::string("open ") + path + ": " + std::strerror(errno);
        return false;
    }
    const size_t arrayBytes =
        static_cast<size_t>(JF_PEDE_ARRAY_FLOATS) * sizeof(float);
    size_t written = 0;
    const float *arrays[3] = {set->g0.data(), set->g1.data(), set->g2.data()};
    for (int a = 0; a < 3; a++) {
        const char *p = reinterpret_cast<const char *>(arrays[a]);
        size_t left = arrayBytes;
        while (left > 0) {
            const ssize_t n = ::write(pfd, p, left);
            if (n <= 0) {
                error = std::string("write ") + path + ": " + std::strerror(errno);
                ::close(pfd);
                return false;
            }
            left -= static_cast<size_t>(n);
            p += n;
            written += static_cast<size_t>(n);
        }
    }
    ::close(pfd);
    (void)written;
    return true;
}

// ---------------------------------------------------------------------------
// GateEngine
// ---------------------------------------------------------------------------
// Calls fn(i) for every pixel of the frame, or only for the gate ROI when one
// is set.  fn returns false to stop early.  Templated so it inlines into the
// pixel loops.
template <typename F>
static inline void forEachGatePixel(uint64_t nPixels, bool roiOn, int x0,
                                    int y0, int x1, int y1, F &&fn) {
    if (!roiOn) {
        for (uint64_t i = 0; i < nPixels; i++) {
            if (!fn(i))
                return;
        }
        return;
    }
    for (int y = y0; y < y1; y++) {
        const uint64_t row = static_cast<uint64_t>(y) * 1024u;
        if (row >= nPixels)
            return;
        for (int x = x0; x < x1; x++) {
            const uint64_t i = row + static_cast<uint64_t>(x);
            if (i >= nPixels)
                return;
            if (!fn(i))
                return;
        }
    }
}

void GateEngine::setRoi(int x0, int y0, int x1, int y1) {
    // clamp to the assembled frame (1024 x 512) and require a usable area
    if (x0 < 0) x0 = 0;
    if (y0 < 0) y0 = 0;
    if (x1 > 1024) x1 = 1024;
    if (y1 > 512) y1 = 512;
    if (x1 - x0 < 4 || y1 - y0 < 4) {
        clearRoi();
        return;
    }
    roiX0_.store(x0);
    roiY0_.store(y0);
    roiX1_.store(x1);
    roiY1_.store(y1);
    roiSet_.store(true); // selected; setRoiOnly() decides whether it is used
}

void GateEngine::clearRoi() {
    roiOn_.store(false);
    roiSet_.store(false);
    roiX0_.store(0);
    roiY0_.store(0);
    roiX1_.store(1024);
    roiY1_.store(512);
}

void GateEngine::roiSnapshot(int &x0, int &y0, int &x1, int &y1) const {
    x0 = roiX0_.load();
    y0 = roiY0_.load();
    x1 = roiX1_.load();
    y1 = roiY1_.load();
}

void GateEngine::setEnabled(bool on) { enabled_.store(on); }

void GateEngine::setThreshold(float adc) { threshold_.store(adc); }

void GateEngine::setMinPixels(uint64_t n) { minPixels_.store(n); }

void GateEngine::setContiguous(bool on) { contiguous_.store(on); }

void GateEngine::setScFilter(int sc) {
    if (sc < 0 || sc > 15)
        sc = -1; // anything else means "all"
    scFilter_.store(sc);
}

void GateEngine::resetCounters() {
    c_.framesIn.store(0);
    c_.framesPass.store(0);
    c_.framesDrop.store(0);
    c_.framesScDrop.store(0);
    c_.pixelsAbove.store(0);
    c_.bytesIn.store(0);
    c_.bytesOut.store(0);
    cCur_.framesIn.store(0);
    cCur_.framesPass.store(0);
    cCur_.framesDrop.store(0);
    cCur_.framesScDrop.store(0);
    cCur_.pixelsAbove.store(0);
    r_.rateIn.store(0.0);
    r_.ratePass.store(0.0);
    r_.rateInMBs.store(0.0);
    r_.ratePassMBs.store(0.0);
}

void GateEngine::noteReceived(uint64_t bytes, bool broken) {
    c_.framesIn.fetch_add(1);
    cCur_.framesIn.fetch_add(1);
    c_.bytesIn.fetch_add(bytes);
    if (broken) {
        c_.framesDrop.fetch_add(1);
        cCur_.framesDrop.fetch_add(1);
    }
}

void GateEngine::noteForwarded(uint64_t bytes) {
    c_.bytesOut.fetch_add(bytes);
}

void GateEngine::setConnected(bool on) { connected_.store(on); }

void GateEngine::setLastError(const std::string &msg) {
    std::lock_guard<std::mutex> lock(errMtx_);
    lastError_ = msg;
}

GateEngine::Counters GateEngine::snapshot() const {
    Counters cs;
    cs.framesIn = c_.framesIn.load();
    cs.framesPass = c_.framesPass.load();
    cs.framesDrop = c_.framesDrop.load();
    cs.pixelsAbove = c_.pixelsAbove.load();
    cs.framesInCur = cCur_.framesIn.load();
    cs.framesPassCur = cCur_.framesPass.load();
    cs.framesDropCur = cCur_.framesDrop.load();
    cs.pixelsAboveCur = cCur_.pixelsAbove.load();
    cs.rateIn = r_.rateIn.load();
    cs.ratePass = r_.ratePass.load();
    cs.rateInMBs = r_.rateInMBs.load();
    cs.ratePassMBs = r_.ratePassMBs.load();
    cs.dropFrac = (cs.framesIn > 0)
                      ? static_cast<double>(cs.framesDrop) /
                            static_cast<double>(cs.framesIn)
                      : 0.0;
    cs.connected = connected_.load();
    cs.pedeLoaded = PedestalStore::instance().loaded() && !gNoPede.load();
    cs.framesScDrop = c_.framesScDrop.load();
    cs.framesScDropCur = cCur_.framesScDrop.load();
    cs.lastSc = lastSc_.load();
    cs.scFilter = scFilter_.load();
    {
        std::lock_guard<std::mutex> lock(errMtx_);
        cs.lastError = lastError_;
    }
    return cs;
}

namespace {
// ------------------------------------------------------------ pedestal drift
// Per-pixel running average of the "empty" frames the signal gate drops.  All
// of it runs on the acq thread (one call per dropped frame, rate limited), so
// the accumulator needs a mutex only because the GUI/HDF5 read the stats.
struct PedeTracker {
    std::mutex mtx;
    std::vector<float> acc;    // running average, raw ADC units
    std::vector<float> used;   // the pedestal the average is compared against
    uint64_t samples = 0;      // pixel samples fed in
    uint64_t updates = 0;      // times the used pedestal was replaced
    double maxDiff = 0.0;      // last measured largest |acc - used|
    int64_t lastUpdateMs = 0;
    int64_t lastDecideMs = 0;  // the ROI distance test runs ~1 Hz
    uint64_t phase = 0;        // sampling phase, rotated every frame
};
PedeTracker g_ptrack;

// The average is compared against the pedestal in use; replacing it means
// installing a copy of the current set with g0 = the average, which every
// consumer picks up on its next snapshot.
void pedeTrackInstall(const PedeSet *cur, const std::vector<float> &acc) {
    auto set = std::make_shared<PedeSet>();
    if (cur)
        *set = *cur; // keeps g1/g2/hasAll
    set->g0 = acc;
    PedestalStore::instance().install(set);
    gPedeTracked.store(true);
}
} // namespace

double pedeTrackSignalDropRate(uint64_t passCur, uint64_t dropCur,
                               uint64_t passTotal, uint64_t dropTotal) {
    // The Cur counters are reset by the 1 Hz rate update, so this is the recent
    // behaviour.  Using the cumulative ones was the bug: after any beam-on
    // stretch the cumulative drop rate stays low, so the tracking never fed
    // again even when the beam went off.
    static double held = -1.0;
    const uint64_t curTot = passCur + dropCur;
    if (curTot >= 1) {
        held = static_cast<double>(dropCur) / static_cast<double>(curTot);
        return held;
    }
    if (held >= 0.0)
        return held; // a window with no frames yet: keep the last value
    const uint64_t tot = passTotal + dropTotal;
    return tot > 0 ? static_cast<double>(dropTotal) / static_cast<double>(tot)
                   : 0.0;
}

PedeTrackStats pedeTrackerStats() {
    PedeTrackStats st;
    st.on = gPedeTracking.load();
    st.tracked = gPedeTracked.load();
    std::lock_guard<std::mutex> lk(g_ptrack.mtx);
    st.samples = g_ptrack.samples;
    st.updates = g_ptrack.updates;
    st.maxDiff = g_ptrack.maxDiff;
    st.sinceUpdateMs = g_ptrack.lastUpdateMs
                           ? (int64_t)nowMs() - g_ptrack.lastUpdateMs
                           : 0;
    return st;
}

// Called from PedestalStore::install(), i.e. both when the operator loads a
// pedestal and when the tracker itself replaces it.  Only the running average
// is dropped (it re-primes from whatever pedestal is now in use); samples and
// updates are cumulative diagnostics and survive.
void pedeTrackerReset() {
    std::lock_guard<std::mutex> lk(g_ptrack.mtx);
    g_ptrack.acc.clear();
    g_ptrack.used.clear();
    g_ptrack.maxDiff = 0.0;
    g_ptrack.lastUpdateMs = 0;
    gPedeTracked.store(false);
}

void pedeTrackerFeed(const short *frame, uint64_t nPixels, const PedeSet *peds,
                     double signalDropRate, int roiX0, int roiY0, int roiX1,
                     int roiY1) {
    if (!gPedeTracking.load() || !frame || !peds)
        return;
    const int64_t now = (int64_t)nowMs();
    // only while the beam is effectively off: the gate is dropping the frames
    if (signalDropRate < PEDE_TRACK_DROP_RATE)
        return;

    // A fixed stride of 1/0.2 % would sample the same pixels on every frame, so
    // the phase is rotated: after `stride` frames every pixel has been taken
    // once.  The weight is per sample, so the pooled window stays
    // PEDE_TRACK_WINDOW samples.
    const uint64_t stride =
        std::max<uint64_t>(1, (uint64_t)std::lround(1.0 / PEDE_TRACK_FRACTION));
    const float alpha = (float)((double)nPixels / PEDE_TRACK_WINDOW);

    bool doInstall = false;
    bool safety = false;
    double md = 0.0, usedMean = 0.0;
    uint64_t updNo = 0, samNo = 0;
    std::vector<float> snapshot;
    {
        std::lock_guard<std::mutex> lk(g_ptrack.mtx);
        if (g_ptrack.acc.size() != peds->g0.size()) {
            g_ptrack.acc = peds->g0; // re-prime (also after an install)
            g_ptrack.used = peds->g0;
            g_ptrack.lastUpdateMs = now;
            g_ptrack.lastDecideMs = 0;
            g_ptrack.phase = 0;
        }
        const uint64_t start = g_ptrack.phase % stride;
        g_ptrack.phase++;
        uint64_t fed = 0;
        for (uint64_t i = start; i < nPixels; i += stride) {
            if (frame[i] & 0xc000)
                continue; // high-gain / gain-3 pixels live on another scale
            const float raw = static_cast<float>(frame[i] & 0x3fff);
            g_ptrack.acc[i] += (raw - g_ptrack.acc[i]) * alpha;
            fed++;
        }
        if (fed == 0)
            fed = 1;
        g_ptrack.samples += fed;

        // The ROI-wide distance test is the only expensive part (~0.5 ms), so
        // it runs about once a second; the sampling above runs on every frame.
        if (now - g_ptrack.lastDecideMs >= 1000) {
            g_ptrack.lastDecideMs = now;
            for (size_t i = 0; i < nPixels && i < g_ptrack.acc.size(); i++) {
                const int px = static_cast<int>(i % 1024);
                const int py = static_cast<int>(i / 1024);
                if (px < roiX0 || px >= roiX1 || py < roiY0 || py >= roiY1)
                    continue;
                const double d = std::fabs((double)g_ptrack.acc[i] -
                                           (double)g_ptrack.used[i]);
                if (d > md)
                    md = d;
            }
            g_ptrack.maxDiff = md;
            for (uint64_t i = 0; i < nPixels && i < g_ptrack.used.size(); i++)
                usedMean += g_ptrack.used[i];
            usedMean = nPixels ? usedMean / double(nPixels) : 0.0;

            // at most 0.1 Hz, and at least once every 10 min
            const bool rateOk = (now - g_ptrack.lastUpdateMs) >=
                                PEDE_TRACK_MIN_PERIOD_MS;
            const bool overdue =
                (now - g_ptrack.lastUpdateMs) >= PEDE_TRACK_MAX_PERIOD_MS;
            doInstall = (md > PEDE_TRACK_MAXDIFF && rateOk) || overdue;
            safety = overdue && md <= PEDE_TRACK_MAXDIFF;
            if (doInstall) {
                snapshot = g_ptrack.acc;
                updNo = ++g_ptrack.updates;
                samNo = g_ptrack.samples;
                g_ptrack.lastUpdateMs = now;
            }
        }
    } // lock released: install() below resets the tracker and takes it again

    if (doInstall) {
        pedeTrackInstall(peds, snapshot);
        double mAcc = 0.0;
        for (uint64_t i = 0; i < nPixels; i++)
            mAcc += snapshot[i];
        printf("[pedetrack] pedestal updated #%llu: max delta %.2f ADU in the "
               "ROI over %llu pixel samples (%s); mean(raw average)=%.2f, "
               "mean(pedestal being replaced)=%.2f\n",
               (unsigned long long)updNo, md, (unsigned long long)samNo,
               safety ? "10 min safety update" : "drift > 5 ADU",
               nPixels ? mAcc / double(nPixels) : 0.0, usedMean);
        fflush(stdout);
    }
}

// The gate decision for one frame.  Called by the acq thread only.
bool GateEngine::evaluate(const short *frame, uint64_t nPixels, int sc,
                          const PedeSet *peds, int g1off, int g2off,
                          uint64_t &above, bool *scDropped) {
    above = 0;
    if (scDropped)
        *scDropped = false;
    lastSc_.store(sc);
    // Storage-cell selection first: it is a pure header check, so frames from
    // other cells are dropped before any pixel is touched.
    const int wantSc = scFilter_.load();
    if (wantSc >= 0 && sc != wantSc) {
        c_.framesScDrop.fetch_add(1);
        cCur_.framesScDrop.fetch_add(1);
        if (scDropped)
            *scDropped = true;
        return false;
    }
    if (!enabled_.load()) { // transparent passthrough
        c_.framesPass.fetch_add(1);
        cCur_.framesPass.fetch_add(1);
        return true;
    }
    above = countPixelsAbove(frame, nPixels, peds, g1off, g2off);
    const bool passCount = (above >= minPixels_.load());
    bool passCluster = true;
    if (passCount && contiguous_.load())
        passCluster = hitsClustered(frame, nPixels, peds, g1off, g2off);

    if (passCount && passCluster) {
        c_.pixelsAbove.fetch_add(above);
        cCur_.pixelsAbove.fetch_add(above);
        c_.framesPass.fetch_add(1);
        cCur_.framesPass.fetch_add(1);
        return true;
    }
    c_.framesDrop.fetch_add(1);
    cCur_.framesDrop.fetch_add(1);
    return false;
}

// Gain-aware pixel counting, identical to the original jf_filter:
//   g0: raw - pede0
//   g1: (pede1 + G1off - raw) * 30
//   g3: (pede2 + G2off - raw) * 340      (g==2 unused -> treated as g0)
// When only the gain-0 array is available (live pedestal acquisition) every
// gain falls back to raw - pede0, exactly like the display's own fallback.
uint64_t GateEngine::countPixelsAbove(const short *frame, uint64_t nPixels,
                                      const PedeSet *peds, int g1off,
                                      int g2off) const {
    const float thr = threshold_.load();
    const bool roiOn = roiOn_.load();
    const int x0 = roiX0_.load(), y0 = roiY0_.load();
    const int x1 = roiX1_.load(), y1 = roiY1_.load();

    const float *p0 = peds ? peds->g0.data() : nullptr;
    const bool gainAware = peds && peds->hasAll;
    const float *p1 = gainAware ? peds->g1.data() : nullptr;
    const float *p2 = gainAware ? peds->g2.data() : nullptr;

    uint64_t nAbove = 0;
    forEachGatePixel(nPixels, roiOn, x0, y0, x1, y1, [&](uint64_t i) {
        float v;
        if (!p0) { // no pedestals at all: raw 14-bit values
            v = static_cast<float>(frame[i] & 0x3fff);
        } else if (!gainAware) {
            v = static_cast<float>(frame[i] & 0x3fff) - p0[i];
        } else {
            const int raw = frame[i] & 0x3fff;
            const int g = (frame[i] & 0xc000) ? ((frame[i] >> 14) & 0x3) : 0;
            if (g == 0) {
                v = static_cast<float>(raw) - p0[i];
            } else if (g == 1) {
                v = (p1[i] + static_cast<float>(g1off) -
                     static_cast<float>(raw)) * 30.0f;
            } else if (g == 3) {
                v = (p2[i] + static_cast<float>(g2off) -
                     static_cast<float>(raw)) * 340.0f;
            } else {
                v = static_cast<float>(raw) - p0[i];
            }
        }
        if (v > thr)
            nAbove++;
        return true;
    });
    return nAbove;
}

// Cheap contiguity proxy: every above-threshold pixel must lie within a
// circle of radius (minPixels/2 + 1) around the above-threshold centroid.
bool GateEngine::hitsClustered(const short *frame, uint64_t nPixels,
                               const PedeSet *peds, int g1off,
                               int g2off) const {
    const float thr = threshold_.load();
    const uint64_t nReq = minPixels_.load();
    const double radius = static_cast<double>(nReq) / 2.0 + 1.0;
    const double r2 = radius * radius;
    const bool roiOn = roiOn_.load();
    const int x0 = roiX0_.load(), y0 = roiY0_.load();
    const int x1 = roiX1_.load(), y1 = roiY1_.load();

    const float *p0 = peds ? peds->g0.data() : nullptr;
    const bool gainAware = peds && peds->hasAll;
    const float *p1 = gainAware ? peds->g1.data() : nullptr;
    const float *p2 = gainAware ? peds->g2.data() : nullptr;

    // value of one pixel, exactly as countPixelsAbove computes it
    auto valueAt = [&](uint64_t i) -> float {
        if (!p0)
            return static_cast<float>(frame[i] & 0x3fff);
        const int raw = frame[i] & 0x3fff;
        if (!gainAware)
            return static_cast<float>(raw) - p0[i];
        const int g = (frame[i] & 0xc000) ? ((frame[i] >> 14) & 0x3) : 0;
        if (g == 1)
            return (p1[i] + static_cast<float>(g1off) -
                    static_cast<float>(raw)) * 30.0f;
        if (g == 3)
            return (p2[i] + static_cast<float>(g2off) -
                    static_cast<float>(raw)) * 340.0f;
        return static_cast<float>(raw) - p0[i];
    };

    // pass 1: count + centroid of the above-threshold pixels
    uint64_t n = 0;
    double sx = 0.0, sy = 0.0;
    forEachGatePixel(nPixels, roiOn, x0, y0, x1, y1, [&](uint64_t i) {
        if (valueAt(i) > thr) {
            n++;
            sx += static_cast<double>(i % 1024);
            sy += static_cast<double>(i / 1024);
        }
        return true;
    });
    if (n < nReq)
        return false;

    const double cx = sx / static_cast<double>(n);
    const double cy = sy / static_cast<double>(n);

    // pass 2: every above-threshold pixel within radius of the centroid
    bool ok = true;
    forEachGatePixel(nPixels, roiOn, x0, y0, x1, y1, [&](uint64_t i) {
        if (valueAt(i) > thr) {
            const double dx = static_cast<double>(i % 1024) - cx;
            const double dy = static_cast<double>(i / 1024) - cy;
            if (dx * dx + dy * dy > r2) {
                ok = false;
                return false; // stop early
            }
        }
        return true;
    });
    return ok;
}

// ---------------------------------------------------------------------------
// Mailboxes
// ---------------------------------------------------------------------------
void FrameMailbox::publish(const short *data, size_t nBytes) {
    const size_t nShorts = nBytes / sizeof(short);
    std::lock_guard<std::mutex> lock(mtx);
    if (have)
        dropped++; // the previous frame never made it to the analysis stage
    if (buf.size() < nShorts)
        buf.resize(nShorts);
    if (nShorts)
        memcpy(buf.data(), data, nShorts * sizeof(short));
    len = nShorts;
    have = true;
    pushed++;
    seq++;
    cv.notify_all();
}

bool FrameMailbox::waitTake(std::vector<short> &out, size_t &lenOut,
                            uint64_t &seqOut, const std::atomic<bool> &running) {
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait_for(lock, std::chrono::milliseconds(100),
                [&] { return have || !running.load(); });
    if (!have)
        return false;
    out = buf;
    lenOut = len;
    seqOut = seq;
    have = false;
    return true;
}

// ---------------------------------------------------------------------------
// Pipeline
// ---------------------------------------------------------------------------
namespace {
void startAnalysisWorkers(int n);
void stopAnalysisWorkers();
} // namespace

Pipeline::Pipeline() {
    // std::atomic default construction leaves the value uninitialised in
    // C++17, which would randomly swallow the first status line
    for (auto &v : portStateShown_)
        v.store(-1);
}

Pipeline::~Pipeline() {
    stop();
    for (void *m : zmqMons_) {
        if (m)
            zmq_close(m);
    }
    zmqMons_.clear();
    for (void *sub : zmqSubs_) {
        if (sub)
            zmq_close(sub);
    }
    zmqSubs_.clear();
    if (zmqPub_) {
        zmq_close(zmqPub_);
        zmqPub_ = nullptr;
    }
    if (zmqCtx_) {
        zmq_ctx_shutdown(zmqCtx_);
        zmq_ctx_term(zmqCtx_);
        zmqCtx_ = nullptr;
    }
}

bool Pipeline::init(std::string &error) {
    zmqCtx_ = zmq_ctx_new();
    if (!zmqCtx_) {
        error = "zmq_ctx_new failed";
        return false;
    }

    int linger = 0, rcvtimeo = 200, sndtimeo = 200;
    int hwm = g_cfg.rxHwm > 0 ? g_cfg.rxHwm : 1000;
    int rcvbuf = 8 * 1024 * 1024;

    rxPorts_ = g_cfg.rxPorts;
    if (rxPorts_.empty())
        rxPorts_.push_back(DEFAULT_RX_PORT);
    if (rxPorts_.size() > JF_MAX_PORTS) {
        error = "too many rx ports (max " + std::to_string(JF_MAX_PORTS) + ")";
        return false;
    }
    slots_.assign(rxPorts_.size(), StreamSlot());

    // One SUB socket + one socket monitor per interface.  The monitors must
    // be armed BEFORE connecting: they tell us whether each receiver stream is
    // really connected, so an idle-but-alive receiver is never mistaken for a
    // stopped one.
    for (size_t i = 0; i < rxPorts_.size(); i++) {
        void *sub = zmq_socket(zmqCtx_, ZMQ_SUB);
        if (!sub) {
            error = "zmq_socket(SUB) failed";
            return false;
        }
        zmq_setsockopt(sub, ZMQ_LINGER, &linger, sizeof(linger));
        zmq_setsockopt(sub, ZMQ_RCVTIMEO, &rcvtimeo, sizeof(rcvtimeo));
        zmq_setsockopt(sub, ZMQ_RCVHWM, &hwm, sizeof(hwm));
        zmq_setsockopt(sub, ZMQ_RCVBUF, &rcvbuf, sizeof(rcvbuf));
        zmq_setsockopt(sub, ZMQ_SUBSCRIBE, "", 0);

        const std::string monAddr =
            "inproc://online_jf_monitor_" + std::to_string(i);
        void *mon = nullptr;
        if (zmq_socket_monitor(sub, monAddr.c_str(), ZMQ_EVENT_ALL) == 0) {
            mon = zmq_socket(zmqCtx_, ZMQ_PAIR);
            if (!mon || zmq_connect(mon, monAddr.c_str()) != 0) {
                if (mon) {
                    zmq_close(mon);
                    mon = nullptr;
                }
                fprintf(stderr,
                        "[zmq] socket monitor unavailable for port %d, "
                        "falling back to frame-arrival staleness\n",
                        rxPorts_[i]);
            }
        } else {
            fprintf(stderr,
                    "[zmq] zmq_socket_monitor failed for port %d: %s\n",
                    rxPorts_[i], zmq_strerror(zmq_errno()));
        }

        const std::string addr =
            "tcp://" + g_cfg.rxHost + ":" + std::to_string(rxPorts_[i]);
        if (zmq_connect(sub, addr.c_str()) != 0) {
            const std::string why = zmq_strerror(zmq_errno());
            zmq_close(sub);
            if (mon)
                zmq_close(mon);
            error = "zmq_connect(" + addr + ") failed: " + why;
            return false;
        }
        zmqSubs_.push_back(sub);
        zmqMons_.push_back(mon);
        if (mon)
            monitorActive_.store(true);
    }

    if (g_cfg.pubPort > 0) {
        zmqPub_ = zmq_socket(zmqCtx_, ZMQ_PUB);
        if (!zmqPub_) {
            error = "zmq_socket(PUB) failed";
            return false;
        }
        zmq_setsockopt(zmqPub_, ZMQ_LINGER, &linger, sizeof(linger));
        zmq_setsockopt(zmqPub_, ZMQ_SNDTIMEO, &sndtimeo, sizeof(sndtimeo));
        const std::string bindAddr = "tcp://*:" + std::to_string(g_cfg.pubPort);
        if (zmq_bind(zmqPub_, bindAddr.c_str()) != 0) {
            error = "zmq_bind(" + bindAddr + ") failed: " +
                    zmq_strerror(zmq_errno());
            return false;
        }
    }

    gate_.setEnabled(g_cfg.gateEnabled);
    gate_.setThreshold(g_cfg.threshold);
    gate_.setMinPixels(g_cfg.minPixels);
    gate_.setContiguous(g_cfg.contiguous);
    return true;
}

void Pipeline::start() {
    if (running_.load())
        return;
    running_.store(true);
    startAnalysisWorkers(g_cfg.analysisWorkers);
    acqThread_ = new std::thread(&Pipeline::acqLoop, this);
    analysisThread_ = new std::thread(&Pipeline::analysisLoop, this);
}

void Pipeline::stop() {
    running_.store(false);
    frames_.cv.notify_all();
    if (acqThread_) {
        if (acqThread_->joinable())
            acqThread_->join();
        delete acqThread_;
        acqThread_ = nullptr;
    }
    if (analysisThread_) {
        if (analysisThread_->joinable())
            analysisThread_->join();
        delete analysisThread_;
        analysisThread_ = nullptr;
    }
    // only after the analysis thread is done, so no worker is mid-frame
    stopAnalysisWorkers();
}

bool Pipeline::connected() const { return gate_.connected_.load(); }

uint64_t Pipeline::portFrames(int portIdx) const {
    if (portIdx < 0 || portIdx >= static_cast<int>(rxPorts_.size()))
        return 0;
    return track_[portIdx].frames.load();
}

int Pipeline::portWidth(int portIdx) const {
    if (portIdx < 0 || portIdx >= static_cast<int>(rxPorts_.size()))
        return 0;
    return track_[portIdx].w.load();
}

int Pipeline::portHeight(int portIdx) const {
    if (portIdx < 0 || portIdx >= static_cast<int>(rxPorts_.size()))
        return 0;
    return track_[portIdx].h.load();
}

uint64_t Pipeline::portLastFrame(int portIdx) const {
    if (portIdx < 0 || portIdx >= static_cast<int>(rxPorts_.size()))
        return 0;
    return track_[portIdx].lastFrame.load();
}

uint64_t Pipeline::resyncDrops() const { return resyncDrops_.load(); }

uint64_t Pipeline::behindDrops() const { return behindDrops_.load(); }

uint64_t Pipeline::incompleteFrames() const { return incomplete_.load(); }

uint64_t Pipeline::displayFrames() const { return framesinstream.load(); }

uint64_t Pipeline::displayDropped() const {
    std::lock_guard<std::mutex> lock(const_cast<FrameMailbox &>(frames_).mtx);
    return frames_.dropped;
}

void Pipeline::requestPedeAcquire(int nFrames) {
    if (nFrames < 1)
        nFrames = 1;
    pedeAcqWant_.store(nFrames);
    pedeAcqDone_.store(0);
    {
        std::lock_guard<std::mutex> lock(frames_.mtx);
        pedeAcq_.reset();
    }
    pedeAcqActive_.store(true);
    printf("[pede] acquiring from %d frames...\n", nFrames);
    fflush(stdout);
}

bool Pipeline::pedeAcquireActive() const { return pedeAcqActive_.load(); }

int Pipeline::pedeAcquireProgress() const { return pedeAcqDone_.load(); }

// acq thread, while a live pedestal acquisition is running
// In "save frames" mode this is the old path.  In virtual-photon-counting mode
// the frames are not written: instead the acq thread keeps the window
// bookkeeping (first frame number and timestamp, frame count) and hands each
// published map to the saver tagged with it.
void Pipeline::offerToSaver(const short *frame, int w, int h,
                            const SavedFrameHeader &hdr) {
    if (gSaveMode.load() != 1) {
        saver_.offer(frame, w, h, hdr);
        return;
    }
    if (pmapWinCount == 0) {
        pmapWinFirstFrame = hdr.frameNumber;
        pmapWinFirstStamp = hdr.timestamp;
    }
    pmapWinCount++;
    PmapSlot &slot = pmapSave();
    std::vector<float> map;
    uint64_t frames = 0;
    {
        std::lock_guard<std::mutex> lock(slot.mtx);
        if (slot.ready) {
            map.swap(slot.map);
            frames = slot.frames;
            slot.ready = false;
        }
    }
    if (map.empty())
        return;
    saver_.offerPhotonMap(map, frames, pmapWinFirstFrame, pmapWinFirstStamp);
    pmapWinCount = 0;
}

void pmapSaveReset() {
    std::lock_guard<std::mutex> lock(gHistoMtx);
    // assign (not fill): this also allocates, so the very first analysed frame
    // after the save started is counted instead of being dropped while the
    // accumulator is still empty
    g_pmapSaveAccum.assign(static_cast<size_t>(NCH), 0.0f);
    g_pmapSaveCount = 0;
}

void Pipeline::acquirePede(const short *frame) {
    if (!pedeAcq_)
        pedeAcq_ = std::make_shared<PedeSet>();
    const int done = pedeAcqDone_.load();
    float *g0 = pedeAcq_->g0.data();
    for (uint64_t i = 0; i < NCH; i++) {
        const float raw = static_cast<float>(frame[i] & 0x3fff);
        g0[i] = (g0[i] * static_cast<float>(done) + raw) /
                static_cast<float>(done + 1);
    }
    const int n = done + 1;
    pedeAcqDone_.store(n);
    if (n < pedeAcqWant_.load())
        return;

    // publish: gain-0 only, so hasAll stays false (same as the old live
    // acquisition, which never filled the G1/G2 arrays)
    PedestalStore::instance().install(pedeAcq_);
    {
        std::lock_guard<std::mutex> lock(frames_.mtx);
        pedeAcq_.reset();
    }
    pedeAcqActive_.store(false);
    gFrameCountReset.store(true);
    printf("[pede] installed pedestals averaged over %d frames (storage cell "
           "%s)\n",
           n, gate_.scFilter() < 0
                  ? "ALL - every cell contributed"
                  : std::to_string(gate_.scFilter()).c_str());
    fflush(stdout);
}

// ---------------------------------------------------------------------------
// Connection tracking (per interface)
// ---------------------------------------------------------------------------
// A receive timeout only means "no frame in the last 200 ms"; it says nothing
// about the receiver being alive.  The ZMQ socket monitor tells us the truth:
// an idle-but-connected receiver stays "Connected"/"Idle", and "stopped" is
// only reported on a real disconnect or on the end-of-acquisition marker.
void Pipeline::drainMonitor(int portIdx) {
    if (portIdx < 0 || portIdx >= static_cast<int>(zmqMons_.size()))
        return;
    void *mon = zmqMons_[portIdx];
    if (!mon)
        return;
    StreamTrack &t = track_[portIdx];
    for (int k = 0; k < 64; k++) { // bounded burst per iteration
        unsigned char buf[256];
        const int n = zmq_recv(mon, buf, sizeof(buf), ZMQ_DONTWAIT);
        if (n < 0)
            break; // EAGAIN: no more events
        uint16_t event = 0;
        if (n >= static_cast<int>(sizeof(uint16_t)))
            memcpy(&event, buf, sizeof(uint16_t));
        int more = 0;
        size_t moreSize = sizeof(more);
        zmq_getsockopt(mon, ZMQ_RCVMORE, &more, &moreSize);
        if (more) { // drain the address part
            char addr[256];
            zmq_recv(mon, addr, sizeof(addr), ZMQ_DONTWAIT);
        }

        switch (event) {
        case ZMQ_EVENT_CONNECTED:
        case ZMQ_EVENT_HANDSHAKE_SUCCEEDED:
            if (!t.peerConnected.exchange(true))
                t.connectedSinceMs.store(nowMs());
            t.everConnected.store(true);
            break;
        case ZMQ_EVENT_DISCONNECTED:
        case ZMQ_EVENT_CLOSED:
        case ZMQ_EVENT_HANDSHAKE_FAILED_NO_DETAIL:
        case ZMQ_EVENT_HANDSHAKE_FAILED_PROTOCOL:
        case ZMQ_EVENT_HANDSHAKE_FAILED_AUTH:
            t.peerConnected.store(false);
            break;
        default:
            break; // CONNECT_DELAYED / RETRIED / LISTENING / ...
        }
    }
}

Pipeline::StreamState Pipeline::streamState(int portIdx,
                                            double *sinceSec) const {
    if (portIdx < 0 || portIdx >= static_cast<int>(rxPorts_.size())) {
        if (sinceSec)
            *sinceSec = 0.0;
        return StreamState::Disconnected;
    }
    const StreamTrack &t = track_[portIdx];
    const int64_t now = nowMs();
    double since = 0.0;
    StreamState st;

    if (zmqMons_[portIdx]) {
        if (!t.peerConnected.load()) {
            st = t.everConnected.load() ? StreamState::Disconnected
                                        : StreamState::Connecting;
            if (t.lastHeaderMs.load() > 0)
                since = (now - t.lastHeaderMs.load()) / 1000.0;
        } else if (t.endOfAcq.load()) {
            st = StreamState::EndOfAcq;
            if (t.lastHeaderMs.load() > 0)
                since = (now - t.lastHeaderMs.load()) / 1000.0;
        } else {
            const int64_t lastHdr = t.lastHeaderMs.load();
            if (lastHdr > 0)
                since = (now - lastHdr) / 1000.0;
            else {
                const int64_t sinceConnect = t.connectedSinceMs.load();
                since = (sinceConnect > 0) ? (now - sinceConnect) / 1000.0 : 0.0;
            }
            st = (since * 1000.0 > kIdleWarnMs) ? StreamState::Idle
                                                : StreamState::Connected;
        }
    } else {
        // no monitor: fall back to frame-arrival staleness
        const int64_t lastHdr = t.lastHeaderMs.load();
        if (lastHdr <= 0) {
            st = t.everConnected.load() ? StreamState::Disconnected
                                        : StreamState::Connecting;
        } else {
            since = (now - lastHdr) / 1000.0;
            st = (since * 1000.0 > kNoMonitorStaleMs) ? StreamState::Disconnected
                                                      : StreamState::Connected;
        }
    }
    if (sinceSec)
        *sinceSec = since;
    return st;
}

// Overall state = the worst port: any dead port dominates, then idle.
Pipeline::StreamState Pipeline::streamState(double *sinceSec) const {
    StreamState worst = StreamState::Connected;
    double worstSince = 0.0;
    int rank = 0; // Connected < Idle < EndOfAcq < Connecting < Disconnected
    for (size_t i = 0; i < rxPorts_.size(); i++) {
        double since = 0.0;
        const StreamState st = streamState(static_cast<int>(i), &since);
        int r = 0;
        switch (st) {
        case StreamState::Connected:
            r = 0;
            break;
        case StreamState::Idle:
            r = 1;
            break;
        case StreamState::EndOfAcq:
            r = 2;
            break;
        case StreamState::Connecting:
            r = 3;
            break;
        case StreamState::Disconnected:
            r = 4;
            break;
        }
        if (i == 0 || r > rank) {
            rank = r;
            worst = st;
            worstSince = since;
        }
    }
    if (sinceSec)
        *sinceSec = worstSince;
    return worst;
}

const char *Pipeline::streamStateName(StreamState st) {
    switch (st) {
    case StreamState::Disconnected:
        return "disconnected";
    case StreamState::Connecting:
        return "connecting";
    case StreamState::Connected:
        return "connected";
    case StreamState::Idle:
        return "idle";
    case StreamState::EndOfAcq:
        return "eoa";
    }
    return "?";
}

const char *Pipeline::streamStateName() const {
    return streamStateName(streamState());
}

std::string Pipeline::streamStateSummary() const {
    std::string out;
    for (size_t i = 0; i < rxPorts_.size(); i++) {
        if (i)
            out += ",";
        out += streamStateName(streamState(static_cast<int>(i)));
    }
    return out;
}

// acq thread: per-port status line, printed only on a real transition
void Pipeline::refreshStreamState(int portIdx) {
    if (portIdx < 0 || portIdx >= static_cast<int>(rxPorts_.size()))
        return;
    double since = 0.0;
    const StreamState st = streamState(portIdx, &since);
    const int shown = static_cast<int>(st);
    const int prev = portStateShown_[portIdx].exchange(shown);
    if (prev == shown)
        return;

    const int port = rxPorts_[portIdx];
    switch (st) {
    case StreamState::Connected:
        if (prev < 0 || prev == static_cast<int>(StreamState::Connecting) ||
            prev == static_cast<int>(StreamState::Disconnected))
            printf("Receiver connected on port %d\n", port);
        break;
    case StreamState::Disconnected:
        printf("Receiver on port %d disconnected, waiting to reconnect...\n",
               port);
        break;
    case StreamState::Idle:
        printf("Receiver on port %d connected but no frames for %.1f s "
               "(still alive)\n",
               port, since);
        break;
    case StreamState::EndOfAcq:
        printf("Receiver on port %d sent end-of-acquisition, waiting for new "
               "stream\n",
               port);
        break;
    case StreamState::Connecting:
        break;
    }
    fflush(stdout);
}

void Pipeline::refreshStreamState() {
    for (size_t i = 0; i < rxPorts_.size(); i++)
        refreshStreamState(static_cast<int>(i));

    const StreamState st = streamState();
    const bool up = (st != StreamState::Disconnected &&
                     st != StreamState::Connecting);
    gate_.setConnected(up);

    if (!multiPort()) // single-port wording is handled per port already
        return;
    const int shown = static_cast<int>(st);
    const int prev = stateShown_.exchange(shown);
    if (prev == shown)
        return;
    if (st == StreamState::Disconnected)
        printf("Stream lost: not all %zu interfaces are connected\n",
               rxPorts_.size());
    else if (st == StreamState::Idle)
        printf("Waiting for frames on at least one interface\n");
    fflush(stdout);
}

// ---------------------------------------------------------------------------
// Reading one interface
// ---------------------------------------------------------------------------
// Reads exactly one header+data pair from an interface into its slot.
// Returns false when there is nothing to read right now.
bool Pipeline::readOne(int portIdx) {
    if (portIdx < 0 || portIdx >= static_cast<int>(zmqSubs_.size()))
        return false;
    StreamTrack &t = track_[portIdx];
    StreamSlot &sl = slots_[portIdx];

    // Discards the rest of the current multipart message, so a rejected
    // header never leaves its data part to be read as the next "header".
    auto drainParts = [&]() {
        int more = 0;
        size_t moreSize = sizeof(more);
        zmq_getsockopt(zmqSubs_[portIdx], ZMQ_RCVMORE, &more, &moreSize);
        char sink[65536];
        while (more) {
            zmq_recv(zmqSubs_[portIdx], sink, sizeof(sink), 0);
            zmq_getsockopt(zmqSubs_[portIdx], ZMQ_RCVMORE, &more, &moreSize);
        }
    };

    std::vector<char> hdr(kHeaderBytes);
    const int hn =
        zmq_recv(zmqSubs_[portIdx], hdr.data(), hdr.size(), ZMQ_DONTWAIT);
    if (hn <= 0) {
        if (hn < 0 && zmq_errno() != EAGAIN)
            t.errors.fetch_add(1);
        return false;
    }
    // zmq_recv returns the full message length even when it truncated it
    if (static_cast<size_t>(hn) > hdr.size()) {
        t.errors.fetch_add(1);
        drainParts();
        return false;
    }

    rapidjson::Document d;
    if (d.Parse(hdr.data(), static_cast<size_t>(hn)).HasParseError()) {
        drainParts();
        return false;
    }
    if (!d.HasMember("jsonversion") || !d.HasMember("data")) {
        drainParts();
        return false;
    }
    if (d["jsonversion"].GetUint() != SLS_DETECTOR_JSON_HEADER_VERSION) {
        drainParts();
        return false;
    }

    // A valid header must be followed by its data part.  The parts of a
    // multipart message are queued together, so RCVMORE is authoritative here;
    // without this check a header alone would make the data recv swallow the
    // next message's header as pixels.
    {
        int moreAfterHdr = 0;
        size_t moreAfterHdrSize = sizeof(moreAfterHdr);
        zmq_getsockopt(zmqSubs_[portIdx], ZMQ_RCVMORE, &moreAfterHdr,
                       &moreAfterHdrSize);
        if (!moreAfterHdr) {
            t.errors.fetch_add(1);
            return false;
        }
    }

    hdrBytes_.assign(hdr.begin(), hdr.begin() + hn);
    hdrLen_ = hn;

    const int64_t now = nowMs();
    t.lastHeaderMs.store(now);
    // Storage cell: bits 8-11 of "detector specific 3" (0..15, 15 = default).
    // Kept per port; the assembled frame uses the first slab's value.
    if (d.HasMember("detSpec3")) {
        const uint32_t ds3 = d["detSpec3"].GetUint();
        t.sc.store(static_cast<int>((ds3 >> 8) & 0xF));
    }
    if (d["data"].GetUint() == 0) { // end-of-acquisition marker
        t.endOfAcq.store(true);
        refreshStreamState(portIdx);
        drainParts(); // none expected (sent without SNDMORE)
        return false;
    }
    t.endOfAcq.store(false);
    {
        // a data header must be followed by its data part
        int more = 0;
        size_t moreSize = sizeof(more);
        zmq_getsockopt(zmqSubs_[portIdx], ZMQ_RCVMORE, &more, &moreSize);
        if (!more)
            return false;
    }

    // Receiver ROI from the "rx_roi" array the receiver puts in every stream
    // header: [xmin, xmax, ymin, ymax] in RAW port coordinates.  This is NOT
    // the gate ROI: it says which part of the port the receiver itself handled.
    // It is drawn as the red box on the ADC image (mapped to the display when
    // the strixel mapping is active).  The old detSpec1 reinterpretation was a
    // hack - detSpec1 is the bunch id - and is gone.
    if (d.HasMember("rx_roi") && d["rx_roi"].IsArray() &&
        d["rx_roi"].Size() >= 4) {
        const int xmin = d["rx_roi"][0].GetInt();
        const int xmax = d["rx_roi"][1].GetInt();
        const int ymin = d["rx_roi"][2].GetInt();
        const int ymax = d["rx_roi"][3].GetInt();
        if (xmax > xmin && ymax > ymin) {
            const bool changed = (xmin != rxRoiXmin.load() ||
                                  xmax != rxRoiXmax.load() ||
                                  ymin != rxRoiYmin.load() ||
                                  ymax != rxRoiYmax.load());
            rxRoiXmin.store(static_cast<uint16_t>(xmin));
            rxRoiXmax.store(static_cast<uint16_t>(xmax));
            rxRoiYmin.store(static_cast<uint16_t>(ymin));
            rxRoiYmax.store(static_cast<uint16_t>(ymax));
            rxRoiValid.store(true);
            if (changed) {
                printf("[rx] receiver ROI x=%d..%d y=%d..%d (raw port coords, "
                       "drawn as the red box)\n",
                       xmin, xmax, ymin, ymax);
                fflush(stdout);
            }
        } else {
            rxRoiValid.store(false);
        }
    } else {
        rxRoiValid.store(false);
    }

    int w = 0, h = 0;
    if (d.HasMember("shape") && d["shape"].IsArray() &&
        d["shape"].Size() >= 2) {
        w = static_cast<int>(d["shape"][0].GetUint());
        h = static_cast<int>(d["shape"][1].GetUint());
    }
    size_t needBytes = static_cast<size_t>(NCH) * 2;
    if (d.HasMember("size")) {
        const unsigned sz = d["size"].GetUint();
        if (sz > 0)
            needBytes = sz;
    } else if (w > 0 && h > 0) {
        needBytes = static_cast<size_t>(w) * static_cast<size_t>(h) * 2;
    }
    if (needBytes > static_cast<size_t>(NCH) * 2 * 4) // sanity clamp
        needBytes = static_cast<size_t>(NCH) * 2;

    if (sl.data.size() < needBytes / 2)
        sl.data.resize(needBytes / 2);
    const int received = zmq_recv(zmqSubs_[portIdx], sl.data.data(),
                                  static_cast<int>(needBytes), 0);
    if (received <= 0) {
        t.errors.fetch_add(1);
        return false;
    }
    // larger than advertised: zmq truncated it to needBytes but returns the
    // full length, which must not become sl.len (it sizes the memcpy later)
    const bool truncated = static_cast<size_t>(received) > needBytes;
    int more = 0;
    size_t moreSize = sizeof(more);
    zmq_getsockopt(zmqSubs_[portIdx], ZMQ_RCVMORE, &more, &moreSize);
    if (more) { // payload larger than advertised: drain the remainder
        char sink[65536];
        while (more) {
            zmq_recv(zmqSubs_[portIdx], sink, sizeof(sink), 0);
            zmq_getsockopt(zmqSubs_[portIdx], ZMQ_RCVMORE, &more, &moreSize);
        }
    }
    if (truncated) {
        t.errors.fetch_add(1);
        return false;
    }

    // Overwriting a slab that was never assembled means it is being discarded
    // rather than shown late (only happens in the deliberate skip-ahead path).
    if (sl.valid)
        behindDrops_.fetch_add(1);

    sl.len = static_cast<size_t>(received) / sizeof(short);
    sl.w = w;
    sl.h = h;
    sl.valid = true;
    sl.stampMs = now;
    sl.frameKey = d.HasMember("frameNumber")
                      ? d["frameNumber"].GetUint64()
                      : (d.HasMember("frameIndex") ? d["frameIndex"].GetUint64()
                                                   : 0);
    // header fields mirrored into the ROI-save parameter datasets
    {
        SavedFrameHeader &h = sl.hdr;
        h = SavedFrameHeader();
        h.frameNumber = sl.frameKey;
        if (d.HasMember("expLength"))
            h.expLength = d["expLength"].GetUint();
        if (d.HasMember("packetNumber"))
            h.packetNumber = d["packetNumber"].GetUint();
        if (d.HasMember("detSpec1"))
            h.detSpec1 = d["detSpec1"].GetUint64();
        if (d.HasMember("timestamp"))
            h.timestamp = d["timestamp"].GetUint64();
        if (d.HasMember("modId"))
            h.modId = static_cast<uint16_t>(d["modId"].GetUint());
        if (d.HasMember("row"))
            h.row = static_cast<uint16_t>(d["row"].GetUint());
        if (d.HasMember("column"))
            h.column = static_cast<uint16_t>(d["column"].GetUint());
        if (d.HasMember("detSpec2"))
            h.detSpec2 = static_cast<uint16_t>(d["detSpec2"].GetUint());
        if (d.HasMember("detSpec3"))
            h.detSpec3 = d["detSpec3"].GetUint();
        if (d.HasMember("detSpec4"))
            h.detSpec4 = static_cast<uint16_t>(d["detSpec4"].GetUint());
        if (d.HasMember("detType"))
            h.detType = static_cast<uint8_t>(d["detType"].GetUint());
        if (d.HasMember("version"))
            h.version = static_cast<uint8_t>(d["version"].GetUint());
    }
    t.frames.fetch_add(1);
    t.lastFrame.store(sl.frameKey);
    if (portIdx < JF_MAX_PORTS)
        lastProgressMs_[portIdx] = now; // this interface is alive and moving
    if (firstDataMs_ == 0)
        firstDataMs_ = now;
    if (w > 0) {
        t.w.store(w);
        fullW_.store(w);
    }
    if (h > 0)
        t.h.store(h);
    fullH_.store(kRawHeight); // N slabs of kRawHeight/N rows assemble

    if (!sl.logged) {
        sl.logged = true;
        const int slabRows =
            multiPort() ? kRawHeight / static_cast<int>(rxPorts_.size())
                        : kRawHeight;
        const int rowBase = static_cast<int>(portIdx) * slabRows;
        printf("[stream] port %d: %dx%d (%zu B) -> rows %d..%d of %dx%d\n",
               rxPorts_[portIdx], w, h, sl.len * sizeof(short), rowBase,
               rowBase + (h > 0 ? h : slabRows) - 1, fullW_.load(),
               fullH_.load());
        fflush(stdout);
    }
    refreshStreamState(portIdx);
    return true;
}

// One frame per poll cycle.  The acq loop only calls this for an interface
// whose slot is EMPTY (flow control): a slab waiting to be paired is never
// overwritten, which is what lets a persistently lagging interface still be
// paired instead of losing the leader's matching frame.
void Pipeline::readStream(int portIdx) { readOne(portIdx); }

// Discards a lagging interface's queued frames until it reaches targetKey.
void Pipeline::drainTo(int portIdx, uint64_t targetKey) {
    StreamSlot &sl = slots_[portIdx];
    for (int k = 0; k < kMaxDrainPerCall; k++) {
        if (sl.valid && sl.frameKey >= targetKey)
            return;
        if (!readOne(portIdx))
            return;
    }
}

// ---------------------------------------------------------------------------
// Frame assembly (multi-interface)
// ---------------------------------------------------------------------------
// Each interface delivers one Y-slab of the module (1024x256 for a Jungfrau
// split in two).  Slabs are stacked in the order the ports were given, the
// first port occupying the LOWEST rows, so "30001+30002" means (bottom, top).
//
// Slabs are paired on the DETECTOR frame number (frameIndex is per-streamer
// and offset by each interface's own first index, so it cannot be a shared
// key).
//
// Alignment policy - an assembled frame is ALWAYS complete and index-aligned:
//   * flow control - an interface holding an unpaired slab is not read again,
//     so the frame the partner still has to match is kept;
//   * a slab behind the leading frame number is discarded and counted in
//     'resync' (it can never be paired any more);
//   * an interface that has not produced a frame for kStallMs is treated as
//     stalled: the pending slabs are discarded (counted in 'incomplete')
//     rather than shown half-filled, so a dead stream cannot freeze the image.
bool Pipeline::tryAssemble(bool force) {
    const size_t n = rxPorts_.size();
    if (n == 0)
        return false;

    int totalH = fullH_.load();
    int width = fullW_.load();
    if (totalH <= 0 || width <= 0) {
        totalH = kRawHeight;
        width = kRawWidth;
    }
    static bool geomWarned = false;
    if (totalH % static_cast<int>(n) != 0) {
        if (!geomWarned) {
            geomWarned = true;
            printf("[frames] ERROR: %zu interfaces cannot evenly split %d rows; "
                   "assembling is disabled (check the rx_port list)\n",
                   n, totalH);
            fflush(stdout);
        }
        return false;
    }
    const int slabRows = totalH / static_cast<int>(n);
    for (size_t i = 0; i < n; i++) {
        const int h = slots_[i].h;
        if (slots_[i].valid && h > 0 && h != slabRows && h != totalH &&
            !geomWarned) {
            geomWarned = true;
            printf("[frames] ERROR: port %d streams %d rows but this "
                   "configuration expects %d (or a full %d); assembling is "
                   "disabled\n",
                   rxPorts_[i], h, slabRows, totalH);
            fflush(stdout);
        }
    }

    // ---- how many interfaces have a slab ready? --------------------------
    size_t validCount = 0;
    uint64_t lo = 0, hi = 0;
    for (size_t i = 0; i < n; i++) {
        if (!slots_[i].valid)
            continue;
        if (validCount == 0)
            lo = hi = slots_[i].frameKey;
        else {
            lo = std::min(lo, slots_[i].frameKey);
            hi = std::max(hi, slots_[i].frameKey);
        }
        validCount++;
    }
    if (validCount == 0)
        return false; // nothing pending

    if (validCount < n) {
        // An interface has nothing pending.  While it keeps producing frames
        // we simply wait (flow control holds the others).  Only a stalled or
        // never-arriving interface ends the wait.
        const int64_t now = nowMs();
        bool stalled = false;
        for (size_t i = 0; i < n; i++) {
            if (slots_[i].valid)
                continue;
            const int64_t last = (i < JF_MAX_PORTS) ? lastProgressMs_[i] : 0;
            if (last > 0) {
                if (now - last > kStallMs)
                    stalled = true;
            } else if (now - acqStartMs_ > kInitialPairTimeoutMs) {
                stalled = true; // never delivered anything at all
            }
        }
        if (!stalled)
            return false;

        // Stalled: discard the pending slabs (never show a half-filled frame)
        // and report why, at most once per 5 s.
        uint64_t dropped = 0;
        for (size_t i = 0; i < n; i++) {
            if (slots_[i].valid) {
                slots_[i].valid = false;
                dropped++;
            }
        }
        if (dropped) {
            incomplete_.fetch_add(1);
            const int64_t now2 = nowMs();
            if (now2 - lastPairWarnMs_.load() > 5000) {
                lastPairWarnMs_.store(now2);
                printf("[frames] an interface has no data: dropping unpaired "
                       "frames to stay aligned (check the interface states)\n");
                fflush(stdout);
            }
        }
        return false;
    }

    // ---- all interfaces have a slab --------------------------------------
    if (lo != hi) {
        const uint64_t gap = hi - lo;
        // Discard the slabs that are behind the leading frame number; the
        // leading one is kept (flow control) so the partner can still reach
        // it.  A large gap is caught up in one burst.
        if (gap > 8) {
            for (size_t i = 0; i < n; i++) {
                if (slots_[i].valid && slots_[i].frameKey < hi)
                    drainTo(static_cast<int>(i), hi);
            }
        }
        uint64_t dropped = 0;
        for (size_t i = 0; i < n; i++) {
            if (slots_[i].valid && slots_[i].frameKey < hi) {
                slots_[i].valid = false;
                dropped++;
            }
        }
        resyncDrops_.fetch_add(dropped);
        if (dropped) {
            const int64_t now = nowMs();
            if (now - lastPairWarnMs_.load() > 5000) {
                lastPairWarnMs_.store(now);
                printf("[frames] interfaces out of step by %llu frame(s); "
                       "discarding the lagging slab to stay aligned\n",
                       static_cast<unsigned long long>(gap));
                fflush(stdout);
            }
        }
        return false;
    }

    // ---- index-aligned: assemble -----------------------------------------
    if (assembled_.size() != static_cast<size_t>(NCH))
        assembled_.assign(static_cast<size_t>(NCH), 0);

    for (size_t i = 0; i < n; i++) {
        StreamSlot &sl = slots_[i];
        const int rowBase = static_cast<int>(i) * slabRows;
        sl.rowBase = rowBase;

        const size_t dstOffset = static_cast<size_t>(rowBase) * width;
        const size_t dstAvail = (dstOffset < assembled_.size())
                                    ? assembled_.size() - dstOffset
                                    : 0;
        size_t srcOffset = 0;
        const size_t rows = static_cast<size_t>(sl.h > 0 ? sl.h : slabRows);
        if (sl.h == totalH && n > 1) // interface streams the whole module
            srcOffset = static_cast<size_t>(rowBase) * width;
        size_t count = rows * static_cast<size_t>(width);
        if (srcOffset + count > sl.len)
            count = (sl.len > srcOffset) ? sl.len - srcOffset : 0;
        if (count > dstAvail)
            count = dstAvail;
        if (count)
            memcpy(assembled_.data() + dstOffset, sl.data.data() + srcOffset,
                   count * sizeof(short));
        if (count < dstAvail)
            memset(reinterpret_cast<char *>(assembled_.data()) + dstOffset +
                       count * sizeof(short),
                   0, (dstAvail - count) * sizeof(short));
    }

    uint64_t bytes = 0;
    const uint64_t pairKey = slots_[0].frameKey;
    for (size_t i = 0; i < n; i++) {
        bytes += slots_[i].len * sizeof(short);
        slots_[i].valid = false;
    }
    pairWaitStartMs_ = 0;

    if (bytes == 0)
        return false;

    static bool firstAssembledLogged = false;
    if (!firstAssembledLogged) {
        firstAssembledLogged = true;
        printf("[frames] first assembled frame: ");
        for (size_t i = 0; i < n; i++) {
            const size_t px =
                static_cast<size_t>(static_cast<int>(i) * slabRows) *
                static_cast<size_t>(width);
            printf("port %d -> row %d = %d%s", rxPorts_[i],
                   static_cast<int>(i) * slabRows,
                   px < assembled_.size()
                       ? static_cast<int>(assembled_[px] & 0x3fff)
                       : -1,
                   (i + 1 < n) ? ", " : "\n");
        }
        fflush(stdout);
    }

    gate_.noteReceived(bytes, false);

    // "Acquire Pede": the pedestal depends on the storage cell, so only frames
    // that the SC filter accepts contribute (SC filter == ALL -> every cell,
    // which would mix their pedestals).  The signal gate is deliberately not
    // applied: its *dropped* frames are the empty ones a pedestal wants.
    if (pedeAcqActive_.load() &&
        (gate_.scFilter() < 0 || gate_.scFilter() == track_[0].sc.load()))
        acquirePede(assembled_.data());

    std::shared_ptr<const PedeSet> peds =
        gNoPede.load() ? nullptr : PedestalStore::instance().get();
    const int g1off = G1Poffset.load();
    const int g2off = G2Poffset.load();
    uint64_t above = 0;
    // the storage cell comes from the first (bottom) interface's header; both
    // interfaces belong to the same frame, so they report the same cell
    bool scDropped = false;
    const bool forward =
        gate_.evaluate(assembled_.data(), static_cast<uint64_t>(NCH),
                       track_[0].sc.load(), peds.get(), g1off, g2off, above,
                       &scDropped);
    // pedestal tracking: a frame the *signal* gate dropped is "beam off" data
    if (!forward && !scDropped && gPedeTracking.load() && gate_.enabled()) {
        const GateEngine::Counters gc = gate_.snapshot();
        int rx0 = 0, ry0 = 0, rx1 = 1024, ry1 = 512;
        if (gate_.roiSet())
            gate_.roiSnapshot(rx0, ry0, rx1, ry1);
        pedeTrackerFeed(assembled_.data(), static_cast<uint64_t>(NCH),
                        peds.get(),
                        pedeTrackSignalDropRate(gc.framesPassCur,
                                                gc.framesDropCur,
                                                gc.framesPass, gc.framesDrop),
                        rx0, ry0, rx1, ry1);
    }

    if (forward) {
        if (saver_.running())
            offerToSaver(assembled_.data(), width, totalH, slots_[0].hdr);
        const size_t fullBytes = static_cast<size_t>(NCH) * 2;
        if (zmqPub_) { // assembled full frame with a synthesised header
            char hdr[512];
            const int hn = snprintf(
                hdr, sizeof(hdr),
                "{\"jsonversion\":%u,\"data\":1,\"size\":%zu,"
                "\"shape\":[%d,%d],\"frameNumber\":%llu}",
                static_cast<unsigned>(SLS_DETECTOR_JSON_HEADER_VERSION),
                fullBytes, width, totalH,
                static_cast<unsigned long long>(pairKey));
            zmq_send(zmqPub_, hdr, hn, ZMQ_SNDMORE);
            zmq_send(zmqPub_, assembled_.data(),
                     static_cast<int>(fullBytes), 0);
        }
        frames_.publish(assembled_.data(), fullBytes);
        gate_.noteForwarded(static_cast<uint64_t>(NCH) * 2);
    }
    return true;
}

// ---------------------------------------------------------------------------
// acq thread: poll all interfaces -> pair -> assemble -> gate -> hand off
// ---------------------------------------------------------------------------
void Pipeline::acqLoop() {
    acqStartMs_ = nowMs();

    // 1 Hz rate bookkeeping
    auto lastRateTime = steady_clock::now();
    uint64_t prevIn = 0, prevPass = 0, prevInB = 0, prevOutB = 0;

    // poll set: one entry per SUB socket and per monitor socket
    std::vector<zmq_pollitem_t> items;
    for (size_t i = 0; i < zmqSubs_.size(); i++) {
        items.push_back(zmq_pollitem_t{zmqSubs_[i], 0, ZMQ_POLLIN, 0});
        // A null socket with fd 0 would poll STDIN: use fd -1 (ignored) when
        // a socket monitor could not be created.
        items.push_back(zmqMons_[i]
                            ? zmq_pollitem_t{zmqMons_[i], 0, ZMQ_POLLIN, 0}
                            : zmq_pollitem_t{nullptr, -1, 0, 0});
    }
    const int nSub = static_cast<int>(zmqSubs_.size());

    while (running_.load()) {
        {
            auto now = steady_clock::now();
            const double dt = duration<double>(now - lastRateTime).count();
            if (dt >= 1.0) {
                GateEngine::Counters s = gate_.snapshot();
                gate_.r_.rateIn.store(s.framesIn > prevIn
                                          ? (s.framesIn - prevIn) / dt
                                          : 0.0);
                gate_.r_.ratePass.store(s.framesPass > prevPass
                                            ? (s.framesPass - prevPass) / dt
                                            : 0.0);
                const uint64_t inB = gate_.c_.bytesIn.load();
                const uint64_t outB = gate_.c_.bytesOut.load();
                gate_.r_.rateInMBs.store(
                    inB > prevInB ? (inB - prevInB) / dt / 1e6 : 0.0);
                gate_.r_.ratePassMBs.store(
                    outB > prevOutB ? (outB - prevOutB) / dt / 1e6 : 0.0);
                prevIn = s.framesIn;
                prevPass = s.framesPass;
                prevInB = inB;
                prevOutB = outB;
                lastRateTime = now;
            }
        }

        // A held slab means "wait for the partner": stop asking about that
        // socket, or the poll reports it readable forever and the acq thread
        // busy-spins at 100 % while it waits.
        for (int i = 0; i < nSub; i++)
            items[2 * i].events = slots_[i].valid ? 0 : ZMQ_POLLIN;

        const int rc = zmq_poll(items.data(), static_cast<int>(items.size()),
                                200 /* ms: also lets us poll running_ */);
        if (rc < 0) {
            if (zmq_errno() == ETERM)
                break;
            continue;
        }
        if (rc == 0) { // idle: NOT "receiver stopped"
            refreshStreamState();
            if (multiPort())
                tryAssemble(false);
            continue;
        }

        for (int i = 0; i < nSub; i++) {
            if (items[2 * i + 1].revents & ZMQ_POLLIN)
                drainMonitor(i);
            if ((items[2 * i].revents & ZMQ_POLLIN) && !slots_[i].valid) {
                // Flow control: never overwrite a slab still waiting to be
                // paired.  The lagging interface catches up instead and the
                // leading interface's socket queue buffers its newer frames.
                readStream(i);
            }
        }
        refreshStreamState();

        if (multiPort()) {
            // Pair the slabs; only index-aligned frames are processed.  The
            // latency is bounded by the small receive queue (--rx-hwm): when
            // the stream is faster than this program the excess is discarded
            // by ZMQ, not copied and thrown away here.
            tryAssemble(false);
        } else {
            StreamSlot &sl = slots_[0];
            if (!sl.valid)
                continue;
            if (assembled_.size() != static_cast<size_t>(NCH))
                assembled_.assign(static_cast<size_t>(NCH), 0);
            const size_t fullBytes = static_cast<size_t>(NCH) * 2;
            const uint64_t bytes = sl.len * sizeof(short);
            const size_t copyBytes = std::min<size_t>(bytes, fullBytes);
            if (copyBytes)
                memcpy(assembled_.data(), sl.data.data(), copyBytes);
            if (copyBytes < fullBytes)
                memset(reinterpret_cast<char *>(assembled_.data()) + copyBytes,
                       0, fullBytes - copyBytes);
            sl.valid = false;

            gate_.noteReceived(bytes, false);

            if (pedeAcqActive_.load() &&
                (gate_.scFilter() < 0 || gate_.scFilter() == track_[0].sc.load()))
                acquirePede(assembled_.data());

            std::shared_ptr<const PedeSet> peds =
                gNoPede.load() ? nullptr : PedestalStore::instance().get();
            const int g1off = G1Poffset.load();
            const int g2off = G2Poffset.load();
            uint64_t above = 0;
            bool scDropped = false;
            const bool forward = gate_.evaluate(
                assembled_.data(), static_cast<uint64_t>(NCH),
                track_[0].sc.load(), peds.get(), g1off, g2off, above,
                &scDropped);
            if (!forward && !scDropped && gPedeTracking.load() &&
                gate_.enabled()) {
                const GateEngine::Counters gc = gate_.snapshot();
                int rx0 = 0, ry0 = 0, rx1 = 1024, ry1 = 512;
                if (gate_.roiSet())
                    gate_.roiSnapshot(rx0, ry0, rx1, ry1);
                pedeTrackerFeed(
                    assembled_.data(), static_cast<uint64_t>(NCH), peds.get(),
                    pedeTrackSignalDropRate(gc.framesPassCur, gc.framesDropCur,
                                            gc.framesPass, gc.framesDrop),
                    rx0, ry0, rx1, ry1);
            }

            if (forward) {
                if (saver_.running())
                    offerToSaver(assembled_.data(), fullW_.load(),
                                 fullH_.load(), sl.hdr);
                if (zmqPub_) { // verbatim receiver header + payload
                    if (hdrLen_ > 0)
                        zmq_send(zmqPub_, hdrBytes_.data(), hdrLen_,
                                 ZMQ_SNDMORE);
                    zmq_send(zmqPub_, assembled_.data(),
                             static_cast<int>(std::min<size_t>(bytes,
                                                               fullBytes)),
                             0);
                }
                frames_.publish(assembled_.data(), fullBytes);
                gate_.noteForwarded(static_cast<uint64_t>(NCH) * 2);
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Analysis worker pool
// ---------------------------------------------------------------------------
// Steps 1 and 2 are pure per-pixel work on disjoint ranges, and since the
// ROOT histograms are only touched by the deferred sync they are free of ROOT
// state: they can run on several threads with no locking at all.  adc_image,
// gain_image, g_pmapAccum and hchptr are written per pixel (disjoint), so only
// the chip spectra - a scatter into 8x502 counters - need per-worker copies
// that the controller merges once per frame (a few thousand adds).
//
// The worker threads are persistent; a frame is handed over with a generation
// counter, so the per-frame cost is two condvar handshakes (~us).
namespace {

struct FrameJob {
    const short *img = nullptr;
    const float *g0 = nullptr;
    const float *g1 = nullptr;
    const float *g2 = nullptr;
    bool hasAll = false;
    bool useGain = false;
    bool doPhotonMap = false;
    int G1 = 0, G2 = 0;
    int phene = 0;
    float pthr = 0.0f;
    // gate ROI of this frame (DATA coordinates, half-open, row 0 at the
    // bottom): drives the ROI spectra.  roiOn == false -> no ROI pixel, the
    // ROI spectra stay empty for this frame.
    bool pmapSave = false; // also accumulate into the save map
    bool roiOn = false;
    int roiX0 = 0, roiY0 = 0, roiX1 = 1024, roiY1 = 512;
    // photon counting value: adcpedecorr, or adcpedecorr / (ADU per keV) when
    // the gain correction is on (the threshold then means keV).  The map is
    // used when loaded, the nominal 40 ADU/keV otherwise.
    const double *pcGain = nullptr;
    double pcNominalGain = 0.0;
    // calibration (g0 of the gain map, ADU/keV); nullptr -> no keV spectra
    const double *gain = nullptr;
    // ADU/keV to use when no map is loaded but the keV view is on (nominal
    // 40 ADU/keV); 0 -> do not accumulate the keV set at all
    double nominalGain = 0.0;
};

std::mutex g_poolMtx;
std::condition_variable g_poolWorkCv;
std::condition_variable g_poolDoneCv;
FrameJob g_job;
uint64_t g_poolGen = 0;
int g_poolPending = 0;
bool g_poolStop = false;
int g_nWorkers = 1;
std::vector<std::thread> g_workerThreads;
std::atomic<bool> g_workerError{false};
struct ChipCounts {
    uint32_t c[8][kChipOverflowBins];
    uint32_t k[8][kChipOverflowBins]; // same, in keV
};
struct RoiCounts {
    uint32_t c[kChipOverflowBins];
    uint32_t k[kChipOverflowBins];
};
std::vector<ChipCounts> g_workerChip;
std::vector<RoiCounts> g_workerRoi; // one spectrum over the gate-ROI pixels
std::vector<double> g_workerFills;

// processes pixel range [i0,i1) of the current job (steps 1 and 2)
void analysisRange(int widx, int i0, int i1, const FrameJob &job) {
    const short *img = job.img;
    const float *g0 = job.g0, *g1 = job.g1, *g2 = job.g2;
    uint32_t(*chip)[kChipOverflowBins] = g_workerChip[widx].c;
    uint32_t(*kevChip)[kChipOverflowBins] = g_workerChip[widx].k;
    uint32_t *roi = g_workerRoi[widx].c;
    uint32_t *roiKev = g_workerRoi[widx].k;
    double fills = 0.0;

    // ---- step 1: ADC / gain images --------------------------------------
    for (int i = i0; i < i1; i++) {
        const int adcvalue = img[i] & 0x3fff;
        const int gain = (img[i] & 0xc000) ? ((img[i] >> 14) & 0x3) : 0;
        float disp = float(adcvalue) - (g0 ? g0[i] : 0.0f);
        if ((i % 256 == 0) || (i % 256 == 255))
            disp = int(disp / factor);
        if ((i / 1024 == 255) || (i / 1024 == 256) || (i / 1024 == 767) ||
            (i / 1024 == 768))
            disp = int(disp / factor);
        adc_image[i] = disp;
        gain_image[i] = gain;
    }

    // ---- step 2: photon counting + PH spectra + photon map ---------------
    for (int i = i0; i < i1; i++) {
        const int adcvalue = img[i] & 0x3fff;
        const int gain = (img[i] & 0xc000) ? ((img[i] >> 14) & 0x3) : 0;
        float adcpedecorr = 0.0f;

        // Pedestal-subtracted ADC, with the Jungfrau gain-bit conversion for
        // pixels flagged as high-gain (g1) or gain-3 (g2).  "Apply gain" does
        // not touch this: it is what divides the value by ADU/keV for the keV
        // spectra (see the kbin block), so the ADU side stays raw ADC.
        if (job.g0) {
            if (gain == 0 || !job.hasAll)
                adcpedecorr = (adcvalue & 0x3fff) - g0[i];
            if (gain == 1 && job.hasAll)
                adcpedecorr = (g1[i] + job.G1 - adcvalue) * 30.0;
            if (gain == 3 && job.hasAll)
                adcpedecorr = (g2[i] + job.G2 - adcvalue) * 340.0;
        } else {
            adcpedecorr = float(adcvalue);
        }
        // Photon counting: the threshold (and the multiplicity that follows
        // from it) is in ADU, or in keV while the gain correction is on - the
        // same ADU/keV the keV spectra use, so the two never disagree.
        float pcval = adcpedecorr;
        if (job.pcGain)
            pcval = adcpedecorr / static_cast<float>(job.pcGain[i]);
        else if (job.pcNominalGain > 0.0)
            pcval = static_cast<float>(adcpedecorr / job.pcNominalGain);
        if (pcval > job.pthr && job.g0) {
            int inc = 1;
            if (job.phene > 0)
                inc = (int)((pcval + job.pthr) /
                            static_cast<float>(job.phene));
            hchptr[i] = hchptr[i] + inc;
            if (job.doPhotonMap) {
                g_pmapAccum[i] += static_cast<float>(inc);
                fills += 1.0;
                if (job.pmapSave && i < g_pmapSaveAccum.size())
                    g_pmapSaveAccum[i] += static_cast<float>(inc);
            }
        }

        // Chip spectra, with the same 20-column / 20-row border exclusion the
        // original display used.  i is the linear data index, so i%1024 is the
        // column and i/1024 the DATA row (row 0 = bottom, see README.md).
        const int px = i % 1024;
        const int py = i / 1024;
        if (px < 1004 && px > 20 && py > 20) {
            const int ichip = i / (256 * 256 * 4) * 4 + ((i / 256) % 4);
            int schip = ichip;
#ifdef JFSTRX
            schip = 0;
            if (ichip == 1) {
                if (i <= (1024 * (64 - 2)))
                    schip = 0;
                if (i > (1024 * (64 + 6)) && i <= (1024 * (64 * 2 - 4)))
                    schip = 1;
                if (i > (1024 * (64 * 2 + 5)))
                    schip = 2;
            }
            if (ichip == 6) {
                if (i <= (1024 * (256 + 2 * 64 - 5)))
                    schip = 7;
                if (i > (1024 * (256 + 2 * 64 + 4)) &&
                    i <= (1024 * (256 + 64 * 3 - 6)))
                    schip = 6;
                if (i > (1024 * (256 + 64 * 3 - 4)))
                    schip = 5;
            }
#endif
            const int bin = chipBin(adcpedecorr);
            chip[schip][bin]++;
            // calibration for the keV spectra: the same pixel/trim, but the
            // value divided by the pixel's ADU/keV.  The keV axes are the ADU
            // axes / KEV_NOMINAL_ADU with the same bin count, so multiplying
            // by the nominal constant maps keV onto the ADU bin index.
            int kbin = -1;
            const double gainVal = job.gain ? job.gain[i] : job.nominalGain;
            if (gainVal > 0.1) {
                const double kev = adcpedecorr / gainVal;
                kbin = chipBin(kev * KEV_NOMINAL_ADU);
                kevChip[schip][kbin]++;
            }
            // the ROI spectra are the same thing restricted to the gate ROI,
            // and keep the border trim so they are a strict subset of hchip
            if (job.roiOn && px >= job.roiX0 && px < job.roiX1 &&
                py >= job.roiY0 && py < job.roiY1) {
                roi[bin]++;
                if (kbin >= 0)
                    roiKev[kbin]++;
            }
        }
    }
    g_workerFills[widx] = fills;
}

void analysisWorkerBody(int widx) {
    uint64_t seen = 0;
    for (;;) {
        int i0 = 0, i1 = 0;
        FrameJob job;
        {
            std::unique_lock<std::mutex> lk(g_poolMtx);
            g_poolWorkCv.wait(lk, [&] {
                return g_poolGen != seen || g_poolStop;
            });
            if (g_poolStop)
                return;
            seen = g_poolGen;
            job = g_job;
            i0 = widx * (NCH / 1024 / g_nWorkers) * 1024;
            i1 = (widx + 1 == g_nWorkers)
                     ? NCH
                     : (widx + 1) * (NCH / 1024 / g_nWorkers) * 1024;
        }
        // An exception must never leave a std::thread's function: that calls
        // std::terminate and kills the whole program.  Report and carry on.
        try {
            analysisRange(widx, i0, i1, job);
        } catch (const std::exception &e) {
            if (!g_workerError.exchange(true))
                fprintf(stderr, "[analysis] worker %d exception: %s\n", widx,
                        e.what());
        } catch (...) {
            if (!g_workerError.exchange(true))
                fprintf(stderr, "[analysis] worker %d unknown exception\n",
                        widx);
        }
        {
            std::lock_guard<std::mutex> lk(g_poolMtx);
            if (--g_poolPending == 0)
                g_poolDoneCv.notify_one();
        }
    }
}

void startAnalysisWorkers(int n) {
    if (n < 1)
        n = 1;
    if (n > 32)
        n = 32;
    g_nWorkers = n;
    g_workerChip.resize(n);
    g_workerRoi.resize(n);
    for (int k = 0; k < n; k++) {
        memset(&g_workerChip[k], 0, sizeof(ChipCounts));
        memset(&g_workerRoi[k], 0, sizeof(RoiCounts));
    }
    g_workerFills.assign(n, 0.0);
    g_poolStop = false;
    g_poolGen = 0;
    for (int k = 1; k < n; k++) // worker 0 is the analysis thread itself
        g_workerThreads.emplace_back(analysisWorkerBody, k);
}

void stopAnalysisWorkers() {
    {
        std::lock_guard<std::mutex> lk(g_poolMtx);
        g_poolStop = true;
    }
    g_poolWorkCv.notify_all();
    for (auto &t : g_workerThreads) {
        if (t.joinable())
            t.join();
    }
    g_workerThreads.clear();
}

// Runs steps 1+2 for a whole frame: worker 0 is the analysis thread, the
// others are woken through the generation counter; the chip spectra of every
// worker are merged (and cleared) once the frame is done.
void runAnalysisFrame(const FrameJob &job) {
    if (g_nWorkers <= 1) {
        analysisRange(0, 0, NCH, job);
    } else {
        {
            std::lock_guard<std::mutex> lk(g_poolMtx);
            g_job = job;
            g_poolPending = g_nWorkers - 1;
            ++g_poolGen;
        }
        g_poolWorkCv.notify_all();
        const int rows = NCH / 1024;
        const int rowsPer = rows / g_nWorkers;
        analysisRange(0, 0, rowsPer * 1024, job);
        {
            std::unique_lock<std::mutex> lk(g_poolMtx);
            g_poolDoneCv.wait(lk, [] { return g_poolPending == 0; });
        }
    }
    for (int k = 0; k < g_nWorkers; k++) {
        auto *w = g_workerChip[k].c;
        auto *wk = g_workerChip[k].k;
        auto *r = g_workerRoi[k].c;
        auto *rk = g_workerRoi[k].k;
        for (int ic = 0; ic < 8; ic++) {
            for (int b = 0; b < kChipOverflowBins; b++) {
                const uint32_t v = w[ic][b];
                if (v) {
                    g_chipAccum[ic][b] += v;
                    w[ic][b] = 0;
                }
                const uint32_t vk = wk[ic][b];
                if (vk) {
                    g_kevAccum[ic][b] += vk;
                    wk[ic][b] = 0;
                }
            }
        }
        for (int b = 0; b < kChipOverflowBins; b++) {
            const uint32_t vr = r[b];
            if (vr) {
                g_roiAccum[b] += vr;
                r[b] = 0;
            }
            const uint32_t vrk = rk[b];
            if (vrk) {
                g_roiKevAccum[b] += vrk;
                rk[b] = 0;
            }
        }
        g_pmapFills += g_workerFills[k];
        g_workerFills[k] = 0.0;
    }
}

} // namespace

// ---------------------------------------------------------------------------
// analysis thread: pedestals -> images -> ROOT fills
// ---------------------------------------------------------------------------
void Pipeline::analysisLoop() {
    std::vector<short> in;
    size_t inLen = 0;
    uint64_t inSeq = 0;
    g_timing = (getenv("JF_TIMING") != nullptr);
    const auto tStart = std::chrono::steady_clock::now();

    while (running_.load()) {
      try { // an escaping exception would abort the process
        if (g_workerError.exchange(false))
            gate_.setLastError("analysis worker failed - see stderr");
        {
            TT tt(g_timing ? &tTake : nullptr);
            if (!frames_.waitTake(in, inLen, inSeq, running_))
                continue;
        }
        if (!running_.load())
            break;

        if (gFrameCountReset.exchange(false))
            framesinstream.store(0);

        {
            TT tt(g_timing ? &tCopy : nullptr);
            const size_t copyBytes =
                std::min(inLen * sizeof(short), sizeof(image_data));
            if (copyBytes)
                memcpy(image_data, in.data(), copyBytes);
        }
        tFrames += g_timing ? 1 : 0;

        if (goout.load() == 0)
            continue; // paused: no counters, no fills, no ROOT redraw

        const std::shared_ptr<const PedeSet> pedsSp =
            gNoPede.load() ? nullptr : PedestalStore::instance().get();
        const PedeSet *peds = pedsSp.get();
        const bool havePede = (peds != nullptr);
        const bool hasAll = havePede && peds->hasAll;
        const float *g0 = havePede ? peds->g0.data() : nullptr;
        const float *g1 = hasAll ? peds->g1.data() : nullptr;
        const float *g2 = hasAll ? peds->g2.data() : nullptr;
        const int G1 = G1Poffset.load();
        const int G2 = G2Poffset.load();
        const bool useGain = gain_flag.load();

        const uint64_t fsn = framesinstream.fetch_add(1) + 1;
        int hde = HDraw_every.load();
        if (hde < 1)
            hde = 1;
        // "every N analysed frames" (the Faster/Slower knob), plus a floor so a
        // slowly gated stream still shows every frame: display anyway when the
        // previous image is older than DISPLAY_FLOOR_MS
        const int64_t dispNow = nowMs();
        bool show2Ds = ((fsn % static_cast<uint64_t>(hde)) ==
                        static_cast<uint64_t>(hde - 1));
        if (!show2Ds && dispNow - g_lastShownMs >= DISPLAY_FLOOR_MS)
            show2Ds = true;
        if (show2Ds) {
            g_lastShownMs = dispNow;
            gImagesShown.fetch_add(1);
        }

        processedf++;
        gFramesForRoot.fetch_add(1);

        // Every mutation of the source histograms below (including the
        // windowed scatter reset) happens under gHistoMtx.  The ROOT thread's
        // "Reset His" takes the same lock, so a Reset() can never race the
        // per-frame Fill().  The lock is released at the end of this
        // iteration (copyBins() on the ROOT side is intentionally lock-free).
        std::lock_guard<std::mutex> histoLock(gHistoMtx);
        ensureAccumulators(); // allocation + zeroing under the same lock

        // open a fresh accumulation window for the scatter plots after each
        // ROOT redraw (the photon map itself stays cumulative)
        if (gSoftResetPending.exchange(false)) {
            his4500->Reset();
            hisLvsR->Reset();
        }
        // "Reset His" requested by the GUI: the analysis thread performs it
        // here, where the lock is already held, so the ROOT thread never has to
        // wait for gHistoMtx (which it can starve under a heavy stream)
        if (gResetHisPending.exchange(false)) {
            historesetSourcesLocked();
            forceRootRedraw(); // repaint the cleared histograms + wake the ROOT thread
        }

        // ---- 1+2) per-frame images, photon counting, spectra, photon map --
        // (pure per-pixel work over disjoint ranges - see runAnalysisFrame)
        {
            TT tt(g_timing ? &tStep1 : nullptr);
            FrameJob job;
            job.img = image_data;
            job.g0 = g0;
            job.g1 = g1;
            job.g2 = g2;
            job.hasAll = hasAll;
            job.useGain = useGain;
            job.doPhotonMap = dophotonmap.load();
            // only while a save is actually running: the save map is pure
            // overhead otherwise
            job.pmapSave = (gSaveMode.load() == 1 && saver_.running());
            job.G1 = G1;
            job.G2 = G2;
            // photon peak ~= 2 x the ADU threshold (the historic relation
            // between the <energy> argument and its threshold)
            // ADU normally, keV while the gain correction is on (the GUI keeps
            // one spinbox per unit and swaps them with the gain button)
            const bool gainStage = gain_flag.load();
            const float pthr = gainStage ? g_pheneThresholdKev
                                         : g_pheneThreshold;
            job.phene = static_cast<int>(2.0f * pthr + 0.5f);
            job.pthr = pthr;
            // calibration snapshot (shared_ptr keeps it alive for this frame)
            const std::shared_ptr<const GainMap> gainSp =
                GainMapStore::instance().get();
            // keV spectra exist only while the gain stage is active: with the
            // gain off the display is ADU, so nothing is converted (and the
            // per-pixel divide is skipped)
            const bool kevOn = kevSpectra.load() && gain_flag.load();
            job.gain = (gainSp && kevOn) ? gainSp->g0() : nullptr;
            // no map but the operator asked for keV: use the nominal
            // 40 ADU/keV ("no gain map selected - using nominal values")
            job.nominalGain = (!gainSp && kevOn) ? KEV_NOMINAL_ADU : 0.0;
            // the photon counting follows the gain stage itself, not the keV PH
            // checkbox (that one only decides what the spectra display)
            job.pcGain = (gainSp && gainStage) ? gainSp->g0() : nullptr;
            job.pcNominalGain = (!gainSp && gainStage) ? KEV_NOMINAL_ADU : 0.0;
            // The gate ROI is read ONCE per frame and published for everything
            // that shows it (ADC overlay is set by the GUI from the same
            // snapshot, ROOT box + ROI spectra from this job), so image and
            // ROOT can never disagree about where the ROI is.  The rectangle
            // is in DATA rows (row 0 = bottom); a torn/degenerate snapshot
            // falls back to "no ROI".
            {
                int rx0 = 0, ry0 = 0, rx1 = kRawWidth, ry1 = kRawHeight;
                // A ROI that exists is enough for the ROI spectra and the box:
                // "ROI only" controls whether the GATE is restricted to it, not
                // whether the ROI is shown.  (roiActive() is only true while
                // "ROI only" is on, which left A7 empty for a plain selection.)
                bool on = gate_.roiSet();
                if (on)
                    gate_.roiSnapshot(rx0, ry0, rx1, ry1);
                if (rx1 <= rx0 || ry1 <= ry0) {
                    on = false;
                    rx0 = 0;
                    ry0 = 0;
                    rx1 = kRawWidth;
                    ry1 = kRawHeight;
                }
                job.roiOn = on;
                job.roiX0 = rx0;
                job.roiY0 = ry0;
                job.roiX1 = rx1;
                job.roiY1 = ry1;
                ROIxmin.store(static_cast<uint16_t>(rx0));
                ROIxmax.store(static_cast<uint16_t>(rx1));
                ROIymin.store(static_cast<uint16_t>(ry0));
                ROIymax.store(static_cast<uint16_t>(ry1));
                ROIactive.store(on);
            }
            runAnalysisFrame(job);
        }

        // ---- 3) T vs B / L vs R from the current frame -------------------
        {
            TT tt(g_timing ? &tStep3 : nullptr);
            for (int ipx = 0; ipx < 1024; ipx++) {
                his4500->Fill(adc_image[ipx + (512 - 21) * 1024],
                              adc_image[ipx + 20 * 1024]);
            }
            for (int ipy = 0; ipy < 512; ipy++) {
                hisLvsR->Fill(adc_image[20 + ipy * 1024],
                              adc_image[1024 - 21 + ipy * 1024]);
            }
        }

        // ---- 4) raw-ADC map accumulation on the display cadence ----------
        {
            TT tt(g_timing ? &tStep4 : nullptr);
            if (!dophotonmap.load() && show2Ds) {
                for (int ipx = 0; ipx < 1024; ipx++) {
                    for (int ipy = 0; ipy < 512; ipy++) {
                        g_pmapAccum[ipx + ipy * 1024] +=
                            adc_image[ipx + ipy * 1024];
                        g_pmapFills += 1.0;
                    }
                }
            }
        }

        // ---- 4b) push the accumulators into the ROOT histograms ----------
        // (at the redraw cadence - the canvases are painted once per
        // rootUpdateMs, so nothing is lost by not doing this per frame)
        {
            TT tt(g_timing ? &tSync : nullptr);
            syncAccumulators(nowMs());
        }

#ifdef JFSTRX
        if (show2Ds) {
            jungfrauLGADStrixelsData *decoder = new jungfrauLGADStrixelsData();
            const int g1_nrows = 165, g2_nrows = 320, g3_nrows = 476;
            // Six-panel output, index 0..2 = chip1 g1/g2/g3, 3..5 = chip6
            // g1/g2/g3, and the mapped-image row at which each panel starts.
            static const int panelStartY[kNumAdcPanels] = {0,   165, 485,
                                                           1757, 1437, 961};
            for (int p = 0; p < kNumAdcPanels; p++) {
                const size_t n = static_cast<size_t>(kAdcPanelW[p]) *
                                 static_cast<size_t>(kAdcPanelH[p]);
                if (g_adcPanel[p].size() != n)
                    g_adcPanel[p].assign(n, 0.0f);
            }
            for (int ipx = 0; ipx < nx; ipx++) {
                for (int ipy = 0; ipy < ny; ipy++) {
                    const int i =
                        int(((decoder->dataMap[ipy][ipx]) - sizeof(header)) / 2);
                    const int group = decoder->groupmap[ipy][ipx];
                    const float value =
                        static_cast<float>(image_data[i] & 0x3fff) -
                        (g0 ? g0[i] : 0.0f);
                    static const int mc1g1y_start = 0;
                    static const int mc1g2y_start = g1_nrows;
                    static const int mc1g3y_start = g1_nrows + g2_nrows;
                    static const int mc6g3y_start =
                        g1_nrows + g2_nrows + g3_nrows;
                    static const int mc6g2y_start = mc6g3y_start + g3_nrows;
                    static const int mc6g1y_start =
                        mc6g3y_start + g3_nrows + g2_nrows;
                    if (group == 0) {
                        if (ipy < 165)
                            his1000->Fill(float(ipx),
                                          float(ipy - mc1g1y_start), value);
                        if (ipy >= mc6g1y_start)
                            his1060->Fill(float(ipx),
                                          float(ipy - mc6g1y_start), value);
                    }
                    if (group == 1) {
                        if (ipy < (165 + 320))
                            his1001->Fill(float(ipx),
                                          float(ipy - mc1g2y_start), value);
                        if (ipy >= mc6g2y_start && ipy < mc6g1y_start) {
                            his1061->Fill(float(ipx),
                                          float(ipy - mc6g2y_start), value);
                        }
                    }
                    if (group == 2) {
                        if (ipy < (165 + 320 + 476))
                            his1002->Fill(float(ipx),
                                          float(ipy - mc1g3y_start), value);
                        if (ipy >= mc6g3y_start && ipy < mc6g2y_start)
                            his1062->Fill(float(ipx),
                                          float(ipy - mc6g3y_start), value);
                    }
                    // Feed the separate six-panel window with the mapped
                    // groups.  The Qt ADC/gain images stay raw 1024x512.
                    if (group >= 0 && group <= 2) {
                        const int base = (ipy >= mc6g3y_start) ? 3 : 0;
                        const int p = base + group;
                        const int lx = ipx;
                        const int ly = ipy - panelStartY[p];
                        if (lx >= 0 && lx < kAdcPanelW[p] && ly >= 0 &&
                            ly < kAdcPanelH[p]) {
                            g_adcPanel[p][static_cast<size_t>(lx) +
                                          static_cast<size_t>(ly) *
                                              kAdcPanelW[p]] = value;
                        }
                    }
                }
            }
            delete decoder;
        }
#endif

        // ---- 5) hand the displayed images to the Qt timer ----------------
        {
            TT tt(g_timing ? &tStep5 : nullptr);
            if (show2Ds) {
                std::lock_guard<std::mutex> lock(images_.mtx);
                // The ADC/gain windows are ALWAYS the raw 1024x512 layout, so
                // the gate-ROI selection stays in raw data coordinates even
                // under JFSTRX.
                const size_t nPix = static_cast<size_t>(NCH);
                if (images_.adc.size() != nPix) {
                    images_.adc.assign(nPix, 0.0f);
                    images_.gain.assign(nPix, 0.0f);
                }
                memcpy(images_.adc.data(), adc_image, nPix * sizeof(float));
                memcpy(images_.gain.data(), gain_image, nPix * sizeof(float));
#ifdef JFSTRX
                // the mapped groups for the separate six-panel window
                for (int p = 0; p < kNumAdcPanels; p++) {
                    const size_t n = g_adcPanel[p].size();
                    if (images_.panels[p].size() != n)
                        images_.panels[p].resize(n);
                    if (n)
                        memcpy(images_.panels[p].data(), g_adcPanel[p].data(),
                               n * sizeof(float));
                }
                images_.panelsReady = true;
#endif
                images_.ready = true;
                images_.seq = inSeq;
            }
        }

        // ---- 5a) the map that will be saved ------------------------------
        // Filled in the same worker pass (job.pmapSave), published every
        // gPmapSaveEvery analysed frames, never cumulative.
        // NOTE: gHistoMtx is already held here (histoLock, top of the frame):
        // taking it again deadlocked the analysis thread on the first frame.
        if (gSaveMode.load() == 1 && saver_.running()) {
            if (g_pmapSaveAccum.size() != static_cast<size_t>(NCH))
                g_pmapSaveAccum.assign(static_cast<size_t>(NCH), 0.0f);
            if (++g_pmapSaveCount >=
                static_cast<uint64_t>(std::max(1, gPmapSaveEvery.load()))) {
                PmapSlot &slot = pmapSave();
                {
                    std::lock_guard<std::mutex> sl(slot.mtx);
                    if (slot.map.size() != static_cast<size_t>(NCH))
                        slot.map.assign(static_cast<size_t>(NCH), 0.0f);
                    memcpy(slot.map.data(), g_pmapSaveAccum.data(),
                           static_cast<size_t>(NCH) * sizeof(float));
                    slot.frames = g_pmapSaveCount;
                    slot.seq++;
                    slot.ready = true;
                }
                std::fill(g_pmapSaveAccum.begin(), g_pmapSaveAccum.end(), 0.0f);
                g_pmapSaveCount = 0;
            }
        }

        // ---- 5b) the photon map, on its own cadence ----------------------
        // Every gPmapEvery analysed frames the window gets the map: the photons
        // counted since the previous publish when it is not cumulative (the
        // default), or the running total when it is.  Independent of the ADC
        // image decimation above.
        {
            const int every = std::max(1, gPmapEvery.load());
            if (static_cast<uint64_t>(every) > 0 &&
                (fsn % static_cast<uint64_t>(every)) == 0 &&
                !g_pmapAccum.empty()) {
                std::lock_guard<std::mutex> lock(images_.mtx);
                if (images_.photon.size() != static_cast<size_t>(NCH))
                    images_.photon.assign(static_cast<size_t>(NCH), 0.0f);
                memcpy(images_.photon.data(), g_pmapAccum.data(),
                       static_cast<size_t>(NCH) * sizeof(float));
                images_.photonReady = true;
                images_.photonSeq = fsn;
                if (!gPmapCumulative.load()) { // fresh image every time
                    std::fill(g_pmapAccum.begin(), g_pmapAccum.end(), 0.0f);
                    g_pmapFills = 0;
                    g_pmapFillsSynced = 0;
                }
            }
        }
      } catch (const std::exception &e) {
        fprintf(stderr, "[analysis] exception: %s (analysis stopped)\n",
                e.what());
        fflush(stderr);
        gate_.setLastError(std::string("analysis: ") + e.what());
        break;
      } catch (...) {
        fprintf(stderr, "[analysis] unknown exception (analysis stopped)\n");
        fflush(stderr);
        gate_.setLastError("analysis: unknown exception");
        break;
      }
    }

    // final flush so the histograms match the accumulators when we stop
    {
        std::lock_guard<std::mutex> lock(gHistoMtx);
        syncAccumulators(nowMs(), true);
    }

    if (g_timing && tFrames > 0) {
        const double n = static_cast<double>(tFrames);
        const double wall = std::chrono::duration<double>(
                                std::chrono::steady_clock::now() - tStart)
                                .count();
        printf("[timing] frames=%llu wall=%.1fs implied_rate=%.0ffps\n",
               static_cast<unsigned long long>(tFrames), wall,
               tFrames / (wall > 0 ? wall : 1));
        printf("[timing] per-frame us: take=%.0f copy=%.0f pixelwork=%.0f "
               "step3_scatter=%.0f step4_rawmap=%.0f sync=%.0f "
               "step5_publish=%.0f | sum=%.0f\n",
               tTake / n, tCopy / n, tStep1 / n, tStep3 / n, tStep4 / n,
               tSync / n, tStep5 / n,
               (tTake + tCopy + tStep1 + tStep3 + tStep4 + tSync + tStep5) / n);
        fflush(stdout);
    }
}

// ---------------------------------------------------------------------------
// "Reset His": reset the source accumulators.  The analysis thread is the
// only writer of the source histograms, so a reset from the ROOT thread must
// take gHistoMtx (the analysis thread holds it around every frame's fills).
// ---------------------------------------------------------------------------
void historesetSources() {
    std::lock_guard<std::mutex> lock(gHistoMtx);
    historesetSourcesLocked();
}

// gHistoMtx already held (the analysis thread applies a pending "Reset His"
// here, at the top of a frame, so the ROOT thread never waits for the lock).
void historesetSourcesLocked() {
    resetAccumulators(); // must match the Reset() below
    if (his4500)
        his4500->Reset();
    if (hisLvsR)
        hisLvsR->Reset();
    if (his3000)
        his3000->Reset();
    if (hchip) {
        for (int k = 0; k < 8; k++) {
            if (hchip[k])
                hchip[k]->Reset();
        }
    }
    if (hroi)
        hroi->Reset();
    if (hchipKev) {
        for (int k = 0; k < 8; k++) {
            if (hchipKev[k])
                hchipKev[k]->Reset();
        }
    }
    if (hroiKev)
        hroiKev->Reset();
#ifdef JFSTRX
    if (his1000)
        his1000->Reset();
    if (his1001)
        his1001->Reset();
    if (his1002)
        his1002->Reset();
    if (his1060)
        his1060->Reset();
    if (his1061)
        his1061->Reset();
    if (his1062)
        his1062->Reset();
#endif
}
