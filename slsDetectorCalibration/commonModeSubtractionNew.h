// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#ifndef COMMONMODESUBTRACTION_H
#define COMMONMODESUBTRACTION_H

#include <cmath>

class commonModeSubtraction {

    /** @short class to calculate the common mode of the pedestals based on an
     * approximated moving average*/

  public:
    /** constructor
        \param nn number of samples for the moving average to calculate the
       average common mode \param iroi number of regions on which one can
       calculate the common mode separately. Defaults to 1 i.e. whole detector

    */
    commonModeSubtraction(int iroi = 1, int ns = 3) : nsigma(ns), nROI(iroi) {
        mean = new double[nROI];
        mean2 = new double[nROI];
        nCm = new double[nROI];
    };

    /** destructor - deletes the moving average(s) and the sum of pedestals
     * calculator(s) */
    virtual ~commonModeSubtraction() {
        delete[] mean;
        delete[] mean2;
        delete[] nCm;
    };

    /* commonModeSubtraction(commonModeSubtraction *cs) { */
    /*   if (cs) new commonModeSubtraction(cs->getNRoi(), cs->nsigma); */
    /* } */

    virtual commonModeSubtraction *Clone() {
        return new commonModeSubtraction(this->nROI, this->nsigma);
    }

    /** clears the moving average and the sum of pedestals calculation - virtual
     * func*/
    virtual void Clear() {
        for (int i = 0; i < nROI; i++) {
            mean[i] = 0;
            nCm[i] = 0;
            mean2[i] = 0;
        }
    };

    /** adds the average of pedestals to the moving average and reinitializes
     * the calculation of the sum of pedestals for all ROIs. - virtual func*/
    virtual void newFrame() {
        // cout << "Reset CM" << endl;
        for (int i = 0; i < nROI; i++) {
            //	if (nCm[i]>0) cmStat[i].Calc(cmPed[i]/nCm[i]);
            nCm[i] = 0;
            mean[i] = 0;
            mean2[i] = 0;
        }
    };

    /** adds the pixel to the sum of pedestals -- virtual func must be
       overloaded to define the regions of interest \param val value to add
        \param ix pixel x coordinate
        \param iy pixel y coordinate
    */
    virtual void addToCommonMode(double val, int ix = 0, int iy = 0) {

        int iroi = getROI(ix, iy);
        // if (iroi==0) val=100;
        // else val=-100;
        // if (isc>=0 && isc<nROI) {
        //	cout << ix << " " << iy << " " << iroi << endl;
        // if (ix==15 && iy==15) cout << "=" << val << endl;
        if (iroi >= 0 && iroi < nROI) {
            //	cout << ix << " " << iy << " " << iroi << endl;
            mean[iroi] += val;
            mean2[iroi] += val * val;
            nCm[iroi]++;
        }
    };

    /** gets the common mode i.e. the difference between the current average sum
       of pedestals mode and the average pedestal \param ix pixel x coordinate
        \param iy pixel y coordinate
        \return the difference  between the current average sum of pedestals and
       the average pedestal
    */
    virtual double getCommonMode(int ix = 0, int iy = 0) {
        int iroi = getROI(ix, iy);
        /* if (iroi==0) */
        /* 	return 100; */
        /* else */
        /* 	return -100; */
        // cout << "*" << ix << " " << iy << " " << iroi << " " << mean[iroi] <<
        // " " << nCm[iroi]<< endl; if (ix==15 && iy==15) cout << "-" <<
        // mean[iroi]/nCm[iroi] << endl;
        if (iroi >= 0 && iroi < nROI) {
            if (nCm[iroi] > 0)
                return mean[iroi] / nCm[iroi];
        }
        return 0;
    };

    /** gets the common mode i.e. the difference between the current average sum
       of pedestals mode and the average pedestal \param ix pixel x coordinate
        \param iy pixel y coordinate
        \return the difference  between the current average sum of pedestals and
       the average pedestal
    */
    virtual double getCommonModeRMS(int ix = 0, int iy = 0) {
        int iroi = getROI(ix, iy);
        if (iroi >= 0 && iroi < nROI) {
            if (nCm[iroi] > 0)
                return sqrt(mean2[iroi] / nCm[iroi] -
                            (mean[iroi] / nCm[iroi]) *
                                (mean[iroi] / nCm[iroi]));
        }
        return 0;
    };

    /**
        gets the common mode ROI for pixel ix, iy -should be overloaded!
     */
    virtual int getROI(int ix, int iy) {
        (void)ix;
        (void)iy;
        return 0;
    };
    int getNRoi() { return nROI; };

  protected:
    double *mean; /**<array of moving average of the pedestal average per region
                     of interest */
    double *
        mean2; /**< array storing the sum of pedestals per region of interest */
    double *nCm; /**< array storing the number of pixels currently contributing
                    to the pedestals */
    int nsigma;  /** number of rms above which the pedestal should be considered
                    as a photon */
    const int
        nROI; /**< constant parameter for number of regions on which the common
                 mode should be calculated separately e.g. supercolumns */
};

#endif
