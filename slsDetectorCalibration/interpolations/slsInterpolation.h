// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#ifndef SLS_INTERPOLATION_H
#define SLS_INTERPOLATION_H

#include <cstdlib>
#ifndef MY_TIFF_IO_H
#include "sls/tiffIO.h"
#endif

#ifndef DEF_QUAD
#define DEF_QUAD
enum quadrant {
    TOP_LEFT = 0,
    TOP_RIGHT = 1,
    BOTTOM_LEFT = 2,
    BOTTOM_RIGHT = 3,
    UNDEFINED_QUADRANT = -1
};
#endif
#include <memory.h>

#include <iostream>
#include <stdio.h>
//using namespace std;

//#ifdef MYROOT1
//: public TObject
//#endif
class slsInterpolation {

  public:
    slsInterpolation(int nx = 400, int ny = 400, int ns = 25, int nsy = -1)
        : nPixelsX(nx), nPixelsY(ny), nSubPixelsX(ns), nSubPixelsY(nsy), id(0) {

        if (nSubPixelsY <= 0)
            nSubPixelsY = nSubPixelsX;
        hint = new int[nSubPixelsX * nx * nSubPixelsY * ny];
    };
    virtual ~slsInterpolation() { delete[] hint; }

    slsInterpolation(slsInterpolation *orig) {
        nPixelsX = orig->nPixelsX;
        nPixelsY = orig->nPixelsY;
        nSubPixelsX = orig->nSubPixelsX;
        nSubPixelsY = orig->nSubPixelsY;

        hint = new int[nSubPixelsX * nPixelsX * nSubPixelsY * nPixelsY];
        memcpy(hint, orig->hint,
               nSubPixelsX * nPixelsX * nSubPixelsY * nPixelsY * sizeof(int));
    };

    virtual int setId(int i) {
        id = i;
        return id;
    };

    virtual slsInterpolation *Clone() = 0; /*{
       return new slsInterpolation(this);
       }*/

    int getNSubPixelsX() { return nSubPixelsX; };
    int getNSubPixelsY() { return nSubPixelsY; };
    int getNSubPixels() {
        if (nSubPixelsX == nSubPixelsY)
            return nSubPixelsX;
        else
            return 0;
    };
    void getNSubPixels(int &nsx, int &nsy) {
        nsx = nSubPixelsX;
        nsy = nsx = nSubPixelsY;
    }

    void setNSubPixels(int ns, int nsy = -1) {

        delete[] hint;
        nSubPixelsX = ns;
        if (nsy > 0)
            nSubPixelsY = nsy;
        else
            nSubPixelsY = ns;

        hint = new int[nSubPixelsX * nPixelsX * nSubPixelsY * nPixelsY];

        // return nSubPixels;
    }

    int getImageSize(int &nnx, int &nny, int &nsx, int &nsy) {
        nnx = nSubPixelsX * nPixelsX;
        nny = nSubPixelsY * nPixelsY;
        nsx = nSubPixelsX;
        nsy = nSubPixelsY;
        return nSubPixelsX * nSubPixelsY * nPixelsX * nPixelsY;
    };

    int getImageSize() {
        return nSubPixelsX * nSubPixelsY * nPixelsX * nPixelsY;
    };

    // create eta distribution, eta rebinnining etc.
    // returns flat field image
    virtual void prepareInterpolation(int &ok) = 0;

    // create interpolated image
    // returns interpolated image
    virtual int *getInterpolatedImage() {
        //  cout << "return interpolated image " << endl;
        /* for (int i=0; i<nSubPixels*  nSubPixels* nPixelsX*nPixelsY; i++) { */
        /*   cout << i << " " << hint[i] << endl; */
        /* } */
        return hint;
    };

