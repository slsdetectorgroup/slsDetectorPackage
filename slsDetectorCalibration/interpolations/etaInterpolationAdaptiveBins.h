// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#ifndef ETA_INTERPOLATION_ADAPTIVEBINS_H
#define ETA_INTERPOLATION_ADAPTIVEBINS_H

#include "sls/tiffIO.h"
#include <cmath>
//#include "etaInterpolationBase.h"
#include "etaInterpolationPosXY.h"

class etaInterpolationAdaptiveBins : public etaInterpolationPosXY {

    // protected:

  private:
    virtual void iterate(float *newhhx, float *newhhy) {

        double bsize = 1. / nSubPixels;

        double hy[nSubPixels][nbeta];  // profile y
        double hx[nSubPixels][nbeta];  // profile x
        double hix[nSubPixels][nbeta]; // integral of projection x
        double hiy[nSubPixels][nbeta]; // integral of projection y
        int ipy, ipx;
        double tot_eta_x[nSubPixels];
        double tot_eta_y[nSubPixels];
        // for (int ipy=0; ipy<nSubPixels; ipy++) {

        for (ipy = 0; ipy < nSubPixels; ipy++) {
            for (int ibx = 0; ibx < nbeta; ibx++) {
                hx[ipy][ibx] = 0;
                hy[ipy][ibx] = 0;
            }
        }

        // cout << ipy << " " << ((ipy)*bsize) << " " << ((ipy+1)*bsize) <<
        // endl;
        for (int ibx = 0; ibx < nbeta; ibx++) {
            for (int iby = 0; iby < nbeta; iby++) {
                ipy = hhy[ibx + iby * nbeta] * nSubPixels;
                if (ipy < 0)
                    ipy = 0;
                if (ipy >= nSubPixels)
                    ipy = nSubPixels - 1;
                hx[ipy][ibx] += heta[ibx + iby * nbeta];

                ipx = hhx[ibx + iby * nbeta] * nSubPixels;
                if (ipx < 0)
                    ipx = 0;
                if (ipx >= nSubPixels)
                    ipx = nSubPixels - 1;
                hy[ipx][iby] += heta[ibx + iby * nbeta];
            }
        }

        for (ipy = 0; ipy < nSubPixels; ipy++) {
            hix[ipy][0] = hx[ipy][0];
            hiy[ipy][0] = hy[ipy][0];
            for (int ib = 1; ib < nbeta; ib++) {
                hix[ipy][ib] = hix[ipy][ib - 1] + hx[ipy][ib];
                hiy[ipy][ib] = hiy[ipy][ib - 1] + hy[ipy][ib];
            }
            tot_eta_x[ipy] = hix[ipy][nbeta - 1] + 1;
            tot_eta_y[ipy] = hiy[ipy][nbeta - 1] + 1;
            // cout << ipy << " " << tot_eta_x[ipy] << " " << tot_eta_y[ipy] <<
            // endl;
        }
        // for (int ipy=0; ipy<nSubPixels; ipy++) {

        for (int ibx = 0; ibx < nbeta; ibx++) {
            for (int iby = 0; iby < nbeta; iby++) {

                //   if ( hhy[ibx+iby*nbeta]>=((ipy)*bsize) &&
                //   hhy[ibx+iby*nbeta]<=((ipy+1)*bsize)) {
                ipy = hhy[ibx + iby * nbeta] * nSubPixels;

                if (ipy < 0)
                    ipy = 0;
                if (ipy >= nSubPixels)
                    ipy = nSubPixels - 1;

                if (ipy >= 0 && ipy < nSubPixels)
                    if (tot_eta_x[ipy] > 0)
                        newhhx[ibx + iby * nbeta] =
                            hix[ipy][ibx] / (tot_eta_x[ipy]);
                    else
                        cout << "Bad tot_etax " << ipy << " " << tot_eta_x[ipy]
                             << endl;
                else
                    cout << "** Bad value ipy " << ibx << " " << iby << " "
                         << ipy << " " << hhy[ibx + iby * nbeta] * nSubPixels
                         << endl;
                // if (newhhx[ibx+iby*nbeta]>=1 || newhhx[ibx+iby*nbeta]<0 )
                // cout << "***"<< ibx << " " << iby << newhhx[ibx+iby*nbeta] <<
                // endl; if (ipy==3 && ibx==10) cout << newhhx[ibx+iby*nbeta] <<
                // " " << hix[ibx] << " " << ibx+iby*nbeta << endl;
                // }
                ipy = hhx[ibx + iby * nbeta] * nSubPixels;
                // if (hhx[ibx+iby*nbeta]>=((ipy)*bsize) &&
                // hhx[ibx+iby*nbeta]<=((ipy+1)*bsize)) {
                if (ipy < 0)
                    ipy = 0;
                if (ipy >= nSubPixels)
                    ipy = nSubPixels - 1;

                if (ipy >= 0 && ipy < nSubPixels)
                    if (tot_eta_y[ipy] > 0)
                        newhhy[ibx + iby * nbeta] =
                            hiy[ipy][iby] / (tot_eta_y[ipy]);
                    else
                        cout << "Bad tot_etay " << ipy << " " << tot_eta_y[ipy]
                             << endl;
                else
                    cout << "** Bad value ipx " << ibx << " " << iby << " "
                         << ipy << " " << hhx[ibx + iby * nbeta] * nSubPixels
                         << endl;
                // if (newhhy[ibx+iby*nbeta]>=1 || newhhy[ibx+iby*nbeta]<0 )
                // cout << "***"<< ibx << " " << iby << newhhy[ibx+iby*nbeta] <<
                // endl;
                //  if (ipy==3 && iby==10) cout << newhhy[ibx+iby*nbeta] << " "
                //  << hiy[iby] << " " << ibx+iby*nbeta << endl;
                //   }
            }
        }
        // }
    }

