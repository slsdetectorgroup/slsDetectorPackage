#ifndef SLS_DETECTOR_USERS_H
#define SLS_DETECTOR_USERS_H



/**
 * 
 *
 *
 * @author Anna Bergamaschi
 * @version 0.1alpha
 */



class detectorData;
#include "multiSlsDetector.h"

#include <cstdint>
#include <string>





/*
   \mainpage 
<CENTER><H1>API for SLS detectors data acquisition</H1></CENTER>
<HR>
 */
/** 
    \mainpage 


<H1>API for SLS detectors data acquisition</H1>

<HR>

   Although the SLS detectors group delvelops several types of detectors (1/2D, counting/integrating etc.) it is common interest of the group to use a common platfor for data acquisition

   The architecture of the acquisitions system is intended as follows:
   \li A socket server running on the detector (or more than one in some special cases)
   \li C++ classes common to all detectors for client-server communication. These can be supplied to users as libraries and embedded also in acquisition systems which are not developed by the SLS
   \li the possibility of using a Qt-based graphical user interface (with eventually root analisys capabilities)
   \li the possibility of running all commands from command line. In order to ensure a fast operation of this so called "text client" the detector parameters should not be re-initialized everytime. For this reason a shared memory block is allocated where the main detector flags and parameters are stored 
   \li a Root library for data postprocessing and detector calibration (energy, angle).


slsDetectorUsers is a class to control the detector which should be instantiated by the users in their acquisition software (EPICS, spec etc.). A callback for dislaying the data can be registered.
More advanced configuration functions are not implemented and can be written in a configuration file tha can be read/written.

slsReceiverUsers is a class to receive the data for detectors with external data receiver (e.g. GOTTHARD). Callbacks can be registered to process the data or save them in specific formats.

detectorData is a structure containing the data and additional information which is used to return the data e.g. to the  GUI for displaying them.


You can  find examples of how this classes can be instatiated in mainClient.cpp and mainReceiver.cpp

Different values from different detectors will give a -1 (return value is integer), a concatenation of all values (return value is a string) or a FAIL (return value is OK or FAIL)


   \authors <a href="mailto:anna.bergamaschi@psi.ch">Anna Bergamaschi</a>, <a href="mailto:dhanya.thattil@psi.ch">Dhanya Thattil</a>
   @version 3.0
<H2>Currently supported detectors</H2>
\li GOTTHARD
\li	EIGER
\li JUNGFRAU



 */

/**
  @short The slsDetectorUsers class is a minimal interface class which should be instantiated by the users in their acquisition software (EPICS, spec etc.). More advanced configuration functions are not implemented and can be written in a configuration or parameters file that can be read/written.

  Class for detector functionalities to embed the detector controls in the users custom interface e.g. EPICS, Lima etc.

 */


class slsDetectorUsers
{

public:

	/**
	 * Constructor
	 * @param ret address of return value. 0 for success or 1 for failure
	 * @param id multi detector id
	 */
	slsDetectorUsers(int multi_id):detector(multi_id){};

	/**
	 * Destructor
	 */
	virtual ~slsDetectorUsers() = default;

	/**
	 * Returns the number of detectors in the multidetector structure
	 * @returns number of detectors
	 */
	int getNumberOfDetectors() const;

	/**
	 * Returns the maximum number of channels of all detectors
	 * (provided by user in config file using detsizechan command)
	 * Offsets are calculated according to these dimensions
	 * @param nx number of channels in horizontal
	 * @param ny number of channels in vertical
	 * @returns the maximum number of channels of all detectors
	 */
	int getMaximumDetectorSize(int &nx, int &ny);

	/**
	 * Returns the size and offsets of detector/multi detector
	 * @param x horizontal position origin in channel number
	 * @param y vertical position origin in channel number
	 * @param nx number of channels in horiziontal
	 * @param ny number of channels in vertical
	 * @param detPos -1 for all detectors in  list or specific detector position
	 * @returns the total number of channels of all sls detectors
	 */
	int getDetectorSize(int &x, int &y, int &nx, int &ny, int detPos = -1);

	/**
	 * Gets detector type
	 * @param detPos -1 for all detectors in  list or specific detector position
	 * @returns detector type (EIGER, JUNGFRAU, GOTTHARD) slsDetectorDefs
	 */
	std::string getDetectorType(int detPos = -1);