    void *writeInterpolatedImage(const char *imgname) {
        // cout << "!" <<endl;
        float *gm = NULL;
        int *dummy = getInterpolatedImage();
        gm = new float[nSubPixelsX * nSubPixelsY * nPixelsX * nPixelsY];
        if (gm) {
            for (int ix = 0; ix < nPixelsX * nSubPixelsX; ix++) {
                for (int iy = 0; iy < nPixelsY * nSubPixelsY; iy++) {
                    gm[iy * nPixelsX * nSubPixelsX + ix] =
                        dummy[iy * nPixelsX * nSubPixelsX + ix];
                }
            }
            WriteToTiff(gm, imgname, nSubPixelsY * nPixelsX,
                        nSubPixelsY * nPixelsY);
            delete[] gm;
        } else
	  std::cout << "Could not allocate float image " << std::endl;
        return NULL;
    }

    // return position inside the pixel for the given photon
    virtual void getInterpolatedPosition(int x, int y, double *data,
                                         double &int_x, double &int_y) = 0;
    virtual void getInterpolatedPosition(int x, int y, int *data, double &int_x,
                                         double &int_y) = 0;
    // return position inside the pixel for the given photon
    virtual void getInterpolatedPosition(int x, int y, double etax, double etay,
                                         int quad, double &int_x,
                                         double &int_y) = 0;
    virtual void getInterpolatedPosition(int x, int y, double totquad, int quad,
                                         int *cluster, double &etax,
                                         double &etay) = 0;
    virtual void getInterpolatedPosition(int x, int y, double totquad, int quad,
                                         double *cluster, double &etax,
                                         double &etay) = 0;

    // return position inside the pixel for the given photon
    virtual void clearInterpolatedImage() {

        for (int ix = 0; ix < nPixelsX * nSubPixelsX; ix++) {
            for (int iy = 0; iy < nPixelsY * nSubPixelsY; iy++) {
                hint[iy * nPixelsX * nSubPixelsX + ix] = 0;
            }
        }
    };

    virtual int *addToImage(double int_x, double int_y) {
        int iy = ((double)nSubPixelsY) * int_y;
        int ix = ((double)nSubPixelsX) * int_x;
        if (ix >= 0 && ix < (nPixelsX * nSubPixelsX) &&
            iy < (nSubPixelsY * nPixelsY) && iy >= 0) {
            // cout << int_x << " " << int_y << " " << "  " << ix << " " << iy
            // << " " << ix+iy*nPixelsX*nSubPixels << " " <<
            // hint[ix+iy*nPixelsX*nSubPixels];
            (*(hint + ix + iy * nPixelsX * nSubPixelsX)) += 1;
            // cout << " " << hint[ix+iy*nPixelsX*nSubPixels] << endl;
        } // else
          // cout << "bad! "<< int_x << " " << int_y << " " << "  " << ix << " "
          // << iy << " " << ix+iy*nPixelsX*nSubPixels << endl;

        return hint;
    };

    //virtual int addToFlatField(double *cluster, double &etax, double &etay) = 0;
    //virtual int addToFlatField(int *cluster, double &etax, double &etay) = 0;
    virtual int addToFlatField(double totquad, int quad, int *cl, double &etax,
                               double &etay) = 0;
    //virtual int addToFlatField(double totquad, int quad, double *cluster,
    //                           double &etax, double &etay) = 0;
    virtual int addToFlatFieldDistribution(double etax, double etay) = 0;

    virtual int *getFlatFieldDistribution() { return NULL; };
    virtual int *setFlatField(int *h, int nbx = -1, int nby = -1, 
			      double emin = -1,
                              double emax = -1) {
        return NULL;
    };
    virtual void *writeFlatField(const char *imgname) { return NULL; };
    virtual void *readFlatField(const char *imgname,
				double emin = 1, double emax = 0) {
        return NULL;
    };
    virtual int *getFlatField(int &nbx, int &nby, double &emin, double &emax) {
        nbx = 0;
	nby=0;
        emin = 0;
        emax = 0;
        return getFlatFieldDistribution();
    };

    virtual void resetFlatField() = 0;

    // virtual void Streamer(TBuffer &b);

    static int calcQuad(int *cl, double &sum, double &totquad,
                        double sDum[2][2]) {
        double cli[3 * 3]; //=new int[3*3];
        for (int i = 0; i < 9; i++)
            cli[i] = cl[i];
        return calcQuad(cli, sum, totquad, sDum);
    }

