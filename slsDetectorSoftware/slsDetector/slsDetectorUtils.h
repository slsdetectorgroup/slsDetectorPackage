

#ifndef SLS_DETECTOR_UTILS_H
#define SLS_DETECTOR_UTILS_H


#ifdef __CINT__
class pthread_mutex_t;
class pthread_t;
#endif


extern "C" {
#include <pthread.h>
}

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <fcntl.h>
#include <unistd.h>


#include <iostream>
#include <fstream>
#include <iomanip>
#include <cstring>
#include <string>
#include <sstream>
#include <queue>
#include <math.h>
#include <semaphore.h>
#include <cstdlib>



#include "postProcessing.h"

#define DEFAULT_HOSTNAME  "localhost"



/**
   @short class containing all the possible detector functionalities 

   (used in the PSi command line interface)
 */


class slsDetectorUtils : public postProcessing {


public:

	slsDetectorUtils();

	virtual ~slsDetectorUtils(){};

	/**
	 * Used when reference is slsDetectorUtils and to determine
	 * if command can be implemented as slsDetector/multiSlsDetector object/
	 */
	virtual bool isMultiSlsDetectorClass() = 0;

	/**
	 * Set acquiring flag in shared memory
	 * @param b acquiring flag
	 */
	virtual void setAcquiringFlag(bool b=false) = 0;

	/**
	 * Get acquiring flag from shared memory
	 * @returns acquiring flag
	 */
	virtual bool getAcquiringFlag() = 0;

	/**
	 * Check if acquiring flag is set, set error if set
	 * @returns FAIL if not ready, OK if ready
	 */
	virtual bool isAcquireReady() = 0;

	/**
	 * Check version compatibility with detector/receiver software
	 * (if hostname/rx_hostname has been set/ sockets created)
	 * @param p port type control port or receiver port
	 * @returns FAIL for incompatibility, OK for compatibility
	 */
	virtual int checkVersionCompatibility(portType t) = 0;

	/**
	 * Free shared memory and delete shared memory structure
	 */
	virtual void freeSharedMemory() = 0;

	/**
	 * Get user details of shared memory
	 * Should only be called from multi detector level
	 * @returns string with user details
	 */
	virtual std::string getUserDetails() = 0;

	/**
	 * Sets the hostname of all sls detectors in shared memory
	 * Connects to them to set up online flag
	 * @param name concatenated hostname of all the sls detectors
	 */
	virtual void setHostname(const char* name)=0;

	/**
	 * Gets the hostname of detector at particular position
	 * or concatenated hostnames of all the sls detectors
	 * @param pos position of detector in array, -1 for all detectors
	 * @returns concatenated hostnames of all detectors or hostname of specific one
	 */
	virtual std::string getHostname(int pos=-1)=0;

	/**
	 * Appends detectors to the end of the list in shared memory
	 * Connects to them to set up online flag
	 * @param name concatenated hostname of the sls detectors to be appended to the list
	 */
	virtual void addMultipleDetectors(const char* name)=0;


	using slsDetectorBase::getDetectorsType;
	/**
	 * Concatenates string types of all sls detectors or
	 * returns the detector type of the first sls detector
	 * @param pos position of sls detector in array, if -1, returns first detector type
	 * @returns detector type of sls detector in position pos, if -1, concatenates
	 */
	virtual std::string sgetDetectorsType(int pos=-1)=0;

	/**
	 * Returns the number of detectors in the multidetector structure
	 * @returns number of detectors
	 */
	virtual int getNumberOfDetectors(){return 1;};

	/**
	 * Returns the total number of channels of all sls detectors from shared memory
	 * @returns the total number of channels of all sls detectors
	 */
	virtual int getTotalNumberOfChannels()=0;

	/**
	 * Returns the total number of channels of all sls detectors in dimension d
	 * from shared memory
	 * @param d dimension d
	 * @returns the total number of channels of all sls detectors in dimension d
	 */
	virtual int getTotalNumberOfChannels(dimension d)=0;

	/**
	 * Returns the maximum number of channels of all sls detectors in each dimension d
	 * from shared memory. multi detector shared memory variable to calculate
	 * offsets for each sls detector
	 * @param d dimension d
	 * @returns the maximum number of channels of all sls detectors in dimension d
	 */
	virtual int getMaxNumberOfChannelsPerDetector(dimension d){return -1;};