  public:
    etaInterpolationAdaptiveBins(int nx = 400, int ny = 400, int ns = 25,
                                 int nb = -1, double emin = 1, double emax = 0)
        : etaInterpolationPosXY(nx, ny, ns, nb, emin, emax){
              //   flat=new double[nSubPixels*nSubPixels]; flat_x=new
              //   double[nSubPixels]; flat_y=new double[nSubPixels];
              //    flat=new double[nSubPixels*nSubPixels];
          };

    etaInterpolationAdaptiveBins(etaInterpolationAdaptiveBins *orig)
        : etaInterpolationPosXY(orig) {
        hintcorr = new int[nPixelsX * nPixelsY * nSubPixels];
    };

    virtual etaInterpolationAdaptiveBins *Clone() = 0;

    /*   return new etaInterpolationAdaptiveBins(this); */

    /* }; */

    virtual void prepareInterpolation(int &ok) {
        prepareInterpolation(ok, 1000);
    }

    virtual void prepareInterpolation(int &ok, int nint) {
        ok = 1;

        ///*Eta Distribution Rebinning*///
        double bsize = 1. / nSubPixels; // precision
        // cout<<"nPixelsX = "<<nPixelsX<<" nPixelsY = "<<nPixelsY<<" nSubPixels
        // = "<<nSubPixels<<endl;
        double tot_eta = 0;
        double tot_eta_x = 0;
        double tot_eta_y = 0;
        for (int ip = 0; ip < nbeta * nbeta; ip++)
            tot_eta += heta[ip];
        if (tot_eta <= 0) {
            ok = 0;
            return;
        };

        double hx[nbeta];  // profile x
        double hy[nbeta];  // profile y
        double hix[nbeta]; // integral of projection x
        double hiy[nbeta]; // integral of projection y
        int ii = 0;

        /** initialize distribution to linear interpolation */
        // for (int ibx=0; ibx<nbeta; ibx++) {
        //  for (int ib=0; ib<nbeta; ib++) {
        //    hhx[ibx+ib*nbeta]=((float)ibx)/((float)nbeta);
        //    hhy[ibx+ib*nbeta]=((float)ib)/((float)nbeta);
        //  }
        // }

        etaInterpolationPosXY::prepareInterpolation(ok);

        double thr = 1. / ((double)nSubPixels);
        double avg = tot_eta / ((double)(nSubPixels * nSubPixels));
        double rms = sqrt(tot_eta);
        cout << "total eta entries is :" << tot_eta << " avg: " << avg
             << " rms: " << sqrt(tot_eta) << endl;
        double old_diff = calcDiff(avg, hhx, hhy), new_diff = old_diff + 1,
               best_diff = old_diff;
        // cout << " chi2= " << old_diff << " (rms= " << sqrt(tot_eta) << ")" <<
        // endl;
        cout << endl;
        cout << endl;
        debugSaveAll(0);
        int iint = 0;
        float *newhhx = new float[nbeta * nbeta]; // profile x
        float *newhhy = new float[nbeta * nbeta]; // profile y
        float *besthhx = hhx;                     // profile x
        float *besthhy = hhy;                     // profile y

        cout << "Iteration " << iint << " Chi2: " << old_diff
             << endl; //" Best: "<< best_diff << " RMS: "<< rms<< endl;
        while (iint < nint && best_diff > rms) {

            /* #ifdef SAVE_ALL */
            /*      if (iint%10==0) */
            /*        debugSaveAll(iint); */
            /* #endif */
            //  cout << "Iteration " << iint << endl;
            iterate(newhhx, newhhy);
            new_diff = calcDiff(avg, newhhx, newhhy);
            //   cout << " chi2= " << new_diff << " (rms= " << sqrt(tot_eta) <<
            //   ")"<<endl;

            if (new_diff < best_diff) {
                best_diff = new_diff;
                besthhx = newhhx;
                besthhy = newhhy;
            }

            if (hhx != besthhx)
                delete[] hhx;
            if (hhy != besthhy)
                delete[] hhy;

            hhx = newhhx;
            hhy = newhhy;

#ifdef SAVE_ALL
            if (new_diff <= best_diff) {
                debugSaveAll(iint);
            }
#endif

            newhhx = new float[nbeta * nbeta]; // profile x
            newhhy = new float[nbeta * nbeta]; // profile y

            /* if (new_diff<old_diff){ */
            /*   cout << "best difference at iteration "<< iint << " (" <<
             * new_diff << " < " << old_diff << ")"<< "Best: "<< best_diff << "
             * RMS: "<< sqrt(tot_eta) << endl; */
            /*   ; */
            /* } else {  */
            // break;
            // }

            old_diff = new_diff;

            iint++;
            cout << "Iteration " << iint << " Chi2: " << new_diff
                 << endl; //" Best: "<< best_diff << " RMS: "<< rms<< endl;
        }
        delete[] newhhx;
        delete[] newhhy;

        if (hhx != besthhx)
            delete[] hhx;
        if (hhy != besthhy)
            delete[] hhy;

        hhx = besthhx;
        hhy = besthhy;

        cout << "Iteration " << iint << " Chi2: " << best_diff
             << endl; //" Best: "<< best_diff << " RMS: "<< rms<< endl;
#ifdef SAVE_ALL
        debugSaveAll(iint);
#endif

        return;
    }
};