    static int calcQuad(double *cl, double &sum, double &totquad,
                        double sDum[2][2]) {

        int corner = UNDEFINED_QUADRANT;
        /* double *cluster[3]; */
        /* cluster[0]=cl; */
        /* cluster[1]=cl+3; */
        /* cluster[2]=cl+6; */

        sum = 0;
        int xoff = 0, yoff = 0;
#ifndef WRITE_QUAD
        double sumBL = 0;
        double sumTL = 0;
        double sumBR = 0;
        double sumTR = 0;
        for (int ix = 0; ix < 3; ix++) {
            for (int iy = 0; iy < 3; iy++) {
                sum += cl[ix + 3 * iy];
                if (ix <= 1 && iy <= 1)
                    sumBL += cl[ix + iy * 3];
                if (ix <= 1 && iy >= 1)
                    sumTL += cl[ix + iy * 3];
                if (ix >= 1 && iy <= 1)
                    sumBR += cl[ix + iy * 3];
                if (ix >= 1 && iy >= 1)
                    sumTR += cl[ix + iy * 3];
            }
        }

        /* sDum[0][0] = cluster[0][0]; sDum[1][0] = cluster[1][0]; */
        /* sDum[0][1] = cluster[0][1]; sDum[1][1] = cluster[1][1]; */
        corner = BOTTOM_LEFT;
        totquad = sumBL;
        xoff = 0;
        yoff = 0;

        if (sumTL >= totquad) {
            /* #ifdef WRITE_QUAD */
            /*       /\* sDum[0][0] = cluster[1][0]; sDum[1][0] = cluster[2][0];
             * *\/ */
            /*       /\* sDum[0][1] = cluster[1][1]; sDum[1][1] = cluster[2][1];
             * *\/ */
            /*       if (sumTL  ==sum) { */
            /* #endif */
            corner = TOP_LEFT;
            totquad = sumTL;
            xoff = 0;
            yoff = 1;
            /* #ifdef WRITE_QUAD */
            /*       } */
            /* #endif */
        }

        if (sumBR >= totquad) {
            /* sDum[0][0] = cluster[0][1]; sDum[1][0] = cluster[1][1]; */
            /* sDum[0][1] = cluster[0][2]; sDum[1][1] = cluster[1][2]; */

            /* #ifdef WRITE_QUAD */
            /*       /\* sDum[0][0] = cluster[1][0]; sDum[1][0] = cluster[2][0];
             * *\/ */
            /*       /\* sDum[0][1] = cluster[1][1]; sDum[1][1] = cluster[2][1];
             * *\/ */
            /*       if (sumBR   ==sum) { */
            /* #endif */
            corner = BOTTOM_RIGHT;
            xoff = 1;
            yoff = 0;
            totquad = sumBR;
            /* #ifdef WRITE_QUAD */
            /*       } */
            /* #endif */
        }

        if (sumTR >= totquad) {
            /* #ifdef WRITE_QUAD */
            /*       /\* sDum[0][0] = cluster[1][0]; sDum[1][0] = cluster[2][0];
             * *\/ */
            /*       /\* sDum[0][1] = cluster[1][1]; sDum[1][1] = cluster[2][1];
             * *\/ */
            /*       if (sumTR ==sum) { */
            /* #endif */
            xoff = 1;
            yoff = 1;
            /* sDum[0][0] = cluster[1][1]; sDum[1][0] = cluster[2][1]; */
            /* sDum[0][1] = cluster[1][2]; sDum[1][1] = cluster[2][2]; */
            corner = TOP_RIGHT;
            totquad = sumTR;
            /* #ifdef WRITE_QUAD */
            /*       } */
            /* #endif */
        }

#endif
#ifdef WRITE_QUAD

        double sumB = 0;
        double sumT = 0;
        double sumR = 0;
        double sumL = 0;

        for (int ix = 0; ix < 3; ix++) {
            for (int iy = 0; iy < 3; iy++) {
                sum += cl[ix + 3 * iy];
                if (ix < 1)
                    sumL += cl[ix + iy * 3];
                if (ix > 1)
                    sumR += cl[ix + iy * 3];
                if (iy < 1)
		  sumB = cl[ix + iy * 3]; //???? not "+="? VH
                if (iy > 1)
                    sumT += cl[ix + iy * 3];
            }
        }

        totquad = sum;
        if (sumT == 0 && sumR == 0) {
            corner = BOTTOM_LEFT;
            xoff = 0;
            yoff = 0;
        } else if (sumB == 0 && sumR == 0) {
            corner = TOP_LEFT;
            xoff = 0;
            yoff = 1;
        } else if (sumT == 0 && sumL == 0) {
            corner = BOTTOM_RIGHT;
            xoff = 1;
            yoff = 0;
        } else if (sumB == 0 && sumL == 0) {
            xoff = 1;
            yoff = 1;
            corner = TOP_RIGHT;
        } else
            printf("** bad 2x2 cluster!\n");

#endif

        for (int ix = 0; ix < 2; ix++) {
            for (int iy = 0; iy < 2; iy++) {
                sDum[iy][ix] = cl[ix + xoff + (iy + yoff) * 3];
            }
        }

        return corner;
    }

