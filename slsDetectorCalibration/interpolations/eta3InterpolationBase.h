// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#ifndef ETA3_INTERPOLATION_BASE_H
#define ETA3_INTERPOLATION_BASE_H

#ifdef MYROOT1
#include <TH2D.h>
#include <TH2F.h>
#include <TObject.h>
#include <TTree.h>
#endif

#include "etaInterpolationBase.h"

class eta3InterpolationBase : public virtual etaInterpolationBase {

  public:
    eta3InterpolationBase(int nx = 400, int ny = 400, int ns = 25, int nsy = 25,
                          int nb = -1, int nby = -1, double emin = 1,
                          double emax = 0)
        : etaInterpolationBase(nx, ny, ns, nsy, nb, nby, emin, emax){
              //  cout << "e3ib " << nb << " " << emin << " " << emax << endl;
              /* if (nbeta<=0) { */
              /*   nbeta=nSubPixels*10;    */
              /* } */

              //   cout << nbeta << " " << etamin << " " << etamax << endl;
          };

    eta3InterpolationBase(eta3InterpolationBase *orig)
        : etaInterpolationBase(orig){};

    /* virtual eta3InterpolationBase* Clone()=0; */

    // virtual void prepareInterpolation(int &ok){};

    //////////////////////////////////////////////////////////////////////////////
    //////////// /*It return position hit for the event in input */
    /////////////////
    virtual void getInterpolatedPosition(int x, int y, int *data, double &int_x,
                                         double &int_y) {
        double tot;
        double etax, etay;

        int corner = calcEta3(data, etax, etay, tot);

        getInterpolatedPosition(x, y, etax, etay, corner, int_x, int_y);

        return;
    };

    virtual void getInterpolatedPosition(int x, int y, double *data,
                                         double &int_x, double &int_y) {
        // double sDum[2][2];
        double tot;
        double etax, etay;

        int corner = calcEta3(data, etax, etay, tot);

        getInterpolatedPosition(x, y, etax, etay, corner, int_x, int_y);

        return;
    };

    virtual void getInterpolatedPosition(int x, int y, double totquad, int quad,
                                         double *cl, double &int_x,
                                         double &int_y) {

        double etax, etay;
        if (nSubPixelsX > 2 || nSubPixelsY > 2) {
            calcEta3(cl, etax, etay, totquad);
        }
        return getInterpolatedPosition(x, y, etax, etay, quad, int_x, int_y);
    }

    virtual void getInterpolatedPosition(int x, int y, double totquad, int quad,
                                         int *cl, double &int_x,
                                         double &int_y) {

        double etax, etay;
        if (nSubPixelsX > 2 || nSubPixelsY > 2) {
            calcEta3(cl, etax, etay, totquad);
        }
        return getInterpolatedPosition(x, y, etax, etay, quad, int_x, int_y);
    }

    virtual void getInterpolatedPosition(int x, int y, double etax, double etay,
                                         int corner, double &int_x,
                                         double &int_y) {

        double xpos_eta = 0, ypos_eta = 0;
        int ex, ey;

        if (nSubPixelsX > 2 || nSubPixelsY > 2) {

#ifdef MYROOT1
            xpos_eta = (hhx->GetBinContent(hhx->GetXaxis()->FindBin(etax),
                                           hhy->GetYaxis()->FindBin(etay))) /
                       ((double)nSubPixelsX);
            ypos_eta = (hhy->GetBinContent(hhx->GetXaxis()->FindBin(etax),
                                           hhy->GetYaxis()->FindBin(etay))) /
                       ((double)nSubPixelsY);
#endif
#ifndef MYROOT1
            ex = (etax - etamin) / etastepX;
            ey = (etay - etamin) / etastepY;
            if (ex < 0) {
                /* cout << etax << " " << etamin << " "; */
                /*  cout << "3x*"<< ex << endl; */
                ex = 0;
            }
            if (ex >= nbetaX) {
                /* cout << etax << " " << etamin << " "; */
                /* cout << "3x?"<< ex << endl; */
                ex = nbetaX - 1;
            }
            if (ey < 0) {
                /* cout << etay << " " << etamin << " "; */
                /* cout << "3y*"<< ey << endl; */
                ey = 0;
            }
            if (ey >= nbetaY) {
                /* cout << etay << " " << etamin << " "; */
                /* cout << "3y?"<< ey << endl; */
                ey = nbetaY - 1;
            }
            xpos_eta =
                (((double)hhx[(ey * nbetaX + ex)])); ///((double)nSubPixels);
            ypos_eta =
                (((double)hhy[(ey * nbetaX + ex)])); ///((double)nSubPixels);

#endif

        } else {
            switch (corner) {
            case BOTTOM_LEFT:
                xpos_eta = -0.25;
                ypos_eta = -0.25;
                break;
            case BOTTOM_RIGHT:
                xpos_eta = 0.25;
                ypos_eta = -0.25;
                break;
            case TOP_LEFT:
                xpos_eta = -0.25;
                ypos_eta = 0.25;
                break;
            case TOP_RIGHT:
                xpos_eta = 0.25;
                ypos_eta = 0.25;
                break;
            default:
                xpos_eta = 0;
                ypos_eta = 0;
            }
        }
        int_x = ((double)x) + xpos_eta;
        int_y = ((double)y) + ypos_eta;
        // int_x=5. + xpos_eta;
        //  int_y=5. + ypos_eta;
    }

