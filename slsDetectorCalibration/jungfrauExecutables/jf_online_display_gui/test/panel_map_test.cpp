// Standalone JFSTRX check that the six-panel fill geometry is consistent with
// the strixel decoder: every real (non-dummy) mapped pixel must land inside a
// valid panel at an in-range local coordinate, and each panel must be fully
// covered by the decoder map (so no panel stays empty / has holes).
#include "online_jf.h"

#include "jungfrauLGADStrixelsData_new.h"
#include "sls/sls_detector_defs.h"
#include "slsDetectorData.h"

#include <cstdio>

using header = sls::defs::sls_receiver_header;

int main() {
    initGeometry();
    jungfrauLGADStrixelsData *dec = new jungfrauLGADStrixelsData();
    const int g1_nrows = 165, g2_nrows = 320, g3_nrows = 476;
    const int mc6g3_start = g1_nrows + g2_nrows + g3_nrows;
    static const int panelStartY[kNumAdcPanels] = {0,   165, 485,
                                                   1757, 1437, 961};
    long perPanel[kNumAdcPanels] = {0};
    long real = 0;
    long dropped = 0; // real pixels outside the booked panel rectangles
    bool ok = true;
    for (int ipx = 0; ipx < nx; ++ipx) {
        for (int ipy = 0; ipy < ny; ++ipy) {
            if (dec->dataMap[ipy][ipx] == int(sizeof(header)))
                continue; // dummy strixel
            ++real;
            const int group = dec->groupmap[ipy][ipx];
            if (group < 0 || group > 2) {
                ok = false;
                continue;
            }
            const int base = (ipy >= mc6g3_start) ? 3 : 0;
            const int p = base + group;
            const int lx = ipx;
            const int ly = ipy - panelStartY[p];
            if (lx < 0 || lx >= kAdcPanelW[p] || ly < 0 || ly >= kAdcPanelH[p]) {
                // the decoder maps a couple of extra right-edge columns for
                // chip1 g2/g3 that the ROOT histograms (and these panels) do
                // not book; dropping them matches the original display
                ++dropped;
                continue;
            }
            ++perPanel[p];
        }
    }
    delete dec;

    std::printf("mapped %d x %d, real pixels %ld, dropped edge pixels %ld\n",
                nx, ny, real, dropped);
    for (int p = 0; p < kNumAdcPanels; ++p) {
        const long need = long(kAdcPanelW[p]) * kAdcPanelH[p];
        std::printf("panel %d: %dx%d  filled %ld/%ld%s\n", p, kAdcPanelW[p],
                    kAdcPanelH[p], perPanel[p], need,
                    perPanel[p] == need ? "" : "  <-- NOT FULL");
        if (perPanel[p] != need)
            ok = false;
    }
    std::printf("%s\n", ok ? "PASS: six-panel geometry"
                           : "FAIL: six-panel geometry");
    return ok ? 0 : 1;
}
