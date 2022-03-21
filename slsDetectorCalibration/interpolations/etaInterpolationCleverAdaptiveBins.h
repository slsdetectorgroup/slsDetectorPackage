// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#ifndef ETA_INTERPOLATION_CLEVER_ADAPTIVEBINS_H
#define ETA_INTERPOLATION_CLEVER_ADAPTIVEBINS_H

#include "sls/tiffIO.h"
#include <cmath>
//#include "etaInterpolationBase.h"
#include "etaInterpolationAdaptiveBins.h"

//#define HSIZE 1

class etaInterpolationCleverAdaptiveBins : public etaInterpolationAdaptiveBins {

  private:
    //  double *gradientX, *gradientY, *gradientXY;

    virtual void iterate(float *newhhx, float *newhhy) {

        double bsize = 1. / nSubPixels;

        /* double hy[nSubPixels*HSIZE][nbeta]; //profile y */
        /* double hx[nSubPixels*HSIZE][nbeta]; //profile x */
        //  double hix[nSubPixels*HSIZE][nbeta]; //integral of projection x
        // double hiy[nSubPixels*HSIZE][nbeta]; //integral of projection y
        int ipy, ipx, ippx, ippy;
        // double tot_eta_x[nSubPixels*HSIZE];
        // double tot_eta_y[nSubPixels*HSIZE];

        double mean = 0;
        double maxflat = 0, minflat = 0, maxgradX = 0, mingradX = 0,
               maxgradY = 0, mingradY = 0, maxgr = 0, mingr = 0;

        int ix_maxflat, iy_maxflat, ix_minflat, iy_minflat, ix_maxgrX,
            iy_maxgrX, ix_mingrX, iy_mingrX, ix_maxgrY, iy_maxgrY, ix_mingrY,
            iy_mingrY, ix_mingr, iy_mingr, ix_maxgr, iy_maxgr;
        int maskMin[nSubPixels * nSubPixels], maskMax[nSubPixels * nSubPixels];

        // for (int ipy=0; ipy<nSubPixels; ipy++) {

        for (ipy = 0; ipy < nSubPixels; ipy++) {
            for (ipx = 0; ipx < nSubPixels; ipx++) {
                //	cout << ipx << " " << ipy << endl;
                mean += flat[ipx + nSubPixels * ipy] /
                        ((double)(nSubPixels * nSubPixels));
            }
        }

        //  cout << "Mean is " << mean << endl;

        /*** Find local minima and maxima within the staistical uncertainty **/
        for (ipy = 0; ipy < nSubPixels; ipy++) {
            for (ipx = 0; ipx < nSubPixels; ipx++) {
                if (flat[ipx + nSubPixels * ipy] < mean - 3. * sqrt(mean))
                    maskMin[ipx + nSubPixels * ipy] = 1;
                else
                    maskMin[ipx + nSubPixels * ipy] = 0;
                if (flat[ipx + nSubPixels * ipy] > mean + 3. * sqrt(mean))
                    maskMax[ipx + nSubPixels * ipy] = 1;
                else
                    maskMax[ipx + nSubPixels * ipy] = 0;
                if (ipx > 0 && ipy > 0) {
                    if (flat[ipx + nSubPixels * ipy] <
                        flat[ipx - 1 + nSubPixels * (ipy - 1)])
                        maskMax[ipx + nSubPixels * ipy] = 0;
                    if (flat[ipx + nSubPixels * ipy] >
                        flat[ipx - 1 + nSubPixels * (ipy - 1)])
                        maskMin[ipx + nSubPixels * ipy] = 0;
                }
                if (ipx > 0 && ipy < nSubPixels - 1) {
                    if (flat[ipx + nSubPixels * ipy] <
                        flat[ipx - 1 + nSubPixels * (ipy + 1)])
                        maskMax[ipx + nSubPixels * ipy] = 0;
                    if (flat[ipx + nSubPixels * ipy] >
                        flat[ipx - 1 + nSubPixels * (ipy + 1)])
                        maskMin[ipx + nSubPixels * ipy] = 0;
                }
                if (ipy > 0 && ipx < nSubPixels - 1) {
                    if (flat[ipx + nSubPixels * ipy] <
                        flat[ipx + 1 + nSubPixels * (ipy - 1)])
                        maskMax[ipx + nSubPixels * ipy] = 0;
                    if (flat[ipx + nSubPixels * ipy] >
                        flat[ipx + 1 + nSubPixels * (ipy - 1)])
                        maskMin[ipx + nSubPixels * ipy] = 0;
                }
                if (ipy < nSubPixels - 1 && ipx < nSubPixels - 1) {
                    if (flat[ipx + nSubPixels * ipy] <
                        flat[ipx + 1 + nSubPixels * (ipy + 1)])
                        maskMax[ipx + nSubPixels * ipy] = 0;
                    if (flat[ipx + nSubPixels * ipy] >
                        flat[ipx + 1 + nSubPixels * (ipy + 1)])
                        maskMin[ipx + nSubPixels * ipy] = 0;
                }
                if (ipy < nSubPixels - 1) {
                    if (flat[ipx + nSubPixels * ipy] <
                        flat[ipx + nSubPixels * (ipy + 1)])
                        maskMax[ipx + nSubPixels * ipy] = 0;
                    if (flat[ipx + nSubPixels * ipy] >
                        flat[ipx + nSubPixels * (ipy + 1)])
                        maskMin[ipx + nSubPixels * ipy] = 0;
                }
                if (ipx < nSubPixels - 1) {
                    if (flat[ipx + nSubPixels * ipy] <
                        flat[ipx + 1 + nSubPixels * (ipy)])
                        maskMax[ipx + nSubPixels * ipy] = 0;
                    if (flat[ipx + nSubPixels * ipy] >
                        flat[ipx + 1 + nSubPixels * (ipy)])
                        maskMin[ipx + nSubPixels * ipy] = 0;
                }

                if (ipy > 0) {
                    if (flat[ipx + nSubPixels * ipy] <
                        flat[ipx + nSubPixels * (ipy - 1)])
                        maskMax[ipx + nSubPixels * ipy] = 0;
                    if (flat[ipx + nSubPixels * ipy] >
                        flat[ipx + nSubPixels * (ipy - 1)])
                        maskMin[ipx + nSubPixels * ipy] = 0;
                }

                if (ipx > 0) {
                    if (flat[ipx + nSubPixels * ipy] <
                        flat[ipx - 1 + nSubPixels * (ipy)])
                        maskMax[ipx + nSubPixels * ipy] = 0;
                    if (flat[ipx + nSubPixels * ipy] >
                        flat[ipx - 1 + nSubPixels * (ipy)])
                        maskMin[ipx + nSubPixels * ipy] = 0;
                }

                //	if (maskMin[ipx+nSubPixels*ipy]) cout << ipx << " " <<
                //ipy << " is a local minimum " << flat[ipx+nSubPixels*ipy] <<
                //endl; 	if (maskMax[ipx+nSubPixels*ipy]) cout << ipx << " " <<
                //ipy << " is a local maximum "<< flat[ipx+nSubPixels*ipy] <<
                //endl;
            }
        }
        int is_a_border = 0;

        // initialize the new partition to the previous one
        //  int ibx_p, iby_p, ibx_n, iby_n;
        int ibbx, ibby;

        memcpy(newhhx, hhx, nbeta * nbeta * sizeof(float));
        memcpy(newhhy, hhy, nbeta * nbeta * sizeof(float));

        for (int ibx = 0; ibx < nbeta; ibx++) {
            for (int iby = 0; iby < nbeta; iby++) {

                ippy = hhy[ibx + iby * nbeta] * nSubPixels;
                ippx = hhx[ibx + iby * nbeta] * nSubPixels;

                is_a_border = 0;

                if (maskMin[ippx + nSubPixels * ippy] ||
                    maskMax[ippx + nSubPixels * ippy]) {

                    for (int ix = -1; ix < 2; ix++) {
                        ibbx = ibx + ix;
                        if (ibbx < 0)
                            ibbx = 0;
                        if (ibbx > nbeta - 1)
                            ibbx = nbeta - 1;
                        for (int iy = -1; iy < 2; iy++) {
                            ibby = iby + iy;
                            if (ibby < 0)
                                ibby = 0;
                            if (ibby > nbeta - 1)
                                ibby = nbeta - 1;

                            ipy = hhy[ibbx + ibby * nbeta] * nSubPixels;
                            ipx = hhx[ibbx + ibby * nbeta] * nSubPixels;

                            if (ipx != ippx || ipy != ippy) {
                                is_a_border = 1;
                                if (maskMin[ippx + nSubPixels * ippy]) {
                                    // increase the region
                                    newhhx[ibbx + ibby * nbeta] =
                                        ((double)ippx + 0.5) /
                                        ((double)nSubPixels);
                                    newhhy[ibbx + ibby * nbeta] =
                                        ((double)ippy + 0.5) /
                                        ((double)nSubPixels);
                                }
                                if (maskMax[ippx + nSubPixels * ippy]) {
                                    // reduce the region
                                    newhhx[ibx + iby * nbeta] =
                                        ((double)ipx + 0.5) /
                                        ((double)nSubPixels);
                                    newhhy[ibx + iby * nbeta] =
                                        ((double)ipy + 0.5) /
                                        ((double)nSubPixels);
                                }

                                //	cout << ippx << " " << ippy << " " <<
                                //ibx << " " << iby << " * " << ipx << " " <<
                                //ipy << " " << ibbx << " " << ibby << endl;
                            }
                        }
                    }
                }
            }
        }

        // Check that the resulting histograms are monotonic and they don't have
        // holes!

        for (int ibx = 0; ibx < nbeta - 1; ibx++) {
            for (int iby = 0; iby < nbeta - 1; iby++) {

                ippy = newhhy[ibx + iby * nbeta] * nSubPixels;
                ippx = newhhx[ibx + iby * nbeta] * nSubPixels;

                ipy = newhhy[ibx + (iby + 1) * nbeta] * nSubPixels;
                ipx = newhhx[ibx + 1 + iby * nbeta] * nSubPixels;

                if (ippx > ipx)
                    newhhx[ibx + 1 + iby * nbeta] = newhhx[ibx + iby * nbeta];
                else if (ipx > ippx + 1)
                    newhhx[ibx + 1 + iby * nbeta] =
                        ((double)(ippx + 1 + 0.5)) / ((double)nSubPixels);

                if (ippy > ipy)
                    newhhy[ibx + (iby + 1) * nbeta] = newhhy[ibx + iby * nbeta];
                else if (ipy > ippy + 1)
                    newhhy[ibx + (iby + 1) * nbeta] =
                        ((double)(ippy + 1 + 0.5)) / ((double)nSubPixels);
            }
        }
    }