    /*   /////////////////////////////////////////////////////////////////////////////////////////////////
     */
    /*    virtual void getPositionETA3(int x, int y, double *data, double
     * &int_x, double &int_y) */
    /*    { */
    /*     double sDum[2][2]; */
    /*     double tot, totquad; */
    /*     double eta3x,eta3y; */
    /*     double ex,ey; */

    /*     calcQuad(data, tot, totquad, sDum);  */
    /*     calcEta3(data,eta3x, eta3y,tot);  */

    /*     double xpos_eta,ypos_eta; */

    /* #ifdef MYROOT1 */
    /*     xpos_eta=((hhx->GetBinContent(hhx->GetXaxis()->FindBin(eta3x),hhy->GetYaxis()->FindBin(eta3y))))/((double)nSubPixels);
     */
    /*     ypos_eta=((hhy->GetBinContent(hhx->GetXaxis()->FindBin(eta3x),hhy->GetYaxis()->FindBin(eta3y))))/((double)nSubPixels);
     */

    /* #endif */
    /* #ifndef MYROOT1 */
    /*     ex=(eta3x-etamin)/etastep; */
    /*     ey=(eta3y-etamin)/etastep; */

    /*     if (ex<0) ex=0; */
    /*     if (ex>=nbeta) ex=nbeta-1; */
    /*     if (ey<0) ey=0; */
    /*     if (ey>=nbeta) ey=nbeta-1; */

    /*     xpos_eta=(((double)hhx[(int)(ey*nbeta+ex)]))/((double)nSubPixels); */
    /*     ypos_eta=(((double)hhy[(int)(ey*nbeta+ex)]))/((double)nSubPixels); */
    /* #endif */

    /*     int_x=((double)x) + xpos_eta; */
    /*     int_y=((double)y) + ypos_eta; */

    /*     return; */
    /*   }; */

    virtual int addToFlatField(double totquad, int quad, int *cl, double &etax,
                               double &etay) {

        calcEta3(cl, etax, etay, totquad);
        return addToFlatFieldDistribution(etax, etay);
    }

    /* virtual int addToFlatField(double totquad, int quad, double *cl, */
    /*                            double &etax, double &etay) { */

    /*     calcEta3(cl, etax, etay, totquad); */
    /*     return addToFlatField(etax, etay); */
    /* } */

    //////////////////////////////////////////////////////////////////////////////////////
    /* virtual int addToFlatField(double *cluster, double &etax, double &etay) { */
    /*     double totquad; */
    /*     calcEta3(cluster, etax, etay, totquad); */
    /*     return addToFlatField(etax, etay); */
    /* }; */

    /* virtual int addToFlatField(int *cluster, double &etax, double &etay) { */

    /*     double totquad; */

    /*     calcEta3(cluster, etax, etay, totquad); */
    /*     return addToFlatField(etax, etay); */
    /* }; */

/*     virtual int addToFlatField(double etax, double etay) { */
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