	/**
	 * Sets/Checks the detectors in multi detector list to online/offline
	 * Must be called before communicating with detector
	 * @param online 1 to set detector online, 0 to set it offline, -1 to get
	 * @param detPos -1 for all detectors in  list or specific detector position
	 * @returns (1)online/(0)offline status
	 */
	int setOnline(int const online = -1, int detPos = -1);

	/**
	 * Sets/Checks the receivers in multi detector list to online/offline
	 * Must be called before communicating with receiver
	 * @param online 1 to set receiver online, 0 to set it receiver, -1 to get
	 * @param detPos -1 for all detectors in  list or specific detector position
	 * @returns (1)online/(0)offline status
	 */
	int setReceiverOnline(int const online = -1, int detPos = -1);

	/**
	 * Load configuration from a configuration File (for one time detector setup)
	 * @param fname configuration file name
	 * @return OK or FAIL
	 */
	int readConfigurationFile(const std::string& fname);

	/**
	 * Write current configuration to a file (for one time detector setup)
	 * @param fname configuration file name
	 * @returns OK or FAIL
	 */
	int writeConfigurationFile(const std::string& fname);

	/**
	 * Loads the detector setup from file (current measurement setup)
	 * @param fname file to read from
	 * @returns OK or FAIL
	 */
	int retrieveDetectorSetup(const std::string& fname);

	/**
	 * Saves the detector setup to file (currentmeasurement setup)
	 * @param fname file to write to
	 * @returns OK or FAIL
	 */
	int dumpDetectorSetup(const std::string& fname);

	/**
	 * Get detector firmware version
	 * @param detPos -1 for all detectors in  list or specific detector position
	 * @returns detector firmware version
	 */
	int64_t getDetectorFirmwareVersion(int detPos = -1);

	/**
	 * Get detector serial number or MAC
	 * @param detPos -1 for all detectors in  list or specific detector position
	 * @returns detector serial number or MAC
	 */
	int64_t getDetectorSerialNumber(int detPos = -1);

	/**
	 * Get on-board detector server software version
	 * @param detPos -1 for all detectors in  list or specific detector position
	 * @returns  on-board detector server software version
	 */
	int64_t getDetectorSoftwareVersion(int detPos = -1);

	/**
	 * (previously getThisSoftwareVersion)
	 * Get client software version
	 * @returns client software version
	 */
	int64_t getClientSoftwareVersion();

	/**
	 * Get receiver software version
	 * @param detPos -1 for all detectors in  list or specific detector position
	 * @returns receiver software version
	 */
	int64_t getReceiverSoftwareVersion(int detPos = -1);

	/**
	 * Check Detector Version Compatibility
	 * @param detPos -1 for all detectors in  list or specific detector position
	 * @returns true if compatible, else false
	 */
	bool isDetectorVersionCompatible(int detPos = -1);

	/**
	 * Check Receiver Version Compatibility
	 * @param detPos -1 for all detectors in  list or specific detector position
	 * @returns  true if compatible, else false
	 */
	bool isReceiverVersionCompatible(int detPos = -1);

	/**
	 * Performs a complete acquisition
	 * resets frames caught in receiver, starts receiver, starts detector,
	 * blocks till detector finished acquisition, stop receiver, increments file index,
	 * loops for measurements, calls required call backs.
	 * @returns OK or FAIL depending on if it already started
	 */
	int startMeasurement();

	/**
	 * Stop detector acquisition
	 * @param detPos -1 for all detectors in  list or specific detector position
	 * @returns OK or FAIL
	 */
	int stopMeasurement(int detPos = -1);

	/**
	 * Get Detector run status
	 * @param detPos -1 for all detectors in  list or specific detector position
	 * @returns status
	 */
	int getDetectorStatus(int detPos = -1);

	/**
	 * (Advanced user, included in startMeasurement)
	 * Start detector acquisition (Non blocking)
	 * @param detPos -1 for all detectors in  list or specific detector position
	 * @returns OK or FAIL if even one does not start properly
	 */
	int startAcquisition(int detPos = -1);

	/**
	 * Stop detector acquisition (Same as stopMeasurement)
	 * @param detPos -1 for all detectors in  list or specific detector position
	 * @returns OK or FAIL
	 */
	int stopAcquisition(int detPos = -1);

