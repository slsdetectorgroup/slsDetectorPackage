// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#ifndef ETA_INTERPOLATION_POSXY_H
#define ETA_INTERPOLATION_POSXY_H

//#include "sls/tiffIO.h"
#include "eta2InterpolationBase.h"
#include "eta3InterpolationBase.h"
#include "etaInterpolationBase.h"

class etaInterpolationRosenblatt : public virtual etaInterpolationBase {
  public:
    etaInterpolationRosenblatt(int nx = 400, int ny = 400, int ns = 25, int nsy = 25,
                          int nb = -1, int nby = -1, double emin = 1,
                          double emax = 0)
        : etaInterpolationBase(nx, ny, ns, nsy, nb, nby, emin, emax){
              //  std::cout << "epxy " << nb << " " << emin << " " << emax << std::endl;
              //  std::cout << nbeta << " " << etamin << " " << etamax << std::endl;
          };

    etaInterpolationRosenblatt(etaInterpolationRosenblatt *orig)
        : etaInterpolationBase(orig){};

    virtual etaInterpolationRosenblatt *Clone() = 0; /* { */

    /* return new etaInterpolationPosXY(this); */

    /* }; */

    virtual void prepareInterpolation(int &ok) {
        ok = 1;

        ///*Eta Distribution Rebinning*///
        // double bsize=1./nSubPixels; //precision
        // std::cout<<"nPixelsX = "<<nPixelsX<<" nPixelsY = "<<nPixelsY<<" nSubPixels
        // = "<<nSubPixels<<endl;
        double tot_eta = 0;
        double tot_eta_x = 0;
        double tot_eta_y = 0;
        for (int ip = 0; ip < nbetaX * nbetaY; ip++)
            tot_eta += heta[ip];
        std::cout << "total eta entries is :" << tot_eta << std::endl;
        if (tot_eta <= 0) {
            ok = 0;
            return;
        };

        double *hx = new double[nbetaX];  // profile x
        double *hy = new double[nbetaY];  // profile y
        double *hix = new double[nbetaX]; // integral of projection x
        double *hiy = new double[nbetaY]; // integral of projection y
        //  int ii=0;
        double etax, etay;
        for (int ib = 0; ib < nbetaX; ib++) {

            // tot_eta_y=0;

            for (int iby = 0; iby < nbetaY; iby++) {
                etay = etamin + iby * etastepY;
                // std::cout << etax << std::endl;

                // tot_eta_x+=hx[iby];
                if (etay >= 0 && etay <= 1)
                    hy[iby] = heta[ib + iby * nbetaX];
                else
                    hy[iby] = 0;
                // tot_eta_y+=hy[iby];
            }

            hiy[0] = hy[0];

            for (int iby = 1; iby < nbetaY; iby++) {
                hiy[iby] = hiy[iby - 1] + hy[iby];
            }

            tot_eta_y = hiy[nbetaY - 1] + 1;

            for (int iby = 0; iby < nbetaY; iby++) {
                if (tot_eta_y <= 0) {
                    hhy[ib + iby * nbetaX] = -1;
                    // ii=(ibx*nSubPixels)/nbeta;
                } else {
                    // if (hiy[ibx]>tot_eta_y*(ii+1)/nSubPixels) ii++;
                    hhy[ib + iby * nbetaX] = hiy[iby] / tot_eta_y;
                }
            }
        }

        for (int ib = 0; ib < nbetaY; ib++) {

            for (int ibx = 0; ibx < nbetaX; ibx++) {
                etax = etamin + ibx * etastepX;
                // std::cout << etax << std::endl;
                if (etax >= 0 && etax <= 1)
                    hx[ibx] = heta[ibx + ib * nbetaX];
                else {
                    hx[ibx] = 0;
                }
            }
            hix[0] = hx[0];

            for (int ibx = 1; ibx < nbetaX; ibx++) {
                hix[ibx] = hix[ibx - 1] + hx[ibx];
            }

	    // tot_eta_x = hix[nbetaX - 1] + 1;

            for (int ibx = 0; ibx < nbetaX; ibx++) {
	      //if (tot_eta_x <= 0) {
              //      hhx[ibx + ib * nbetaX] = -1;
              //  } else {
	      hhx[ibx + ib * nbetaX] = hix[ibx];// / tot_eta_x;
	      //}
            }

            
        }


	for (int ib = 0; ib < nbetaY; ib++) {
	  int val=0;
	  for (int ibx = 0; ibx < nbetaX; ibx++) {
	    val+=hhx[ibx + ib * nbetaX];
	  }
	  for (int ibx = 0; ibx < nbetaX; ibx++) {
	    hhx[ibx + ib * nbetaX]=val;
          }    
	}
	tot_eta_x = hhx[nbetaX - 1] + 1;
	for (int ib = 0; ib < nbetaY; ib++) {
	    
	  //tot_eta_x = hix[nbetaX - 1] + 1;

            for (int ibx = 0; ibx < nbetaX; ibx++) {
                if (tot_eta_x <= 0) {
                    hhx[ibx + ib * nbetaX] = -1;
                } else {
                    hhx[ibx + ib * nbetaX] = hhx[ibx + ib * nbetaX] / tot_eta_x;
                }
            }
	}

        int ibx, iby, ib;

        iby = 0;
        while (hhx[iby * nbetaY + nbetaY / 2] < 0)
            iby++;
        for (ib = 0; ib < iby; ib++) {
            for (ibx = 0; ibx < nbetaX; ibx++)
                hhx[ibx + nbetaX * ib] = hhx[ibx + nbetaX * iby];
        }
        iby = nbetaY - 1;

        while (hhx[iby * nbetaY + nbetaY / 2] < 0)
            iby--;
        for (ib = iby + 1; ib < nbetaY; ib++) {
            for (ibx = 0; ibx < nbetaX; ibx++)
                hhx[ibx + nbetaX * ib] = hhx[ibx + nbetaX * iby];
        }

        iby = 0;
        while (hhy[nbetaX / 2 * nbetaX + iby] < 0)
            iby++;
        for (ib = 0; ib < iby; ib++) {
            for (ibx = 0; ibx < nbetaY; ibx++)
                hhy[ib + nbetaX * ibx] = hhy[iby + nbetaX * ibx];
        }
        iby = nbetaX - 1;

        while (hhy[nbetaX / 2 * nbetaX + iby] < 0)
            iby--;
        for (ib = iby + 1; ib < nbetaX; ib++) {
            for (ibx = 0; ibx < nbetaY; ibx++)
                hhy[ib + nbetaX * ibx] = hhy[iby + nbetaX * ibx];
        }

#ifdef SAVE_ALL
        debugSaveAll();
#endif
        delete[] hx;
        delete[] hy;
        delete[] hix;
        delete[] hiy;

        return;
    }
};

