// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#ifndef SINGLEPHOTONDETECTOR_H
#define SINGLEPHOTONDETECTOR_H

#include "analogDetector.h"

#include "single_photon_hit.h"

//#define MYROOT1

#ifdef MYROOT1
#include <TTree.h>
#endif

#ifndef EVTYPE_DEF
#define EVTYPE_DEF
/** enum to define the even types */
enum eventType {
    PEDESTAL = 0, /** pedestal */
    NEIGHBOUR =
        1, /** neighbour i.e. below threshold, but in the cluster of a photon */
    PHOTON = 2,     /** photon i.e. above threshold */
    PHOTON_MAX = 3, /** maximum of a cluster satisfying the photon conditions */
    NEGATIVE_PEDESTAL =
        4, /** negative value, will not be accounted for as pedestal in order to
              avoid drift of the pedestal towards negative values */
    UNDEFINED_EVENT = -1 /** undefined */
};
#endif

// template <class dataType> class singlePhotonDetector :
// public analogDetector<dataType> {
class singlePhotonDetector : public analogDetector<uint16_t> {

    /** @short class to perform pedestal subtraction etc. and find single photon
     * clusters for an analog detector */

  public:
    /**

       Constructor (no error checking if datasize and offsets are compatible!)
       \param d detector data structure to be used
       \param csize cluster size (should be an odd number). Defaults to 3
       \param nsigma number of rms to discriminate from the noise. Defaults to 5
       \param sign 1 if photons are positive, -1 if  negative
       \param cm common mode subtraction algorithm, if any. Defaults to NULL
       i.e. none \param nped number of samples for pedestal averaging \param nd
       number of dark frames to average as pedestals without photon
       discrimination at the beginning of the measurement


    */

    singlePhotonDetector(slsDetectorData<uint16_t> *d, int csize = 3,
                         double nsigma = 5, int sign = 1,
                         commonModeSubtraction *cm = NULL, int nped = 1000,
                         int nd = 100, int nnx = -1, int nny = -1,
                         double *gm = NULL, ghostSummation<uint16_t> *gs = NULL)
        : analogDetector<uint16_t>(d, sign, cm, nped, nnx, nny, gm, gs),
          nDark(nd), eventMask(NULL), nSigma(nsigma), eMin(-1), eMax(-1),
          clusterSize(csize), clusterSizeY(csize), c2(1), c3(1), clusters(NULL),
          quad(UNDEFINED_QUADRANT), tot(0), quadTot(0) {

        fm = new pthread_mutex_t;
	pthread_mutex_init(fm, NULL);

        eventMask = new eventType *[ny];
        //  val=new double*[ny];
        for (int i = 0; i < ny; i++) {
            eventMask[i] = new eventType[nx];
            // val[i]=new double[nx];
        }

        if (ny == 1)
            clusterSizeY = 1;

        c2 = sqrt((clusterSizeY + 1) / 2 * (clusterSize + 1) / 2);
        c3 = sqrt(clusterSizeY * clusterSize);
        // cluster=new single_photon_hit(clusterSize,clusterSizeY);
        clusters = new single_photon_hit[nx * ny];

        //  cluster=clusters;
        setClusterSize(csize);
        nphTot = 0;
        nphFrame = 0;
    };
    /**
       destructor. Deletes the cluster structure, the pdestalSubtraction and the
       image array
    */
    virtual ~singlePhotonDetector() {
        delete[] clusters;
        for (int i = 0; i < ny; i++)
            delete[] eventMask[i];
        delete[] eventMask;
    };

    /**
       copy constructor
       \param orig detector to be copied

    */