class eta2InterpolationAdaptiveBins
    : public virtual eta2InterpolationBase,
      public virtual etaInterpolationAdaptiveBins {
  public:
    eta2InterpolationAdaptiveBins(int nx = 400, int ny = 400, int ns = 25,
                                  int nb = -1, double emin = 1, double emax = 0)
        : etaInterpolationBase(nx, ny, ns, nb, emin, emax),
          eta2InterpolationBase(nx, ny, ns, nb, emin, emax),
          etaInterpolationAdaptiveBins(nx, ny, ns, nb, emin, emax) {
        cout << "NSUBPIX is " << ns << " " << nSubPixels << endl;
        //  cout << "e2pxy " << nb << " " << emin << " " << emax << endl;
    };

    eta2InterpolationAdaptiveBins(eta2InterpolationAdaptiveBins *orig)
        : etaInterpolationBase(orig), etaInterpolationAdaptiveBins(orig){};

    virtual eta2InterpolationAdaptiveBins *Clone() {
        return new eta2InterpolationAdaptiveBins(this);
    };
};

class eta3InterpolationAdaptiveBins
    : public virtual eta3InterpolationBase,
      public virtual etaInterpolationAdaptiveBins {
  public:
    eta3InterpolationAdaptiveBins(int nx = 400, int ny = 400, int ns = 25,
                                  int nb = -1, double emin = 1, double emax = 0)
        : etaInterpolationBase(nx, ny, ns, nb, emin, emax),
          eta3InterpolationBase(nx, ny, ns, nb, emin, emax),
          etaInterpolationAdaptiveBins(nx, ny, ns, nb, emin, emax) {
        cout << "e3pxy " << nbeta << " " << etamin << " " << etamax << " "
             << nSubPixels << endl;
    };

    eta3InterpolationAdaptiveBins(eta3InterpolationAdaptiveBins *orig)
        : etaInterpolationBase(orig), etaInterpolationAdaptiveBins(orig){};

    virtual eta3InterpolationAdaptiveBins *Clone() {
        return new eta3InterpolationAdaptiveBins(this);
    };
};

#endif