  public:
    etaInterpolationCleverAdaptiveBins(int nx = 400, int ny = 400, int ns = 25,
                                       int nb = -1, double emin = 1,
                                       double emax = 0)
        : etaInterpolationAdaptiveBins(nx, ny, ns, nb, emin, emax){

          };

    etaInterpolationCleverAdaptiveBins(etaInterpolationCleverAdaptiveBins *orig)
        : etaInterpolationAdaptiveBins(orig){};

    virtual etaInterpolationCleverAdaptiveBins *Clone() = 0;

    /*   return new etaInterpolationCleverAdaptiveBins(this); */

    /* }; */
};

class eta2InterpolationCleverAdaptiveBins
    : public virtual eta2InterpolationBase,
      public virtual etaInterpolationCleverAdaptiveBins {
  public:
    eta2InterpolationCleverAdaptiveBins(int nx = 400, int ny = 400, int ns = 25,
                                        int nb = -1, double emin = 1,
                                        double emax = 0)
        : etaInterpolationBase(nx, ny, ns, nb, emin, emax),
          eta2InterpolationBase(nx, ny, ns, nb, emin, emax),
          etaInterpolationCleverAdaptiveBins(nx, ny, ns, nb, emin, emax){};

    eta2InterpolationCleverAdaptiveBins(
        eta2InterpolationCleverAdaptiveBins *orig)
        : etaInterpolationBase(orig),
          etaInterpolationCleverAdaptiveBins(orig){};

    virtual eta2InterpolationCleverAdaptiveBins *Clone() {
        return new eta2InterpolationCleverAdaptiveBins(this);
    };

    // virtual int *getInterpolatedImage(){return
    // eta2InterpolationBase::getInterpolatedImage();};

    /* virtual int *getInterpolatedImage(){ */
    /*   int ipx, ipy; */
    /*   cout << "ff" << endl; */
    /*   calcDiff(1, hhx, hhy); //get flat */
    /*   double avg=0; */
    /*  for (ipx=0; ipx<nSubPixels; ipx++) */
    /*    for (ipy=0; ipy<nSubPixels; ipy++) */
    /*      avg+=flat[ipx+ipy*nSubPixels]; */
    /*  avg/=nSubPixels*nSubPixels; */

    /*  for (int ibx=0 ; ibx<nSubPixels*nPixelsX; ibx++) { */
    /*    ipx=ibx%nSubPixels-nSubPixels; */
    /*    if (ipx<0) ipx=nSubPixels+ipx; */
    /*    for (int iby=0 ; iby<nSubPixels*nPixelsY; iby++) { */
    /*    ipy=iby%nSubPixels-nSubPixels; */
    /*    if (ipy<0) ipy=nSubPixels+ipy; */

    /*    if (flat[ipx+ipy*nSubPixels]>0) */
    /*      hintcorr[ibx+iby*nSubPixels*nPixelsX]=hint[ibx+iby*nSubPixels*nPixelsX]*(avg/flat[ipx+ipy*nSubPixels]);
     */
    /*    else */
    /*      hintcorr[ibx+iby*nSubPixels*nPixelsX]=hint[ibx+iby*nSubPixels*nPixelsX];
     */

    /*    } */
    /*  } */

    /*   return hintcorr; */
    /* }; */
};

class eta3InterpolationCleverAdaptiveBins
    : public virtual eta3InterpolationBase,
      public virtual etaInterpolationCleverAdaptiveBins {
  public:
    eta3InterpolationCleverAdaptiveBins(int nx = 400, int ny = 400, int ns = 25,
                                        int nb = -1, double emin = 1,
                                        double emax = 0)
        : etaInterpolationBase(nx, ny, ns, nb, emin, emax),
          eta3InterpolationBase(nx, ny, ns, nb, emin, emax),
          etaInterpolationCleverAdaptiveBins(nx, ny, ns, nb, emin, emax){

          };

    eta3InterpolationCleverAdaptiveBins(
        eta3InterpolationCleverAdaptiveBins *orig)
        : etaInterpolationBase(orig),
          etaInterpolationCleverAdaptiveBins(orig){};

    virtual eta3InterpolationCleverAdaptiveBins *Clone() {
        return new eta3InterpolationCleverAdaptiveBins(this);
    };
};

#endif