	/**
	 * Sets the maximum number of channels of all sls detectors in each dimension d
	 * from shared memory, multi detector shared memory variable to calculate
	 * offsets for each sls detector
	 * @param d dimension d
	 * @param i maximum number of channels for multi structure in dimension d
	 * @returns the maximum number of channels of all sls detectors in dimension d
	 */
	virtual int setMaxNumberOfChannelsPerDetector(dimension d,int i){return -1;};

	/**
	 * Checks if each of the detectors are online/offline
	 * @returns empty string if they are all online,
	 * else returns concatenation of strings of all detectors that are offline
	 */
	virtual std::string checkOnline()=0;

	/**
	 * Set/Gets TCP Port of detector or receiver
	 * @param t port type
	 * @param num port number (-1 gets)
	 * @returns port number
	 */
	virtual int setPort(portType t, int num=-1)=0;

	/**
	 * Get last client IP saved on detector server
	 * @returns last client IP saved on detector server
	 */
	virtual std::string getLastClientIP()=0;

	/**
	 * Exit detector server
	 * @returns OK or FAIL
	 */
	virtual int exitServer()=0;

	/**
	 * Write current configuration to a file
	 * @param fname configuration file name
	 * @returns OK or FAIL
	 */
	virtual int writeConfigurationFile(std::string const fname)=0;

	/**
	 * Returns the trimfile or settings file name (Useless??)
	 * @returns the trimfile or settings file name
	 */
	virtual std::string getSettingsFile()=0;

	/**
	 * Set threshold energy (Mythen and Eiger)
	 * @param e_eV threshold in eV
	 * @param imod module number (-1 all)
	 * @param isettings ev. change settings
	 * @param tb 1 to include trimbits, 0 to exclude
	 * @returns current threshold value for imod in ev (-1 failed)
	 */
	virtual int setThresholdEnergy(int e_eV, int imod=-1, detectorSettings isettings=GET_SETTINGS,int tb=1)=0;

	/**
	 * Returns the detector trimbit/settings directory  \sa sharedSlsDetector
	 * @returns the trimbit/settings directory
	 */
	virtual std::string getSettingsDir()=0;

	/**
	 * Sets the detector trimbit/settings directory  \sa sharedSlsDetector
	 * @param s trimbits/settings directory
	 * @returns the trimbit/settings directory
	 */
	virtual std::string setSettingsDir(std::string s)=0;

	/**
	 * Returns the calibration files directory   \sa  sharedSlsDetector (Mythen)
	 * @returns the calibration files directory
	 */
	virtual std::string getCalDir()=0;

	/**
	 * Sets the calibration files directory   \sa  sharedSlsDetector (Mythen)
	 * @param s the calibration files directory
	 * @returns the calibration files directory
	 */
	virtual std::string setCalDir(std::string s)=0;

	/**
	 * Loads the modules settings/trimbits reading from a specific file
	 * file name extension is automatically generated.
	 * @param fname specific settings/trimbits file
	 * @param imod module number (-1 for all)
	 * returns OK or FAIL
	 */
	virtual int loadSettingsFile(std::string fname, int imod=-1)=0;

	/**
	 * Saves the modules settings/trimbits to a specific file
	 * file name extension is automatically generated.
	 * @param fname specific settings/trimbits file
	 * @param imod module number (-1 for all)
	 * returns OK or FAIL
	 */
	virtual int saveSettingsFile(std::string fname, int imod=-1)=0;

	/**
	 * Give an internal software trigger to the detector (Eiger only)
	 * @return OK or FAIL
	 */
	virtual int sendSoftwareTrigger()=0;

	/**
	 * Start detector acquisition and read all data (Blocking until end of acquisition)
	 * @returns OK or FAIL
	 */
	virtual int startAndReadAll()=0;

	/**
	 * Requests and  receives all data from the detector (Eiger store in ram)
	 * @returns OK or FAIL
	 */
	virtual int readAll()=0;

	/**
	 * Configures in detector the destination for UDP packets (Not Mythen)
	 * @returns OK or FAIL
	 */
	virtual int configureMAC()=0;

	/**
	 * Set/get timer value left in acquisition (not all implemented for all detectors)
	 * @param index timer index
	 * @param t time in ns or number of...(e.g. frames, gates, probes)
	 * @param imod module number
	 * @returns timer set value in ns or number of...(e.g. frames, gates, probes)
	 */
	virtual int64_t getTimeLeft(timerIndex index, int imod = -1)=0;

