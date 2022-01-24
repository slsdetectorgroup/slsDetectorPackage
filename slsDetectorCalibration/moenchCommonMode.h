// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#ifndef MOENCHCOMMONMODE_H
#define MOENCHCOMMONMODE_H

#include "commonModeSubtractionNew.h"

class moenchCommonMode : public commonModeSubtraction {
    /** @short class to calculate the common mode noise for moench02 i.e. on 4
     * supercolumns separately */
  public:
    /** constructor -  initalizes a commonModeSubtraction with 4 different
       regions of interest \param nn number of samples for the moving average
    */

    moenchCommonMode(int nn = 0) : commonModeSubtraction(0){};

    /* /\** add value to common mode as a function of the pixel value,
     * subdividing the region of interest in the 4 supercolumns of 40 columns
     * each; */
    /* 	\param val value to add to the common mode */
    /* 	\param ix pixel coordinate in the x direction */
    /* 	\param iy pixel coordinate in the y direction */
    /* *\/ */
    /* virtual void addToCommonMode(double val, int ix=0, int iy=0) { */
    /*   (void) iy;   */
    /*   int isc=ix/40; */
    /*   if (isc>=0 && isc<nROI) { */
    /* 	cmPed[isc]+=val;  */
    /* 	nCm[isc]++; */
    /*   } */
    /* }; */
    /*  /\**returns common mode value as a function of the pixel value,
     * subdividing the region of interest in the 4 supercolumns of 40 columns
     * each; */
    /* 	\param ix pixel coordinate in the x direction */
    /* 	\param iy pixel coordinate in the y direction */
    /* 	\returns common mode value */
    /* *\/ */
    /* virtual double getCommonMode(int ix=0, int iy=0) { */
    /*   (void) iy; */
    /*   int isc=ix/40; */
    /*   if (isc>=0 && isc<nROI) {  */
    /* 	if (nCm[isc]>0) return cmPed[isc]/nCm[isc]-cmStat[isc].Mean();  */
    /*   } */
    /*   return 0; */
    /* }; */
};

#endif
