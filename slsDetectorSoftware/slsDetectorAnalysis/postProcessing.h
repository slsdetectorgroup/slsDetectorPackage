#ifndef POSTPROCESSING_H
#define POSTPROCESSING_H


#include "detectorData.h"
#include "slsDetectorBase.h"

#include <iostream>
#include <fstream>
#include <iomanip>
#include <cstring>
#include <string>
#include <sstream>
#include <queue>
#include <math.h>
#include <semaphore.h>



/**
   @short methods for data postprocessing 
   (including thread for writing data files and plotting in parallel with the acquisition) 
*/

class postProcessing: public virtual slsDetectorBase {

public:
	postProcessing();
	virtual ~postProcessing();


	/**
      set/get if the data processing and file writing should be done by a separate threads
      \param b 0 sequencial data acquisition and file writing, 1 separate thread, -1 get
      \returns thread flag
	 */

	int setThreadedProcessing(int b=-1) {if (b>=0) *threadedProcessing=b; return  *threadedProcessing;};

	/** processes the data
      \param delflag 0 leaves the data in the final data queue
      \returns nothing
	 */
	void *processData(int delflag);

	int checkJoinThread();
	void setJoinThread(int v);


	void registerDataCallback(int( *userCallback)(detectorData*, int, int, void*),  void *pArg) {	\
		dataReady = userCallback;																	\
		pCallbackArg = pArg;																		\
		if (setReceiverOnline() == slsDetectorDefs::ONLINE_FLAG) {								\
			enableDataStreamingToClient(1);														\
			enableDataStreamingFromReceiver(1);}};												\




protected:

			/**
		    start data processing thread
			 */
			void startThread(int delflag=1);

			/** mutex to synchronize main and data processing threads */
			pthread_mutex_t mp;

			/** mutex to synchronizedata processing and plotting threads */
			pthread_mutex_t mg;

			/** mutex to synchronize slsdetector threads */
			pthread_mutex_t ms;

			int *threadedProcessing;

			/** sets when the acquisition is finished */
			int jointhread;

			/** set when detector finishes acquiring */
			int acquiringDone;


			/** the data processing thread */

			pthread_t dataProcessingThread;

			double *fdata;
			detectorData *thisData;

			int (*dataReady)(detectorData*,int, int, void*);
			void *pCallbackArg;


private:
			int kbhit(void);


};


#endif
