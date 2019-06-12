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
class multiSlsDetector;
class multiSlsDetectorCommand;

#include "sls_detector_defs.h"


#include <stdint.h>
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


   \authors <a href="mailto:anna.bergamaschi@psi.ch">Anna Bergamaschi</a>, <a href="mailto:dhanya.thattil@psi.ch">Dhanya Thattil</a>
   @version 3.0
<H2>Currently supported detectors</H2>
\li MYTHEN
\li GOTTHARD controls
\li GOTTHARD data receiver
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

  /** @short default constructor
   * @param ret address of return value. It will be set to 0 for success, else 1 for failure
   * @param id multi detector id
   * in creating multidetector object
   */
   slsDetectorUsers(int& ret, int id=0);
   
   /**  @short virtual destructor */
   virtual ~slsDetectorUsers();

   /**
       @short useful to define subset of working functions
      \returns "PSI" or "Dectris"
   */
   std::string getDetectorDeveloper();

  /**  @short sets the onlineFlag
      \param online can be: -1 returns wether the detector is in online (1) or offline (0) state; 0 detector in offline state; 1  detector in online state
      \returns 0 (offline) or 1 (online)
  */
  int setOnline(int const online=-1);

  /**  @short sets the receivers onlineFlag
      \param online can be: -1 returns wether the receiver is in online (1) or offline (0) state; 0 receiver in offline state; 1  receiver in online state
      \returns 0 (offline) or 1 (online)
  */
  int setReceiverOnline(int const online=-1);


  /**
      @short start measurement and acquires
    \returns OK/FAIL
  */
  void startMeasurement();

  /**
      @short stop measurement
    \returns OK/FAIL
  */
   int stopMeasurement();
 
  /**
      @short get run status
    \returns status mask
  */
   int getDetectorStatus();

  /**
      @short returns the default output files path
  */
   std::string getFilePath();

 /**
      @short sets the default output files path
     \param s file path
     \returns file path
  */
   std::string setFilePath(std::string s);

  /** 
      @short 
      \returns the default output files root name
  */
   std::string getFileName();  

  /**
      @short sets the default output files path
     \param s file name
     \returns the default output files root name
     
  */
   std::string setFileName(std::string s);  
  
  /** 
      @short 
     \returns the default output file index
  */
   int getFileIndex();
  
  /**
          @short sets the default output file index
     \param i file index
     \returns the default output file index
  */
   int setFileIndex(int i);

   /**
         @short get flat field corrections file directory
    \returns flat field correction file directory
  */
   std::string getFlatFieldCorrectionDir(); 
  
  /** 
           @short set flat field corrections file directory
      \param dir flat field correction file directory
      \returns flat field correction file directory
  */
   std::string setFlatFieldCorrectionDir(std::string dir);
  
  /** 
           @short get flat field corrections file name
      \returns flat field correction file name
  */
   std::string getFlatFieldCorrectionFile();
  
  /** 
           @short set flat field correction file
      \param fname name of the flat field file (or "" if disable)
      \returns 0 if disable (or file could not be read), >0 otherwise
  */
   int setFlatFieldCorrectionFile(std::string fname=""); 

  

  /** 
           @short enable/disable flat field corrections (without changing file name)
      \param i 0 disables, 1 enables, -1 gets
      \returns 0 if ff corrections disabled, 1 if enabled
  */
   int enableFlatFieldCorrection(int i=-1);

  /**
           @short enable/disable count rate corrections 
      \param i 0 disables, 1 enables with default values, -1 gets
      \returns 0 if count corrections disabled, 1 if enabled
  */
   int enableCountRateCorrection(int i=-1);

  /**
           @short enable/disable bad channel corrections 
      \param i 0 disables, 1 enables, -1 gets
      \returns 0 if bad channels corrections disabled, 1 if enabled
  */
   int enablePixelMaskCorrection(int i=-1);

  /**
           @short enable/disable angular conversion 
      \param i 0 disables, 1 enables, -1 gets
      \returns 0 if angular conversion disabled, 1 if enabled
  */
   int enableAngularConversion(int i=-1);

  /**Enable write file function included*/

   int enableWriteToFile(int i=-1);

  /** 
           @short set  positions for the acquisition
      \param nPos number of positions
      \param pos array with the encoder positions
      \returns number of positions
  */
   int setPositions(int nPos, double *pos);
  
  /** 
           @short get  positions for the acquisition
      \param pos array which will contain the encoder positions
      \returns number of positions
  */
   int getPositions(double *pos=NULL);
  
  /**
     @short sets the detector size (only 1 ROI)
     \param x0 horizontal position origin in channel number (-1 unchanged)
     \param y0 vertical position origin in channel number (-1 unchanged)
     \param nx number of channels in horiziontal  (-1 unchanged)
     \param  ny number of channels in vertical  (-1 unchanged)
     \returns OK/FAIL
  */
   int setDetectorSize(int x0=-1, int y0=-1, int nx=-1, int ny=-1);

  /**
     @short gets detector size (roi size if only one roi)
     \param x0 horizontal position origin in channel number 
     \param y0 vertical position origin in channel number 
     \param nx number of channels in horiziontal
     \param  ny number of channels in vertical 
     \returns total number of channels
  */
   int getDetectorSize(int &x0, int &y0, int &nx, int &ny);
  /**
     @short gets the maximum detector size
     \param nx number of channels in horiziontal
     \param  ny number of channels in vertical 
     \returns OK/FAIL
  */
   int getMaximumDetectorSize(int &nx, int &ny);


    /** 
	@short set/get dynamic range
      \param i dynamic range (-1 get)
      \returns current dynamic range
  */
   int setBitDepth(int i=-1);


 
   /**
         @short set detector settings
    \param isettings  settings index (-1 gets)
    \returns current settings
  */
   int setSettings(int isettings=-1);
   
  /**
         @short get threshold energy
    \returns current threshold value for imod in ev (-1 failed)
  */
   int getThresholdEnergy();  


  /**
     @short set threshold energy
    \param e_eV threshold in eV
    \returns current threshold value for imod in ev (-1 failed)
  */
   int setThresholdEnergy(int e_eV);

   /**
    @short set threshold energy with choice to load trimbits (eiger only)
    \param e_ev threshold in ev
    \param tb 1 loads trimbits, 0 does not load trimbits
    \param isettings settings index (-1 uses current setting)
    \param id module index (-1 for all)
    \returns current threshold value in ev (-1 failed)
    */
   int setThresholdEnergy(int e_ev, int tb, int isettings = -1, int id = -1);


   /**
        @short set/get exposure time value
        \param t time in sn  (-1 gets)
        \param inseconds true if the value is in s, else ns
        \param imod module number (-1 for all)
        \returns timer set value in ns, or s if specified
    */

   double setExposureTime(double t=-1, bool inseconds=false, int imod = -1);

    /**
         @short set/get exposure period
        \param t time in ns   (-1 gets)
        \param inseconds true if the value is in s, else ns
        \param imod module number (-1 for all)
        \returns timer set value in ns, or s if specified
    */
   double setExposurePeriod(double t=-1, bool inseconds=false, int imod = -1);

    /**
         @short set/get delay after trigger
        \param t time in ns   (-1 gets)
        \param inseconds true if the value is in s, else ns
        \param imod module number (-1 for all)
        \returns timer set value in ns, or s if specified
    */
   double setDelayAfterTrigger(double t=-1, bool inseconds=false, int imod = -1);

  /** 
       @short set/get number of gates
      \param t number of gates  (-1 gets)
      \param imod module number (-1 for all)
      \returns number of gates
  */
   int64_t setNumberOfGates(int64_t t=-1, int imod = -1);
  
  /** 
       @short set/get number of frames i.e. number of exposure per trigger
      \param t number of frames  (-1 gets) 
      \param imod module number (-1 for all)
      \returns number of frames
  */
   int64_t setNumberOfFrames(int64_t t=-1, int imod = -1);

  /** 
       @short set/get number of cycles i.e. number of triggers
      \param t number of frames  (-1 gets) 
      \param imod module number (-1 for all)
      \returns number of frames
  */
   int64_t setNumberOfCycles(int64_t t=-1, int imod = -1);

 /** 
      @short set/get the external communication mode 
      \param pol value to be set \sa getTimingMode
      \returns current external communication mode
  */
   int setTimingMode(int pol=-1);

  /**
      @short Reads the configuration file -- will contain all the informations needed for the configuration (e.g. for a PSI detector caldir, settingsdir, angconv, badchannels, hostname etc.)
     \param fname file name
     \returns OK or FAIL
  */  
   int readConfigurationFile(std::string const fname);  


  /** 
       @short Reads the parameters from the detector and writes them to file
      \param fname file to write to
      \returns OK or FAIL
  
  */
   int dumpDetectorSetup(std::string const fname); 
  /** 
      @short Loads the detector setup from file
      \param fname file to read from
      \returns OK or FAIL
  
  */
   int retrieveDetectorSetup(std::string const fname);

  /**
      @short useful for data plotting etc.
     \returns Mythen, Eiger, Gotthard etc.
  */
   std::string getDetectorType();

   /**
       @short sets the mode by which gui requests data from receiver
       \param n is 0 for random requests for fast acquisitions and greater than 0 for nth read requests
      \returns the mode set in the receiver
   */
   int setReceiverMode(int n=-1);

  /**
     @short register calbback for accessing detector final data, also enables data streaming in client and receiver (if receiver exists)
     \param userCallback function for plotting/analyzing the data. Its arguments are  the data structure d and the frame number f, s is for subframe number for eiger for 32 bit mode
     \param pArg argument
  */

   void registerDataCallback(int( *userCallback)(detectorData* d, int f, int s, void*), void *pArg);

  /**
     @short register callback for accessing raw data - if the rawDataCallback is registered, no filewriting/postprocessing will be carried on automatically by the software - the raw data are deleted by the software
     \param userCallback function for postprocessing and saving the data -  p is the pointer to the data, n is the number of channels
     \param pArg argument
  */
  
   void registerRawDataCallback(int( *userCallback)(double* p, int n, void*), void *pArg);

  /** 
     @short function to initalize a set of measurements (reset binning if angular conversion, reset summing otherwise)  - can be overcome by the user's functions thanks to the virtual property
     \param refresh if 1, all parameters like ffcoefficients, badchannels, ratecorrections etc. are reset (should be called at least onece with this option), if 0 simply reset merging/ summation
  */
  
  virtual void initDataset(int refresh);


  /**
     @short adds frame to merging/summation  - can be overcome by the user's functions thanks to the virtual property
     \param data pointer to the raw data
     \param pos encoder position
     \param i0 beam monitor readout for intensity normalization (if 0 not performed)
     \param t exposure time in seconds, required only if rate corrections
     \param fname file name (unused since filewriting would be performed by the user)
     \param var optional parameter - unused.
  */
  
  virtual void addFrame(double *data, double pos, double i0, double t, std::string fname, double var);

  /**
     @short finalizes the data set returning the array of angles, values and errors to be used as final data - can be overcome by the user's functions thanks to the virtual property
     \param a pointer to the array of angles - can be null if no angular coversion is required
     \param v pointer to the array of values
     \param e pointer to the array of errors
     \param np reference returning the number of points
  */
  
  virtual void finalizeDataset(double *a, double *v, double *e, int &np); 


  /** Enable or disable streaming data from receiver (creates transmitting sockets)
   * @param i 0 to disable 1 to enable -1 to only get the value
   * @returns data streaming from receiver enable
  */
   int enableDataStreamingFromReceiver(int i=-1);

   /**
    * Enable data streaming to client (creates receiving sockets)
    * @param i 0 to disable, 1 to enable, -1 to get the value
    * @returns data streaming to client enable
    */
   int enableDataStreamingToClient(int i=-1);

   /** (for expert users)
    * Set/Get receiver streaming out ZMQ port
    * For multi modules, it calculates (increments), sets the ports and restarts the sockets
    * @param i sets, -1 gets
    * @returns receiver streaming out ZMQ port (if multiple, of first receiver socket)
    */
   int setReceiverDataStreamingOutPort(int i=-1);

   /** (for expert users)
    * Set/Get client streaming in ZMQ port
    * For multi modules, it calculates (increments), sets the ports and restarts the sockets
    * @param i sets, -1 gets
    * @returns client streaming in ZMQ port (if multiple, of first client socket)
    */
   int setClientDataStreamingInPort(int i=-1);

   /** (for expert users)
    * Set/Get receiver streaming out ZMQ IP
    * By default, it is the IP of receiver hostname
    * @param ip sets, empty std::string gets
    * @returns receiver streaming out ZMQ IP
    */
   std::string setReceiverDataStreamingOutIP(std::string ip="");

   /** (for expert users)
    * Set/Get client streaming in ZMQ IP
    * By default, it is the IP of receiver hostname
    * @param ip sets, empty std::string gets
    * @returns client streaming in ZMQ IP
    */
   std::string setClientDataStreamingInIP(std::string ip="");

  /**
     get get Module Firmware Version
     @param imod module number
     \returns id
  */
  int64_t getModuleFirmwareVersion(int imod=-1);

  /**
     get get Module Serial Number (only mythen)
     @param imod module number
     \returns id
  */
  int64_t getModuleSerialNumber(int imod=-1);

  /**
     get get Detector Firmware Version
     @param imod module number
     \returns id
  */
  int64_t getDetectorFirmwareVersion(int imod=-1);

  /**
     get get Detector Serial Number
     @param imod module number
     \returns id
  */
  int64_t getDetectorSerialNumber(int imod=-1);

  /**
     get get Detector Software Version
     @param imod module number
     \returns id
  */
  int64_t getDetectorSoftwareVersion(int imod=-1);

  /**
     get this Software Version
     \returns id
  */
  int64_t getThisSoftwareVersion();

  /**
   * Enable gap pixels, only for Eiger and for 8,16 and 32 bit mode.
   * 4 bit mode gap pixels only in gui call back (registerDataCallback)
   * @param enable 1 sets, 0 unsets, -1 gets
   * @return gap pixel enable or -1 for error
   */
  int enableGapPixels(int enable=-1);

  /**
   * Sets the frames discard policy in receiver
   * frame discard policy options:
   * @param f nodiscard (default),discardempty, discardpartial (fastest), get to get the value
   * @returns f nodiscard (default),discardempty, discardpartial (fastest)
   */
  std::string setReceiverFramesDiscardPolicy(std::string f="get");

  /**
   * Sets the frame padding in receiver
   * @param f 0 does not partial frames, 1 pads partial frames (-1 gets)
   * @returns partial frames padding enable
   */
  int setReceiverPartialFramesPadding(int f = -1);

  /**
   * Sets the frames per file in receiver
   * @param f frames per file, 0 is infinite ie. every frame in same file (-1 gets)
   * @returns frames per file
   */
  int setReceiverFramesPerFile(int f = -1);

  /**
   * Sends a software internal trigger (EIGER only)
   * @returns 0 for success, 1 for fail
   */
  int sendSoftwareTrigger();

  /**
   * get measured period between previous two frames(EIGER only)
   * @param inseconds true if the value is in s, else ns
   * @param imod module number (-1 for all)
   * @returns measured period
  */
  double getMeasuredPeriod(bool inseconds=false, int imod = -1);

  /**
   * get measured sub period between previous two sub frames in 32 bit mode (EIGER only)
   * @param inseconds true if the value is in s, else ns
   * @param imod module number (-1 for all)
   * @returns measured sub period
  */
  double getMeasuredSubFramePeriod(bool inseconds=false, int imod = -1);

  /**
     @short register calbback for accessing detector final data
     \param func function to be called at the end of the acquisition. gets detector status and progress index as arguments
     \param pArg argument
  */
   void registerAcquisitionFinishedCallback(int( *func)(double,int, void*), void *pArg);
  
  /**
     @short register calbback for reading detector position
     \param func function for reading the detector position
     \param arg argument
  */
  
   void registerGetPositionCallback( double (*func)(void*),void *arg);
  /**
     @short register callback for connecting to the epics channels
     \param func function for connecting to the epics channels
     \param arg argument
  */
   void registerConnectChannelsCallback( int (*func)(void*),void *arg);
  /**
     @short register callback to disconnect the epics channels
     \param func function to disconnect the epics channels
     \param arg argument
  */
   void registerDisconnectChannelsCallback( int (*func)(void*),void *arg);  
  /**
     @short register callback for moving the detector
     \param func function for moving the detector
     \param arg argument
  */
   void registerGoToPositionCallback( int (*func)(double,void*),void *arg);
  /**
     @short register callback for moving the detector without waiting
     \param func function for moving the detector
     \param arg argument
  */
   void registerGoToPositionNoWaitCallback( int (*func)(double,void*),void *arg);
  /**
     @short register calbback reading to I0
     \param func function for reading the I0 (called with parameter 0 before the acquisition, 1 after and the return value used as I0)
     \param arg argument
  */
   void registerGetI0Callback( double (*func)(int,void*),void *arg);

   /**
     @short sets parameters in command interface http://www.psi.ch/detectors/UsersSupportEN/slsDetectorClientHowTo.pdf
     \param narg value to be set
     \param args value to be set
     \param pos position of detector in multislsdetector list
     \returns answer std::string
    */
   std::string putCommand(int narg, char *args[], int pos=-1);

   /**
     @short gets parameters in command interface http://www.psi.ch/detectors/UsersSupportEN/slsDetectorClientHowTo.pdf
     \param narg value to be set
     \param args value to be set
     \param pos position of detector in multislsdetector list
     \returns answer std::string
    */
   std::string getCommand(int narg, char *args[], int pos=-1);

   /************************************************************************

                            ADVANCED FUNCTIONS

   *********************************************************************/
   /**
      @short sets clock divider of detector
      \param value value to be set (-1 gets)
      \returns speed of detector
    */
   int setClockDivider(int value);

    /**
      @short sets parallel mode
      \param value 0 for non parallel, 1 for parallel, 2 for safe mode (-1 gets)
      \returns gets parallel mode
    */
   int setParallelMode(int value);

   /**
    * @short show saturated for overflow in subframes in 32 bit mode (eiger only)
    * \param value 0 for do not show saturatd, 1 for show saturated (-1 gets)
    * \returns overflow mode enable in 32 bit mode
    */
   int setOverflowMode(int value);

    /**
      @short sets all trimbits to value (only available for eiger)
      \param val value to be set (-1 gets)
      \param id module index (-1 for all)
      \returns value set
    */
   int setAllTrimbits(int val, int id = -1);

   /**
      @short set dac value
      \param dac dac as std::string. can be vcmp_ll, vcmp_lr, vcmp_rl, vcmp_rr, vthreshold, vrf, vrs, vtr, vcall, vcp. others not supported
      \param val value to be set (-1 gets)
      \param id module index (-1 for all)
      \returns dac value or -1 (if id=-1 & dac value is different for all modules) or -9999 if dac std::string does not match
    */
   int setDAC(std::string dac, int val, int id = -1);

   /**
      @short get adc value
      \param adc adc as std::string. can be temp_fpga, temp_fpgaext, temp_10ge, temp_dcdc, temp_sodl, temp_sodr, temp_fpgafl, temp_fpgafr. others not supported
      \param id module index (-1 for all)
      \returns adc value in millidegree Celsius or -1 (if id=-1 & adc value is different for all modules) or -9999 if adc std::string does not match
    */
   int getADC(std::string adc, int id = -1);

   /**
      @short start receiver listening mode
      \returns returns OK or FAIL
    */
   int startReceiver();

   /**
      @short stop receiver listening mode
      \returns returns OK or FAIL
    */
   int stopReceiver();

   /**
      start detector real time acquisition in non blocking mode
      does not include scans, scripts, incrementing file index, s
      tarting/stopping receiver, resetting frames caught in receiver
      \returns OK if all detectors are properly started, FAIL otherwise
   */
   int startAcquisition();

   /**
      stop detector real time acquisition
      \returns OK if all detectors are properly started, FAIL otherwise
   */
   int stopAcquisition();

   /**
    * set receiver in silent mode
    * @param i 1 sets, 0 unsets (-1 gets)
    * @returns silent mode of receiver
    */
   int setReceiverSilentMode(int i);

   /**
    * set high voltage
    * @param i > 0 sets, 0 unsets, (-1 gets)
    * @returns high voltage
    */
   int setHighVoltage(int i);

   /**
    * reset frames caught in receiver
    * should be called before startReceiver()
    * @returns OK or FAIL
    */
   int resetFramesCaughtInReceiver();

   /**
    * set receiver fifo depth
    * @param i number of images in fifo depth (-1 gets)
    * @returns receiver fifo depth
    */
   int setReceiverFifoDepth(int i = -1);

   /**
    * set flow control for 10Gbe (Eiger only)
    * @param i 1 sets, 0 unsets (-1 gets)
    * @return flow control enable for 10 Gbe
    */
   int setFlowControl10G(int i = -1);

   /**
    * enable/disable 10GbE (Eiger only)
    * @param i 1 sets, 0 unsets (-1 gets)
    * @return 10GbE enable
    */
   int setTenGigabitEthernet(int i = -1);

   /**
    * returns total number of detector modules
    * @returns the total number of detector modules
    */
   int getNMods();

   /**
    * Set sub frame exposure time (only for Eiger)
    * @param t sub frame exposure time (-1 gets)
    * @param inseconds true if the value is in s, else ns
    * @param imod module number (-1 for all)
    * @returns sub frame exposure time in ns, or s if specified
    */
   double setSubFrameExposureTime(double t=-1, bool inseconds=false, int imod = -1);

   /**
    * Set sub frame dead time (only for Eiger)
    * Very advanced feature. Meant to be a constant in config file by an expert for each individual module
    * @param t sub frame dead time (-1 gets)
    * @param inseconds true if the value is in s, else ns
    * @param imod module number (-1 for all)
    * @returns sub frame dead time in ns, or s if specified
    */
   double setSubFrameExposureDeadTime(double t=-1, bool inseconds=false, int imod = -1);

   /**
    * set/get number of additional storage cells  (Jungfrau)
    * @param t number of additional storage cells. Default is 0.  (-1 gets)
    * @param imod module number (-1 for all)
    * @returns number of additional storage cells
   */
    int64_t setNumberOfStorageCells(int64_t t=-1, int imod = -1);

	/**
	 * Set storage cell that stores first acquisition of the series (Jungfrau)
	 * @param pos storage cell index. Value can be 0 to 15. Default is 15. (-1 gets)
	 * @returns the storage cell that stores the first acquisition of the series
	 */
	int setStoragecellStart(int pos=-1);

	/**
     * Set ROI (Gotthard) (>= 1 roi, but max 1 roi per module)
     * At the moment only one set allowed
     * @param n number of rois
     * @param roiLimits array of roi
     * @param imod module number (-1 for all)
     * @returns OK or FAIL
     */
    int setROI(int n=-1, slsDetectorDefs::ROI roiLimits[]=NULL, int imod = -1);

    /**
     * Get ROI from each detector and convert it to the multi detector scale (Gotthard)
     * >= 1 roi, but max 1 roi per module
     * @param n number of rois
     * @param imod module number (ignored)
     * @returns pointer to array of ROI structure
     */
    slsDetectorDefs::ROI* getROI(int &n, int imod = -1);

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
    case 6:     return std::string("stoppped");             \
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
     @short returns external communication mode index from std::string
     \param s auto, trigger, ro_trigger, gating, triggered_gating, burst_trigger, unknown when wrong mode
     \returns index for communication mode
  */

  static int getTimingMode(std::string s){			\
    if (s== "auto") return 0;						\
    if (s== "trigger") return 1;					\
    if (s== "ro_trigger") return 2;					\
    if (s== "gating") return 3;						\
    if (s== "triggered_gating") return 4;			\
    if (s== "burst_trigger") return 5;			   \
    return -1;							};          \


 private:
  multiSlsDetector *myDetector;
  multiSlsDetectorCommand *myCmd;
 };

#endif