	/**
	 * (Only in non blocking acquire mode)
	 * Give an internal software trigger to the detector (Eiger)
	 * @param detPos -1 for all detectors in  list or specific detector position
	 * @return OK or FAIL
	 */
	int sendSoftwareTrigger(int detPos = -1);

	/**
	 * Set Rate correction ( Eiger)
	 * @param t (1) enable rate correction to default dead time,
	 * (0) disable rate correction, (-1) gets
	 * @param detPos -1 for all detectors in  list or specific detector position
	 * @returns rate correction tau
	 */
	int enableCountRateCorrection(int i = -1, int detPos = -1);

	/**
	 * Set/get dynamic range
	 * @param i dynamic range (-1 get)
	 * Options: Eiger(4, 8, 16, 32), Jungfrau(16), Gotthard(16)
	 * Background operation:
	 * (Eiger: If i is 32, also sets clkdivider to 2, if 16, sets clkdivider to 1)
	 * @param detPos -1 for all detectors in  list or specific detector position
	 * @returns current dynamic range
	 */
	int setBitDepth(int i = -1, int detPos = -1);

	/**
	 * Set detector settings
	 * (Eiger only stores in shared memory. A get will overwrite this. One must use set threshold energy)
	 * @param isettings settings (-1 gets)
	 * Options: (slsDetectorDefs::detectorSettings)
	 * Eiger (STANDARD, HIGHGAIN, LOWGAIN, VERYHIGHGAIN, VERYLOWGAIN)
	 * Jungfrau (DYNAMICGAIN, DYNAMICHG0, FIXGAIN1, FIXGAIN2, FORCESWITCHG1, FORCESWITCHG2)
	 * Gotthard (DYNAMICGAIN, HIGHGAIN, LOWGAIN, MEDIUMGAIN, VERYHIGHGAIN)
	 * @param detPos -1 for all detectors in  list or specific detector position
	 * @returns current settings (can also return UNDEFINED, UNINITIALIZED)
	 */
	int setSettings(int isettings = -1, int detPos = -1);

	/**
	 * Get threshold energy (Eiger)
	 * @param detPos -1 for all detectors in  list or specific detector position
	 * @returns current threshold value
	 */
	int getThresholdEnergy(int detPos = -1);

	/**
	 * Set threshold energy (Eiger)
	 * @param e_eV threshold in eV
	 * @param tb 1 to load trimbits, 0 to exclude trimbits
	 * @param isettings settings (-1 current settings)
	 * @param detPos -1 for all detectors in  list or specific detector position
	 * @returns current threshold value
	 */
	int setThresholdEnergy(int e_ev, int tb = 1, int isettings = -1, int detPos = -1);

	/**
	 * Set/get exposure time
	 * @param t time (-1 gets)
	 * @param inseconds true if the value is in s, else ns
	 * @param detPos -1 for all detectors in  list or specific detector position
	 * @returns exposure time in ns, or s if specified
	 */
	double setExposureTime(double t = -1, bool inseconds = false, int detPos = -1);

	/**
	 * Set/get exposure period
	 * @param t time (-1 gets)
	 * @param inseconds true if the value is in s, else ns
	 * @param detPos -1 for all detectors in  list or specific detector position
	 * @returns exposure period in ns, or s if specified
	 */
	double setExposurePeriod(double t = -1, bool inseconds = false, int detPos = -1);

	/**
	 * Set/get delay after trigger (Gotthard, Jungfrau(not for this release))
	 * @param t time (-1 gets)
	 * @param inseconds true if the value is in s, else ns
	 * @param detPos -1 for all detectors in  list or specific detector position
	 * @returns delay after trigger in ns, or s if specified
	 */
	double setDelayAfterTrigger(double t = -1, bool inseconds = false, int detPos = -1);

	/**
	 * (Advanced users)
	 * Set/get sub frame exposure time (Eiger in 32 bit mode)
	 * @param t time (-1 gets)
	 * @param inseconds true if the value is in s, else ns
	 * @param detPos -1 for all detectors in  list or specific detector position
	 * @returns sub frame exposure time in ns, or s if specified
	 */
	double setSubFrameExposureTime(double t = -1, bool inseconds = false, int detPos = -1);

	/**
	 *  (Advanced users)
	 * Set/get sub frame dead time (Eiger in 32 bit mode)
	 * @param t time (-1 gets)
	 * @param inseconds true if the value is in s, else ns
	 * @param detPos -1 for all detectors in  list or specific detector position
	 * @returns sub frame dead time in ns, or s if specified
	 */
	double setSubFrameExposureDeadTime(double t = -1, bool inseconds = false, int detPos = -1);

