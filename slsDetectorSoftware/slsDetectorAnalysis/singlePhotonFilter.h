  /********************************************//**
 * @file singlePhotonFilter.h
 * @short single photon filter using trees
 ***********************************************/
#ifndef SINGLEPHOTONFILTER_H
#define SINGLEPHOTONFILTER_H

//#include "sls_detector_defs.h"


#ifdef __CINT
#define MYROOT1
#endif


#ifdef MYROOT1
#include <TFile.h>
#include <TMath.h>
#include <TTree.h>
#include <TChain.h>
#endif

#include <stdint.h>
#include <stdio.h>
#include <iostream>
#include <deque>
#include <list>
#include <queue>
#include <fstream>
#include <pthread.h>
#include <semaphore.h>

#include "circularFifo.h"
#include "runningStat.h"
#include "movingStat.h"
#include "single_photon_hit.h"
using namespace std;


typedef  double double32_t;
typedef  float float32_t;
typedef  int int32_t;
#define MAX_STR_LENGTH 1000

/**
    return values
*/
enum  {
  OK, /**< function succeeded */
  FAIL /**< function failed */
};


/**
   @short class handling trees and its data file
 */

class singlePhotonFilter{
public:
	/**
	 * Constructor
	 * @param nx Number of Channels in X direction
	 * @param ny Number of Channels in Y direction
	 * @param fmask frame index mask
	 * @param pmask packet index mask
	 * @param foffset frame index offset
	 * @param poffset packet index offset
	 * @param pperf packets per frame
	 * @param iValue increment value (only for gotthard to increment index to have matching frame number)
	 * @param m Map to data without headers
	 * @param s mask as to which adcs are inverted
	 * @param f circular fifo buffer, which needs to be freed
	 * @param d Size of data with the headers
	 * @param tfcaught pointer to total frames caught- needs updation for client
	 * @param fcaught pointer to frames caught - needs updation for client
	 * @param cframenum pointer to currentframe num- needs updation for progress for gui
	 */
	singlePhotonFilter(
			int nx,
			int ny,
			int fmask,
			int pmask,
			int foffset,
			int poffset,
			int pperf,
			int iValue,
			int16_t *m,
			int16_t *s,
			CircularFifo<char>* f,
			int d,
			int* tfcaught,
			int* fcaught,
			uint32_t* cframenum);

	/** virtual destructor */
	virtual ~singlePhotonFilter();

#ifdef MYROOT1
	/**
	 * returns tree
	 */
	TTree *getTree(){ return myTree; };
#endif

	/**
	 * Returns packets per frame
	 */
	int getPacketsPerFrame(){ return packets_per_frame;};

	/**
	 * returns struct
	 */
	single_photon_hit* getStructure(){ return myPhotonHit; };

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

	/**
	 * Construct a tree, populate struct for the single photon hit and provide all the masks and offsets
	 * @param outdir Output file directory/Output file name
	 * returns OK if successful, else FAIL

	 */
	int initTree();

	/**
	 * Writes tree/struct to file
	 * returns OK if successful, else FAIL
	 */
	int writeToFile();

	/**
	 * Closes file
	 * returns OK if successful, else FAIL
	 */
	int closeFile();

	/**
	 * Reset Indices before starting acquisition
	 */
	void setupAcquisitionParameters(char *outfpath, char* outfname, int outfIndex);

	/** reconstruct the frame with all the right packets
	 * @param inData the data from socket to be verified
	 * returns
	 * 0: 	waiting for next packet of new frame
	 * 1:	finished with full frame,
	 *	   	start new frame
	 * -1:	last packet of current frame,
	 * 		invalidate remaining packets,
	 * 		start new frame
	 * -2: 	first packet of new frame,
	 * 		invalidate remaining packets,
	 * 		check buffer needs to be pushed,
	 * 		start new frame with the current packet,
	 * 		then ret = 0
	 * -3: 	last packet of new frame,
	 * 		invalidate remaining packets,
	 * 		check buffer needs to be pushed,
	 * 		start new frame with current packet,
	 * 		then ret = -1 (invalidate remaining packets and start a new frame)
	 */
	int verifyFrame(char *inData);