	/**
	 * Set speed
	 * @param sp speed type  (clkdivider option for Jungfrau and Eiger, others for Mythen/Gotthard)
	 * @param value (clkdivider 0,1,2 for full, half and quarter speed). Other values check manual
	 * @returns value of speed set
	 */
	virtual int setSpeed(speedVariable sp, int value=-1)=0;

	/**
	 * Set/get dacs value
	 * @param val value (in V)
	 * @param index DAC index
	 * @param mV 0 in dac units or 1 in mV
	 * @param imod module number (if -1 all modules)
	 * @returns current DAC value
	 */
	virtual dacs_t setDAC(dacs_t val, dacIndex index , int mV, int imod=-1)=0;

	/**
	 * Get adc value
	 * @param index adc(DAC) index
	 * @param imod module number (if -1 all modules)
	 * @returns current adc value (temperature for eiger and jungfrau in millidegrees)
	 */
	virtual dacs_t getADC(dacIndex index, int imod=-1)=0;

	/**
	 * Set/get external signal flags (to specify triggerinrising edge etc) (Gotthard, Mythen)
	 * @param pol external signal flag (-1 gets)
	 * @param signalindex singal index (0 - 3)
	 * @returns current timing mode
	 */
	virtual externalSignalFlag setExternalSignalFlags(externalSignalFlag pol=GET_EXTERNAL_SIGNAL_FLAG , int signalindex=0)=0;

	/**
	 * Set/get readout flags (Eiger, Mythen)
	 * @param flag readout flag (Eiger options: parallel, nonparallel, safe etc.) (-1 gets)
	 * @returns readout flag
	 */
	virtual int setReadOutFlags(readOutFlags flag=GET_READOUT_FLAGS)=0;

	/**
	 * Write in a register. For Advanced users
	 * @param addr address of register
	 * @param val value to write into register
	 * @returns value read after writing
	 */
	virtual uint32_t writeRegister(uint32_t addr, uint32_t val)=0;

	/**
	 * Read from a register. For Advanced users
	 * @param addr address of register
	 * @returns value read from register
	 */
	virtual uint32_t readRegister(uint32_t addr)=0;

	/**
	 * Set bit in a register. For Advanced users
	 * @param addr address of register
	 * @param n nth bit
	 * @returns value read from register
	 */
	virtual uint32_t setBit(uint32_t addr, int n)=0;

	/**
	 * Clear bit in a register. For Advanced users
	 * @param addr address of register
	 * @param n nth bit
	 * @returns value read from register
	 */
	virtual uint32_t clearBit(uint32_t addr, int n)=0;

	/**
	 * Set network parameter
	 * @param p network parameter type
	 * @param s network parameter value
	 * @returns network parameter value set (from getNetworkParameter)
	 */
	virtual std::string setNetworkParameter(networkParameter p, std::string s)=0;

	/**
	 * Get network parameter
	 * @param p network parameter type
	 * @returns network parameter value set (from getNetworkParameter)
	 */
	virtual std::string getNetworkParameter(networkParameter)=0;

	/**
	 * Execute a digital test (Gotthard, Mythen)
	 * @param mode testmode type
	 * @param imod module index (-1 for all)
	 * @returns result of test
	 */
	virtual int digitalTest(digitalTestMode mode, int imod=0)=0;

	/**
	 * Load dark or gain image to detector (Gotthard)
	 * @param index image type
	 * @param fname file name from which to load image
	 * @returns OK or FAIL
	 */
	virtual int loadImageToDetector(imageType index,std::string const fname)=0;

	/**
	 * Writes the counter memory block from the detector (Gotthard)
	 * @param fname file name to load data from
	 * @param startACQ is 1 to start acquisition after reading counter
	 * @returns OK or FAIL
	 */
	virtual int writeCounterBlockFile(std::string const fname,int startACQ=0)=0;

	/**
	 * Resets counter in detector (Gotthard)
	 * @param startACQ is 1 to start acquisition after resetting counter
	 * @returns OK or FAIL
	 */
	virtual int resetCounterBlock(int startACQ=0)=0;