	/**
	 * Set/get number of frames
	 * @param t number of frames (-1 gets)
	 * @param detPos -1 for all detectors in  list or specific detector position
	 * @returns number of frames
	 */
	int64_t setNumberOfFrames(int64_t t = -1, int detPos = -1);

	/**
	 * Set/get number of cycles
	 * @param t number of cycles (-1 gets)
	 * @param detPos -1 for all detectors in  list or specific detector position
	 * @returns number of cycles
	 */
	int64_t setNumberOfCycles(int64_t t = -1, int detPos = -1);

	/**
	 * Set/get number of gates (none of the detectors at the moment)
	 * @param t number of gates (-1 gets)
	 * @param detPos -1 for all detectors in  list or specific detector position
	 * @returns number of gates
	 */
	int64_t setNumberOfGates(int64_t t = -1, int detPos = -1);

	/**
	 * Set/get number of additional storage cells  (Jungfrau)
	 * @param t number of additional storage cells. Default is 0.  (-1 gets)
	 * @param detPos -1 for all detectors in  list or specific detector position
	 * @returns number of additional storage cells
	 */
	int64_t setNumberOfStorageCells(int64_t t = -1, int detPos = -1);

	/**
	 * Get measured period between previous two frames (EIGER)
	 * @param t time (-1 gets)
	 * @param inseconds true if the value is in s, else ns
	 * @param detPos -1 for all detectors in  list or specific detector position
	 * @returns sub frame dead time in ns, or s if specified
	 */
	double getMeasuredPeriod(bool inseconds = false, int detPos = -1);

	/**
	 * Get sub period between previous two sub frames in 32 bit mode (EIGER)
	 * @param t time (-1 gets)
	 * @param inseconds true if the value is in s, else ns
	 * @param detPos -1 for all detectors in  list or specific detector position
	 * @returns sub frame dead time in ns, or s if specified
	 */
	double getMeasuredSubFramePeriod(bool inseconds = false, int detPos = -1);

	/**
	 * Set/get timing mode
	 * @param pol timing mode (-1 gets)
	 * Options (slsDetectorDefs::externalCommunicationMode)
	 * (Eiger: AUTO_TIMING, TRIGGER_EXPOSURE, BURST_TRIGGER, GATE_FIX_NUMBER)
	 * (Jungfrau: AUTO_TIMING, TRIGGER_EXPOSURE)
	 * (Gotthard: AUTO_TIMING, TRIGGER_EXPOSURE)
	 * @param detPos -1 for all detectors in  list or specific detector position
	 * @returns current timing mode
	 */
	int setTimingMode(int pol = -1, int detPos = -1);

	/**
	 * Sets clock speed of the detector (Eiger, Jungfrau)
	 * (Jungfrau also writes adcphase to recommended default)
	 * (Eiger: 0(full speed not for 32 bit mode), 1 (half speed), 2(quarter speed))
	 * (Jungfrau: 0(full speed not implemented), 1(half speed), 2(quarter speed))
	 * @param detPos -1 for all detectors in  list or specific detector position
	 * @returns clock speed
	 */
	int setClockDivider(int value, int detPos = -1);

	/**
	 * Set parallel readout mode (Eiger)
	 * @param value readout mode (-1 gets)
	 * Options: slsDetectorDefs::readOutFlags
	 * (PARALLEL, NONPARALLEL (Default), SAFE)
	 * @param detPos -1 for all detectors in  list or specific detector position
	 * @returns mode register,
	 * result must be ANDED with PARALLEL/NONPARALLEL/SAFE to get mode
	 */
	int setParallelMode(int value, int detPos = -1);

	/**
	 * Set overflow readout mode (Eiger in 32 bit)
	 * @param value readout mode (-1 gets)
	 * Options: 1(SHOW_OVERFLOW), 0(NOOVERFLOW) (Default)
	 * @param detPos -1 for all detectors in  list or specific detector position
	 * @returns 1 if overflow mode else 0
	 */
	int setOverflowMode(int value, int detPos = -1);

	/**
	 * (Advanced user)
	 * Sets all the trimbits to a particular value (Eiger)
	 * @param val trimbit value
	 * @param detPos -1 for all detectors in  list or specific detector position
	 * @returns OK or FAIL
	 */
	int setAllTrimbits(int val, int detPos = -1);