    static int calcEta(double totquad, double sDum[2][2], double &etax,
                       double &etay) {
        double t, r;

        if (totquad > 0) {
            t = sDum[1][0] + sDum[1][1];
            r = sDum[0][1] + sDum[1][1];
            etax = r / totquad;
            etay = t / totquad;
        }
        return 0;
    }

    static int calcEta(double *cl, double &etax, double &etay, double &sum,
                       double &totquad, double sDum[2][2]) {
        int corner = calcQuad(cl, sum, totquad, sDum);
        calcEta(totquad, sDum, etax, etay);

        return corner;
    }

    static int calcEta(int *cl, double &etax, double &etay, double &sum,
                       double &totquad, double sDum[2][2]) {
        int corner = calcQuad(cl, sum, totquad, sDum);
        calcEta(totquad, sDum, etax, etay);

        return corner;
    }

    static int calcEtaL(double totquad, int corner, double sDum[2][2],
                        double &etax, double &etay) {
        double t, r, toth, totv;
        if (totquad > 0) {
            switch (corner) {
            case TOP_LEFT:
                t = sDum[1][1];
                r = sDum[0][1];
                toth = sDum[0][1] + sDum[0][0];
                totv = sDum[0][1] + sDum[1][1];
                break;
            case TOP_RIGHT:
                t = sDum[1][0];
                r = sDum[0][1];
                toth = sDum[0][1] + sDum[0][0];
                totv = sDum[1][0] + sDum[0][0];
                break;
            case BOTTOM_LEFT:
                r = sDum[1][1];
                t = sDum[1][1];
                toth = sDum[1][0] + sDum[1][1];
                totv = sDum[0][1] + sDum[1][1];
                break;
            case BOTTOM_RIGHT:
                t = sDum[1][0];
                r = sDum[1][1];
                toth = sDum[1][0] + sDum[1][1];
                totv = sDum[1][0] + sDum[0][0];
                break;
            default:
                etax = -1000;
                etay = -1000;
                return 0;
            }
            // etax=r/totquad;
            // etay=t/totquad;
            etax = r / toth;
            etay = t / totv;
        }
        return 0;
    }

    static int calcEtaL(double *cl, double &etax, double &etay, double &sum,
                        double &totquad, double sDum[2][2]) {
        int corner = calcQuad(cl, sum, totquad, sDum);
        calcEtaL(totquad, corner, sDum, etax, etay);

        return corner;
    }

    static int calcEtaL(int *cl, double &etax, double &etay, double &sum,
                        double &totquad, double sDum[2][2]) {
        int corner = calcQuad(cl, sum, totquad, sDum);
        calcEtaL(totquad, corner, sDum, etax, etay);

        return corner;
    }