class eta2InterpolationRosenblatt : public virtual eta2InterpolationBase,
                               public virtual etaInterpolationRosenblatt {
  public:
    eta2InterpolationRosenblatt(int nx = 400, int ny = 400, int ns = 25,
                           int nsy = 25, int nb = -1, int nby = -1,
                           double emin = 1, double emax = 0)
        : etaInterpolationBase(nx, ny, ns, nsy, nb, nby, emin, emax),
          eta2InterpolationBase(nx, ny, ns, nsy, nb, nby, emin, emax),
          etaInterpolationRosenblatt(nx, ny, ns, nsy, nb, nby, emin, emax){
              //  std::cout << "e2pxy " << nb << " " << emin << " " << emax << std::endl;
          };

    eta2InterpolationRosenblatt(eta2InterpolationRosenblatt *orig)
        : etaInterpolationBase(orig), etaInterpolationRosenblatt(orig){};

    virtual eta2InterpolationRosenblatt *Clone() {
        return new eta2InterpolationRosenblatt(this);
    };
};

class eta3InterpolationRosenblatt : public virtual eta3InterpolationBase,
                               public virtual etaInterpolationRosenblatt {
  public:
    eta3InterpolationRosenblatt(int nx = 400, int ny = 400, int ns = 25,
                           int nsy = 25, int nb = -1, int nby = -1,
                           double emin = 1, double emax = 0)
        : etaInterpolationBase(nx, ny, ns, nsy, nb, nby, emin, emax),
          eta3InterpolationBase(nx, ny, ns, nsy, nb, nby, emin, emax),
          etaInterpolationRosenblatt(nx, ny, ns, nsy, nb, nby, emin, emax){
              //   std::cout << "e3pxy " << nbeta << " " << etamin << " " << etamax
              //   << " " << nSubPixels<< std::endl;
          };

    eta3InterpolationRosenblatt(eta3InterpolationRosenblatt *orig)
        : etaInterpolationBase(orig), etaInterpolationRosenblatt(orig){};

    virtual eta3InterpolationRosenblatt *Clone() {
        return new eta3InterpolationRosenblatt(this);
    };
};

#endif