	/**
	 * (Advanced user)
	 * Set/get dacs value
	 * @param val value (in V) (-1 gets)
	 * @param index DAC index
	 * Options: slsDetectorDefs::dacIndex
	 * (Eiger: E_SvP up to IO_DELAY, THRESHOLD, HIGH_VOLTAGE)
	 * (Jungfrau: 0-7)
	 * (Gotthard: G_VREF_DS up to G_IB_TESTC, HIGH_VOLTAGE)
	 * @param detPos -1 for all detectors in  list or specific detector position
	 * @returns current DAC value
	 */
	int setDAC(int val, int index , int detPos = -1);

	/**
	 * Get adc value
	 * @param index adc(DAC) index
	 * Options: slsDetectorDefs::dacIndex
	 * (Eiger: TEMPERATURE_FPGA, TEMPERATURE_FPGAEXT upto TEMPERATURE_FPGA3)
	 * (Jungfrau: TEMPERATURE_FPGA)
	 * (Gotthard: TEMPERATURE_ADC, TEMPERATURE_FPGA)
	 * @param detPos -1 for all detectors in  list or specific detector position
	 * @returns current adc value (temperature for eiger and jungfrau in millidegrees)
	 */
	int getADC(int index, int detPos = -1);

	/**
	 * Enable/disable or 10Gbe (Eiger)
	 * @param i is -1 to get, 0 to disable and 1 to enable
	 * @param detPos -1 for all detectors in  list or specific detector position
	 * @returns if 10Gbe is enabled
	 */
	int setTenGigabitEthernet(int i = -1, int detPos = -1);

	/**
	 * Set storage cell that stores first acquisition of the series (Jungfrau)
	 * @param value storage cell index. Value can be 0 to 15. (-1 gets)
	 * @param detPos -1 for all detectors in  list or specific detector position
	 * @returns the storage cell that stores the first acquisition of the series
	 */
	int setStoragecellStart(int pos=-1, int detPos = -1);

	/**
	 * set high voltage (Gotthard, Jungfrau, Eiger)
	 * @param i > 0 sets, 0 unsets, (-1 gets)
	 * (Eiger: )
	 * (Jungfrau: )
	 * (Gotthard: )
	 * @returns high voltage
	 */
	int setHighVoltage(int i = -1, int detPos = -1);

	/**
	 * Set 10GbE Flow Control (Eiger)
	 * @param enable 1 to set, 0 to unset, -1 gets
	 * @param detPos -1 for all detectors in  list or specific detector position
	 * @returns 10GbE flow Control
	 */
	int setFlowControl10G(int enable = -1, int detPos = -1);

    /**
     * Set ROI (Gotthard) (>= 1 roi, but max 1 roi per module)
     * At the moment only one set allowed
     * @param n number of rois
     * @param roiLimits array of roi
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns OK or FAIL
     */
    int setROI(int n=-1, slsDetectorDefs::ROI roiLimits[]=NULL, int detPos = -1);

    /**
     * Get ROI from each detector and convert it to the multi detector scale (Gotthard)
     * >= 1 roi, but max 1 roi per module
     * @param n number of rois
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns pointer to array of ROI structure
     */
    const slsDetectorDefs::ROI* getROI(int &n, int detPos = -1);



	/************************************************************************

                            RECEIVER FUNCTIONS

	 *********************************************************************/

	/**
	 * (Advanced user, included in startMeasurement)
	 * Receiver starts listening to packets
	 * @param detPos -1 for all detectors in  list or specific detector position
	 * @returns OK or FAIL
	 */
	int startReceiver(int detPos = -1);

	/**
	 * (Advanced user, included in startMeasurement)
	 * Stops the listening mode of receiver
	 * @param detPos -1 for all detectors in  list or specific detector position
	 * @returns OK or FAIL
	 */
	int stopReceiver(int detPos = -1);

	/**
	 * Set/get receiver silent mode
	 * @param i is -1 to get, 0 unsets silent mode, 1 sets silent mode
	 * @param detPos -1 for all detectors in  list or specific detector position
	 * @returns the receiver silent mode enable
	 */
	int setReceiverSilentMode(int i = -1, int detPos = -1);