    static int calcEtaC3(double *cl, double &etax, double &etay, double &sum,
                         double &totquad, double sDum[2][2]) {

        int corner = calcQuad(cl, sum, totquad, sDum);
        calcEta(sum, sDum, etax, etay);
        return corner;
    }

    static int calcEtaC3(int *cl, double &etax, double &etay, double &sum,
                         double &totquad, double sDum[2][2]) {

        int corner = calcQuad(cl, sum, totquad, sDum);
        calcEta(sum, sDum, etax, etay);
        return corner;
    }

    static int calcEta3(double *cl, double &etax, double &etay, double &sum) {
        double l = 0, r = 0, t = 0, b = 0, val;
        sum = 0;
        //  int quad;
        for (int ix = 0; ix < 3; ix++) {
            for (int iy = 0; iy < 3; iy++) {
                val = cl[ix + 3 * iy];
                sum += val;
                if (iy == 0)
                    b += val;
                if (iy == 2)
                    t += val;
                if (ix == 0)
                    l += val;
                if (ix == 2)
                    r += val;
            }
        }
        if (sum > 0) {
            etax = (-l + r) / sum;
            etay = (-b + t) / sum;
        }

        if (etax >= 0 && etay >= 0)
            return TOP_RIGHT;
        if (etax < 0 && etay >= 0)
            return TOP_LEFT;
        if (etax < 0 && etay < 0)
            return BOTTOM_LEFT;
        return BOTTOM_RIGHT;
    }

    static int calcEta3(int *cl, double &etax, double &etay, double &sum) {
        double cli[9];
        for (int ix = 0; ix < 9; ix++)
            cli[ix] = cl[ix];

        return calcEta3(cli, etax, etay, sum);
    }

    static int calcEta3X(double *cl, double &etax, double &etay, double &sum) {
        double l, r, t, b;
        sum = cl[0] + cl[1] + cl[2] + cl[3] + cl[4] + cl[5] + cl[6] + cl[7] +
              cl[8];
        if (sum > 0) {
            l = cl[3];
            r = cl[5];
            b = cl[1];
            t = cl[7];
            etax = (-l + r) / sum;
            etay = (-b + t) / sum;
        }
        return -1;
    }

    static int calcEta3X(int *cl, double &etax, double &etay, double &sum) {
        double cli[9];
        for (int ix = 0; ix < 9; ix++)
            cli[ix] = cl[ix];

        return calcEta3X(cli, etax, etay, sum);
    }

    /************************************************/
    /* Additional strixel eta functions by Viktoria */
    /************************************************/
    //Etax: only central row, etay: only central column
    static int calcEta1x3( double* cl, double& etax, double& etay, double& toth, double& totv ) {
      double l, r, t, b;
      //sum = cl[0] + cl[1] + cl[2] + cl[3] + cl[4] + cl[5] + cl[6] + cl[7] + cl[8];
      toth = cl[3] + cl[4] + cl[5];
      if (toth > 0) {
	l = cl[3];
	r = cl[5];
      }
      etax = (-l + r) / toth;
      totv = cl[1] + cl[4] + cl[7];
      if (toth > 0) {
	b = cl[1];
	t = cl[7];
      }
      etay = (-b + t) / totv;
      return -1;
    }

    static int calcEta1x3( int* cl, double& etax, double& etay, double& toth, double& totv ) {
      double cli[9];
      for ( int ix = 0; ix != 9; ++ix )
	cli[ix] = cl[ix];
      return calcEta1x3( cli, etax, etay, toth , totv );
    }

