// SPDX-License-Identifier: LGPL-3.0-or-other
// ===========================================================================
// online_jf_h5.cpp -- decimated gate-passed ROI frames to HDF5
//
// The file layout mirrors slsReceiverSoftware/src/HDF5DataFile.cpp so that
// existing readers of receiver files work unchanged:
//
//   <dir>/<prefix>_f0_0.h5
//     attribute "version" = HDF5 writer version (double)
//     /data                        rank 3, U16LE, dims {nframes, h, w},
//                                  max {UNLIMITED, h, w}, chunk {1, h, w}
//     /frame number                rank 1, U64,  one value per frame
//     /exp length or sub exposure time      U32
//     /packets caught              U32
//     /detector specific 1         U64
//     /timestamp                   U64
//     /mod id                      U16
//     /row                         U16
//     /column                      U16
//     /detector specific 2         U16
//     /detector specific 3         U32
//     /detector specific 4         U16
//     /detector type               U8
//     /detector header version     U8
//     /packets caught bit mask     fixed size string (all zero here)
//
// Differences from a receiver file, by design:
//   * /data holds the ROI crop of the module, not the whole module;
//   * only frames that passed the gate are written, but each keeps its
//     ORIGINAL frame number in /frame number (so it is obvious which
//     acquisition frame each slice came from);
//   * raw mode (default): slice 0 of every dataset is the PEDESTAL in the
//     same ROI (frame number 0, marked by "pedestal_in_frame_0");
//   * corrected mode (option): /data holds pedestal-corrected float32 values
//     and has NO pedestal slice; the pedestal image lives in the separate
//     "/pedestal data" dataset instead (that image is the pedestal at save
//     start; each frame is corrected with the pedestal in use when it was
//     captured, so live drift tracking is followed like the display, and the
//     "/pedestal tracking" series is the authoritative history);
//   * /data slices are the raw 16-bit words as received (ADC in bits 0-13,
//     gain in bits 14-15), exactly like the receiver; slice 0 is the gain-0
//     pedestal in ADC units;
//   * a few extra root attributes describe the ROI and the gate.
//
// A bounded queue decouples the acq thread (which must never block on I/O)
// from the writer thread; frames that do not fit are counted as dropped and
// reported in the JSON sidecar and in the GUI.
// ===========================================================================
#include "online_jf.h"

#include "H5Cpp.h"
#include "sls/sls_detector_defs.h"

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstdio>
#include <cstring>
#include <ctime>
#include <deque>
#include <memory>
#include <stdexcept>

namespace {

// same value the receiver writes into the "version" attribute
const double kHdf5WriterVersion = 7.0;
// queue budget between the acq thread and the writer thread
constexpr size_t kQueueBudgetBytes = 256u * 1024u * 1024u;
constexpr size_t kQueueMaxItems = 4096;
// frames added per dataset extension (metadata operations are expensive)
constexpr hsize_t kExtendBlock = 128;

bool makeDir(const std::string &dir) {
    if (dir.empty() || dir == ".")
        return true;
    struct stat st;
    if (stat(dir.c_str(), &st) == 0)
        return S_ISDIR(st.st_mode);
    return mkdir(dir.c_str(), 0777) == 0;
}

std::string timestampName() {
    const std::time_t t = std::time(nullptr);
    struct tm tmv;
    localtime_r(&t, &tmv);
    char buf[32];
    strftime(buf, sizeof(buf), "%Y%m%d_%H%M%S", &tmv);
    return std::string(buf);
}

std::string jsonEscape(const std::string &s) {
    std::string out;
    for (char c : s) {
        switch (c) {
        case '"':
            out += "\\\"";
            break;
        case '\\':
            out += "\\\\";
            break;
        case '\n':
            out += "\\n";
            break;
        case '\r':
            out += "\\r";
            break;
        case '\t':
            out += "\\t";
            break;
        default:
            if (static_cast<unsigned char>(c) < 0x20) {
                char b[8];
                snprintf(b, sizeof(b), "\\u%04x", c);
                out += b;
            } else {
                out += c;
            }
        }
    }
    return out;
}

std::string isoNow() {
    const std::time_t t = std::time(nullptr);
    struct tm tmv;
    localtime_r(&t, &tmv);
    char buf[32];
    strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S", &tmv);
    return std::string(buf);
}

// fsync the directory holding a file so a rename() survives a crash.  Best
// effort: a failure just means the rename may not be durable, never fatal.
void syncDirOf(const std::string &path) {
    const size_t slash = path.find_last_of('/');
    const std::string dir =
        (slash == std::string::npos) ? std::string(".") : path.substr(0, slash);
    const int dfd = ::open(dir.c_str(), O_RDONLY | O_DIRECTORY);
    if (dfd >= 0) {
        ::fsync(dfd);
        ::close(dfd);
    }
}

} // namespace

// ---------------------------------------------------------------------------
// RoiSaver
// ---------------------------------------------------------------------------
struct RoiSaver::Impl {
    // ---- configuration of the running save -------------------------------
    StartInfo info;
    std::string h5Path, jsonPath;
    size_t roiPixels = 0; // w*h

    // ---- queue (acq thread -> writer thread) -----------------------------
    struct Item {
        std::vector<float> pixels; // roiPixels: raw words or corrected values
        SavedFrameHeader hdr;
    };
    // one virtual-photon-counting map on its way to the file
    struct PmapItem {
        std::vector<float> map; // roiPixels, photons per pixel
        uint64_t frames = 0;    // analysed frames counted into it
        uint64_t firstFrame = 0, firstStamp = 0; // provenance of the window
    };
    std::mutex qMtx;
    std::condition_variable qCv;
    std::deque<Item> queue;
    std::vector<Item> freeList; // recycled buffers
    size_t queuedBytes = 0;

