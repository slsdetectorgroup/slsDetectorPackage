// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#ifndef ETA2_INTERPOLATION_BASE_H
#define ETA2_INTERPOLATION_BASE_H

#ifdef MYROOT1
#include <TH2D.h>
#include <TH2F.h>
#include <TObject.h>
#include <TTree.h>
#endif

#include "etaInterpolationBase.h"

class eta2InterpolationBase : public virtual etaInterpolationBase {

  public:
    eta2InterpolationBase(int nx = 400, int ny = 400, int ns = 25, int nsy = 25,
                          int nb = -1, int nby = -1, double emin = 1,
                          double emax = 0)
        : etaInterpolationBase(nx, ny, ns, nsy, nb, nby, emin, emax){

              /* if (etamin>=etamax) { */
              /*   etamin=-1; */
              /*   etamax=2;  */
              /*   //  std::cout << ":" <<endl; */
              /* } */
              /* etastep=(etamax-etamin)/nbeta; */

          };

    eta2InterpolationBase(eta2InterpolationBase *orig)
        : etaInterpolationBase(orig){};

    //////////////////////////////////////////////////////////////////////////////
    //////////// /*It return position hit for the event in input */
    /////////////////
    virtual void getInterpolatedPosition(int x, int y, int *data, double &int_x,
                                         double &int_y) {
        double sDum[2][2];
        double tot, totquad;
        double etax = 0, etay = 0;

        int corner;
        corner = calcQuad(data, tot, totquad, sDum);
        if (nSubPixelsX > 2 || nSubPixelsY > 2)
            calcEta(totquad, sDum, etax, etay);
        getInterpolatedPosition(x, y, etax, etay, corner, int_x, int_y);

        return;
    };

    virtual void getInterpolatedPosition(int x, int y, double *data,
                                         double &int_x, double &int_y) {
        double sDum[2][2];
        double tot, totquad;
        double etax = 0, etay = 0;

        int corner;
        corner = calcQuad(data, tot, totquad, sDum);
        if (nSubPixelsX > 2 || nSubPixelsY > 2)
            calcEta(totquad, sDum, etax, etay);
        getInterpolatedPosition(x, y, etax, etay, corner, int_x, int_y);

        return;
    };

    virtual void getInterpolatedPosition(int x, int y, double totquad, int quad,
                                         double *cl, double &int_x,
                                         double &int_y) {

        double cc[2][2];
        int xoff = 0, yoff = 0;
        switch (quad) {
        case BOTTOM_LEFT:
            xoff = 0;
            yoff = 0;
            break;
        case BOTTOM_RIGHT:
            xoff = 1;
            yoff = 0;
            break;
        case TOP_LEFT:
            xoff = 0;
            yoff = 1;
            break;
        case TOP_RIGHT:
            xoff = 1;
            yoff = 1;
            break;
        default:;
        }
        double etax = 0, etay = 0;
        if (nSubPixelsX > 2 || nSubPixelsY > 2) {
            cc[0][0] = cl[xoff + 3 * yoff];
            cc[1][0] = cl[xoff + 3 * (yoff + 1)];
            cc[0][1] = cl[xoff + 1 + 3 * yoff];
            cc[1][1] = cl[xoff + 1 + 3 * (yoff + 1)];
            calcEta(totquad, cc, etax, etay);
        }

        return getInterpolatedPosition(x, y, etax, etay, quad, int_x, int_y);
    }

    virtual void getInterpolatedPosition(int x, int y, double totquad, int quad,
                                         int *cl, double &int_x,
                                         double &int_y) {

        double cc[2][2];
        int xoff = 0, yoff = 0;

        switch (quad) {
        case BOTTOM_LEFT:
            xoff = 0;
            yoff = 0;
            break;
        case BOTTOM_RIGHT:
            xoff = 1;
            yoff = 0;
            break;
        case TOP_LEFT:
            xoff = 0;
            yoff = 1;
            break;
        case TOP_RIGHT:
            xoff = 1;
            yoff = 1;
            break;
        default:;
        }
        double etax = 0, etay = 0;
        if (nSubPixelsX > 2 || nSubPixelsY > 2) {
            cc[0][0] = cl[xoff + 3 * yoff];
            cc[1][0] = cl[xoff + 3 * (yoff + 1)];
            cc[0][1] = cl[xoff + 1 + 3 * yoff];
            cc[1][1] = cl[xoff + 1 + 3 * (xoff + 1)];
            calcEta(totquad, cc, etax, etay);
        }
        return getInterpolatedPosition(x, y, etax, etay, quad, int_x, int_y);
    }