	/**
	 * (Advanced user, included in startMeasurement)
	 * Resets framescaught in receiver
	 * Use this when using startAcquisition instead of acquire
	 * @param detPos -1 for all detectors in  list or specific detector position
	 * @returns OK or FAIL
	 */
	int resetFramesCaughtInReceiver(int detPos = -1);

	/**
	 * (Advanced user)
	 * Set/get receiver fifo depth
	 * @param i is -1 to get, any other value to set the fifo deph
	 * @param detPos -1 for all detectors in  list or specific detector position
	 * @returns the receiver fifo depth
	 */
	int setReceiverFifoDepth(int i = -1, int detPos = -1);

	/**
	 * Returns output file directory
	 * @param detPos -1 for all detectors in  list or specific detector position
	 * @returns output file directory
	 */
	std::string getFilePath(int detPos = -1);

	/**
	 * Sets up the file directory
	 * @param detPos -1 for all detectors in  list or specific detector position
	 * @param s file directory
	 * @returns file dir
	 */
	std::string setFilePath(const std::string& s, int detPos = -1);

	/**
	 * Returns file name prefix
	 * @param detPos -1 for all detectors in  list or specific detector position
	 * @returns file name prefix
	 */
	std::string getFileName(int detPos = -1);

	/**
	 * Sets up the file name prefix
	 * @param detPos -1 for all detectors in  list or specific detector position
	 * @param s file name prefix
	 * @returns file name prefix
	 */
	std::string setFileName(const std::string& s, int detPos = -1);

	/**
	 * Returns file index
	 * @param detPos -1 for all detectors in  list or specific detector position
	 * @returns file index
	 */
	int getFileIndex(int detPos = -1);

	/**
	 * Sets up the file index
	 * @param i file index
	 * @param detPos -1 for all detectors in  list or specific detector position
	 * @returns file index
	 */
	int setFileIndex(int i, int detPos = -1);

	/**
	 * Sets/Gets receiver file write enable
	 * @param enable 1 or 0 to set/reset file write enable
	 * @param detPos -1 for all detectors in  list or specific detector position
	 * @returns file write enable
	 */
	int enableWriteToFile(int enable = -1, int detPos = -1);

	/**
	 * Sets/Gets file overwrite enable
	 * @param enable 1 or 0 to set/reset file overwrite enable
	 * @param detPos -1 for all detectors in  list or specific detector position
	 * @returns file overwrite enable
	 */
	int enableOverwriteFile(int enable = -1, int detPos = -1);

	/**
	 * (previously setReceiverMode)
	 * Sets the receiver streaming frequency
	 * @param freq nth frame streamed out, if 0, streamed out at a timer of 200 ms
	 * frames in between are not streamed
	 * @param detPos -1 for all detectors in  list or specific detector position
	 * @returns receiver streaming frequency
	 */
	int setReceiverStreamingFrequency(int freq = -1, int detPos = -1);

	/**
	 * Sets the receiver streaming timer
	 * If receiver streaming frequency is 0, then this timer between each
	 * data stream is set. Default is 200 ms.
	 * @param time_in_ms timer between frames
	 * @param detPos -1 for all detectors in  list or specific detector position
	 * @returns receiver streaming timer in ms
	 */
	int setReceiverStreamingTimer(int time_in_ms=500, int detPos = -1);

	/**
	 * Enable data streaming to client (data call back in client processing thread)
	 * @param enable 0 to disable, 1 to enable, -1 to get the value
	 * @returns data streaming to client enable
	 */
	int enableDataStreamingToClient(int enable=-1);

	/**
	 * Enable or disable streaming data from receiver (starts streaming threads)
	 * @param enable 0 to disable 1 to enable -1 to only get the value
	 * @param detPos -1 for all detectors in  list or specific detector position
	 * @returns data streaming from receiver enable
	 */
	int enableDataStreamingFromReceiver(int enable=-1, int detPos = -1);

	/**
	 * (advanced users)
	 * Set/Get receiver streaming out ZMQ port and restarts receiver sockets
	 * @param i sets, -1 gets
	 * If detPos is -1(multi module), port calculated (increments) for all the individual detectors using i
	 * @param detPos -1 for all detectors in  list or specific detector position
	 * @returns receiver streaming out ZMQ port (if multiple, of first receiver socket)
	 */
	int setReceiverDataStreamingOutPort(int i = -1, int detPos = -1);

