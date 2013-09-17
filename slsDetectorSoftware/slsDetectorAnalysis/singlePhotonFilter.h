  /********************************************//**
 * @file singlePhotonFilter.h
 * @short single photon filter using trees
 ***********************************************/
#ifndef SINGLEPHOTONFILTER_H
#define SINGLEPHOTONFILTER_H

//#include "sls_detector_defs.h"


#ifdef __CINT
#define MYROOT
#endif


#ifdef MYROOT
#include <TFile.h>
#include <TMath.h>
#include <TTree.h>
#include <TChain.h>
#endif

#include <stdio.h>
#include <iostream>
#include <deque>
#include <list>
#include <queue>
#include <fstream>


#include "runningStat.h"
#include "movingStat.h"
using namespace std;


typedef  double double32_t;
typedef  float float32_t;
typedef  int int32_t;


/**
    return values
*/
enum  {
  OK, /**< function succeeded */
  FAIL /**< function failed */
};
/**
    @short structure for a single photon hit
*/
typedef struct{
	vector < vector<double> > data;		/**< data size */
	int 	x; 			/**< x-coordinate of the center of hit */
	int		y; 			/**< x-coordinate of the center of hit */
	double	rms; 		/**< noise of central pixel */
	double 	ped; 		/**< pedestal of the central pixel */
	int		iframe; 	/**< frame number */
}single_photon_hit;




/**
   @short class handling trees and its data file
 */

class singlePhotonFilter{
public:
	/**
	 * Constructor
	 * @param nChannelsX Number of Channels in X direction
	 * @param nChannelsY Number of Channels in Y direction
	 * @param m Map to data without headers
	 * @param s mask as to which adcs are inverted
	 * @param d Size of data with the headers
	 */

	/** why is the datasize -1, you need to know the datasize with headers  so that you dont go over the limits */
	singlePhotonFilter(int x, int y, vector <vector<int16_t> >m, vector <vector<int16_t> >s, int d = -1);
	/*map[56][63]=656; data[map[56][63]] ^ 0x7fff*/

	/** virtual destructor */
	virtual ~singlePhotonFilter(){};

	/**
	 * Construct a tree, populate struct for the single photon hit
	 * @param outdir Output file directory
	 * @param fname Output file name
	 */
	void initTree(char *outdir, char *fname);

	/**
	 * Reset Indices before starting acquisition and provide all the masks and offsets
	 * @param fmask frame index mask
	 * @param pmask packet index mask
	 * @param foffset frame index offset
	 * @param poffset packet index offset
	 * @param pperf packets per frame
	 */
	void initialize(int fmask, int pmask, int foffset, int poffset, int pperf);

	/** Verify if all packets exist for the frame
	 * @param inData the data from socket to be verified
	 * @param inDataSize datasize of packet
	 * @param myData frame with all the packets
	 * @param firstTime the first frame received from socket
	 * */
	int verifyFrame(char *inData, int inDataSize, int16_t* myData, int firstTime);

	/**
	 * Writes tree/struct to file
	 * returns OK if successful, else FAIL
	 */
	int writeToFile();

	/**
	 * Find Hits frame by frame
	 * @param myData data for one frame
	 * @param myDataSize data size for one frame with headers
	 * returns number of hits
	 */
	int findHits(int16_t *myData, int myDataSize);



#ifdef MYROOT
	/**
	 * returns tree
	 */
	TTree *getTree(){ return myTree; };
#endif

	/**
	 * returns struct
	 */
	single_photon_hit getStructure(){ return myPhotonHit; };

	/** Set number of frames to calculate pedestal at beginning */
	void setNPed(int n){ nped = n; };

	/** Get number of frames to calculate pedestal at beginning */
	int getNPed(){return nped;};

	/** Set Distance from pedestal to detect a hit */
	void setNSigma(int n){ nsigma = n; };

	/** Get Distance from pedestal to detect a hit */
	int getNSigma(){return nsigma;};

	/** Set background */
	void setNBackground(int n){ nbackground = n; };

	/** Get background */
	int getNBackground(){return nbackground;};

	/** Set correction */
	void setOutCorr(double d){ outcorr = d; };

	/** Get correction */
	double getOutCorr(){return outcorr;};




private:

#ifdef MYROOT
	/** Tree where the hits are stored */
	TTree *myTree;

	/** File where the tree is saved */
	TFile *myFile;
#else
    FILE *myFile;
#endif

	/** Number of Channels in X  direction */
	int nChannelsX;

	/** Number of Channels in Y direction */
	int nChannelsY;

	/** Cluster size in X direction */
	int nClusterX;

	/** Cluster size in Y direction */
	int nClusterY;

	/** map to the data without headers */
	vector <vector<int16_t> >map;

	/** Size of data with headers */
	int dataSize;

	/** mask as to which adcs are inverted */
	vector <vector<int16_t> >mask;

	/** movingStat object */
	vector <vector<movingStat> > stat;

	/** single Photon Hit structure */
	single_photon_hit myPhotonHit;

	/** Cluster size */
	const static int CLUSTER_SIZE = 3;

	/** Default Number of frames at the beginning to calculate pedestal */
	const static int DEFAULT_NUM_PEDESTAL = 500;

	/** Default Distance from pedestal to detect a hit */
	const static int DEFAULT_SIGMA = 5;

	/** Default Background */
	const static int DEFAULT_BACKGROUND = 1000;

	/** Default Correction Percentage */
	const static double DEFAULT_CORRECTION = 1.0;

	/** Number of frames at the beginning to calculate pedestal */
	int nped;

	/** Distance from pedestal to detect a hit */
	int nsigma;

	/** background */
	int nbackground;

	/** correction */
	double outcorr;

	/** previous frame index */
	unsigned int fnum;

	/** previous packet index */
	unsigned int pnum;

	/** total packets received */
	unsigned int ptot;

	/** first frame number */
	unsigned int f0;

	/** frame index mask */
	int frame_index_mask;

	/** packet index mask */
	int packet_index_mask;

	/** frame index offset */
	int frame_index_offset;

	/** packet index offset */
	int packet_index_offset;

	/** number of packets per frame */
	int packets_per_frame;

};

#endif
