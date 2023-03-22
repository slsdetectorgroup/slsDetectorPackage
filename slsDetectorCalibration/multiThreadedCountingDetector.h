// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#ifndef MULTITHREADED_COUNTING_DETECTOR_H
#define MULTITHREADED_COUNTING_DETECTOR_H

#include "multiThreadedAnalogDetector.h"
#include "singlePhotonDetector.h"
//#include <mutex>

using namespace std;

/* class threadedCountingDetector : public threadedAnalogDetector */
/* { */
/* public: */
/*  threadedCountingDetector(singlePhotonDetector *d, int fs=10000) :
 * threadedAnalogDetector(d,fs) {}; */

/* }; */

class multiThreadedCountingDetector : public multiThreadedAnalogDetector {
  public:
 multiThreadedCountingDetector(singlePhotonDetector *d, int n, int fs = 1000)
        : multiThreadedAnalogDetector(d, n, fs){};
    // virtual
    // ~multiThreadedCountingDetector{multiThreadedAnalogDetector::~multiThreadedAnalogDetector();};
    virtual double setNSigma(double n) {
        double ret = (dets[0])->setNSigma(n);
        for (int i = 1; i < nThreads; i++)
            (dets[i])->setNSigma(n);
        return ret;
    };
    virtual void setEnergyRange(double emi, double ema) {
        for (int i = 0; i < nThreads; i++)
            (dets[i])->setEnergyRange(emi, ema);
    };
    virtual void setClusterSize(int sizex, int sizey) {
        for (int i = 0; i < nThreads; i++)
	  ((dets[i]))->setClusterSize(sizex, sizey);
	//std::cout << "+++++++++++++ sizex " << sizex << std::endl;
    };
};

#endif