    // ---- writer ----------------------------------------------------------
    std::thread thread;
    std::atomic<bool> stopRequested{false};
    uint64_t pmapPending = 0; // photon maps offered
    std::atomic<uint64_t> pmapsInFile{0}; // maps actually written
    std::deque<PmapItem> pmapQueue;
    std::atomic<bool> running{false};
    std::atomic<uint64_t> saved{0};
    std::atomic<uint64_t> dropped{0};
    std::atomic<uint64_t> firstFrame{0};
    std::atomic<uint64_t> lastFrame{0};

    mutable std::mutex sMtx;
    std::string stopReason, lastError;

    void setStop(const std::string &r) {
        std::lock_guard<std::mutex> lk(sMtx);
        if (stopReason.empty())
            stopReason = r;
    }
    void setError(const std::string &e) {
        std::lock_guard<std::mutex> lk(sMtx);
        lastError = e;
    }

    void writerLoop();
};

RoiSaver::RoiSaver() : impl_(new Impl) {}

RoiSaver::~RoiSaver() {
    requestStop("destructor");
    if (impl_ && impl_->thread.joinable())
        impl_->thread.join();
}

bool RoiSaver::start(const StartInfo &info, std::string &error) {
    if (impl_->running.load()) {
        error = "a save is already running";
        return false;
    }
    // A save that ended on its own - frame budget reached, "user" stop, or an
    // I/O error - leaves a JOINABLE std::thread behind, because only the
    // destructor and finish() joined it.  Assigning a new std::thread to a
    // joinable one calls std::terminate, which is what killed the program the
    // second time a file was started ("terminate called without an active
    // exception").  running == false means the writer loop is done (it is the
    // last thing it does), so reap it here and the saver is restartable.
    if (impl_->thread.joinable())
        impl_->thread.join();
    if (info.w < 4 || info.h < 4 || info.w > 1024 || info.h > 512) {
        error = "invalid ROI size";
        return false;
    }
    if (!makeDir(info.dir)) {
        error = "cannot create directory '" + info.dir + "'";
        return false;
    }
    // receiver-style name: <base>_<timestamp>_f0_0.h5, with the timestamp so
    // re-using a base never overwrites an earlier run
    std::string base = info.baseName.empty() ? std::string("jfroi")
                                             : info.baseName;
    for (char &c : base) { // keep it a file name
        if (c == '/' || c == '\t' || c == '\n' || c == '\r')
            c = '_';
    }
    const std::string stem = info.dir + "/" + base + "_" + timestampName() +
                             "_f0_0";
    impl_->info = info;
    impl_->h5Path = stem + ".h5";
    impl_->jsonPath = stem + ".json";
    impl_->roiPixels = static_cast<size_t>(info.w) * static_cast<size_t>(info.h);
    impl_->queuedBytes = 0;
    {
        std::lock_guard<std::mutex> lk(impl_->qMtx);
        impl_->queue.clear();
        impl_->freeList.clear();
    }
    impl_->saved.store(0);
    impl_->dropped.store(0);
    impl_->firstFrame.store(0);
    impl_->lastFrame.store(0);
    impl_->stopRequested.store(false);
    {
        std::lock_guard<std::mutex> lk(impl_->sMtx);
        impl_->stopReason.clear();
        impl_->lastError.clear();
    }
    impl_->running.store(true);
    impl_->thread = std::thread(&RoiSaver::Impl::writerLoop, impl_.get());
    return true;
}

// Virtual photon counting: the map is queued like a frame and written by the
// writer thread.  Not reachable yet: gSaveMode is still 0 everywhere, the Save
// panel control and the dataset writer come next (see NOTES.md).
void RoiSaver::offerPhotonMap(const std::vector<float> &map, uint64_t frames,
                              uint64_t firstFrame, uint64_t firstStamp) {
    if (!impl_->running.load() || !impl_->info.photonMode || map.empty())
        return;
    Impl::PmapItem it;
    it.map = map;
    it.frames = frames;
    it.firstFrame = firstFrame;
    it.firstStamp = firstStamp;
    {
        std::lock_guard<std::mutex> lk(impl_->qMtx);
        impl_->pmapPending++;
        impl_->pmapQueue.push_back(std::move(it));
    }
    impl_->qCv.notify_one();
}