    virtual void getInterpolatedPosition(int x, int y, double etax, double etay,
                                         int corner, double &int_x,
                                         double &int_y) {

        double xpos_eta = 0, ypos_eta = 0;
        double dX, dY;
        int ex, ey;
        switch (corner) {
        case TOP_LEFT:
            dX = -1.;
            dY = 0;
            break;
        case TOP_RIGHT:;
            dX = 0;
            dY = 0;
            break;
        case BOTTOM_LEFT:
            dX = -1.;
            dY = -1.;
            break;
        case BOTTOM_RIGHT:
            dX = 0;
            dY = -1.;
            break;
        default:
            std::cout << "bad quadrant" << std::endl;
            dX = 0.;
            dY = 0.;
        }

        if (nSubPixelsX > 2 || nSubPixelsY > 2) {

            ex = (etax - etamin) / etastepX;
            ey = (etay - etamin) / etastepY;
            if (ex < 0) {
                std::cout << "x*" << ex << std::endl;
                ex = 0;
            }
            if (ex >= nbetaX) {
                std::cout << "x?" << ex << std::endl;
                ex = nbetaX - 1;
            }
            if (ey < 0) {
                std::cout << "y*" << ey << " " << nbetaY << std::endl;
                ey = 0;
            }
            if (ey >= nbetaY) {
                std::cout << "y?" << ey << " " << nbetaY << std::endl;
                ey = nbetaY - 1;
            }

            xpos_eta = (((double)hhx[(ey * nbetaX + ex)])) +
                       dX; ///((double)nSubPixels);
            ypos_eta = (((double)hhy[(ey * nbetaX + ex)])) +
                       dY; ///((double)nSubPixels);

        } else {
            xpos_eta = 0.5 * dX + 0.25;
            ypos_eta = 0.5 * dY + 0.25;
        }

        int_x = ((double)x) + xpos_eta + 0.5;
        int_y = ((double)y) + ypos_eta + 0.5;
    }

    virtual int addToFlatField(double totquad, int quad, int *cl, double &etax,
                               double &etay) {
        double cc[2][2];
        int xoff = 0, yoff = 0;

        switch (quad) {
        case BOTTOM_LEFT:
            xoff = 0;
            yoff = 0;
            break;
        case BOTTOM_RIGHT:
            xoff = 1;
            yoff = 0;
            break;
        case TOP_LEFT:
            xoff = 0;
            yoff = 1;
            break;
        case TOP_RIGHT:
            xoff = 1;
            yoff = 1;
            break;
        default:;
        }
        cc[0][0] = cl[xoff + 3 * yoff];
        cc[1][0] = cl[xoff + 3 * (yoff + 1)];
        cc[0][1] = cl[xoff + 1 + 3 * yoff];
        cc[1][1] = cl[xoff + 1 + 3 * (yoff + 1)];

        // calcMyEta(totquad,quad,cl,etax, etay);
        calcEta(totquad, cc, etax, etay);

        //     std::cout <<"******"<< etax << " " << etay << std::endl;

        return addToFlatFieldDistribution(etax, etay);
    }

    /* virtual int addToFlatField(double totquad, int quad, double *cl, */
    /*                            double &etax, double &etay) { */
    /*     double cc[2][2]; */
    /*     int xoff = 0, yoff = 0; */

    /*     switch (quad) { */
    /*     case BOTTOM_LEFT: */
    /*         xoff = 0; */
    /*         yoff = 0; */
    /*         break; */
    /*     case BOTTOM_RIGHT: */
    /*         xoff = 1; */
    /*         yoff = 0; */
    /*         break; */
    /*     case TOP_LEFT: */
    /*         xoff = 0; */
    /*         yoff = 1; */
    /*         break; */
    /*     case TOP_RIGHT: */
    /*         xoff = 1; */
    /*         yoff = 1; */
    /*         break; */
    /*     default:; */
    /*     } */
    /*     cc[0][0] = cl[xoff + 3 * yoff]; */
    /*     cc[1][0] = cl[(yoff + 1) * 3 + xoff]; */
    /*     cc[0][1] = cl[yoff * 3 + xoff + 1]; */
    /*     cc[1][1] = cl[(yoff + 1) * 3 + xoff + 1]; */

