// Standalone check for the ROI writer (no hardware, no GUI):
//
//   1. a Pedestal corrected save follows the LIVE pedestal, not the snapshot
//      taken at Start Save (one frame before, one after a mid-run swap);
//   2. the periodic checkpoint leaves a CLEAN pair on disk while the save is
//      still running: the JSON sidecar is published atomically (.part ->
//      rename) and the HDF5 is shrunk to exactly the frames written, so a
//      copy taken at a checkpoint boundary is self-consistent.
//
// Not part of the build; compile by hand, see README "Testing without
// hardware".
#include "online_jf.h"

#include "H5Cpp.h"

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

using namespace H5;

static bool fileExists(const std::string &p) {
    std::ifstream f(p);
    return f.good();
}

static std::string readFile(const std::string &p) {
    std::ifstream f(p, std::ios::binary);
    std::ostringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

// Read the /data frame count and the first pixel of one slice from the LIVE
// file with the SWMR read flag - exactly what an external checker does while
// the save runs.
static bool readSlice(const std::string &path, hsize_t slice, hsize_t &frames,
                      float &px) {
    hid_t f = H5Fopen(path.c_str(), H5F_ACC_RDONLY | H5F_ACC_SWMR_READ,
                      H5P_DEFAULT);
    if (f < 0)
        return false;
    hid_t d = H5Dopen2(f, "/data", H5P_DEFAULT);
    if (d < 0) {
        H5Fclose(f);
        return false;
    }
    hid_t s = H5Dget_space(d);
    hsize_t dims[3] = {0, 0, 0};
    H5Sget_simple_extent_dims(s, dims, nullptr);
    frames = dims[0];
    bool ok = false;
    if (dims[0] > slice) {
        hsize_t off[3] = {slice, 0, 0};
        hsize_t cnt[3] = {1, 1, 1};
        H5Sselect_hyperslab(s, H5S_SELECT_SET, off, nullptr, cnt, nullptr);
        hid_t m = H5Screate_simple(3, cnt, nullptr);
        ok = H5Dread(d, H5T_NATIVE_FLOAT, m, s, H5P_DEFAULT, &px) >= 0;
        H5Sclose(m);
    }
    H5Sclose(s);
    H5Dclose(d);
    H5Fclose(f);
    return ok;
}

// wait until the periodic JSON says at least `saved` frames are committed
static std::string waitJson(const std::string &jsonPath, uint64_t saved) {
    const std::string needle = "\"saved\": " + std::to_string(saved);
    for (int i = 0; i < 100; i++) {
        if (fileExists(jsonPath)) {
            const std::string t = readFile(jsonPath);
            if (t.find(needle) != std::string::npos)
                return t;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    return {};
}

int main() {
    const int W = 1024, H = 4;
    const hsize_t h = 4, w = 8;

    RoiSaver saver;
    RoiSaver::StartInfo info;
    info.x = 0;
    info.y = 0;
    info.w = static_cast<int>(w);
    info.h = static_cast<int>(h);
    info.dir = ".";
    info.baseName = "pede_live_test";
    info.corrected = true;
    info.checkpointMs = 300; // fast checkpoints for the test

    auto setA = std::make_shared<PedeSet>();
    std::fill(setA->g0.begin(), setA->g0.end(), 100.0f);
    PedestalStore::instance().install(setA);
    info.peds = setA; // the snapshot the GUI would have taken at Start Save

    std::string err;
    if (!saver.start(info, err)) {
        std::printf("FAIL start: %s\n", err.c_str());
        return 1;
    }
    const std::string h5 = saver.status().h5Path;
    const std::string js = saver.status().jsonPath;

    std::vector<short> frame(static_cast<size_t>(W) * H, 1000);
    SavedFrameHeader h1;
    h1.frameNumber = 1;
    saver.offer(frame.data(), W, H, h1);

    // --- checkpoint 1: exactly one frame, corrected with pedestal A -------
    const std::string j1 = waitJson(js, 1);
    if (j1.empty()) {
        std::printf("FAIL: no periodic JSON with 1 frame\n");
        return 2;
    }
    const bool liveMid = j1.find("\"complete\": false") != std::string::npos;
    hsize_t n1 = 0;
    float p1 = -1.0f;
    const bool live1 = readSlice(h5, 0, n1, p1); // live file, SWMR read
    std::printf("checkpoint1: periodic json (complete:false)=%s  live swmr "
                "read=%s  h5 extent=%llu  px=%.1f (expect true, true, 1, 900)\n",
                liveMid ? "true" : "false", live1 ? "true" : "false",
                (unsigned long long)n1, p1);

    // --- the tracker swaps the pedestal mid-run ---------------------------
    auto setB = std::make_shared<PedeSet>();
    std::fill(setB->g0.begin(), setB->g0.end(), 150.0f);
    PedestalStore::instance().install(setB);
    SavedFrameHeader h2;
    h2.frameNumber = 2;
    saver.offer(frame.data(), W, H, h2);

    const std::string j2 = waitJson(js, 2);
    if (j2.empty()) {
        std::printf("FAIL: no periodic JSON with 2 frames\n");
        return 3;
    }
    hsize_t n2 = 0;
    float p2 = -1.0f;
    const bool live2 = readSlice(h5, 1, n2, p2); // live file, SWMR read
    std::printf("checkpoint2: live swmr read=%s  h5 extent=%llu  slice1 px=%.1f "
                "(expect true, 2, 850)\n",
                live2 ? "true" : "false", (unsigned long long)n2, p2);

    const bool noPart = !fileExists(js + ".part");

    saver.requestStop("test");
    saver.finish();
    const std::string jf = readFile(js);
    const bool finalComplete =
        jf.find("\"complete\": true") != std::string::npos;

    const bool ok = liveMid && live1 && (n1 == 1) && (p1 == 900.0f) && live2 &&
                    (n2 == 2) && (p2 == 850.0f) && noPart && finalComplete;
    std::printf("final: complete=%s  no .part left=%s\n",
                finalComplete ? "true" : "false", noPart ? "true" : "false");
    std::printf("%s\n",
                ok ? "PASS: live pedestal + periodic clean checkpoint"
                   : "FAIL");
    return ok ? 0 : 4;
}