	/**
	 * (advanced users)
	 * Set/Get client streaming in  ZMQ port and restarts client sockets
	 * @param i sets, -1 gets
	 * If detPos is -1(multi module), port calculated (increments) for all the individual detectors using i
	 * @param detPos -1 for all detectors in  list or specific detector position
	 * @returns receiver streaming out ZMQ port (if multiple, of first receiver socket)
	 */
	int setClientDataStreamingInPort(int i = -1, int detPos = -1);

	/**
	 * (advanced users)
	 * Set/Get receiver streaming out ZMQ IP and restarts receiver sockets
	 * @param i sets, empty string gets
	 * By default, it is the IP of receiver hostname
	 * @param detPos -1 for all detectors in  list or specific detector position
	 * @returns receiver streaming out ZMQ IP
	 */
	std::string setReceiverDataStreamingOutIP(const std::string& ip="", int detPos = -1);

	/**
	 * (advanced users)
	 * Set/Get client streaming in ZMQ IP and restarts client sockets
	 * @param i sets, empty string gets
	 * By default, it is the IP of receiver hostname
	 * @param detPos -1 for all detectors in  list or specific detector position
	 * @returns client streaming in ZMQ IP
	 */
	std::string setClientDataStreamingInIP(const std::string& ip = "", int detPos = -1);

	/**
	 * Enable gap pixels in receiver (Eiger for 8,16 and 32 bit mode)
	 * 4 bit mode gap pixels only in data call back in client
	 * @param val 1 sets, 0 unsets, -1 gets
	 * @param detPos -1 for all detectors in  list or specific detector position
	 * @returns gap pixel enable
	 */
	int enableGapPixels(int val=-1, int detPos = -1);

	/**
	 * Sets the frame discard policy in receiver
	 * @param f frame discard policy (-1 gets)
	 * Options: (slsDetectorDefs::frameDiscardPolicy)
	 * (NO_DISCARD (default), DISCARD_EMPTY_FRAMES, DISCARD_PARTIAL_FRAMES (fastest))
	 * @param detPos -1 for all detectors in  list or specific detector position
	 * @returns current frame discard policy
	 */
	int setReceiverFramesDiscardPolicy(int f = -1, int detPos = -1);

	/**
	 * Sets the frame padding in receiver
	 * @param f 0 does not partial frames, 1 pads partial frames (-1 gets)
	 * @param detPos -1 for all detectors in  list or specific detector position
	 * @returns partial frames padding enable
	 */
	int setReceiverPartialFramesPadding(int f = -1, int detPos = -1);

	/**
	 * Sets the frames per file in receiver
	 * @param f frames per file, 0 is infinite ie. every frame in same file (-1 gets)
	 * @param detPos -1 for all detectors in  list or specific detector position
	 * @returns frames per file
	 */
	int setReceiverFramesPerFile(int f = -1, int detPos = -1);

	/**
	 * Sets the detector minimum/maximum energy threshold in processor (for Moench only)
	 * @param index 0 for emin, antyhing else for emax
	 * @param v value to set (-1 gets)
     * @param detPos -1 for all detectors in  list or specific detector position
	 * @returns detector minimum/maximum energy threshold
	 */
	int setDetectorMinMaxEnergyThreshold(const int index, int v, int detPos = -1);

	/**
	 * Sets the frame mode in processor (Moench only)
	 * @param value frame mode value (-1 gets)
	 * Options (slsDetectorDefs::frameModeType)
	 * PEDESTAL, NEW_PEDESTAL, FLATFIELD, NEW_FLATFIELD
     * @param detPos -1 for all detectors in  list or specific detector position
	 * @returns frame mode (-1 for not found or error in computing json parameter value)
	 */
	int setFrameMode(int value, int detPos = -1);

    /**
     * Sets the detector mode in processor (Moench only)
     * @param value detector mode value (-1 gets)
     * Options (slsDetectorDefs::detectorModeType)
     * COUNTING, INTERPOLATING, ANALOG
     * @param detPos -1 for all detectors in  list or specific detector position
     * @returns detector mode (-1 for not found or error in computing json parameter value)
     */
    int setDetectorMode(int value, int detPos = -1);

	/************************************************************************

	                       CALLBACKS & COMMAND LINE PARSING

	 *********************************************************************/

	/**
	 * register callback for accessing detector final data in client,
	 * also enables data streaming in client and receiver
	 * @param userCallback function for plotting/analyzing the data.
	 * Its arguments are
	 * the data structure d and the frame number f,
	 * s is for subframe number for eiger for 32 bit mode
	 * @param pArg argument
	 */