	/**
	 * Set/get counter bit in detector (Gotthard)
	 * @param i is -1 to get, 0 to reset and any other value to set the counter bit
	 * @returns the counter bit in detector
	 */
	virtual int setCounterBit(int i = -1)=0;

	/**
	 * Set ROI (Gotthard)
	 * At the moment only one set allowed
	 * @param n number of rois
	 * @param roiLimits array of roi
	 * @returns OK or FAIL
	 */
	virtual int setROI(int n=-1,ROI roiLimits[]=NULL)=0;

	/**
	 * Get ROI from each detector and convert it to the multi detector scale (Gotthard)
	 * @param n number of rois
	 * @returns OK or FAIL
	 */
	virtual ROI* getROI(int &n)=0;

	/**
	 * Write to ADC register (Gotthard, Jungfrau, ChipTestBoard). For expert users
	 * @param addr address of adc register
	 * @param val value
	 * @returns return value  (mostly -1 as it can't read adc register)
	 */
	virtual int writeAdcRegister(int addr, int val)=0;

	/**
	 * Returns the enable if data will be flipped across x or y axis (Eiger)
	 * @param d axis across which data is flipped
	 * @returns 1 for flipped, else 0
	 */
	virtual int getFlippedData(dimension d=X)=0;

	/**
	 * Sets the enable which determines if
	 * data will be flipped across x or y axis (Eiger)
	 * @param d axis across which data is flipped
	 * @param value 0 or 1 to reset/set or -1 to get value
	 * @returns enable flipped data across x or y axis
	 */
	virtual int setFlippedData(dimension d=X, int value=-1)=0;

	/**
	 * Sets all the trimbits to a particular value (Eiger)
	 * @param val trimbit value
	 * @param imod module number, -1 means all modules
	 * @returns OK or FAIL
	 */
	virtual int setAllTrimbits(int val, int imod=-1)=0;

	/**
	 * Enable gap pixels, only for Eiger and for 8,16 and 32 bit mode. (Eiger)
	 * 4 bit mode gap pixels only in gui call back
	 * @param val 1 sets, 0 unsets, -1 gets
	 * @returns gap pixel enable or -1 for error
	 */
	virtual int enableGapPixels(int val=-1)=0;

	/**
	 * Sets the number of trim energies and their value  (Eiger)
	 * \sa sharedSlsDetector
	 * @param nen number of energies
	 * @param en array of energies
	 * @returns number of trim energies
	 */
	virtual int setTrimEn(int nen, int *en=NULL)=0;

	/**
	 * Returns the number of trim energies and their value  (Eiger)
	 * \sa sharedSlsDetector
	 * @param en array of energies
	 * @returns number of trim energies
	 */
	virtual int getTrimEn(int *en=NULL)=0;

	/**
	 * Pulse Pixel (Eiger)
	 * @param n is number of times to pulse
	 * @param x is x coordinate
	 * @param y is y coordinate
	 * @returns OK or FAIL
	 */
	virtual int pulsePixel(int n=0,int x=0,int y=0)=0;

	/**
	 * Pulse Pixel and move by a relative value (Eiger)
	 * @param n is number of times to pulse
	 * @param x is relative x value
	 * @param y is relative y value
	 * @returns OK or FAIL
	 */
	virtual int pulsePixelNMove(int n=0,int x=0,int y=0)=0;

	/**
	 * Pulse Chip (Eiger)
	 * @param n is number of times to pulse
	 * @returns OK or FAIL
	 */
	virtual int pulseChip(int n=0)=0;

	/**
	 * Set/gets threshold temperature (Jungfrau)
	 * @param val value in millidegrees, -1 gets
	 * @param imod module number, -1 is all
	 * @returns threshold temperature in millidegrees
	 */
	virtual int setThresholdTemperature(int val=-1, int imod=-1)=0;

	/**
	 * Enables/disables temperature control (Jungfrau)
	 * @param val value, -1 gets
	 * @param imod module number, -1 is all
	 * @returns temperature control enable
	 */
	virtual int setTemperatureControl(int val=-1, int imod=-1)=0;

	/**
	 * Resets/ gets over-temperature event (Jungfrau)
	 * @param val value, -1 gets
	 * @param imod module number, -1 is all
	 * @returns over-temperature event
	 */
	virtual int setTemperatureEvent(int val=-1, int imod=-1)=0;

