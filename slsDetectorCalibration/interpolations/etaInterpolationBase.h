// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#ifndef ETA_INTERPOLATION_BASE_H
#define ETA_INTERPOLATION_BASE_H

#ifdef MYROOT1
#include <TH2D.h>
#include <TH2F.h>
#include <TObject.h>
#include <TTree.h>
#endif
#include "slsInterpolation.h"
#include "sls/tiffIO.h"
#include <cmath>

class etaInterpolationBase : public slsInterpolation {

  public:
    etaInterpolationBase(int nx = 400, int ny = 400, int ns = 25, int nsy = 25,
                         int nb = -1, int nby = -1, double emin = 1,
                         double emax = 0)
        : slsInterpolation(nx, ny, ns, nsy), hhx(NULL), hhy(NULL), heta(NULL),
          nbetaX(nb), nbetaY(nby), etamin(emin), etamax(emax) {
        // std::cout << "eb " << nb << " " << emin << " " << emax << std::endl;
        // std::cout << nb << " " << etamin << " " << etamax << std::endl;
        if (nbetaX <= 0) {
            // std::cout << "aaa:" <<endl;
            nbetaX = nSubPixelsX * 10;
        }
        if (nbetaY <= 0) {
            // std::cout << "aaa:" <<endl;
            nbetaY = nSubPixelsY * 10;
        }
        if (etamin >= etamax) {
            etamin = -1;
            etamax = 2;
        }
        etastepX = (etamax - etamin) / nbetaX;
        etastepY = (etamax - etamin) / nbetaY;
        heta = new int[nbetaX * nbetaY];
        hhx = new float[nbetaX * nbetaY];
        hhy = new float[nbetaX * nbetaY];
        rangeMin = etamin;
        rangeMax = etamax;
        flat = new double[nSubPixelsX * nSubPixelsY];
        hintcorr = new int[nSubPixelsX * nSubPixelsY * nPixelsX * nPixelsY];
    };

    etaInterpolationBase(etaInterpolationBase *orig) : slsInterpolation(orig) {
        nbetaX = orig->nbetaX;
        nbetaY = orig->nbetaY;
        etamin = orig->etamin;
        etamax = orig->etamax;
        rangeMin = orig->rangeMin;
        rangeMax = orig->rangeMax;

        etastepX = (etamax - etamin) / nbetaX;
        etastepY = (etamax - etamin) / nbetaY;
        heta = new int[nbetaX * nbetaY];
        memcpy(heta, orig->heta, nbetaX * nbetaY * sizeof(int));
        hhx = new float[nbetaX * nbetaY];
        memcpy(hhx, orig->hhx, nbetaX * nbetaY * sizeof(float));
        hhy = new float[nbetaX * nbetaY];
        memcpy(hhy, orig->hhy, nbetaX * nbetaY * sizeof(float));
        hintcorr = new int[nSubPixelsX * nSubPixelsY * nPixelsX * nPixelsY];
    };

    virtual void resetFlatField() {
        for (int ibx = 0; ibx < nbetaX * nbetaY; ibx++) {
            heta[ibx] = 0;
            hhx[ibx] = 0;
            hhy[ibx] = 0;
        }
    };

    int *setEta(int *h, int nb = -1, int nby = -1, double emin = 1,
                double emax = 0) {
        if (h) {
            if (heta)
                delete[] heta;
            heta = h;
            nbetaX = nb;
            nbetaY = nby;
            if (nbetaX <= 0)
                nbetaX = nSubPixelsX * 10;
            if (nbetaY <= 0)
                nbetaY = nSubPixelsY * 10;

            etamin = emin;
            etamax = emax;
            if (etamin >= etamax) {
                etamin = -1;
                etamax = 2;
            }
            rangeMin = etamin;
            rangeMax = etamax;
            etastepX = (etamax - etamin) / nbetaX;
            etastepY = (etamax - etamin) / nbetaY;
        }
        return heta;
    };

    int *setFlatField(int *h, int nb = -1, int nby = -1, double emin = 1,
                      double emax = 0) {
        return setEta(h, nb, nby, emin, emax);
    };

    int *getFlatField() { return setEta(NULL); };

    int *getFlatField(int &nb, int &nby, double &emin, double &emax) {
        nb = nbetaX;
        nby = nbetaY;
        emin = etamin;
        emax = etamax;
        return getFlatField();
    };

    void *writeFlatField(const char *imgname) {
        float *gm = NULL;
        gm = new float[nbetaX * nbetaY];
        for (int ix = 0; ix < nbetaX; ix++) {
            for (int iy = 0; iy < nbetaY; iy++) {
                gm[iy * nbetaX + ix] = heta[iy * nbetaX + ix];
            }
        }
        WriteToTiff(gm, imgname, nbetaX, nbetaY);
        delete[] gm;
        return NULL;
    };