    singlePhotonDetector(singlePhotonDetector *orig)
        : analogDetector<uint16_t>(orig) {

        nDark = orig->nDark;
        myFile = orig->myFile;

        eventMask = new eventType *[ny];
        // val=new double*[ny];
        for (int i = 0; i < ny; i++) {
            eventMask[i] = new eventType[nx];
            // val[i]=new double[nx];
        }
        eMin = orig->eMin;
        eMax = orig->eMax;

        nSigma = orig->nSigma;
        clusterSize = orig->clusterSize;
        clusterSizeY = orig->clusterSizeY;
        // cluster=new single_photon_hit(clusterSize,clusterSizeY);

        c2 = sqrt((clusterSizeY + 1) / 2 * (clusterSize + 1) / 2);
        c3 = sqrt(clusterSizeY * clusterSize);

        clusters = new single_photon_hit[nx * ny];

        // cluster=clusters;

        setClusterSize(clusterSize);
        fm = orig->fm;

        quad = UNDEFINED_QUADRANT;
        tot = 0;
        quadTot = 0;
        gmap = orig->gmap;
        nphTot = 0;
        nphFrame = 0;
        nphTot = 0;
        nphFrame = 0;
    }

    /**
       duplicates the detector structure
       \returns new single photon detector with same parameters

    */
    virtual singlePhotonDetector *Clone() {
        return new singlePhotonDetector(this);
    }
    /** sets/gets number of rms threshold to detect photons
        \param n number of sigma to be set (0 or negative gets)
        \returns actual number of sigma parameter
    */
    double setNSigma(double n = -1) {
        if (n >= 0)
            nSigma = n;
        return nSigma;
    }

    /** sets/gets cluster size
        \param n cluster size to be set, (0 or negative gets). If even is
       incremented by 1. \returns actual cluster size
    */
    int setClusterSize(int n = -1) {
        if (n > 0 && n != clusterSize) {
            if (n % 2 == 0)
                n += 1;
            clusterSize = n;
            //	if (clusters)
            //  delete [] clusters;
            if (ny > clusterSize)
                clusterSizeY = clusterSize;
            else
                clusterSizeY = 1;
            for (int ip = 0; ip < nx * ny; ip++) {
                (clusters + ip)->set_cluster_size(clusterSize, clusterSizeY);
	    }
            // cluster=new single_photon_hit(clusterSize,clusterSizeY);
        }
        return clusterSize;
    }

    /**
       converts the image into number of photons
       \param data pointer to data
       \param nph pointer where to add the calculated photons. If NULL, the
       internal image will be used \returns array with data converted into
       number of photons.
    */