	/**
	 * Set storage cell that stores first acquisition of the series (Jungfrau)
	 * @param value storage cell index. Value can be 0 to 15. (-1 gets)
	 * @returns the storage cell that stores the first acquisition of the series
	 */
	virtual int setStoragecellStart(int pos=-1)=0;

	/**
	 * Programs FPGA with pof file (Jungfrau)
	 * @param fname file name
	 * @returns OK or FAIL
	 */
	virtual int programFPGA(std::string fname)=0;

	/**
	 * Resets FPGA (Jungfrau)
	 * @returns OK or FAIL
	 */
	virtual int resetFPGA()=0;

	/**
	 * Power on/off Chip (Jungfrau)
	 * @param ival on is 1, off is 0, -1 to get
	 * @returns OK or FAIL
	 */
	virtual int powerChip(int ival= -1)=0;

	/**
	 * Automatic comparator disable (Jungfrau)
	 * @param ival on is 1, off is 0, -1 to get
	 * @returns OK or FAIL
	 */
	virtual int setAutoComparatorDisableMode(int ival= -1)=0;

	/**
	 * Calibrate Pedestal (ChipTestBoard)
	 * Starts acquisition, calibrates pedestal and writes to fpga
	 * @param frames number of frames
	 * @returns number of frames
	 */
	virtual int calibratePedestal(int frames = 0)=0;

	/**
	 * Get rate correction tau (Mythen, Eiger)
	 * @returns 0 if rate correction disabled, otherwise the tau used for the correction
	 */
	virtual double getRateCorrectionTau()=0;

	/**
	 * Checks if the receiver is really online
	 * @returns empty string if all online, else concatenates hostnames of all
	 * detectors that are offline
	 */
	virtual std::string checkReceiverOnline()=0;

	/**
	 * Locks/Unlocks the connection to the receiver
	 * @param lock sets (1), usets (0), gets (-1) the lock
	 * @returns lock status of the receiver
	 */
	virtual int lockReceiver(int lock=-1)=0;

	/**
	 * Returns the IP of the last client connecting to the receiver
	 * @returns IP of last client connecting to receiver
	 */
	virtual std::string getReceiverLastClientIP()=0;

	/**
	 * Turns off the receiver server!
	 * @returns OK or FAIL
	 */
	virtual int exitReceiver()=0;

	/**
	 * Returns output file directory
	 * @returns output file directory
	 */
	virtual std::string getFilePath()=0;

	/**
	 * Sets up the file directory
	 * @param s file directory
	 * @returns file dir
	 */
	virtual std::string setFilePath(std::string s)=0;

	/**
	 * Returns file name prefix
	 * @returns file name prefix
	 */
	virtual std::string getFileName()=0;

	/**
	 * Sets up the file name prefix
	 * @param s file name prefix
	 * @returns file name prefix
	 */
	virtual std::string setFileName(std::string s)=0;

	/**
	 * Sets the max frames per file in receiver
	 * @param f max frames per file
	 * @returns max frames per file in receiver
	 */
	virtual int setReceiverFramesPerFile(int f=-1)=0;

	/**
	 * Sets the frames discard policy in receiver
	 * @param f frames discard policy
	 * @returns frames discard policy set in receiver
	 */
	virtual frameDiscardPolicy setReceiverFramesDiscardPolicy(frameDiscardPolicy f = GET_FRAME_DISCARD_POLICY)=0;

	/**
	 * Sets the partial frames padding enable in receiver
	 * @param f partial frames padding enable
	 * @returns partial frames padding enable in receiver
	 */
	virtual int setReceiverPartialFramesPadding(int f = -1)=0;

	/**
	 * Returns file format
	 * @returns file name
	 */
	virtual fileFormat getFileFormat()=0;

	/**
	 * Sets up the file format
	 * @param f file format
	 * @returns file format
	 */
	virtual fileFormat setFileFormat(fileFormat f)=0;

	/**
	 * Gets the number of frames caught by receiver
	 * @returns number of frames caught by receiver
	 */
	virtual int getFramesCaughtByReceiver()=0;

	/**
	 * Gets the number of frames caught by any one receiver (to avoid using threadpool)
	 * @returns number of frames caught by any one receiver (master receiver if exists)
	 */
	virtual int getFramesCaughtByAnyReceiver()=0;

	/**
	 * Gets the current frame index of receiver
	 * @returns current frame index of receiver
	 */
	virtual int getReceiverCurrentFrameIndex()=0;