void RoiSaver::offer(const short *frame, int frameW, int frameH,
                     const SavedFrameHeader &hdr) {
    // nothing new is queued once a stop (user, budget, error) was requested:
    // otherwise a gate faster than the disk keeps the queue non-empty and the
    // writer, which drains before it exits, never stops
    if (!impl_->running.load() || !impl_->info.w ||
        impl_->stopRequested.load())
        return;
    Impl::Item item;
    {
        std::lock_guard<std::mutex> lk(impl_->qMtx);
        if (!impl_->freeList.empty()) {
            item = std::move(impl_->freeList.back());
            impl_->freeList.pop_back();
        } else {
            item.pixels.resize(impl_->roiPixels);
        }
        if (impl_->queue.size() >= kQueueMaxItems ||
            impl_->queuedBytes + impl_->roiPixels * sizeof(float) >
                kQueueBudgetBytes) {
            // keep the buffer for reuse and count the loss
            impl_->dropped.fetch_add(1);
            impl_->freeList.push_back(std::move(item));
            return;
        }
    }
    // copy the ROI out of the assembled frame (strided by the frame width),
    // either as the raw 16-bit word or pedestal-corrected exactly like the
    // display and the gate do it
    const int w = impl_->info.w, h = impl_->info.h;
    const int cx0 = std::max(0, impl_->info.x);
    const int cy0 = std::max(0, impl_->info.y);
    const int cx1 = std::min(frameW, impl_->info.x + w);
    const int cy1 = std::min(frameH, impl_->info.y + h);
    const int span = (cx1 > cx0) ? (cx1 - cx0) : 0;
    std::fill(item.pixels.begin(), item.pixels.end(), 0.0f);
    // Use the pedestal actually in force for THIS frame, exactly as the display
    // and the gate do.  The drift tracker (and a manual Read/Load/Acquire Pede)
    // swaps in a new immutable PedeSet while a save runs, so the snapshot the
    // GUI took at Start Save would silently keep correcting with a stale
    // pedestal for the whole night.  The shared_ptr keeps this frame's snapshot
    // alive for the copy below.  With "No pede" on, corrected mode falls back
    // to raw words in the float dataset, mirroring the display's raw path.
    std::shared_ptr<const PedeSet> livePeds;
    if (impl_->info.corrected)
        livePeds = gNoPede.load() ? nullptr : PedestalStore::instance().get();
    const PedeSet *peds = livePeds.get();
    const bool hasAll = peds && peds->hasAll;
    const float *g0 = peds ? peds->g0.data() : nullptr;
    const float *g1 = hasAll ? peds->g1.data() : nullptr;
    const float *g2 = hasAll ? peds->g2.data() : nullptr;
    const int G1 = impl_->info.g1off, G2 = impl_->info.g2off;

    for (int sy = cy0; sy < cy1; sy++) {
        const short *src = frame + static_cast<size_t>(sy) * frameW + cx0;
        float *dst = item.pixels.data() +
                     static_cast<size_t>(sy - impl_->info.y) * w +
                     (cx0 - impl_->info.x);
        if (!peds) { // raw 16-bit word (including the gain bits)
            for (int k = 0; k < span; k++)
                dst[k] = static_cast<float>(
                    static_cast<uint16_t>(src[k]));
            continue;
        }
        for (int k = 0; k < span; k++) {
            const size_t fi =
                static_cast<size_t>(sy) * frameW + cx0 + k; // full-frame index
            const int raw = src[k] & 0x3fff;
            const int g = (src[k] & 0xc000) ? ((src[k] >> 14) & 0x3) : 0;
            // pedestal-subtracted ADC with the gain-bit conversion, exactly as
            // the analysis loop computes it ("Apply gain" is a display choice,
            // not part of the saved values)
            float v;
            if (g == 1 && hasAll)
                v = (g1[fi] + G1 - raw) * 30.0f;
            else if (g == 3 && hasAll)
                v = (g2[fi] + G2 - raw) * 340.0f;
            else
                v = static_cast<float>(raw) - g0[fi];
            dst[k] = v;
        }
    }
    item.hdr = hdr;
    {
        std::lock_guard<std::mutex> lk(impl_->qMtx);
        if (impl_->stopRequested.load()) { // stop arrived during the copy
            impl_->freeList.push_back(std::move(item));
            return;
        }
        impl_->queuedBytes += item.pixels.size() * sizeof(float);
        impl_->queue.push_back(std::move(item));
    }
    impl_->qCv.notify_one();
}

void RoiSaver::requestStop(const std::string &reason) {
    impl_->setStop(reason);
    impl_->stopRequested.store(true);
    impl_->qCv.notify_all();
}

bool RoiSaver::running() const { return impl_->running.load(); }

RoiSaver::Status RoiSaver::status() const {
    Status s;
    s.running = impl_->running.load();
    s.saved = impl_->saved.load();
    s.dropped = impl_->dropped.load();
    s.pmaps = impl_->pmapsInFile.load();
    s.photonMode = impl_->info.photonMode;
    s.h5Path = impl_->h5Path;
    s.jsonPath = impl_->jsonPath;
    {
        std::lock_guard<std::mutex> lk(impl_->sMtx);
        s.stopReason = impl_->stopReason;
        s.lastError = impl_->lastError;
    }
    return s;
}

void RoiSaver::finish() {
    if (impl_->thread.joinable())
        impl_->thread.join();
    impl_->running.store(false);
}