    /*     /\* std::cout << cl[0] << " " << cl[1] << " " << cl[2] << std::endl;   *\/ */
    /*     /\* std::cout << cl[3] << " " << cl[4] << " " << cl[5] << std::endl;   *\/ */
    /*     /\* std::cout << cl[6] << " " << cl[7] << " " << cl[8] << std::endl;   *\/ */
    /*     /\* std::cout <<"******"<<totquad << " " << quad << std::endl;  *\/ */
    /*     /\* std::cout << cc[0][0]<< " " << cc[0][1] << std::endl;  *\/ */
    /*     /\* std::cout << cc[1][0]<< " " << cc[1][1] << std::endl;  *\/ */
    /*     // calcMyEta(totquad,quad,cl,etax, etay); */
    /*     calcEta(totquad, cc, etax, etay); */

    /*     //     std::cout <<"******"<< etax << " " << etay << std::endl; */

    /*     return addToFlatFieldDistribution(etax, etay); */
    /* } */

    //////////////////////////////////////////////////////////////////////////////////////
    /* virtual int addToFlatField(double *cluster, double &etax, double &etay) { */
    /*     double sDum[2][2]; */
    /*     double tot, totquad; */
    /*     // int corner; */
    /*     // corner= */
    /*     calcQuad(cluster, tot, totquad, sDum); */

    /*     // double xpos_eta,ypos_eta; */
    /*     // double dX,dY; */

    /*     calcEta(totquad, sDum, etax, etay); */

    /*     return addToFlatField(etax, etay); */
    /* }; */

    /* virtual int addToFlatField(int *cluster, double &etax, double &etay) { */
    /*     double sDum[2][2]; */
    /*     double tot, totquad; */
    /*     // int corner; */
    /*     // corner= */
    /*     calcQuad(cluster, tot, totquad, sDum); */

    /*     // double xpos_eta,ypos_eta; */
    /*     // double dX,dY; */

    /*     calcEta(totquad, sDum, etax, etay); */

    /*     return addToFlatField(etax, etay); */
    /* }; */

/*     virtual int addToFlatFieldDistribution(double etax, double etay) { */
/* #ifdef MYROOT1 */
/*         heta->Fill(etax, etay); */
/* #endif */
/* #ifndef MYROOT1 */
/*         int ex, ey; */
/*         ex = (etax - etamin) / etastepX; */
/*         ey = (etay - etamin) / etastepY; */
/*         if (ey < nbetaY && ex < nbetaX && ex >= 0 && ey >= 0) */
/*             heta[ey * nbetaX + ex]++; */
/* #endif */
/*         return 0; */
/*     }; */

    virtual int *getInterpolatedImage() {
        int ipx, ipy;
        // std::cout << "ff" << std::endl;
        calcDiff(1, hhx, hhy); // get flat
        double avg = 0;
        for (ipx = 0; ipx < nSubPixelsX; ipx++)
            for (ipy = 0; ipy < nSubPixelsY; ipy++)
                avg += flat[ipx + ipy * nSubPixelsX];
        avg /= nSubPixelsY * nSubPixelsX;

        for (int ibx = 0; ibx < nSubPixelsX * nPixelsX; ibx++) {
            ipx = ibx % nSubPixelsX - nSubPixelsX / 2;
            if (ipx < 0)
                ipx = nSubPixelsX + ipx;
            for (int iby = 0; iby < nSubPixelsY * nPixelsY; iby++) {
                ipy = iby % nSubPixelsY - nSubPixelsY / 2;
                if (ipy < 0)
                    ipy = nSubPixelsY + ipy;
                // std::cout << ipx << " " << ipy << " " << ibx << " " << iby <<
                // std::endl;
                if (flat[ipx + ipy * nSubPixelsX] > 0)
                    hintcorr[ibx + iby * nSubPixelsX * nPixelsX] =
                        hint[ibx + iby * nSubPixelsX * nPixelsX] *
                        (avg / flat[ipx + ipy * nSubPixelsX]);
                else
                    hintcorr[ibx + iby * nSubPixelsX * nPixelsX] =
                        hint[ibx + iby * nSubPixelsX * nPixelsX];
            }
        }

        return hintcorr;
    };

    /*  protected: */

    /* #ifdef MYROOT1 */
    /*   TH2D *heta; */
    /*   TH2D *hhx; */
    /*   TH2D *hhy; */
    /* #endif */
    /* #ifndef MYROOT1 */
    /*   int *heta; */
    /*   float *hhx; */
    /*   float *hhy; */
    /* #endif */
    /*   int nbeta; */
    /*   double etamin, etamax, etastep; */
};

#endif
