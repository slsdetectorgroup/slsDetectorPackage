// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#ifndef MOENCH03GHOSTSUMMATION_H
#define MOENCH03GHOSTSUMMATION_H

#include "ghostSummation.h"

class moench03GhostSummation : public ghostSummation<uint16_t> {

    /** @short virtual calss to handle ghosting*/

  public:
    /** constructor
        \param xt crosstalk
    */
    moench03GhostSummation(slsDetectorData<uint16_t> *d, double xt = 0.0004)
        : ghostSummation<uint16_t>(d, xt) {}

    virtual void calcGhost(char *data) {
        for (int iy = 0; iy < 200; iy++) {
            for (int ix = 0; ix < 25; ix++) {
                calcGhost(data, ix, iy);
            }
        }
    };

    moench03GhostSummation(moench03GhostSummation *orig)
        : ghostSummation(orig) {}

    virtual moench03GhostSummation *Clone() {
        return new moench03GhostSummation(this);
    }

    virtual double calcGhost(char *data, int x, int y = 0) {
        int ix = x % 25;
        int iy = y;
        if (y >= 200)
            iy = 399 - y;
        if (iy < 0 || ix < 0)
            return 0;
        double val = 0;
        val = 0;
        for (int isc = 0; isc < 16; isc++) {
            val += det->getChannel(data, ix + 25 * isc, iy);
            //  cout << val << " " ;
            val += det->getChannel(data, ix + 25 * isc, 399 - iy);
            //  cout << val << " " ;
        }
        ghost[iy * nx + ix] = xtalk * val;
        // if (ix==15 && iy==15) cout << ":" << ghost[iy*nx+ix] << " " << val <<
        // endl;
        return ghost[iy * nx + ix];
    };

    virtual double getGhost(int ix, int iy) {
        if (iy >= 0 && iy < 200)
            return ghost[iy * nx + (ix % 25)];
        if (iy < 400)
            return ghost[(399 - iy) * nx + (ix % 25)];
        return 0;
    };
};

#endif