	/**
	 * Resets framescaught in receiver
	 * Use this when using startAcquisition instead of acquire
	 * @returns OK or FAIL
	 */
	virtual int resetFramesCaught()=0;

	/**
	 * Create Receiving Data Sockets
	 * @param destroy is true to destroy all the sockets
	 * @returns OK or FAIL
	 */
	virtual int createReceivingDataSockets(const bool destroy = false){return -1;};

	/**
	 * Reads frames from receiver through a constant socket
	 * Called during acquire() when call back registered or when using gui
	 */
	virtual void readFrameFromReceiver(){};

	/**
	 * Sets/Gets receiver file write enable
	 * @param enable 1 or 0 to set/reset file write enable
	 * @returns file write enable
	 */
	virtual int enableWriteToFile(int enable=-1)=0;

	/**
	 * Sets/Gets file overwrite enable
	 * @param enable 1 or 0 to set/reset file overwrite enable
	 * @returns file overwrite enable
	 */
	virtual int overwriteFile(int enable=-1)=0;

	/**
	 * Sets the read receiver frequency
	 * if data required from receiver randomly readRxrFrequency=0,
	 * else every nth frame to be sent to gui/callback
	 * @param freq is the receiver read frequency. Value 0 is 200 ms timer (other
	 * frames not sent), 1 is every frame, 2 is every second frame etc.
	 * @returns read receiver frequency
	 */
	virtual int setReadReceiverFrequency(int freq=-1)=0;

	/**
	 * Enable data streaming to client
	 * @param enable 0 to disable, 1 to enable, -1 to get the value
	 * @returns data streaming to client enable
	 */
	virtual int enableDataStreamingToClient(int enable=-1)=0;

	/**
	 * Enable or disable streaming data from receiver to client
	 * @param enable 0 to disable 1 to enable -1 to only get the value
	 * @returns data streaming from receiver enable
	 */
	virtual int enableDataStreamingFromReceiver(int enable=-1)=0;

	/**
	 * Enable/disable or 10Gbe
	 * @param i is -1 to get, 0 to disable and 1 to enable
	 * @returns if 10Gbe is enabled
	 */
	virtual int enableTenGigabitEthernet(int i = -1)=0;

	/**
	 * Set/get receiver fifo depth
	 * @param i is -1 to get, any other value to set the fifo deph
	 * @returns the receiver fifo depth
	 */
	virtual int setReceiverFifoDepth(int i = -1)=0;

	/**
	 * Set/get receiver silent mode
	 * @param i is -1 to get, 0 unsets silent mode, 1 sets silent mode
	 * @returns the receiver silent mode enable
	 */
	virtual int setReceiverSilentMode(int i = -1)=0;

	/**
	 * Opens pattern file and sends pattern to CTB
	 * @param fname pattern file to open
	 * @returns OK/FAIL
	 */
	virtual int setCTBPattern(std::string fname)=0;

	/**
	 * Writes a pattern word to the CTB
	 * @param addr address of the word, -1 is I/O control register,
	 * -2 is clk control register
	 * @param word 64bit word to be written, -1 gets
	 * @returns actual value
	 */
	virtual uint64_t setCTBWord(int addr,uint64_t word=-1)=0;

	/**
	 * Sets the pattern or loop limits in the CTB
	 * @param level -1 complete pattern, 0,1,2, loop level
	 * @param start start address if >=0
	 * @param stop stop address if >=0
	 * @param n number of loops (if level >=0)
	 * @returns OK/FAIL
	 */
	virtual int setCTBPatLoops(int level,int &start, int &stop, int &n)=0;

	/**
	 * Sets the wait address in the CTB
	 * @param level  0,1,2, wait level
	 * @param addr wait address, -1 gets
	 * @returns actual value
	 */
	virtual int setCTBPatWaitAddr(int level, int addr=-1)=0;

	/**
	 * Sets the wait time in the CTB
	 * @param level  0,1,2, wait level
	 * @param t wait time, -1 gets
	 * @returns actual value
	 */
	virtual int setCTBPatWaitTime(int level, uint64_t t=-1)=0;












	int enableCountRateCorrection(int i=-1) {if (i>0) setRateCorrection(-1); else if (i==0) setRateCorrection(0); return getRateCorrection();};