// ---------------------------------------------------------------------------
// Writer thread: creates the file, writes slice 0 (pedestal) and every queued
// gate-passed frame, then the JSON sidecar.
// ---------------------------------------------------------------------------
void RoiSaver::Impl::writerLoop() {
    using namespace H5;
    const int w = info.w, h = info.h;
    const uint64_t maxFrames = info.maxFrames;
    const size_t bitsetBytes = sizeof(sls::defs::bitset_storage);

    std::unique_ptr<H5File> file;
    std::unique_ptr<DataSet> data;
    std::vector<std::unique_ptr<DataSet>> para;
    std::unique_ptr<DataSpace> dataSpace, paraSpace;
    std::vector<std::string> paraNames;
    hsize_t framesAllocated = 0;
    uint64_t framesWritten = 0;
    // in "virtual photon counting" the budget counts maps, not frames
    uint64_t pmapsWritten = 0;
    auto wallStart = std::chrono::steady_clock::now();
    const std::string startedAt = isoNow();

    // Rewrite the JSON sidecar in one atomic step: fill <name>.json.part,
    // fsync it, rename it over <name>.json.  A reader therefore always sees a
    // complete JSON (the previous one until the rename), and a crash during the
    // write leaves only a stale .part behind.  Called periodically and once at
    // the end; writer thread only.
    auto writeJsonSidecar = [&](bool complete) {
        // refresh the live statistics just before reporting them
        if (info.statsProvider) {
            try {
                info.statsProvider(info.gateAtEnd, info.stats, info.streams);
            } catch (...) {
            }
        }
        std::string stopR, errR;
        {
            std::lock_guard<std::mutex> lk(sMtx);
            stopR = stopReason;
            errR = lastError;
        }
        const double wall =
            std::chrono::duration<double>(std::chrono::steady_clock::now() -
                                          wallStart)
                .count();
        const std::string partPath = jsonPath + ".part";
        FILE *f = fopen(partPath.c_str(), "w");
        if (!f)
            return;
        const StartInfo &in = info;
        fprintf(f, "{\n");
        fprintf(f, "  \"complete\": %s,\n", complete ? "true" : "false");
        fprintf(f, "  \"hdf5_format\": \"v110 (SWMR)\",\n");
        fprintf(f, "  \"save_mode\": \"%s\",\n",
                in.photonMode ? "virtual photon counting" : "frames");
        fprintf(f, "  \"photon_map_every\": %d,\n", in.photonEvery);
        fprintf(f, "  \"photon_threshold\": %.3f,\n", in.photonThreshold);
        fprintf(f, "  \"photon_threshold_unit\": \"%s\",\n",
                in.photonKev ? "keV" : "ADU");
        fprintf(f, "  \"file\": \"%s\",\n", jsonEscape(h5Path).c_str());
        fprintf(f, "  \"started\": \"%s\",\n", startedAt.c_str());
        fprintf(f, "  \"duration_s\": %.3f,\n", wall);
        fprintf(f, "  \"format\": \"slsReceiver HDF5DataFile (writer "
                   "version %.1f), ROI crop, gate-passed frames only\",\n",
                kHdf5WriterVersion);
        fprintf(f, "  \"roi\": {\"x\": %d, \"y\": %d, \"w\": %d, "
                   "\"h\": %d, \"coords\": \"data rows, row 0 at bottom\"},\n",
                in.x, in.y, in.w, in.h);
        fprintf(f, "  \"frame_0_is_pedestal\": %s,\n",
                in.corrected ? "false" : "true");
        fprintf(f, "  \"pedestal_available\": %s,\n",
                in.pedestal.size() == roiPixels ? "true" : "false");
        fprintf(f, "  \"pedestal_corrected\": %s,\n",
                in.corrected ? "true" : "false");
        fprintf(f, "  \"data_dtype\": \"%s\",\n",
                in.corrected ? "float32" : "uint16");
        if (in.corrected)
            fprintf(f, "  \"data_encoding\": \"pedestal corrected, "
                       "gain normalised (g0: raw-pede, g1: x30, g3: x340), "
                       "float32; pedestal image in /pedestal data\",\n");
        else
            fprintf(f, "  \"data_encoding\": \"raw 16-bit word: bits 0-13 "
                       "= ADC, bits 14-15 = gain; slice 0 = gain-0 pedestal "
                       "in ADC units\",\n");
        fprintf(f, "  \"gate\": {\"enabled\": %s, \"threshold\": %.3f, "
                   "\"min_pixels\": %llu, \"contiguous\": %s, "
                   "\"no_pede\": %s},\n",
                in.gate.enabled ? "true" : "false",
                static_cast<double>(in.gate.threshold),
                static_cast<unsigned long long>(in.gate.minPixels),
                in.gate.contiguous ? "true" : "false",
                in.gate.noPede ? "true" : "false");
        fprintf(f, "  \"gate_roi\": {\"active\": %s, \"x\": %d, \"y\": %d, "
                   "\"w\": %d, \"h\": %d},\n",
                in.gate.roiActive ? "true" : "false", in.gate.roiX,
                in.gate.roiY, in.gate.roiW, in.gate.roiH);
        if (gGateChangedDuringSave.load())
            fprintf(f, "  \"gate_settings_modified_during_run\": true,\n"
                       "  \"gate_settings_note\": \"the signal-gate threshold "
                       "and/or min-pixels were changed while this file was being "
                       "written; the \\\"gate\\\" block above is the criterion "
                       "in force at save start\",\n");
        fprintf(f, "  \"stop_reason\": \"%s\",\n",
                jsonEscape(stopR).c_str());
        fprintf(f, "  \"error\": \"%s\",\n", jsonEscape(errR).c_str());
        fprintf(f, "  \"frames\": {\"saved\": %llu, \"dropped_queue_full\": "
                   "%llu, \"budget\": %llu, \"first_frame_number\": %llu, "
                   "\"last_frame_number\": %llu},\n",
                static_cast<unsigned long long>(
                    in.photonMode ? pmapsWritten : saved.load()),
                static_cast<unsigned long long>(dropped.load()),
                static_cast<unsigned long long>(maxFrames),
                static_cast<unsigned long long>(firstFrame.load()),
                static_cast<unsigned long long>(lastFrame.load()));
        if (in.photonMode)
            fprintf(f, "  \"photon_maps\": {\"saved\": %llu, \"every\": %d, "
                       "\"threshold\": %.3f, \"threshold_unit\": \"%s\", "
                       "\"cumulative\": false, \"pedestal_active\": %s, "
                       "\"dataset\": \"/virtual photon counting\"},\n",
                    static_cast<unsigned long long>(pmapsWritten),
                    in.photonEvery, in.photonThreshold,
                    in.photonKev ? "keV" : "ADU",
                    in.peds ? "true" : "false");
        const GateEngine::Counters g = in.gateAtEnd;
        fprintf(f, "  \"gate_statistics\": {\"frames_in\": %llu, "
                   "\"frames_passed\": %llu, \"frames_dropped\": %llu, "
                   "\"pixels_above\": %llu, \"drop_fraction\": %.6f, "
                   "\"rate_in_fps\": %.3f, \"rate_pass_fps\": %.3f, "
                   "\"rate_in_MBps\": %.3f, \"rate_pass_MBps\": %.3f},\n",
                static_cast<unsigned long long>(g.framesIn),
                static_cast<unsigned long long>(g.framesPass),
                static_cast<unsigned long long>(g.framesDrop),
                static_cast<unsigned long long>(g.pixelsAbove), g.dropFrac,
                g.rateIn, g.ratePass, g.rateInMBs, g.ratePassMBs);
        fprintf(f, "  \"alignment\": {\"resync_drops\": %llu, "
                   "\"behind_drops\": %llu, \"unpaired\": %llu, "
                   "\"display_frames\": %llu, \"display_dropped\": %llu},\n",
                static_cast<unsigned long long>(in.stats.resync),
                static_cast<unsigned long long>(in.stats.behind),
                static_cast<unsigned long long>(in.stats.unpaired),
                static_cast<unsigned long long>(in.stats.displayFrames),
                static_cast<unsigned long long>(in.stats.displayDropped));
        fprintf(f, "  \"streams\": [");
        for (size_t i = 0; i < in.streams.size(); i++) {
            fprintf(f, "%s{\"port\": %d, \"state\": \"%s\", \"frames\": "
                       "%llu, \"last_frame\": %llu}",
                    i ? ", " : "", in.streams[i].port,
                    jsonEscape(in.streams[i].state).c_str(),
                    static_cast<unsigned long long>(in.streams[i].frames),
                    static_cast<unsigned long long>(in.streams[i].lastFrame));
        }
        fprintf(f, "]\n}\n");
        // make the .part content durable, then publish it with one rename
        fflush(f);
        ::fsync(::fileno(f));
        fclose(f);
        if (::rename(partPath.c_str(), jsonPath.c_str()) == 0)
            syncDirOf(jsonPath);
    };

    // Periodic clean checkpoint: shrink the over-allocated datasets back to the
    // frames actually written, flush the HDF5 to disk and refresh the JSON.
    // At every checkpoint boundary the pair on disk is self-consistent, so a
    // copy/open then sees exactly the committed frames and a crash loses at
    // most one interval.  Writer thread only.
    auto h5Checkpoint = [&]() {
        if (!file)
            return;
        if (framesWritten > 0 && framesWritten != framesAllocated) {
            hsize_t nd[3] = {framesWritten, hsize_t(h), hsize_t(w)};
            data->extend(nd);
            for (auto &p : para) {
                hsize_t np[1] = {framesWritten};
                p->extend(np);
            }
            framesAllocated = framesWritten;
            dataSpace.reset(new DataSpace(data->getSpace()));
            paraSpace.reset(new DataSpace(para[0]->getSpace()));
        }
        H5Fflush(file->getId(), H5F_SCOPE_GLOBAL);
        writeJsonSidecar(false);
    };

    try {
        Exception::dontPrint();

        // ---- file + version attribute (as the receiver writes it) --------
        // Create in the latest (v110) file format: SWMR requires it, and it is
        // what makes the file guaranteed readable while it is being written and
        // after a writer crash.  Readers need HDF5 >= 1.10.
        // Built with the C++ constructor so H5File owns the only reference:
        // wrapping a raw H5Fcreate id (H5File(fid)) adds a second one, and
        // close() then left the file open until the process exited.
        {
            FileAccPropList fapl;
            fapl.setLibverBounds(H5F_LIBVER_LATEST, H5F_LIBVER_LATEST);
            file.reset(new H5File(h5Path, H5F_ACC_TRUNC,
                                  FileCreatPropList::DEFAULT, fapl));
        }
        {
            DataSpace scalar(H5S_SCALAR);
            Attribute a =
                file->createAttribute("version", PredType::NATIVE_DOUBLE,
                                      scalar);
            double v = kHdf5WriterVersion;
            a.write(PredType::NATIVE_DOUBLE, &v);
        }
        // ---- extra self-describing attributes (ignored by receiver readers)
        {
            DataSpace scalar(H5S_SCALAR);
            Attribute ax = file->createAttribute("roi_x", PredType::NATIVE_UINT32,
                                                 scalar);
            uint32_t v = static_cast<uint32_t>(info.x);
            ax.write(PredType::NATIVE_UINT32, &v);
            Attribute ay = file->createAttribute("roi_y", PredType::NATIVE_UINT32,
                                                 scalar);
            v = static_cast<uint32_t>(info.y);
            ay.write(PredType::NATIVE_UINT32, &v);
            Attribute aw = file->createAttribute("roi_w", PredType::NATIVE_UINT32,
                                                 scalar);
            v = static_cast<uint32_t>(w);
            aw.write(PredType::NATIVE_UINT32, &v);
            Attribute ah = file->createAttribute("roi_h", PredType::NATIVE_UINT32,
                                                 scalar);
            v = static_cast<uint32_t>(h);
            ah.write(PredType::NATIVE_UINT32, &v);
            Attribute ap = file->createAttribute(
                "pedestal_in_frame_0", PredType::NATIVE_UINT8, scalar);
            uint8_t one = info.corrected ? 0 : 1;
            ap.write(PredType::NATIVE_UINT8, &one);
            Attribute ac = file->createAttribute(
                "pedestal_corrected", PredType::NATIVE_UINT8, scalar);
            uint8_t corr = info.corrected ? 1 : 0;
            ac.write(PredType::NATIVE_UINT8, &corr);
            StrType st(PredType::C_S1, H5T_VARIABLE);
            Attribute ag = file->createAttribute(
                "gate", st, scalar);
            std::string g = info.gateJson;
            const char *gp = g.c_str();
            ag.write(st, &gp);
        }

        // ---- /data -------------------------------------------------------
        paraNames = {"frame number",
                     "exp length or sub exposure time",
                     "packets caught",
                     "detector specific 1",
                     "timestamp",
                     "mod id",
                     "row",
                     "column",
                     "detector specific 2",
                     "detector specific 3",
                     "detector specific 4",
                     "detector type",
                     "detector header version",
                     "packets caught bit mask"};

        // raw mode starts with one slice (the pedestal), corrected mode with
        // none: /data then contains exactly the gate-passed frames
        const hsize_t n0 = info.corrected ? 0 : 1;
        const DataType dataType = info.corrected ? DataType(PredType::IEEE_F32LE)
                                                 : DataType(PredType::STD_U16LE);
        hsize_t dims[3] = {n0, hsize_t(h), hsize_t(w)};
        hsize_t dimsMax[3] = {H5S_UNLIMITED, hsize_t(h), hsize_t(w)};
        hsize_t chunk[3] = {1, hsize_t(h), hsize_t(w)};
        DataSpace ds(3, dims, dimsMax);
        DSetCreatPropList plist;
        plist.setChunk(3, chunk);
        data.reset(new DataSet(
            file->createDataSet("/data", dataType, ds, plist)));
        framesAllocated = n0;

        // corrected mode: the pedestal image as its own single-image dataset
        std::unique_ptr<DataSet> pedSet;
        if (info.corrected) {
            hsize_t pd[3] = {1, hsize_t(h), hsize_t(w)};
            hsize_t pdMax[3] = {1, hsize_t(h), hsize_t(w)};
            DataSpace pds3(3, pd, pdMax);
            DSetCreatPropList ppl;
            ppl.setChunk(3, chunk);
            pedSet.reset(new DataSet(file->createDataSet(
                "/pedestal data", PredType::IEEE_F32LE, pds3, ppl)));
            std::vector<float> pv(roiPixels, 0.0f);
            if (info.pedestal.size() == roiPixels)
                pv = info.pedestal;
            const hsize_t pc[3] = {1, hsize_t(h), hsize_t(w)};
            DataSpace mspace(3, pc); // writes the whole 1-image dataset
            pedSet->write(pv.data(), PredType::IEEE_F32LE, mspace);
            mspace.close();
        }

        // ---- virtual photon counting: one map per publish ----------------
        // Created before SWMR starts, like every other object.
        std::unique_ptr<DataSet> pmapSet, pmapFrameSet, pmapStampSet;
        hsize_t pmapWritten = 0;
        if (info.photonMode) {
            hsize_t md[3] = {0, hsize_t(h), hsize_t(w)};
            hsize_t mdMax[3] = {H5S_UNLIMITED, hsize_t(h), hsize_t(w)};
            DataSpace mds(3, md, mdMax);
            DSetCreatPropList mpl;
            mpl.setChunk(3, chunk);
            pmapSet.reset(new DataSet(file->createDataSet(
                "/virtual photon counting", PredType::IEEE_F32LE, mds, mpl)));
            const char *names[2] = {"/virtual photon counting frame",
                                    "/virtual photon counting timestamp"};
            std::unique_ptr<DataSet> *sets[2] = {&pmapFrameSet, &pmapStampSet};
            for (int k = 0; k < 2; k++) {
                hsize_t d1[1] = {0};
                hsize_t d1Max[1] = {H5S_UNLIMITED};
                hsize_t c1[1] = {1};
                DataSpace s1(1, d1, d1Max);
                DSetCreatPropList p1;
                p1.setChunk(1, c1);
                sets[k]->reset(new DataSet(
                    file->createDataSet(names[k], PredType::STD_U64LE, s1, p1)));
            }
        }
        auto writePhotonMap = [&](const Impl::PmapItem &it) {
            if (!pmapSet)
                return;
            // the map is accumulated full-frame (1024x512 data layout); the
            // saved map is the gate ROI, cropped exactly like /data (same
            // x/y/w/h, same data rows)
            const int cx0 = std::max(0, info.x);
            const int cy0 = std::max(0, info.y);
            const int cx1 = std::min(kRawWidth, info.x + w);
            const int cy1 = std::min(kRawHeight, info.y + h);
            const int cw = (cx1 > cx0) ? (cx1 - cx0) : 0;
            std::vector<float> crop(static_cast<size_t>(w) * h, 0.0f);
            for (int y = 0; cy0 + y < cy1; y++)
                memcpy(&crop[static_cast<size_t>(y) * w],
                       &it.map[static_cast<size_t>(cy0 + y) * kRawWidth + cx0],
                       static_cast<size_t>(cw) * sizeof(float));
            hsize_t nd[3] = {pmapWritten + 1, hsize_t(h), hsize_t(w)};
            pmapSet->extend(nd);
            const hsize_t count[3] = {1, hsize_t(h), hsize_t(w)};
            const hsize_t start[3] = {pmapWritten, 0, 0};
            DataSpace ds3 = pmapSet->getSpace();
            ds3.selectHyperslab(H5S_SELECT_SET, count, start);
            const hsize_t memDims[2] = {hsize_t(h), hsize_t(w)};
            DataSpace mem(2, memDims);
            pmapSet->write(crop.data(), PredType::IEEE_F32LE, mem, ds3);
            mem.close();
            ds3.close();
            const uint64_t prov[2] = {it.firstFrame, it.firstStamp};
            std::unique_ptr<DataSet> *sets[2] = {&pmapFrameSet, &pmapStampSet};
            for (int k = 0; k < 2; k++) {
                hsize_t fnd[1] = {pmapWritten + 1};
                (*sets[k])->extend(fnd);
                hsize_t fc[1] = {1};
                hsize_t fs[1] = {pmapWritten};
                DataSpace fds = (*sets[k])->getSpace();
                fds.selectHyperslab(H5S_SELECT_SET, fc, fs);
                const hsize_t fm[1] = {1};
                DataSpace fms(1, fm);
                (*sets[k])->write(&prov[k], PredType::STD_U64LE, fms, fds);
                fms.close();
                fds.close();
            }
            pmapWritten++;
            pmapsInFile.store(pmapWritten); // Impl member, no shadowing
            printf("[pmap] map %llu written (%llu frames, first frame %llu)\n",
                   (unsigned long long)pmapWritten,
                   (unsigned long long)it.frames,
                   (unsigned long long)it.firstFrame);
            fflush(stdout);
        };

        // ---- pedestal tracking: one image per PEDE_TRACK_H5_PERIOD_MS ----
        // Created up front (empty) because SWMR mode cannot create objects after
        // it starts.  The series is still written only while tracking is on
        // (plus one baseline at the start), so a run without tracking carries
        // two extra empty datasets.
        std::unique_ptr<DataSet> trackSet, trackFrameSet;
        {
            hsize_t td[3] = {0, hsize_t(h), hsize_t(w)};
            hsize_t tdMax[3] = {H5S_UNLIMITED, hsize_t(h), hsize_t(w)};
            DataSpace tds(3, td, tdMax);
            DSetCreatPropList tpl;
            tpl.setChunk(3, chunk);
            trackSet.reset(new DataSet(file->createDataSet(
                "/pedestal tracking", PredType::IEEE_F32LE, tds, tpl)));
            hsize_t fd[1] = {0};
            hsize_t fdMax[1] = {H5S_UNLIMITED};
            hsize_t fchunk[1] = {1};
            DataSpace fds(1, fd, fdMax);
            DSetCreatPropList fpl;
            fpl.setChunk(1, fchunk);
            trackFrameSet.reset(new DataSet(file->createDataSet(
                "/pedestal tracking frame", PredType::STD_U64LE, fds, fpl)));
        }
        hsize_t trackWritten = 0;
        int64_t lastTrackMs = 0;
        auto writeTrackedPedestal = [&](uint64_t frameNo, bool force) {
            if (!gPedeTracking.load() && !force)
                return;
            const int64_t now =
                std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::steady_clock::now().time_since_epoch())
                    .count();
            if (trackWritten > 0 && !force &&
                now - lastTrackMs < PEDE_TRACK_H5_PERIOD_MS)
                return;
            std::shared_ptr<const PedeSet> ps = PedestalStore::instance().get();
            if (!ps)
                return;
            std::vector<float> img(static_cast<size_t>(h) * w, 0.0f);
            for (int ry = 0; ry < h; ry++)
                for (int rx = 0; rx < w; rx++) {
                    const size_t fi = static_cast<size_t>(info.y + ry) * 1024 +
                                      info.x + rx;
                    if (fi < ps->g0.size())
                        img[static_cast<size_t>(ry) * w + rx] = ps->g0[fi];
                }
            {
                hsize_t nd[3] = {trackWritten + 1, hsize_t(h), hsize_t(w)};
                trackSet->extend(nd);
                hsize_t count[3] = {1, hsize_t(h), hsize_t(w)};
                hsize_t start[3] = {trackWritten, 0, 0};
                DataSpace ds3 = trackSet->getSpace();
                ds3.selectHyperslab(H5S_SELECT_SET, count, start);
                const hsize_t memDims[2] = {hsize_t(h), hsize_t(w)};
                DataSpace mem(2, memDims);
                trackSet->write(img.data(), PredType::IEEE_F32LE, mem, ds3);
                mem.close();
                ds3.close();
                hsize_t fnd[1] = {trackWritten + 1};
                trackFrameSet->extend(fnd);
                hsize_t fcount[1] = {1};
                hsize_t fstart[1] = {trackWritten};
                DataSpace fds = trackFrameSet->getSpace();
                fds.selectHyperslab(H5S_SELECT_SET, fcount, fstart);
                const hsize_t fmem[1] = {1};
                DataSpace fms(1, fmem);
                trackFrameSet->write(&frameNo, PredType::STD_U64LE, fms, fds);
                fms.close();
                fds.close();
            }
            trackWritten++;
            lastTrackMs = now;
            printf("[pedetrack] pedestal image %llu written to %s (frame %llu)\n",
                   (unsigned long long)trackWritten, h5Path.c_str(),
                   (unsigned long long)frameNo);
            fflush(stdout);
        };

        // ---- parameter datasets (same names/types as the receiver) --------
        hsize_t pdims[1] = {n0};
        hsize_t pdimsMax[1] = {H5S_UNLIMITED};
        hsize_t pchunk[1] = {1};
        DataSpace pds(1, pdims, pdimsMax);
        DSetCreatPropList pplist;
        pplist.setChunk(1, pchunk);
        StrType strType(PredType::C_S1, bitsetBytes);
        const DataType types[14] = {
            PredType::STD_U64LE, PredType::STD_U32LE, PredType::STD_U32LE,
            PredType::STD_U64LE, PredType::STD_U64LE, PredType::STD_U16LE,
            PredType::STD_U16LE, PredType::STD_U16LE, PredType::STD_U16LE,
            PredType::STD_U32LE, PredType::STD_U16LE, PredType::STD_U8LE,
            PredType::STD_U8LE,  strType};
        for (int i = 0; i < 14; i++)
            para.emplace_back(new DataSet(file->createDataSet(
                "/" + paraNames[i], types[i], pds, pplist)));
        dataSpace.reset(new DataSpace(data->getSpace()));
        paraSpace.reset(new DataSpace(para[0]->getSpace()));

        // ---- switch to SWMR ----------------------------------------------
        // Every object now exists (attributes, /data, /pedestal data,
        // /pedestal tracking, the parameter datasets); SWMR forbids creating
        // new ones afterwards.  From here on the H5Fflush at each checkpoint
        // publishes a consistent view to concurrent SWMR readers, and the file
        // stays valid even if this process dies without closing it.
        if (H5Fstart_swmr_write(file->getId()) < 0)
            throw std::runtime_error("H5Fstart_swmr_write failed for " + h5Path);
        writeTrackedPedestal(0, /*force=*/true); // baseline at the start

        // ---- write one frame (image + parameters) ------------------------
        auto writeFrame = [&](const SavedFrameHeader &hd,
                              const void *pixels) {
            writeTrackedPedestal(hd.frameNumber, /*force=*/false);
            if (framesWritten + 1 > framesAllocated) {
                hsize_t nd[3] = {framesAllocated + kExtendBlock, hsize_t(h),
                                 hsize_t(w)};
                data->extend(nd);
                for (auto &p : para) {
                    hsize_t np[1] = {nd[0]};
                    p->extend(np);
                }
                framesAllocated = nd[0];
                dataSpace.reset(new DataSpace(data->getSpace()));
                paraSpace.reset(new DataSpace(para[0]->getSpace()));
            }
            const hsize_t count[3] = {1, hsize_t(h), hsize_t(w)};
            const hsize_t start[3] = {framesWritten, 0, 0};
            const hsize_t memDims[2] = {hsize_t(h), hsize_t(w)};
            dataSpace->selectHyperslab(H5S_SELECT_SET, count, start);
            DataSpace mem(2, memDims);
            data->write(pixels, dataType, mem, *dataSpace);
            mem.close();

            const hsize_t pcount[1] = {1};
            const hsize_t pstart[1] = {framesWritten};
            paraSpace->selectHyperslab(H5S_SELECT_SET, pcount, pstart);
            DataSpace pscalar(H5S_SCALAR);
            unsigned char empty[sizeof(sls::defs::bitset_storage)];
            memset(empty, 0, sizeof(empty));
            para[0]->write(&hd.frameNumber, types[0], pscalar, *paraSpace);
            para[1]->write(&hd.expLength, types[1], pscalar, *paraSpace);
            para[2]->write(&hd.packetNumber, types[2], pscalar, *paraSpace);
            para[3]->write(&hd.detSpec1, types[3], pscalar, *paraSpace);
            para[4]->write(&hd.timestamp, types[4], pscalar, *paraSpace);
            para[5]->write(&hd.modId, types[5], pscalar, *paraSpace);
            para[6]->write(&hd.row, types[6], pscalar, *paraSpace);
            para[7]->write(&hd.column, types[7], pscalar, *paraSpace);
            para[8]->write(&hd.detSpec2, types[8], pscalar, *paraSpace);
            para[9]->write(&hd.detSpec3, types[9], pscalar, *paraSpace);
            para[10]->write(&hd.detSpec4, types[10], pscalar, *paraSpace);
            para[11]->write(&hd.detType, types[11], pscalar, *paraSpace);
            para[12]->write(&hd.version, types[12], pscalar, *paraSpace);
            para[13]->write(empty, types[13], pscalar, *paraSpace);
            pscalar.close();
            framesWritten++;
        };

        // ---- raw mode: slice 0 = the pedestal in the same ROI -------------
        if (!info.corrected) {
            std::vector<uint16_t> ped(roiPixels, 0);
            if (info.pedestal.size() == roiPixels) {
                for (size_t i = 0; i < roiPixels; i++) {
                    const float v = info.pedestal[i];
                    ped[i] = (v <= 0.0f)
                                 ? 0
                                 : (v >= 65535.0f
                                        ? 65535
                                        : static_cast<uint16_t>(v + 0.5f));
                }
            }
            SavedFrameHeader hd; // frame number 0 marks the pedestal slice
            writeFrame(hd, ped.data());
        }

        // ---- drain the queue ---------------------------------------------
        const int checkpointMs = info.checkpointMs;
        auto steadyMs = []() {
            return std::chrono::duration_cast<std::chrono::milliseconds>(
                       std::chrono::steady_clock::now().time_since_epoch())
                .count();
        };
        int64_t lastCheckpointMs = steadyMs();
        for (;;) {
            Impl::Item item;
            bool have = false;
            {
                std::unique_lock<std::mutex> lk(qMtx);
                qCv.wait_for(lk, std::chrono::milliseconds(100), [&] {
                    return !queue.empty() || stopRequested.load();
                });
                if (!queue.empty()) {
                    item = std::move(queue.front());
                    queue.pop_front();
                    queuedBytes -= item.pixels.size() * sizeof(float);
                    have = true;
                }
            }
            // periodic clean checkpoint (HDF5 exact+flushed, JSON renamed)
            if (checkpointMs > 0) {
                const int64_t nowMs = steadyMs();
                if (nowMs - lastCheckpointMs >= checkpointMs) {
                    h5Checkpoint();
                    lastCheckpointMs = nowMs;
                }
            }
            for (;;) { // virtual photon counting maps
                Impl::PmapItem it;
                {
                    std::lock_guard<std::mutex> lk(qMtx);
                    if (pmapQueue.empty())
                        break;
                    it = std::move(pmapQueue.front());
                    pmapQueue.pop_front();
                }
                writePhotonMap(it);
                pmapsWritten++;
                if (maxFrames > 0 && pmapsWritten >= maxFrames) {
                    setStop("photon-map budget reached");
                    stopRequested.store(true);
                    break;
                }
            }
            if (stopRequested.load())
                break;
            if (!have) {
                // idle tick: the tracked pedestal is written every
                // PEDE_TRACK_H5_PERIOD_MS of wall clock while the save runs,
                // even if no frame passes the gate
                writeTrackedPedestal(lastFrame.load(), /*force=*/false);
            }
            if (have) {
                if (info.corrected) {
                    writeFrame(item.hdr, item.pixels.data());
                } else {
                    static std::vector<uint16_t> raw;
                    raw.resize(item.pixels.size());
                    for (size_t k = 0; k < item.pixels.size(); k++) {
                        const float v = item.pixels[k];
                        raw[k] = (v <= 0.0f)
                                     ? 0
                                     : (v >= 65535.0f
                                            ? 65535
                                            : static_cast<uint16_t>(v + 0.5f));
                    }
                    writeFrame(item.hdr, raw.data());
                }
                if (firstFrame.load() == 0)
                    firstFrame.store(item.hdr.frameNumber);
                lastFrame.store(item.hdr.frameNumber);
                const uint64_t n = saved.fetch_add(1) + 1;
                {
                    std::lock_guard<std::mutex> lk(qMtx);
                    freeList.push_back(std::move(item));
                }
                if (maxFrames > 0 && n >= maxFrames) {
                    setStop("frame budget reached");
                    stopRequested.store(true);
                    break; // frames still queued are beyond the budget
                }
                continue;
            }
            if (stopRequested.load())
                break;
        }

        // ---- shrink to what we actually wrote, then close -----------------
        {
            const hsize_t finalFrames = framesWritten ? framesWritten : 1;
            hsize_t nd[3] = {finalFrames, hsize_t(h), hsize_t(w)};
            data->extend(nd);
            for (auto &p : para) {
                hsize_t np[1] = {finalFrames};
                p->extend(np);
            }
        }
        dataSpace.reset();
        paraSpace.reset();
        data.reset();
        para.clear();
        pedSet.reset();
        trackSet.reset();
        trackFrameSet.reset();
        file->close();
        file.reset();
    } catch (const H5::Exception &e) {
        setError(std::string("HDF5: ") + e.getDetailMsg());
        setStop("error");
        stopRequested.store(true);
    } catch (const std::exception &e) {
        setError(std::string("save failed: ") + e.what());
        setStop("error");
        stopRequested.store(true);
    }

    // ---- final JSON sidecar (atomic <name>.json.part -> rename) --------
    // Periodic snapshots have already refreshed it during the run.
    writeJsonSidecar(true);
    running.store(false);
}