	void registerDataCallback(int( *userCallback)(detectorData* d, int f, int s, void*), void *pArg);

	/**
	 * register callback for accessing acquisition final data in client,
	 * @param func function to be called at the end of the acquisition.
	 * gets detector status and progress index as arguments
	 * @param pArg argument
	 */
	void registerAcquisitionFinishedCallback(int( *func)(double,int, void*), void *pArg);

	/**
	 * register callback for accessing measurement final data in client,
	 * @param func function to be called at the end of the acquisition.
	 * gets detector status and progress index as arguments
	 * @param pArg argument
	 */
	void registerMeasurementFinishedCallback(int( *func)(int,int, void*), void *pArg);

	/**
	 * register callback for accessing detector progress in client,
	 * @param func function to be called at the end of the acquisition.
	 * gets detector status and progress index as arguments
	 * @param pArg argument
	 */
	void registerProgressCallback(int( *func)(double,void*), void *pArg);

	/**
     @short [usage strongly discouraged] sets parameters trough command line interface http://www.psi.ch/detectors/UsersSupportEN/slsDetectorClientHowTo.pdf
     \param command string as it would be written on the command line
     \returns void
	 */
	void putCommand(const std::string& command);





	/************************************************************************

                           STATIC FUNCTIONS

	 *********************************************************************/

	/** @short returns std::string from run status index
      \param s run status index
      \returns std::string error, waiting, running, data, finished or unknown when wrong index
	 */
	static std::string runStatusType(int s){					\
		switch (s) {							\
		case 0:     return std::string("idle");					\
		case 1:       return std::string("error");				\
		case 2:      return  std::string("waiting");				\
		case 3:      return std::string("finished");				\
		case 4:      return std::string("data");					\
		case 5:      return std::string("running");				\
        case 6:      return std::string("stopped");             \
		default:       return std::string("unknown");				\
		}};



	/** @short returns detector settings std::string from index
      \param s can be standard, fast, highgain, dynamicgain, lowgain, mediumgain, veryhighgain
      \returns   setting index (-1 unknown std::string)
	 */

	static int getDetectorSettings(std::string s){		\
		if (s=="standard") return 0;			\
		if (s=="fast") return 1;				\
		if (s=="highgain") return 2;			\
		if (s=="dynamicgain") return 3;			\
		if (s=="lowgain") return 4;				\
		if (s=="mediumgain") return 5;			\
		if (s=="veryhighgain") return 6;			\
		return -1;				         };

	/** @short returns detector settings std::string from index
      \param s settings index
      \returns standard, fast, highgain, dynamicgain, lowgain, mediumgain, veryhighgain, undefined when wrong index
	 */
	static std::string getDetectorSettings(int s){\
		switch(s) {						\
		case 0:      return std::string("standard");\
		case 1:      return std::string("fast");\
		case 2:      return std::string("highgain");\
		case 3:    return std::string("dynamicgain");	\
		case 4:    return std::string("lowgain");		\
		case 5:    return std::string("mediumgain");	\
		case 6:    return std::string("veryhighgain");			\
		default:    return std::string("undefined");			\
		}};



	/**
     @short returns external communication mode std::string from index
     \param f index for communication mode
     \returns  auto, trigger, ro_trigger, gating, triggered_gating, unknown when wrong mode
	 */

	static std::string getTimingMode(int f){	\
		switch(f) {						 \
		case 0:      return std::string( "auto");			\
		case 1: return std::string("trigger");			\
		case 2: return std::string("ro_trigger");				\
		case 3: return std::string("gating");			\
		case 4: return std::string("triggered_gating");	\
		case 5: return std::string("burst_trigger");	\
		default:    return std::string( "unknown");				\
		}      };

	/**
     @short returns external communication mode std::string from index
     \param s index for communication mode
     \returns  auto, trigger, ro_trigger, gating, triggered_gating, unknown when wrong mode
	 */

	static int getTimingMode(std::string s){			\
		if (s== "auto") return 0;						\
		if (s== "trigger") return 1;					\
		if (s== "ro_trigger") return 2;					\
		if (s== "gating") return 3;						\
		if (s== "triggered_gating") return 4;			\
		return -1;							};


private:
	multiSlsDetector detector;
};

#endif