	/**
	 * Set/Get receiver streaming out ZMQ port
	 * For multi modules, it calculates (increments) and sets the ports
	 * @param i sets, -1 gets
	 * @returns receiver streaming out ZMQ port
	 */
	int setReceiverDataStreamingOutPort(int i) {								\
		if (i >= 0) {															\
			std::ostringstream ss; ss << i; std::string s = ss.str();					\
			int prev_streaming = enableDataStreamingFromReceiver();			\
			setNetworkParameter(RECEIVER_STREAMING_PORT, s);					\
			if (prev_streaming) {												\
				enableDataStreamingFromReceiver(0);							\
				enableDataStreamingFromReceiver(1);}}							\
				return atoi(getNetworkParameter(RECEIVER_STREAMING_PORT).c_str());};	\

	/**
	 * Set/Get client streaming in ZMQ port
	 * For multi modules, it calculates (increments) and sets the ports
	 * @param i sets, -1 gets
	 * @returns client streaming in ZMQ port
	 */
	int setClientDataStreamingInPort(int i){										\
		if (i >= 0) {															\
			std::ostringstream ss; ss << i; std::string s = ss.str();					\
			int prev_streaming = enableDataStreamingToClient();				\
			setNetworkParameter(CLIENT_STREAMING_PORT, s);					\
			if (prev_streaming) {												\
				enableDataStreamingToClient(0);								\
				enableDataStreamingToClient(1);}}								\
				return atoi(getNetworkParameter(CLIENT_STREAMING_PORT).c_str());};	\

	/**
	 * Set/Get receiver streaming out ZMQ port
	 * For multi modules, it calculates (increments) and sets the ports
	 * @param i sets, -1 gets
	 * @returns receiver streaming out ZMQ port
	 */
	std::string setReceiverDataStreamingOutIP(std::string ip) {							\
		if (ip.length()) {														\
			int prev_streaming = enableDataStreamingFromReceiver();				\
			setNetworkParameter(RECEIVER_STREAMING_SRC_IP, ip);					\
			if (prev_streaming) {												\
				enableDataStreamingFromReceiver(0);								\
				enableDataStreamingFromReceiver(1);}}							\
				return getNetworkParameter(RECEIVER_STREAMING_SRC_IP);};				\

	/**
	 * Set/Get client streaming in ZMQ port
	 * For multi modules, it calculates (increments) and sets the ports
	 * @param i sets, -1 gets
	 * @returns client streaming in ZMQ port
	 */
	std::string setClientDataStreamingInIP(std::string ip){								\
		if (ip.length()) {														\
			int prev_streaming = enableDataStreamingToClient();					\
			setNetworkParameter(CLIENT_STREAMING_SRC_IP, ip);					\
			if (prev_streaming) {												\
				enableDataStreamingToClient(0);									\
				enableDataStreamingToClient(1);}}								\
				return getNetworkParameter(CLIENT_STREAMING_SRC_IP);};					\


	int setFlowControl10G(int i = -1) {
		std::string sret="";
		if (i != -1) {
			std::ostringstream o;
			o << ((i >= 1) ? 1 : 0);
			std::string sval = o.str();
			sret = setNetworkParameter(FLOW_CONTROL_10G, sval);
		} else
			sret = getNetworkParameter(FLOW_CONTROL_10G);

		return atoi(sret.c_str());
	}


	/** performs a complete acquisition including scansand data processing
	 * moves the detector to next position <br>
	 * starts and reads the detector <br>
	 * reads the IC (if required) <br>
	 * reads the encoder (iof required for angualr conversion) <br>
	 * processes the data (flat field, rate, angular conversion and merging ::processData())
	 * @param delflag 0 leaves the data in the final data queue
	 * @returns OK or FAIL depending on if it already started
	 */
	int acquire(int delflag=1);

	int setTotalProgress();

	double getCurrentProgress();

	void incrementProgress();

	void setCurrentProgress(int i=0);

	/**
	 * Saves the detector setup to file
	 * @param fname file to write to
	 * @param level if 2 reads also trimbits, flat field, angular correction etc.
	 * and writes them to files with automatically added extension
	 * @returns OK or FAIL
	 */
	int dumpDetectorSetup(std::string const fname, int level=0);

	/** (used by multi and sls)
	 * reads a short int rawdata file
	 * @param name of the file to be read
	 * @param data array of data value
	 * @param nch number of channels
	 * @returns OK or FAIL if it could not read the file or data=NULL
	 */
	int readDataFile(std::string fname, short int *data, int nch);