	/**
	 * Find Hits frame by frame and save it in file/tree
	 */
	void findHits();

	/** Enable or disable compression
	 * @param enable true to enable compression and false to disable
	 * returns OK for success or FAIL for failure, incase threads fail to start
	 * */
	int enableCompression(bool enable);

	/** create threads for compression
	 * @param this_pointer obejct of this class
	 * */
	static void* createThreads(void *this_pointer);

	/** assignjobs to each thread
	 * @param thisData a bunch of frames
	 * @param numThisData number of frames
	 * */
	void assignJobsForThread(char *thisData, int numThisData);

	/** Checks if all the threads are done processing
	 * @param returns 1 for jobs done and 0 for jobs not done
	 * */
	int checkIfJobsDone();

	/**
	 * call back to free fifo
	 * call back arguments are
	 * fbuffer buffer address to be freed
	 */
	void registerCallBackFreeFifo(void (*func)(char*, void*),void *arg){freeFifoCallBack=func; pFreeFifo=arg;};



private:

#ifdef MYROOT1
	/** Tree where the hits are stored */
	TTree *myTree;

	/** File where the tree is saved */
	TFile *myFile;
#else
    FILE *myFile;

	/** pointer to array of structs when only using files */
	//single_photon_hit* photonHitList;
	single_photon_hit  ** photonHitList;

	/** Number of Hits per file */
	int nHitsPerFile;

	/** Total Number of Hits Per Acquisition */
	int nTotalHits;
#endif

	/** Number of Hits per frame*/
	int nHitsPerFrame;

	/** Maximum Number of hits written to file */
	const static int MAX_HITS_PER_FILE = 2000000;

	/** Number of Channels in X  direction */
	int nChannelsX;

	/** Number of Channels in Y direction */
	int nChannelsY;

	/** Cluster size in X direction */
	int nClusterX;

	/** Cluster size in Y direction */
	int nClusterY;

	/** map to the data without headers */
	int16_t *map;

	/** Size of data with headers */
	int dataSize;

	/** mask as to which adcs are inverted */
	int16_t *mask;

	/** movingStat object */
	movingStat *stat;

	movingStat *nHitStat;

	/** single Photon Hit structure */
	single_photon_hit* myPhotonHit;

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

	/** increment value for index for gotthard */
	int incrementValue;

	/** first packet */
	bool firstTime;

	/** return status */
	int ret;

	/** current packet index */
	int pIndex;

	/** current frame index */
	int fIndex;

	/** thread related variables */
	static const int NUM_THREADS = 15;
	pthread_t   find_hits_thread[NUM_THREADS];
	volatile int thread_started;
	volatile int threads_mask;
	pthread_mutex_t write_mutex;
	pthread_mutex_t running_mutex;
	pthread_mutex_t frnum_mutex;

	static const int PROGRESS_INCREMENT = 100;
	/** current thread the job being allotted to */
	int currentThread;

	/** current index alloted for each thread */
	int thisThreadIndex;

	/** semaphore to synchronize between different jobs on same thread */
	sem_t smp[NUM_THREADS];

	/** starting memory of data for different threads */
	char* mem0[NUM_THREADS];

	/** number of frames alloted for each thread to process */
	int* numFramesAlloted;



	/** final file name */
	char savefilename[MAX_STR_LENGTH];

	/** file path */
	char filePath[MAX_STR_LENGTH];

	/** file prefix */
	char fileName[MAX_STR_LENGTH];

	/** file acquisition index */
	int fileIndex;


	/** 0 for 1d and 1 for 2d */
	int deltaX;

	/** index of center of cluster for 1d and for 2d*/
	int clusterCenterPixel;

	/** squareroot of cluster */
	double sqrtCluster;

	/** circular fifo buffer to be freed */
	CircularFifo<char>* fifo;

	/**total frames caught */
	int* totalFramesCaught;

	/** frames caught */
	int* framesCaught;

	/** current frame number */
	uint32_t* currentframenum;

	/** call back function */
	void (*freeFifoCallBack)(char*, void*);

	/** call back arguments */
	void *pFreeFifo;
};




#endif