    void *readFlatField(const char *imgname, double emin = 1, double emax = 0) {
        if (emax >= 1)
            etamax = emax;
        if (emin <= 0)
            etamin = emin;

        if (etamin >= etamax) {
            etamin = -1;
            etamax = 2;
        }

        etastepX = (etamax - etamin) / nbetaX;
        etastepY = (etamax - etamin) / nbetaY;
        uint32_t nnx;
        uint32_t nny;
        float *gm = ReadFromTiff(imgname, nnx, nny);
        /* if (nnx!=nny) { */
        /*   std::cout << "different number of bins in x " << nnx << "  and y " <<
         * nny<< " !"<< std::endl; */
        /*   std::cout << "Aborting read"<< std::endl; */
        /*   return 0; */
        /* } */
        nbetaX = nnx;
        nbetaY = nny;
        if (gm) {
            if (heta) {
                delete[] heta;
                delete[] hhx;
                delete[] hhy;
            }

            heta = new int[nbetaX * nbetaY];
            hhx = new float[nbetaX * nbetaY];
            hhy = new float[nbetaX * nbetaY];

            for (int ix = 0; ix < nbetaX; ix++) {
                for (int iy = 0; iy < nbetaY; iy++) {
                    heta[iy * nbetaX + ix] = gm[iy * nbetaX + ix];
                }
            }
            delete[] gm;
            return heta;
        }
        return NULL;
    };

    float *gethhx() {
        // hhx->Scale((double)nSubPixels);
        return hhx;
    };

    float *gethhy() {
        // hhy->Scale((double)nSubPixels);
        return hhy;
    };
    virtual int addToFlatFieldDistribution(double etax, double etay) {
#ifdef MYROOT1
        heta->Fill(etax, etay);
#endif
#ifndef MYROOT1
        int ex, ey;
        ex = (etax - etamin) / etastepX;
        ey = (etay - etamin) / etastepY;
        if (ey < nbetaY && ex < nbetaX && ex >= 0 && ey >= 0)
            heta[ey * nbetaX + ex]++;
#endif
        return 0;
    };

    // virtual void prepareInterpolation(int &ok)=0;

    void debugSaveAll(int ind = 0) {
        int ibx, iby;
        char tit[10000];

        float tot_eta = 0;

        float *etah = new float[nbetaX * nbetaY];
        // int etabins=nbeta;
        int ibb = 0;

        for (int ii = 0; ii < nbetaX * nbetaY; ii++) {

            etah[ii] = heta[ii];
            tot_eta += heta[ii];
        }
        sprintf(tit, "/scratch/eta_%d.tiff", ind);
        WriteToTiff(etah, tit, nbetaX, nbetaY);

        for (int ii = 0; ii < nbetaX * nbetaY; ii++) {
            ibb = (hhx[ii] * nSubPixelsX);
            etah[ii] = ibb;
        }
        sprintf(tit, "/scratch/eta_hhx_%d.tiff", ind);
        WriteToTiff(etah, tit, nbetaX, nbetaY);

        for (int ii = 0; ii < nbetaX * nbetaY; ii++) {
            ibb = hhy[ii] * nSubPixelsY;
            etah[ii] = ibb;
        }
        sprintf(tit, "/scratch/eta_hhy_%d.tiff", ind);
        WriteToTiff(etah, tit, nbetaX, nbetaY);

        float *ftest = new float[nSubPixelsX * nSubPixelsY];

        for (int ib = 0; ib < nSubPixelsX * nSubPixelsY; ib++)
            ftest[ib] = 0;

        // int ibx=0, iby=0;

        for (int ii = 0; ii < nbetaX * nbetaY; ii++) {

            ibx = nSubPixelsX * hhx[ii];
            iby = nSubPixelsY * hhy[ii];
            if (ibx < 0)
                ibx = 0;
            if (iby < 0)
                iby = 0;
            if (ibx >= nSubPixelsX)
                ibx = nSubPixelsX - 1;
            if (iby >= nSubPixelsY)
                iby = nSubPixelsY - 1;

            if (ibx >= 0 && ibx < nSubPixelsX && iby >= 0 &&
                iby < nSubPixelsY) {
                //
                // if (ibx>0 && iby>0) std::cout << ibx << " " << iby << " " << ii <<
                // std::endl;
                ftest[ibx + iby * nSubPixelsX] += heta[ii];
            } else
                std::cout << "Bad interpolation " << ii << " " << ibx << " " << iby
                     << std::endl;
        }

        sprintf(tit, "/scratch/ftest_%d.tiff", ind);
        WriteToTiff(ftest, tit, nSubPixelsX, nSubPixelsY);

        // int ibx=0, iby=0;
        tot_eta /= nSubPixelsX * nSubPixelsY;
        int nbad = 0;
        for (int ii = 0; ii < nbetaX * nbetaY; ii++) {
            ibx = nSubPixelsX * hhx[ii];
            iby = nSubPixelsY * hhy[ii];
            if (ftest[ibx + iby * nSubPixelsX] < tot_eta * 0.5f) {
                etah[ii] = 1;
                nbad++;
            } else if (ftest[ibx + iby * nSubPixelsX] > tot_eta * 2.f) {
                etah[ii] = 2;
                nbad++;
            } else
                etah[ii] = 0;
        }
        sprintf(tit, "/scratch/eta_bad_%d.tiff", ind);
        WriteToTiff(etah, tit, nbetaX, nbetaY);
        // std::cout << "Index: " << ind << "\t Bad bins: "<< nbad << std::endl;
        // int ibx=0, iby=0;

        delete[] ftest;
        delete[] etah;
    }

