// Unit test for the gate's contiguity proxy ("Contiguous (N px within circle,
// r = N/2+1)").
//
// The rule must be: pass when SOME circle of radius r contains at least N
// above-threshold pixels.  It used to require every above-threshold pixel to be
// within r of the centroid, which rejected perfectly valid events (two
// separated clusters) - the bug this test pins down.
//
// Build: make gate_contig_test   Run: ./test/gate_contig_test
#include "online_jf.h"

#include <algorithm>
#include <cstdio>
#include <utility>
#include <vector>

namespace {

int gFail = 0;

void expect(const char *what, bool got, bool want) {
    const bool ok = (got == want);
    if (!ok)
        gFail++;
    std::printf("%-58s %s (got %s, want %s)\n", what, ok ? "ok  " : "FAIL",
                got ? "pass" : "drop", want ? "pass" : "drop");
}

struct Frame {
    std::vector<short> px = std::vector<short>(NCH, 0);

    void hit(int x, int y, short v = 2000) {
        px[static_cast<size_t>(y) * kRawWidth + x] = v;
    }
    void clear() { std::fill(px.begin(), px.end(), 0); }
};

// Runs the real GateEngine over the frame, raw ADC values, no pedestals.
bool passes(GateEngine &g, Frame &f) {
    uint64_t above = 0;
    bool scDropped = false;
    return g.evaluate(f.px.data(), NCH, 0, nullptr, 0, 0, above, &scDropped);
}

} // namespace

int main() {
    GateEngine g;
    g.setEnabled(true);
    g.setThreshold(1000.0f); // the frame's hits are 2000 -> above threshold
    g.setMinPixels(5);
    g.setContiguous(true);

    Frame f;

    // 1. one tight cluster of exactly N hits: must pass
    f.clear();
    for (int i = 0; i < 5; i++)
        f.hit(100 + i, 100);
    expect("5 hits in a row (spread 4 px <= 2r)", passes(g, f), true);

    // 2. a compact blob: must pass
    f.clear();
    f.hit(300, 200);
    f.hit(301, 200);
    f.hit(302, 200);
    f.hit(301, 201);
    f.hit(301, 199);
    expect("5 hits in a compact cross", passes(g, f), true);

    // 3. N hits far apart, no cluster: must be rejected
    f.clear();
    for (int i = 0; i < 5; i++)
        f.hit(100 + 100 * i, 300);
    expect("5 isolated hits 100 px apart", passes(g, f), false);

    // 4. fewer than N hits, however tight: must be rejected
    f.clear();
    for (int i = 0; i < 4; i++)
        f.hit(500 + i, 400);
    expect("4 hits (below min pixels)", passes(g, f), false);

    // 5. THE REPORTED BUG: two separated clusters, each with N hits.
    //    The centroid falls between them, which used to reject the frame.
    f.clear();
    for (int i = 0; i < 5; i++)
        f.hit(100 + i, 50); // cluster A
    for (int i = 0; i < 5; i++)
        f.hit(700 + i, 450); // cluster B
    expect("two clusters of 5, 700 px apart (must pass)", passes(g, f), true);

    // 6. two groups of 4: no circle holds 5 -> reject
    f.clear();
    for (int i = 0; i < 4; i++)
        f.hit(100 + i, 60);
    for (int i = 0; i < 4; i++)
        f.hit(600 + i, 440);
    expect("two clusters of 4 (no circle holds 5)", passes(g, f), false);

    // 7. a spread-out event with one dense core: pass
    f.clear();
    for (int i = 0; i < 5; i++)
        f.hit(400 + i, 300);
    f.hit(800, 100);
    f.hit(900, 200);
    expect("dense core plus 2 stragglers", passes(g, f), true);

    // 8. N follows min_pixels: 3 hits pass when min_pixels is 3
    g.setMinPixels(3);
    f.clear();
    for (int i = 0; i < 3; i++)
        f.hit(200 + i, 200);
    expect("3 hits with min_pixels = 3", passes(g, f), true);
    f.clear();
    for (int i = 0; i < 2; i++)
        f.hit(200 + i, 200);
    expect("2 hits with min_pixels = 3", passes(g, f), false);

    // 9. contiguity off: the plain count decides (2 hits < 3 -> reject)
    g.setContiguous(false);
    f.clear();
    f.hit(10, 10);
    f.hit(900, 400);
    expect("contiguity off, 2 hits with min_pixels = 3", passes(g, f), false);
    f.clear();
    for (int i = 0; i < 3; i++)
        f.hit(10 + i, 10);
    expect("contiguity off, 3 hits with min_pixels = 3", passes(g, f), true);

    // ---- hot pixel mask taken from a photon map ---------------------------
    {
        GateEngine h;
        h.setEnabled(true);
        h.setThreshold(1000.0f);
        h.setMinPixels(5);
        h.setContiguous(false);
        h.setHotSuppress(true);

        // The map has two very bright pixels: those are the hot ones.
        std::vector<float> map(NCH, 0.0f);
        map[10 * kRawWidth + 10] = 400.0f;
        map[10 * kRawWidth + 11] = 900.0f;
        hotSetMaskFromMap(map.data(), map.size(), 100.0);
        std::printf("mask from map > 100: %llu px (want 2)\n",
                    static_cast<unsigned long long>(hotPixelCount()));
        if (hotPixelCount() != 2)
            gFail++;

        Frame f3;
        // 5 hits, two of them on the masked pixels -> only 3 count -> drop
        f3.clear();
        for (int i = 0; i < 5; i++)
            f3.hit(10 + i, 10);
        expect("5 hits, 2 on hot pixels (min 5)", passes(h, f3), false);

        // a real cluster away from the mask still passes
        f3.clear();
        for (int i = 0; i < 5; i++)
            f3.hit(300 + i, 300);
        expect("real cluster of 5 with the mask active", passes(h, f3), true);

        // cleared mask: the hot pixels count again
        hotClearMask();
        expect("after hotClearMask, 5 hits count", passes(h, f3), true);
        f3.clear();
        for (int i = 0; i < 5; i++)
            f3.hit(10 + i, 10);
        expect("after hotClearMask, the 5 hits pass", passes(h, f3), true);

        // suppression off: the mask is ignored by the gate
        hotSetMaskFromMap(map.data(), map.size(), 100.0);
        h.setHotSuppress(false);
        f3.clear();
        for (int i = 0; i < 5; i++)
            f3.hit(10 + i, 10);
        expect("suppression off, masked pixels count again", passes(h, f3),
               true);
        hotClearMask();
    }

    std::printf("\n%s (%d failure(s))\n", gFail ? "FAILED" : "all tests passed",
                gFail);
    return gFail ? 1 : 0;
}