    //Eta 1x2 essentially the same as etaL, but we also return toth and totv
    static int calcEta1x2(double totquad, int corner, double sDum[2][2],
			  double &etax, double &etay, double& toth, double& totv) {
        double t, r;
        if (totquad > 0) {
            switch (corner) {
            case TOP_LEFT:
                t = sDum[1][1];
                r = sDum[0][1];
                toth = sDum[0][1] + sDum[0][0];
                totv = sDum[0][1] + sDum[1][1];
                break;
            case TOP_RIGHT:
                t = sDum[1][0];
                r = sDum[0][1];
                toth = sDum[0][1] + sDum[0][0];
                totv = sDum[1][0] + sDum[0][0];
                break;
            case BOTTOM_LEFT:
                r = sDum[1][1];
                t = sDum[1][1];
                toth = sDum[1][0] + sDum[1][1];
                totv = sDum[0][1] + sDum[1][1];
                break;
            case BOTTOM_RIGHT:
                t = sDum[1][0];
                r = sDum[1][1];
                toth = sDum[1][0] + sDum[1][1];
                totv = sDum[1][0] + sDum[0][0];
                break;
            default:
                etax = -1000;
                etay = -1000;
                return 0;
            }
            // etax=r/totquad;
            // etay=t/totquad;
            etax = r / toth;
            etay = t / totv;
        }
        return 0;
    }

    static int calcEta1x2( double *cl, double &etax, double &etay, double &sum,
			   double &totquad, double sDum[2][2], double& toth, double& totv ) {
        int corner = calcQuad( cl, sum, totquad, sDum );
        calcEta1x2( totquad, corner, sDum, etax, etay, toth, totv );
        return corner;
    }

    static int calcEta1x2( int *cl, double &etax, double &etay, double &sum,
			 double &totquad, double sDum[2][2], double& toth, double& totv ) {
        int corner = calcQuad( cl, sum, totquad, sDum );
        calcEta1x2( totquad, corner, sDum, etax, etay, toth , totv );
        return corner;
    }

    //Two functions to calculate 2x3 or 3x2 eta
    static int calcEta2x3( double* cl, double totquad, int corner, double& etax, double& etay, double& tot6 ) {
      double t, b, r;
      if (totquad > 0) {
	switch (corner) {
	case TOP_LEFT:
	case BOTTOM_LEFT:
	  t = cl[6] + cl[7];
	  b = cl[0] + cl[1];
	  r = cl[1] + cl[4] + cl[7];
	  tot6 = cl[0] + cl[1] + cl[3] + cl[4] + cl[6] + cl[7];
	  break;
	case TOP_RIGHT:
	case BOTTOM_RIGHT:
	  t = cl[7] + cl[8];
	  b = cl[1] + cl[2];
	  r = cl[2] + cl[5] + cl[8];
	  tot6 = cl[1] + cl[2] + cl[4] + cl[5] + cl[7] + cl[8];
	  break;
	default:
	  etax = -1000;
	  etay = -1000;
	  return -1;
	}
	etax = r / tot6;
	etay = (-b + t) / tot6;
      }
      return -1;
    }

    static int calcEta3x2( double* cl, double totquad, int corner, double& etax, double& etay, double& tot6 ) {
      double t, l, r;
      if (totquad > 0) {
	switch (corner) {
	case TOP_LEFT:
	case TOP_RIGHT:
	  l = cl[3] + cl[6];
	  r = cl[5] + cl[8];
	  t = cl[6] + cl[7] + cl[8];
	  tot6 = cl[3] + cl[4] + cl[5] + cl[6] + cl[7] + cl[8];
	  break;
	case BOTTOM_LEFT:
	case BOTTOM_RIGHT:
	  l = cl[0] + cl[3];
	  r = cl[2] + cl[5];
	  t = cl[3] + cl[4] + cl[5];
	  tot6 = cl[0] + cl[1] + cl[2] + cl[3] + cl[4] + cl[5];
	  break;
	default:
	  etax = -1000;
	  etay = -1000;
	  return -1;
	}
	etax = (-l + r) / tot6;
	etay = t / tot6;
      }
      return -1;
    }