  protected:
    double calcDiff(double avg, float *hx, float *hy) {
        // double p_tot=0;
        double diff = 0, d;
        // double bsize=1./nSubPixels;
        int nbad = 0;
        double *p_tot_x = new double[nSubPixelsX];
        double *p_tot_y = new double[nSubPixelsY];
        double *p_tot = new double[nSubPixelsX * nSubPixelsY];

        double maxdiff = 0, mindiff = avg * nSubPixelsX * nSubPixelsY;

        int ipx, ipy;
        for (ipy = 0; ipy < nSubPixelsY; ipy++) {
            for (ipx = 0; ipx < nSubPixelsX; ipx++) {
                p_tot[ipx + ipy * nSubPixelsX] = 0;
            }
            p_tot_y[ipy] = 0;
            p_tot_x[ipy] = 0;
        }

        for (int ibx = 0; ibx < nbetaX; ibx++) {
            for (int iby = 0; iby < nbetaY; iby++) {
                ipx = hx[ibx + iby * nbetaX] * nSubPixelsX;
                if (ipx < 0)
                    ipx = 0;
                if (ipx >= nSubPixelsX)
                    ipx = nSubPixelsX - 1;

                ipy = hy[ibx + iby * nbetaX] * nSubPixelsY;
                if (ipy < 0)
                    ipy = 0;
                if (ipy >= nSubPixelsY)
                    ipy = nSubPixelsY - 1;

                p_tot[ipx + ipy * nSubPixelsX] += heta[ibx + iby * nbetaX];
                p_tot_y[ipy] += heta[ibx + iby * nbetaX];
                p_tot_x[ipx] += heta[ibx + iby * nbetaX];
            }
        }

        //  std::cout << std::endl << std::endl;
        for (ipy = 0; ipy < nSubPixelsY; ipy++) {
            std::cout.width(5);
            // flat_y[ipy]=p_tot_y[ipy];//avg/nSubPixels;
            for (ipx = 0; ipx < nSubPixelsX; ipx++) {

                //	flat_x[ipx]=p_tot_x[ipx];///avg/nSubPixels;
                flat[ipx + nSubPixelsX * ipy] =
                    p_tot[ipx + nSubPixelsX * ipy]; /// avg;
                d = p_tot[ipx + nSubPixelsX * ipy] - avg;
                if (d < 0)
                    d *= -1.;
                if (d > 5 * sqrt(avg))
                    nbad++;
                diff += d * d;
                if (d < mindiff)
                    mindiff = d;
                if (d > maxdiff)
                    maxdiff = d;
                //	cout << setprecision(4) << p_tot[ipx+nSubPixels*ipy] <<
                //"   ";
            }

            /* std::cout << "** "  << setprecision(4) <<  flat_y[ipy]; */
            // std::cout << "\n";
        }
        /* std::cout << "**" << std::endl; std::cout.width(5); */
        /* for (ipx=0; ipx<nSubPixels; ipx++) { */
        /*   std::cout  << setprecision(4) <<  flat_x[ipx] << " "; */
        /* } */
        // std::cout << "**" << std::endl; std::cout.width(5);
        // std::cout << "Min diff: " << mindiff/sqrt(avg) << " Max diff: " <<
        // maxdiff/sqrt(avg) << " Nbad: " << nbad << std::endl;

        //   std::cout << "Bad pixels: " <<
        //   100.*(float)nbad/((float)(nSubPixels*nSubPixels)) << " %" << std::endl;
        delete[] p_tot_x;
        delete[] p_tot_y;
        delete[] p_tot;

        return sqrt(diff);
    }

    float *hhx;
    float *hhy;
    int *heta;
    int nbetaX, nbetaY;
    double etamin, etamax, etastepX, etastepY;
    double rangeMin, rangeMax;

    double *flat;
    int *hintcorr;
};

#endif
