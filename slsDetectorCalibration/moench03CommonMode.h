// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#ifndef MOENCH03COMMONMODE_H
#define MOENCH03COMMONMODE_H

#include "commonModeSubtractionNew.h"

class commonModeSubtractionColumn : public commonModeSubtraction {
  public:
    commonModeSubtractionColumn(int nr = 200)
        : commonModeSubtraction(800), rows(nr){};
    virtual int getROI(int ix, int iy) { return ix + (iy / 200) * 400; };

    virtual void addToCommonMode(double val, int ix = 0, int iy = 0) {
        if (iy < rows || iy > 399 - rows) {
            int iroi = getROI(ix, iy);
            // cout << iy << " " << ix << " " << iroi ;
            if (iroi >= 0 && iroi < nROI) {
                mean[iroi] += val;
                mean2[iroi] += val * val;
                nCm[iroi]++;
                if (nCm[iroi] > rows)
                    std::cout << "Too many pixels added " << nCm[iroi] << std::endl;
                /* if (ix==10 && iy==20) */
		/*   std::cout << " ** "<<val << " " << mean[iroi] << " " << */
		/*     nCm[iroi] << " " << getCommonMode(ix, iy) << std::endl; */
            }
        }
    };

    virtual commonModeSubtractionColumn *Clone() {
        return new commonModeSubtractionColumn(this->rows);
    }

  private:
    int rows;
};

class commonModeSubtractionRow : public commonModeSubtraction {
  public:
    commonModeSubtractionRow(int nc = 400)
        : commonModeSubtraction(200), cols(nc){};
  virtual int getROI(int ix, int iy) { 
    if (iy/200<1) return 199-iy%200; 
    else return iy%200;
  } 

    virtual void addToCommonMode(double val, int ix = 0, int iy = 0) {
        if (ix < cols || ix > 399 - cols) {
            int iroi = getROI(ix, iy);
            // cout << iy << " " << ix << " " << iroi ;
            if (iroi >= 0 && iroi < nROI) {
	      mean[iroi] += val;
	      mean2[iroi] += val * val;
	      nCm[iroi]++;
	      if (nCm[iroi] > 4*cols)
		std::cout << "Too many pixels added " << nCm[iroi] << std::endl;
                /* if (ix==10 && iy==20) */
		/*   std::cout << " ** "<<val << " " << mean[iroi] << " " << */
		/*     nCm[iroi] << " " << getCommonMode(ix, iy) << std::endl; */
            }
        }
    };

    virtual commonModeSubtractionRow *Clone() {
        return new commonModeSubtractionRow(this->cols);
    };

  private:
    int cols;
};

class commonModeSubtractionSuperColumn : public commonModeSubtraction {
  public:
    commonModeSubtractionSuperColumn() : commonModeSubtraction(32){};
    virtual int getROI(int ix, int iy) { return ix / 25 + (iy / 200) * 16; };
};

class commonModeSubtractionHalf : public commonModeSubtraction {
  public:
    commonModeSubtractionHalf() : commonModeSubtraction(2){};
    virtual int getROI(int ix, int iy) {
        (void)ix;
        return iy / 200;
    };
};

class moench03CommonMode : public commonModeSubtractionRow {
    /** @short class to calculate the common mode noise for moench02 i.e. on 4
     * supercolumns separately */
  public:
    /** constructor -  initalizes a commonModeSubtraction with 4 different
       regions of interest \param nn number of samples for the moving average
    */
    //moench03CommonMode(int nr = 20) : commonModeSubtractionColumn(nr){};
    moench03CommonMode(int nr = 20) : commonModeSubtractionRow(nr){};
};

#endif