    virtual int *getNPhotons(char *data, int *nph = NULL) {
        // cout << "spc frame" << endl;
        nphFrame = 0;
        double val;
        if (nph == NULL)
            nph = image;
        // nph=new int[nx*ny];

        // int cy=(clusterSizeY+1)/2; //quad size
        // int cs=(clusterSize+1)/2; //quad size

        // int ccs=clusterSize; //cluster size
        // int ccy=clusterSizeY; //cluster size

        // double g=1.;

        double tthr = thr, tthr1, tthr2;
        int nn = 0;
        double max = 0, tl = 0, tr = 0, bl = 0, br = 0, v;
        double rms = 0;
        int cm = 0;
        if (cmSub)
            cm = 1;

        if (thr > 0) {
            // cy=1;
            // cs=1;
            // ccs=1;
            // ccy=1;
        }
        if (iframe < nDark) {
            //	cout << "ped " << iframe << endl;
            // this already adds to common mode
            addToPedestal(data);
            return nph;
        } else {
            if (thr > 0) {
                double *rest = new double[ny * nx];
                newFrame(data);
                if (cmSub) {
		  //cout << "add to common mode?" << endl;
                    addToCommonMode(data);
                }
                for (int iy = ymin; iy < ymax; ++iy) {
                    for (int ix = xmin; ix < xmax; ++ix) {
                        if (det->isGood(ix, iy)) {
                            val = subtractPedestal(data, ix, iy, cm);

                            nn = analogDetector<uint16_t>::convertToPhotons(
                                data, ix, iy); // val/thr;//
                            if (nn > 0) {
                                nph[ix + nx * iy] += nn;
                                rest[iy * nx + ix] =
                                    (val - nn * thr); //?+0.5*thr
                                nphFrame += nn;
                                nphTot += nn;
                            } else
                                rest[iy * nx + ix] = val;
                        }
                    }
                }

                for (int iy = ymin; iy < ymax; ++iy) {
                    for (int ix = xmin; ix < xmax; ++ix) {

                        if (det->isGood(ix, iy)) {
                            eventMask[iy][ix] = PEDESTAL;
                            max = 0;
                            tl = 0;
                            tr = 0;
                            bl = 0;
                            br = 0;
                            tot = 0;
                            quadTot = 0;

                            if (rest[iy * nx + ix] > 0.25 * thr) {
                                eventMask[iy][ix] = NEIGHBOUR;
                                for (int ir = -(clusterSizeY / 2);
                                     ir < (clusterSizeY / 2) + 1; ir++) {
                                    for (int ic = -(clusterSize / 2);
                                         ic < (clusterSize / 2) + 1; ic++) {
                                        if ((iy + ir) >= 0 && (iy + ir) < ny &&
                                            (ix + ic) >= 0 && (ix + ic) < nx) {
                                            // clusters->set_data(rest[iy+ir][ix+ic],
                                            // ic, ir);

                                            v = rest
                                                [(iy + ir) * nx + ix +
                                                 ic]; // clusters->get_data(ic,ir);
                                            tot += v;

                                            if (ir <= 0 && ic <= 0)
                                                bl += v;
                                            if (ir <= 0 && ic >= 0)
                                                br += v;
                                            if (ir >= 0 && ic <= 0)
                                                tl += v;
                                            if (ir >= 0 && ic >= 0)
                                                tr += v;

                                            if (v > max) {
                                                max = v;
                                            }
                                            // if (ir==0 && ic==0) {
                                            //}
                                        }
                                    }

                                    if (rest[iy * nx + ix] >= max) {
                                        if (bl >= br && bl >= tl && bl >= tr) {
                                            quad = BOTTOM_LEFT;
                                            quadTot = bl;
                                        } else if (br >= bl && br >= tl &&
                                                   br >= tr) {
                                            quad = BOTTOM_RIGHT;
                                            quadTot = br;
                                        } else if (tl >= br && tl >= bl &&
                                                   tl >= tr) {
                                            quad = TOP_LEFT;
                                            quadTot = tl;
                                        } else if (tr >= bl && tr >= tl &&
                                                   tr >= br) {
                                            quad = TOP_RIGHT;
                                            quadTot = tr;
                                        }

                                        if (nSigma == 0) {
                                            tthr = thr;
                                            tthr1 = thr;
                                            tthr2 = thr;
                                        } else {

                                            rms = getPedestalRMS(ix, iy);
                                            tthr = nSigma * rms;

                                            tthr1 = nSigma *
                                                    sqrt(clusterSize *
                                                         clusterSizeY) *
                                                    rms;
                                            tthr2 =
                                                nSigma *
                                                sqrt(
                                                    (clusterSize + 1) / 2. *
                                                    ((clusterSizeY + 1) / 2.)) *
                                                rms;

                                            if (thr > 2 * tthr)
                                                tthr = thr - tthr;
                                            if (thr > 2 * tthr1)
                                                tthr1 = tthr - tthr1;
                                            if (thr > 2 * tthr2)
                                                tthr2 = tthr - tthr2;
                                        }

                                        if (tot > tthr1 || quadTot > tthr2 ||
                                            max > tthr) {
                                            eventMask[iy][ix] = PHOTON;
                                            nph[ix + nx * iy]++;
                                            rest[iy * nx + ix] -= thr;
                                            nphFrame++;
                                            nphTot++;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                delete[] rest;
            } else
                return getClusters(data, nph);
        }
        return NULL;
    };

    /**
       Loops in the region of interest to find the clusters
       \param data pointer to the data structure
       \returns number of clusters found

     */

    int *getClusters(char *data, int *ph = NULL) {

        int nph = 0;
        // const int cy=(clusterSizeY+1)/2;
        // const int cs=(clusterSize+1)/2;

        // int ir, ic;
        eventType ee;
        double max = 0, tl = 0, tr = 0, bl = 0, br = 0, *v;
        int cm = 0;
        int good = 1;
        int ir, ic;
        // double quadTot;
        // quadrant quad;
        double rms;

        if (ph == NULL)
            ph = image;

        if (iframe < nDark) {
            addToPedestal(data);
            return 0;
        }
        newFrame(data);

        if (cmSub) {
            addToCommonMode(data);
            cm = 1;
        }

        double *val = new double[ny * nx];

        for (int iy = ymin; iy < ymax; ++iy) {
            for (int ix = xmin; ix < xmax; ++ix) {
                if (det->isGood(ix, iy) == 0)
                    continue;

                max = 0;
                tl = 0;
                tr = 0;
                bl = 0;
                br = 0;
                tot = 0;
                quadTot = 0;
                quad = UNDEFINED_QUADRANT;

                // eventMask[iy][ix]
                ee = PEDESTAL;

                rms = getPedestalRMS(ix, iy);
                // cluster=clusters+nph;

                // cout << ix << " " << iy << endl;
                for (ir = -(clusterSizeY / 2); ir < (clusterSizeY / 2) + 1;
                     ir++) {
                    for (ic = -(clusterSize / 2); ic < (clusterSize / 2) + 1;
                         ic++) {

                        if ( (iy + ir) >= 0 && (iy + ir) < ny &&
                            (ix + ic) >= 0 && (ix + ic) < nx) {


			  if ((iy + ir) > iy && (ix + ic) > ix ) {
                            
			    val[(iy + ir) * nx + ix + ic] =
			      subtractPedestal(data, ix + ic, iy + ir, cm);
			    
			    
			  }
			  v = &(val[(iy + ir) * nx + ix + ic]);
			  tot += *v;
			  if (ir <= 0 && ic <= 0)
			    bl += *v;
			  if (ir <= 0 && ic >= 0)
			    br += *v;
			  if (ir >= 0 && ic <= 0)
			    tl += *v;
			  if (ir >= 0 && ic >= 0)
			  tr += *v;
			  if (*v > max) //{
			    max = *v;
			  //}
                        }
                    }
                }
                /* if (ix==50 && iy==50) */
                /*   cout << id << " " << ix << " " << iy << " " <<
                 * det->getValue(data,ix,iy)<< " "  << val[iy][ix] << " " <<
                 * getPedestal(ix,iy) << " " << rms << endl;  */
                if (val[iy * nx + ix] < -nSigma * rms) {
                    ee = NEGATIVE_PEDESTAL;
                    continue;
                }
                if (max > nSigma * rms) {
                    //	  cout << "ph1 " << max << " " << nSigma*rms << endl;
                    ee = PHOTON;
                    if (val[iy * nx + ix] < max)
                        continue;
                } else if (tot > c3 * nSigma * rms) {
                    //	  cout << "ph3 " << tot << " " << c3*nSigma*rms << endl;
                    ee = PHOTON;
                }
#ifndef WRITE_QUAD
                else {
#endif
                    quad = BOTTOM_RIGHT;
                    quadTot = br;
                    if (bl >= quadTot) {
                        quad = BOTTOM_LEFT;
                        quadTot = bl;
                    }
                    if (tl >= quadTot) {
                        quad = TOP_LEFT;
                        quadTot = tl;
                    }
                    if (tr >= quadTot) {
                        quad = TOP_RIGHT;
                        quadTot = tr;
                    }

                    if (quadTot > c2 * nSigma * rms) {
                        //   cout << "ph2 " << quadTot << " " << c2*nSigma*rms
                        //   << endl;
                        ee = PHOTON;
                    }

#ifndef WRITE_QUAD
                }
#endif
                if (ee == PHOTON && val[iy * nx + ix] == max) {
                    good = 1;
                    ee = PHOTON_MAX;
                    // cout << "**" <<id<< " " << iframe << " "  << nDark << " "
                    // << ix << " " << iy << " " << rms << " " << max << " " <<
                    // quadTot << " " << tot << endl;
                    (clusters + nph)->tot = tot;
                    (clusters + nph)->x = ix;
                    (clusters + nph)->y = iy;
                    (clusters + nph)->quad = quad;
                    (clusters + nph)->quadTot = quadTot;
                    //(clusters+nph)->rms=rms;
                    //	  (clusters+nph)->iframe=det->getFrameNumber(data);
                    // cout << det->getFrameNumber(data) << " " <<
                    // (clusters+nph)->iframe << endl;
                    //  (clusters+nph)->ped=getPedestal(ix,iy,0);
                    for (ir = -(clusterSizeY / 2); ir < (clusterSizeY / 2) + 1;
                         ir++) {
                        for (ic = -(clusterSize / 2);
                             ic < (clusterSize / 2) + 1; ic++) {
                            if ((iy + ir) >= 0 && (iy + ir) < ny &&
                                (ix + ic) >= 0 && (ix + ic) < nx) {
			      (clusters + nph)
                                    ->set_data(val[(iy + ir) * nx + ix + ic],
                                               ic, ir);
			      if (val[(iy + ir) * nx + ix + ic]>max) 
				good=0;
			    }
                        }
                    }
		    if (good==0) {
		      (clusters + nph)->print();
		      cout << max << " " <<  val[iy * nx + ix] << endl;
		    }
		    //else (clusters + nph)->print();
                    if (eMin > 0 && tot < eMin)
                        good = 0;
                    if (eMax > 0 && tot > eMax)
                        good = 0;
                    if (good) {
                        nph++;
                        image[iy * nx + ix]++;
                    }

                } else if (ee == PEDESTAL) {
                    addToPedestal(data, ix, iy, cm);
                } /*else {
                    eventMask[iy][ix]=PHOTON;
                    }*/
                  // eventMask[iy][ix]=ee;
            }
        }

        nphFrame = nph;
        nphTot += nph;
        // cout << nphFrame << endl;
        // cout <<id << " **********************************"<< iframe << " " <<
        // det->getFrameNumber(data) << " " << nphFrame << endl;
        writeClusters(det->getFrameNumber(data));
        delete[] val;
        return image;
    };

    /**<
       returns the total signal in a cluster
       \param size cluser size  should be 1,2 or 3
       \returns cluster center if size=1, sum of the maximum quadrant if size=2,
       total of the cluster if size=3 or anything else
    */

    double getClusterTotal(int size) {
        switch (size) {
        case 1:
            return getClusterElement(0, 0);
        case 2:
            return quadTot;
        default:
            return tot;
        };
    };

    /**<
     retrurns the quadrant with maximum signal
     \returns quadrant where the cluster is located    */

    quadrant getQuadrant() { return quad; };

    /** returns value for cluster element in relative coordinates
        \param ic x coordinate (center is (0,0))
        \param ir y coordinate (center is (0,0))
        \returns cluster element
    */
    double getClusterElement(int ic, int ir = 0) {
        return clusters->get_data(ic, ir);
    };

    /** returns event mask for the given pixel
        \param ic x coordinate (center is (0,0))
        \param ir y coordinate (center is (0,0))
        \returns event mask enum for the given pixel
    */
    eventType getEventMask(int ic, int ir = 0) { return eventMask[ir][ic]; };

#ifdef MYROOT1
    /** generates a tree and maps the branches
        \param tname name for the tree
        \param iFrame pointer to the frame number
        \returns returns pointer to the TTree
    */
    TTree *initEventTree(char *tname, int *iFrame = NULL) {
        TTree *tall = new TTree(tname, tname);

        if (iFrame)
            tall->Branch("iFrame", iFrame, "iframe/I");
        else
            tall->Branch("iFrame", &(clusters->iframe), "iframe/I");

        tall->Branch("x", &(clusters->x), "x/I");
        tall->Branch("y", &(clusters->y), "y/I");
        char tit[100];
        sprintf(tit, "data[%d]/D", clusterSize * clusterSizeY);
        tall->Branch("data", clusters->data, tit);
        tall->Branch("pedestal", &(clusters->ped), "pedestal/D");
        tall->Branch("rms", &(clusters->rms), "rms/D");
        tall->Branch("tot", &(clusters->tot), "tot/D");
        tall->Branch("quadTot", &(clusters->quadTot), "quadTot/D");
        tall->Branch("quad", &(clusters->quad), "quad/I");
        return tall;
    };
#else
    /** write cluster to filer
         \param f file pointer
    */
    void writeCluster(FILE *f) { clusters->write(f); };

    /**
        write clusters to file
        \param f file pointer
        \param clusters array of clusters structures
        \param nph number of clusters to be written to file

    */

    static void writeClusters(FILE *f, single_photon_hit *cl, int nph,
                              int fn = 0) {
        if (f) {
            if (nph > 0) {
#ifndef OLDFORMAT
                if (fwrite((void *)&fn, 1, sizeof(int), f))
                    if (fwrite((void *)&nph, 1, sizeof(int), f))
#endif
                        for (int i = 0; i < nph; i++)
                            (cl + i)->write(f);
            }
        }
    };

    void writeClusters(FILE *f, int fn = 0) {
        writeClusters(f, clusters, nphFrame, fn);
        // for (int i=0; i<nphFrame; i++)
        //(clusters+i)->write(f);
    };
    void writeClusters(int fn) {
        if (myFile) {
            // cout << "++" << endl;
            pthread_mutex_lock(fm);
            //   cout <<"**********************************"<< fn << " " <<
            //   nphFrame << endl;
            writeClusters(myFile, clusters, nphFrame, fn);
            // for (int i=0; i<nphFrame; i++)
            //  (clusters+i)->write(myFile);
            pthread_mutex_unlock(fm);
            // cout << "--" << endl;
        }
    };
#endif

    virtual void processData(char *data, int *val = NULL) {
        // cout << "sp" << endl;
        switch (fMode) {
        case ePedestal:
            // cout <<"spc add to ped " << endl;
            addToPedestal(data, 1);
            break;
        default:
            switch (dMode) {
            case eAnalog:
                analogDetector<uint16_t>::processData(data, val);
                break;
            default:
                // cout <<"spc " << endl;
                getNPhotons(data, val);
            }
        }
        iframe++;
        //	cout << "done" << endl;
    };
    int getPhFrame() { return nphFrame; };
    int getPhTot() { return nphTot; };

    void setEnergyRange(double emi, double ema) {
        eMin = emi;
        eMax = ema;
    };
    void getEnergyRange(double &emi, double &ema) {
        emi = eMin;
        ema = eMax;
    };

    void setMutex(pthread_mutex_t *m) { fm = m; };

  protected:
    int nDark; /**< number of frames to be used at the beginning of the dataset
                  to calculate pedestal without applying photon discrimination
                */
    eventType **eventMask; /**< matrix of event type or each pixel */
    double nSigma; /**< number of sigma parameter for photon discrimination */
    double eMin, eMax;
    int clusterSize;  /**< cluster size in the x direction */
    int clusterSizeY; /**< cluster size in the y direction i.e. 1 for strips,
                         clusterSize for pixels */
    double c2, c3;
    //  single_photon_hit *cluster; /**< single photon hit data structure */
    single_photon_hit *clusters; /**< single photon hit data structure */
    quadrant quad;               /**< quadrant where the photon is located */
    double tot;                  /**< sum of the 3x3 cluster */
    double quadTot;              /**< sum of the maximum 2x2cluster */
    int nphTot;
    int nphFrame;

    //    double **val;
    pthread_mutex_t *fm;
};

#endif