	/** (used by multi and sls)
	 * reads a short int raw data file
	 * @param infile input file stream
	 * @param data array of data values
	 * @param nch number of channels
	 * @param offset start channel value
	 * @returns OK or FAIL if it could not read the file or data=NULL
	 */
	int readDataFile(std::ifstream &infile, short int *data, int nch, int offset);

	/**
	 * reads a short int raw data file
	 * @param fname of the file to be read
	 * @param data array of data values
	 * @returns OK or FAIL if it could not read the file or data=NULL
	 */
	int readDataFile(std::string fname, short int *data){
		return readDataFile(fname, data, getTotalNumberOfChannels());
	};
	/**
	 * reads a short int raw data file
	 * @param infile input file stream
	 * @param data array of data values
	 * @param offset first channel number to be expected
	 * @returns OK or FAIL if it could not read the file or data=NULL
	 */
	int readDataFile(std::ifstream &infile, short int *data, int offset=0){
		return readDataFile(infile, data, getTotalNumberOfChannels(),offset);
	};

	/** (used by multi and sls)
	 * writes a short int raw data file
	 * @param fname of the file to be written
	 * @param nch number of channels
	 * @param data array of data values
	 * @returns OK or FAIL if it could not write the file or data=NULL
	 */
	int writeDataFile(std::string fname,int nch, short int *data);

	/** (used by multi and sls)
	 * writes a short int raw data file
	 * @param outfile output file stream
	 * @param nch number of channels
	 * @param data array of data values
	 * @param offset start channel number
	 * @returns OK or FAIL if it could not write the file or data=NULL
	 */
	int writeDataFile(std::ofstream &outfile,int nch,  short int *data, int offset=0) ;

	/**
	 * writes a data file  of short ints
	 * @param fname of the file to be written
	 * @param data array of data values
	 * @returns OK or FAIL if it could not write the file or data=NULL
	 */
	int writeDataFile(std::string fname, short int *data){
		return writeDataFile(fname, getTotalNumberOfChannels(), data);
	};

	/**
	 * writes a data file of short ints
	 * @param outfile output file stream
	 * @param data array of data values
	 * @param offset start channel number
	 * @returns OK or FAIL if it could not write the file or data=NULL
	 */
	int writeDataFile(std::ofstream &outfile, short int *data, int offset=0){
		return writeDataFile(outfile, getTotalNumberOfChannels(), data, offset);
	};

	/**
	 * Loads the detector setup from file
	 * @param fname file to read from
	 * @param level if 2 reads also reads trimbits, angular conversion coefficients etc.
	 * from files with default extensions as generated by dumpDetectorSetup
	 * @returns OK or FAIL
	 */
	int retrieveDetectorSetup(std::string const fname, int level=0);

	void registerAcquisitionFinishedCallback(int( *func)(double,int, void*), void *pArg){acquisition_finished=func; acqFinished_p=pArg;};

	void registerMeasurementFinishedCallback(int( *func)(int,int, void*), void *pArg){measurement_finished=func; measFinished_p=pArg;};

	void registerProgressCallback(int( *func)(double,void*), void *pArg){progress_call=func; pProgressCallArg=pArg;};

	static int dummyAcquisitionFinished(double prog,int status,void* p){std::cout <<"Acquisition finished callback! " << prog << " " << status << std::endl; return 0;}

	static int dummyMeasurementFinished(int im,int findex,void* p){std::cout <<"Measurement finished callback! " << im << " " << findex << std::endl; return 0;}


protected:

	static const int64_t thisSoftwareVersion=0x20141013;

	int *stoppedFlag;
	int64_t *timerValue;
	detectorSettings *currentSettings;
	int *currentThresholdEV;

	int totalProgress;
	int progressIndex;

	int (*acquisition_finished)(double,int,void*);
	int (*measurement_finished)(int,int,void*);
	void *acqFinished_p, *measFinished_p;
	int (*progress_call)(double,void*);
	void *pProgressCallArg;

	/** semaphore to let postprocessing thread continue for next scan/measurement */
	sem_t sem_newRTAcquisition;

	/** semaphore to let main thread know it got all the dummy packets (also from ext. process) */
	sem_t sem_endRTAcquisition;

};



#endif