    //overload including both eta2x3 and eta3x2
    //ornt (orientation of long side (3) of eta) decides which eta is chosen
    enum orientation {
      HORIZONTAL_ORIENTATION = 0,
      VERTICAL_ORIENTATION = 1, 
      UNDEFINED_ORIENTATION = -1
    };
    static int calcEta2x3( int ornt, double* cl, double& etax, double& etay, double& tot6 ) {
      double sum{};
      double totquad{};
      double sDum[2][2]{};
      int corner = calcQuad( cl, sum, totquad, sDum );
      switch (ornt) {
      case HORIZONTAL_ORIENTATION:
	calcEta3x2( cl, totquad, corner, etax, etay, tot6 );
	break;
      case VERTICAL_ORIENTATION:
	calcEta2x3( cl, totquad, corner, etax, etay, tot6 );
	break;
      default:
	etax = -1000;
	etay = -1000;
	return -1;
      }
      return corner;
    }

    static int calcEta2x3( int strxo, int* cl, double& etax, double& etay, double& tot6 ) {
      double cli[9]{};
      for ( int ix = 0; ix != 9; ++ix )
	cli[ix] = cl[ix];
      return calcEta2x3( strxo, cli, etax, etay, tot6 );
    }

    /* static int calcMyEta(double totquad, int quad, double *cl, double &etax,
     * double &etay) { */
    /*   double l,r,t,b, sum; */
    /*   int yoff; */
    /*   switch (quad) { */
    /*    case BOTTOM_LEFT: */
    /*    case BOTTOM_RIGHT: */
    /*      yoff=0; */
    /*      break; */
    /*    case TOP_LEFT: */
    /*    case TOP_RIGHT: */
    /*      yoff=1; */
    /*      break; */
    /*    default: */
    /*      ; */
    /*    }  */
    /*     l=cl[0+yoff*3]+cl[0+yoff*3+3]; */
    /*     r=cl[2+yoff*3]+cl[2+yoff*3+3]; */
    /*     b=cl[0+yoff*3]+cl[1+yoff*3]*cl[2+yoff*3]; */
    /*     t=cl[0+yoff*3+3]+cl[1+yoff*3+3]*cl[0+yoff*3+3]; */
    /*     sum=t+b; */
    /*   if (sum>0) { */
    /*     etax=(-l+r)/sum; */
    /*     etay=(+t)/sum; */
    /*   } */

    /*   return -1; */
    /* } */

    /* static int calcMyEta(double totquad, int quad, int *cl, double &etax,
     * double &etay) { */
    /*   double l,r,t,b, sum; */
    /*   int yoff; */
    /*   switch (quad) { */
    /*    case BOTTOM_LEFT: */
    /*    case BOTTOM_RIGHT: */
    /*      yoff=0; */
    /*      break; */
    /*    case TOP_LEFT: */
    /*    case TOP_RIGHT: */
    /*      yoff=1; */
    /*      break; */
    /*    default: */
    /*      ; */
    /*    }  */
    /*     l=cl[0+yoff*3]+cl[0+yoff*3+3]; */
    /*     r=cl[2+yoff*3]+cl[2+yoff*3+3]; */
    /*     b=cl[0+yoff*3]+cl[1+yoff*3]*cl[2+yoff*3]; */
    /*     t=cl[0+yoff*3+3]+cl[1+yoff*3+3]*cl[0+yoff*3+3]; */
    /*     sum=t+b; */
    /*   if (sum>0) { */
    /*     etax=(-l+r)/sum; */
    /*     etay=(+t)/sum; */
    /*   } */

    /*   return -1; */
    /* } */

    /* static int calcEta3X(int *cl, double &etax, double &etay, double &sum) {
     */
    /*   double l,r,t,b; */
    /*   sum=cl[0]+cl[1]+cl[2]+cl[3]+cl[4]+cl[5]+cl[6]+cl[7]+cl[8]; */
    /*   if (sum>0) { */
    /*     l=cl[3]; */
    /*     r=cl[5]; */
    /*     b=cl[1]; */
    /*     t=cl[7]; */
    /*     etax=(-l+r)/sum; */
    /*     etay=(-b+t)/sum; */
    /*   } */
    /*   return -1; */
    /* } */

  protected:
    int nPixelsX, nPixelsY;
    int nSubPixelsX, nSubPixelsY;
    int id;
    int *hint;
};

#endif
