// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#ifndef COMMONMODEJF_H
#define COMMONMODEJF_H

#include "commonModeSubtractionNew.h"

class commonModeSubtractionSuperColumnJF : public commonModeSubtraction {
  public:
    commonModeSubtractionSuperColumnJF()
        : commonModeSubtraction(32){};
    virtual int getROI(int ix, int iy) { int top=iy/256; int sc=ix/64; return sc+top*16; };
/*
    virtual void addToCommonMode(double val, int ix = 0, int iy = 0) {
            int iroi = getROI(ix, iy);
            // cout << iy << " " << ix << " " << iroi ;
            if (iroi >= 0 && iroi < nROI) {
                mean[iroi] += val;
                mean2[iroi] += val * val;
                nCm[iroi]++;
                if (nCm[iroi] > 64*256)
                    std::cout << "Too many pixels added " << nCm[iroi] << std::endl;
           
        }
    };
*/
    virtual commonModeSubtractionSuperColumnJF *Clone() {
        return new commonModeSubtractionSuperColumnJF();
    };
};


class commonModeSubtractionChip : public commonModeSubtraction {
  public:
    commonModeSubtractionChip()
        : commonModeSubtraction(8){};
    virtual int getROI(int ix, int iy) { int top=iy/256; int sc=ix/256; return sc+top*4; };
/*
    virtual void addToCommonMode(double val, int ix = 0, int iy = 0) {
            int iroi = getROI(ix, iy);
            // cout << iy << " " << ix << " " << iroi ;
            if (iroi >= 0 && iroi < nROI) {
                mean[iroi] += val;
                mean2[iroi] += val * val;
                nCm[iroi]++;
                if (nCm[iroi] > 64*256)
                    std::cout << "Too many pixels added " << nCm[iroi] << std::endl;
           
        }
    };
*/
    virtual commonModeSubtractionChip *Clone() {
        return new commonModeSubtractionChip();
    };
};


#endif
